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

  // what the screen sees
  ck(weatherShown(1000) == WX_NONE, "nothing fetched: the normal sky");
  WeatherNow r; r.kind = WX_RAIN; r.tempC = 9;
  weatherSet(r, 1000);
  ck(weatherShown(5000) == WX_RAIN && weatherLast().tempC == 9, "fetched: it rains");
  ck(weatherShown(1000 + WEATHER_FRESH_MS + 1) == WX_NONE, "three hours stale: the normal sky again");

  if (bad) printf("FAILURES\n");
  return bad?1:0;
}
