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
#define SYNC_WEATHER_MS (1UL * 3600 * 1000)  // with a place set: the weather changes faster
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
  p.end();
  gState = gSsid[0] ? WIFI_ST_IDLE : WIFI_ST_NONE;
}

void wifiBegin() { load(); }

static void syncTask(void *) {
  bool ok = false;
  WiFi.mode(WIFI_STA);
  WiFi.begin(gSsid, gPass);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) delay(200);
  if (WiFi.status() == WL_CONNECTED) {
    configTime(0, 0, "pool.ntp.org", "time.google.com");
    t0 = millis();
    while (time(nullptr) < 1700000000 && millis() - t0 < 10000) delay(200);
    time_t utc = time(nullptr);
    int32_t off = gTzMin * 60;
    bool haveOff = gTzSet;
    if (gHasPlace) {
      // coarse on purpose: two decimals is a neighbourhood, not a house
      char url[200];
      snprintf(url, sizeof(url),
               "https://api.open-meteo.com/v1/forecast?latitude=%.2f&longitude=%.2f"
               "&current=temperature_2m,weather_code&timezone=auto", gLat, gLon);
      WiFiClientSecure tls;
      tls.setInsecure();        // weather is public data; there is no secret to protect here
      HTTPClient http;
      http.setTimeout(8000);
      if (http.begin(tls, url)) {
        if (http.GET() == 200) {
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

void wifiSyncNow() { gNextAt = millis(); if (gState == WIFI_ST_FAIL) gState = WIFI_ST_IDLE; }
uint8_t wifiState() { return gState; }
uint32_t wifiLastSync() { return gLastSync; }
const char *wifiNetwork() { return gSsid; }
