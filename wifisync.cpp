#include "wifisync.h"
#include "weather.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>
#include <time.h>

#define SYNC_FIRST_MS   15000UL              // after boot: let the splash and the SD settle
#define SYNC_EVERY_MS   (6UL * 3600 * 1000)  // the clock alone
#define SYNC_WEATHER_MS (30UL * 60 * 1000)   // with a place set: Open-Meteo refreshes every 15 min
#define SYNC_RETRY_MS   (15UL * 60 * 1000)   // after a failure

static char gSsid[33] = "", gPass[65] = "";
static int16_t gTzMin = 0;
static bool gTzSet = false;
static float gLat = 0, gLon = 0;
static bool gHasPlace = false;

static volatile uint8_t gState = WIFI_ST_NONE;
static volatile bool gBusy = false;
static volatile uint32_t gNextAt = SYNC_FIRST_MS;
static uint32_t gLastSync = 0;

// handed from the task to the main loop
static volatile bool gTimeReady = false;
static volatile uint32_t gUtc = 0, gUtcAtMs = 0;
static volatile int32_t gOffset = 0;
static volatile bool gWxReady = false;
static WeatherNow gWxNew;

// What went wrong last time, for STATS: which step stopped, the reason the access
// point gave for dropping us, the signal, and the weather request's HTTP code.
enum : uint8_t { DG_NONE = 0, DG_JOIN, DG_NTP, DG_NO_TZ, DG_OK };
static volatile uint8_t gDiagStep = DG_NONE;
static volatile uint16_t gDiagReason = 0;
static volatile int8_t gDiagRssi = 0;
static volatile int16_t gDiagHttp = 0;
static bool gEventsOn = false;

static void load() {
  Preferences p;
  p.begin("wifi", true);
  p.getString("ssid", gSsid, sizeof(gSsid));
  p.getString("pass", gPass, sizeof(gPass));
  gTzSet = p.isKey("tz");
  gTzMin = p.getShort("tz", 0);
  gHasPlace = p.isKey("lat");
  gLat = p.getFloat("lat", 0);
  gLon = p.getFloat("lon", 0);
  weatherSetNySnow(p.getBool("nys", true));
  p.end();
  gState = gSsid[0] ? WIFI_ST_IDLE : WIFI_ST_NONE;
}

void wifiBegin() {
  load();
  if (!gEventsOn) {
    gEventsOn = true;
    WiFi.onEvent([](WiFiEvent_t, WiFiEventInfo_t info) {
      gDiagReason = info.wifi_sta_disconnected.reason;
    }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  }
}

static void syncTask(void *) {
  bool ok = false;
  gDiagStep = DG_JOIN; gDiagReason = 0; gDiagRssi = 0; gDiagHttp = 0;
  WiFi.mode(WIFI_STA);
  // A name typed in the wrong letter case still finds its network: look around
  // first, and if the exact name is not there but the same one in other case is,
  // join that. A hidden network is not in the scan, so the name is tried as typed.
  char use[33];
  strncpy(use, gSsid, sizeof(use) - 1);
  use[sizeof(use) - 1] = 0;
  int found = WiFi.scanNetworks();
  bool exact = false;
  for (int i = 0; i < found && !exact; i++) exact = WiFi.SSID(i) == gSsid;
  for (int i = 0; i < found && !exact; i++)
    if (ssidSameIgnoringCase(WiFi.SSID(i).c_str(), gSsid)) { strncpy(use, WiFi.SSID(i).c_str(), sizeof(use) - 1); break; }
  WiFi.scanDelete();
  WiFi.begin(use, gPass);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) delay(200);
  if (WiFi.status() == WL_CONNECTED) {
    gDiagRssi = (int8_t)WiFi.RSSI();
    gDiagStep = DG_NTP;
    configTime(0, 0, "pool.ntp.org", "time.google.com");
    t0 = millis();
    while (time(nullptr) < 1700000000 && millis() - t0 < 10000) delay(200);
    time_t utc = time(nullptr);
    int32_t off = gTzMin * 60;
    bool haveOff = gTzSet;
    if (gHasPlace) {
      // coarse on purpose: two decimals is a neighbourhood, not a house
      char url[256];
      // New York rides along in the same request, for its snow
      snprintf(url, sizeof(url),
               "https://api.open-meteo.com/v1/forecast?latitude=%.2f," NY_LAT "&longitude=%.2f," NY_LON
               "&current=temperature_2m,weather_code,precipitation,cloud_cover&timezone=auto", gLat, gLon);
      WiFiClientSecure tls;
      tls.setInsecure();        // weather is public data; there is no secret to protect here
      HTTPClient http;
      http.setTimeout(8000);
      if (http.begin(tls, url)) {
        int code = http.GET();
        gDiagHttp = (int16_t)code;
        if (code == 200) {
          String body = http.getString();
          WeatherNow w;
          if (parseOpenMeteo(body.c_str(), w)) {
            gWxNew = w;
            gWxReady = true;
            if (w.hasOffset) { off = w.utcOffset; haveOff = true; }
          }
        }
        http.end();
      }
    }
    gDiagStep = utc < 1700000000 ? DG_NTP : !haveOff ? DG_NO_TZ : DG_OK;
    if (utc >= 1700000000 && haveOff) {
      gUtc = (uint32_t)utc;
      gUtcAtMs = millis();
      gOffset = off;
      gTimeReady = true;
      ok = true;
    } else if (gWxReady) ok = true;
  }
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);          // the radio costs real current: off until next time
  if (!ok) gNextAt = millis() + SYNC_RETRY_MS;   // sooner than the usual hours
  gState = ok ? WIFI_ST_OK : WIFI_ST_FAIL;
  gBusy = false;
  vTaskDelete(nullptr);
}

void wifiTick(uint32_t nowMs, bool radioBusy) {
  if (gWxReady) { gWxReady = false; weatherSet(gWxNew, nowMs); }
  if (gBusy || !gSsid[0] || radioBusy) return;
  if ((int32_t)(nowMs - gNextAt) < 0) return;
  gNextAt = nowMs + (gHasPlace ? SYNC_WEATHER_MS : SYNC_EVERY_MS);
  gBusy = true;
  gState = WIFI_ST_BUSY;
  if (xTaskCreate(syncTask, "wifisync", 8192, nullptr, 1, nullptr) != pdPASS) {
    gBusy = false;
    gState = WIFI_ST_FAIL;
  }
}

bool wifiBusy() { return gBusy; }

bool wifiTakeTime(uint32_t *localEpoch) {
  if (!gTimeReady) return false;
  gTimeReady = false;
  uint32_t e = gUtc + (uint32_t)gOffset + (millis() - gUtcAtMs) / 1000;
  gLastSync = e;
  if (localEpoch) *localEpoch = e;
  return true;
}

void wifiSetNetwork(const char *ssid, const char *pass) {
  Preferences p;
  p.begin("wifi", false);
  p.putString("ssid", ssid);
  p.putString("pass", pass);
  p.end();
  load();
  gNextAt = millis() + 2000;
}

void wifiForget() {
  Preferences p;
  p.begin("wifi", false);
  p.clear();
  p.end();
  load();
}

void wifiSetTz(int16_t minutesEast) {
  Preferences p;
  p.begin("wifi", false);
  p.putShort("tz", minutesEast);
  p.end();
  load();
}

void wifiSetPlace(float lat, float lon) {
  Preferences p;
  p.begin("wifi", false);
  p.putFloat("lat", lat);
  p.putFloat("lon", lon);
  p.end();
  load();
}

void wifiSetNySnow(bool on) {
  Preferences p;
  p.begin("wifi", false);
  p.putBool("nys", on);
  p.end();
  weatherSetNySnow(on);
}

void wifiSyncNow() { gNextAt = millis(); if (gState == WIFI_ST_FAIL) gState = WIFI_ST_IDLE; }
uint8_t wifiState() { return gState; }

static const char *reasonName(uint16_t r) {
  switch (r) {
    case 0: return "";
    case 2: case 202: return "auth failed (password?)";
    case 15: case 204: return "handshake timeout (password?)";
    case 201: return "network not found (5 GHz only, or too far?)";
    case 200: return "beacon timeout (weak signal)";
    case 203: return "association failed";
    case 8: return "left";
    default: return "see esp_wifi reason codes";
  }
}

void wifiDiag(char *out, size_t n) {
  static const char *STEP[] = { "-", "join", "ntp", "no-timezone", "ok" };
  snprintf(out, n, "step=%s reason=%u %s rssi=%d http=%d", STEP[gDiagStep < 5 ? gDiagStep : 0],
           (unsigned)gDiagReason, reasonName(gDiagReason), (int)gDiagRssi, (int)gDiagHttp);
}

void wifiScanPrint() {
  if (gBusy) { Serial.println("scan: a sync is running, try again"); return; }
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  int n = WiFi.scanNetworks();
  Serial.printf("scan: %d networks (2.4 GHz only: this radio cannot see 5 GHz)\n", n);
  for (int i = 0; i < n; i++)
    Serial.printf("  %-32s ch%-2d %4d dBm %s%s\n", WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i),
                  WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "locked",
                  WiFi.SSID(i) == gSsid ? "  <- yours" : "");
  WiFi.scanDelete();
  WiFi.mode(WIFI_OFF);
}
uint32_t wifiLastSync() { return gLastSync; }
const char *wifiNetwork() { return gSsid; }
