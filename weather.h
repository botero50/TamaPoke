#pragma once
#include <stdint.h>
#include <stddef.h>

// The weather outside, from Open-Meteo through the Wi-Fi sync (wifisync.cpp).
// This half is pure: the WMO code table and the reply parser, so the desktop
// tests can reach them. The radio half lives in wifisync.cpp.
enum WeatherKind : uint8_t { WX_NONE = 0, WX_CLEAR, WX_CLOUDY, WX_FOG, WX_RAIN, WX_SNOW, WX_STORM };

struct WeatherNow {
  uint8_t kind = WX_NONE;
  int8_t tempC = 0;
  bool hasOffset = false;
  int32_t utcOffset = 0;      // seconds east of UTC at the place, DST included
};

#define WEATHER_FRESH_MS (3UL * 3600 * 1000)   // older than this and the sky goes back to normal

uint8_t wmoKind(int code);                     // a WMO weather code -> WeatherKind
bool parseOpenMeteo(const char *body, WeatherNow &out);   // false if the reply has no current weather

// The last weather fetched, and when (millis). The scene reads weatherShown().
void weatherSet(const WeatherNow &w, uint32_t atMs);
const WeatherNow &weatherLast();
uint8_t weatherShown(uint32_t nowMs);          // WX_NONE when unknown or stale
