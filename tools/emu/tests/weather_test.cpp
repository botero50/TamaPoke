// Phase 22: the weather parser and the WMO table. The radio half (wifisync.cpp)
// only runs on the board; this is everything it hands the screen.
#include "Arduino.h"
#include "Preferences.h"
#include "pet.h"
#include "weather.h"
#include <cstdio>
uint32_t g_seed=5; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
volatile int g_touchX=0,g_touchY=0; volatile bool g_touchDown=false; bool wasPressed=false;
uint32_t millis(){return 0;} void FakeESP::restart(){exit(0);}
int FakeSerial::available(){return 0;} String FakeSerial::readStringUntil(char){return String("");}
void sfxPlay(uint8_t){}
static int bad=0;
static void ck(bool ok,const char*w){printf("%s  %s\n",ok?"PASS":"FAIL",w); if(!ok)bad++;}

int main(){
  ck(wmoKind(0) == WX_CLEAR && wmoKind(1) == WX_CLEAR, "0 and 1 are clear");
  ck(wmoKind(2) == WX_CLOUDY && wmoKind(3) == WX_CLOUDY, "2 and 3 are cloudy");
  ck(wmoKind(45) == WX_FOG && wmoKind(48) == WX_FOG, "45 and 48 are fog");
  ck(wmoKind(61) == WX_RAIN && wmoKind(81) == WX_RAIN && wmoKind(53) == WX_RAIN, "drizzle and showers are rain");
  ck(wmoKind(73) == WX_SNOW && wmoKind(86) == WX_SNOW, "snow and snow showers");
  ck(wmoKind(95) == WX_STORM && wmoKind(99) == WX_STORM, "thunderstorms");
  ck(wmoKind(-1) == WX_NONE, "no code, no weather");

  // the shape Open-Meteo actually sends (current_units first, with the same names)
  const char *reply =
    "{\"latitude\":4.71,\"longitude\":-74.07,\"generationtime_ms\":0.03,"
    "\"utc_offset_seconds\":-18000,\"timezone\":\"America/Bogota\",\"timezone_abbreviation\":\"GMT-5\","
    "\"elevation\":2582.0,\"current_units\":{\"time\":\"iso8601\",\"interval\":\"seconds\","
    "\"temperature_2m\":\"°C\",\"weather_code\":\"wmo code\"},"
    "\"current\":{\"time\":\"2026-11-01T22:00\",\"interval\":900,\"temperature_2m\":13.6,\"weather_code\":61}}";
  WeatherNow w;
  ck(parseOpenMeteo(reply, w), "a real reply parses");
  ck(w.kind == WX_RAIN && w.tempC == 14, "light rain, 13.6 rounds to 14");
  ck(w.hasOffset && w.utcOffset == -18000, "and the place's offset from UTC, -5 h");
  const char *cold = "{\"utc_offset_seconds\":3600,\"current\":{\"temperature_2m\":-2.6,\"weather_code\":71}}";
  ck(parseOpenMeteo(cold, w) && w.kind == WX_SNOW && w.tempC == -3 && w.utcOffset == 3600, "below zero rounds away from zero");
  WeatherNow keep = w;
  ck(!parseOpenMeteo("{\"error\":true,\"reason\":\"bad\"}", w) && w.tempC == keep.tempC, "an error reply changes nothing");
  ck(!parseOpenMeteo(nullptr, w), "nor does no reply");
  ck(!parseOpenMeteo("{\"current_units\":{\"temperature_2m\":\"C\"}}", w), "units alone are not weather");

  // two places in one request: here first, New York second (a real reply, 1 Nov 2026)
  const char *both =
    "[{\"latitude\":4.7451673,\"longitude\":-74.10025,\"generationtime_ms\":0.069,\"utc_offset_seconds\":-18000,"
    "\"timezone\":\"America/Bogota\",\"timezone_abbreviation\":\"GMT-5\",\"elevation\":2558.0,"
    "\"current_units\":{\"time\":\"iso8601\",\"interval\":\"seconds\",\"temperature_2m\":\"°C\",\"weather_code\":\"wmo code\"},"
    "\"current\":{\"time\":\"2026-11-01T23:45\",\"interval\":900,\"temperature_2m\":12.6,\"weather_code\":3}},"
    "{\"latitude\":40.710335,\"longitude\":-73.99308,\"generationtime_ms\":0.094,\"utc_offset_seconds\":-18000,"
    "\"timezone\":\"America/New_York\",\"timezone_abbreviation\":\"GMT-5\",\"elevation\":27.0,\"location_id\":1,"
    "\"current_units\":{\"time\":\"iso8601\",\"interval\":\"seconds\",\"temperature_2m\":\"°C\",\"weather_code\":\"wmo code\"},"
    "\"current\":{\"time\":\"2026-11-01T23:45\",\"interval\":900,\"temperature_2m\":8.4,\"weather_code\":0}}]";
  ck(parseOpenMeteo(both, w), "the two-place reply parses");
  ck(w.kind == WX_CLOUDY && w.tempC == 13 && w.utcOffset == -18000, "home comes first: cloudy, 13C, -5 h");
  ck(w.nyKind == WX_CLEAR, "and New York second: clear");
  ck(parseOpenMeteo(reply, w) && w.nyKind == WX_NONE, "one place: no New York");

  // snow from New York: only on a plain day at home, and only while it is on
  {
    WeatherNow s; s.tempC = 22; s.nyKind = WX_SNOW;
    s.kind = WX_CLEAR; weatherSet(s, 0);
    ck(weatherShown(10) == WX_SNOW_NY && weatherLast().tempC == 22, "clear here, snowing in New York: its snow, our temperature");
    s.kind = WX_CLOUDY; weatherSet(s, 0);
    ck(weatherShown(10) == WX_SNOW_NY, "cloudy here too");
    s.kind = WX_RAIN; weatherSet(s, 0);
    ck(weatherShown(10) == WX_RAIN, "rain here beats New York's snow");
    s.kind = WX_FOG; weatherSet(s, 0);
    ck(weatherShown(10) == WX_FOG, "and so does fog");
    s.kind = WX_SNOW; weatherSet(s, 0);
    ck(weatherShown(10) == WX_SNOW, "real snow here is our own");
    s.kind = WX_CLEAR; s.nyKind = WX_RAIN; weatherSet(s, 0);
    ck(weatherShown(10) == WX_CLEAR, "rain in New York is not borrowed");
    s.nyKind = WX_SNOW; weatherSet(s, 0);
    weatherSetNySnow(false);
    ck(weatherShown(10) == WX_CLEAR && !weatherNySnow(), "the switch off: our own sky");
    weatherSetNySnow(true);
    ck(wxIsSnow(WX_SNOW) && wxIsSnow(WX_SNOW_NY) && !wxIsSnow(WX_RAIN), "both snows draw as snow");
    WeatherNow none; weatherSet(none, 0);
  }

  // honest weather (phase 29): falling weather needs something falling, grey needs clouds
  {
    const char *drizzle =
      "[{\"utc_offset_seconds\":-18000,\"current_units\":{\"temperature_2m\":\"°C\",\"weather_code\":\"wmo code\",\"precipitation\":\"mm\",\"cloud_cover\":\"%\"},"
      "\"current\":{\"time\":\"2026-11-02T12:30\",\"interval\":900,\"temperature_2m\":22.4,\"weather_code\":61,\"precipitation\":0.00,\"cloud_cover\":35}},"
      "{\"utc_offset_seconds\":-18000,\"current_units\":{\"temperature_2m\":\"°C\",\"weather_code\":\"wmo code\",\"precipitation\":\"mm\",\"cloud_cover\":\"%\"},"
      "\"current\":{\"time\":\"2026-11-02T12:30\",\"interval\":900,\"temperature_2m\":3.1,\"weather_code\":71,\"precipitation\":0.00,\"cloud_cover\":100}}]";
    WeatherNow d;
    ck(parseOpenMeteo(drizzle, d), "a reply with precipitation and cloud cover parses");
    ck(d.rawCode == 61 && d.precip10 == 0 && d.cloud == 35, "the raw code, the precipitation and the cloud are kept for STATS");
    ck(d.kind == WX_CLEAR, "rain code, nothing falling, 35% cloud: a sunny sky, not rain");
    ck(d.nyKind == WX_CLOUDY, "New York's snow code with nothing falling is not snow either");
    ck(wxHonest(WX_RAIN, true, 4, true, 90) == WX_RAIN, "0.4 mm really falling: rain");
    ck(wxHonest(WX_RAIN, true, 2, true, 90) == WX_CLOUDY && wxHonest(WX_RAIN, true, 3, true, 90) == WX_RAIN,
       "rain needs 0.3 mm: 0.2 is a grey sky");
    // the real reading on a sunny day (2 Nov): drizzle, 0.1 mm, 96% cloud -- never rain now
    WeatherNow dz;
    ck(parseOpenMeteo("{\"current\":{\"temperature_2m\":22,\"weather_code\":51,\"precipitation\":0.1,\"cloud_cover\":96}}", dz) &&
       dz.kind == WX_CLOUDY && dz.rawCode == 51, "drizzle at 0.1 mm: at most a grey sky");
    ck(parseOpenMeteo("{\"current\":{\"temperature_2m\":22,\"weather_code\":53,\"precipitation\":0.9,\"cloud_cover\":96}}", dz) &&
       dz.kind == WX_CLOUDY, "even heavy drizzle is not drawn as rain");
    ck(parseOpenMeteo("{\"current\":{\"temperature_2m\":22,\"weather_code\":61,\"precipitation\":0.5,\"cloud_cover\":96}}", dz) &&
       dz.kind == WX_RAIN, "real rain, 0.5 mm: rain");
    ck(wxHonest(WX_RAIN, true, 0, true, 90) == WX_CLOUDY, "nothing falling under thick cloud: cloudy");
    ck(wxHonest(WX_CLOUDY, true, 0, true, 40) == WX_CLEAR && wxHonest(WX_CLOUDY, true, 0, true, 75) == WX_CLOUDY, "cloudy only from 60% cloud");
    ck(wxHonest(WX_STORM, true, 0, false, 255) == WX_CLOUDY, "a storm with no rain is just cloud");
    ck(wxHonest(WX_FOG, true, 0, true, 10) == WX_FOG, "fog stays fog");
    ck(wxHonest(WX_RAIN, false, 0, false, 255) == WX_RAIN, "an older reply without the fields keeps its code");
    ck(WEATHER_FRESH_MS == 75UL * 60 * 1000, "a missed refresh clears the sky after 75 minutes, not 3 hours");
  }

  // the network name in any letter case (phase 26)
  ck(ssidSameIgnoringCase("Alejandro", "alejandro") && ssidSameIgnoringCase("MyNet_2.4G", "mynet_2.4g"), "the same name in other letter case matches");
  ck(!ssidSameIgnoringCase("Alejandro", "Alejandro2") && !ssidSameIgnoringCase("abc", "abd"), "a different name does not");
  ck(!ssidSameIgnoringCase(nullptr, "x") && ssidSameIgnoringCase("", ""), "and no name is no match");

  // what the screen sees
  ck(weatherShown(1000) == WX_NONE, "nothing fetched: the normal sky");
  WeatherNow r; r.kind = WX_RAIN; r.tempC = 9;
  weatherSet(r, 1000);
  ck(weatherShown(5000) == WX_RAIN && weatherLast().tempC == 9, "fetched: it rains");
  ck(weatherShown(1000 + WEATHER_FRESH_MS + 1) == WX_NONE, "three hours stale: the normal sky again");

  if (bad) printf("FAILURES\n");
  return bad?1:0;
}
