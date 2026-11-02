#pragma once
#include <stdint.h>
#include <stddef.h>

// The weather outside, from Open-Meteo through the Wi-Fi sync (wifisync.cpp).
// This half is pure: the WMO code table and the reply parser, so the desktop
// tests can reach them. The radio half lives in wifisync.cpp.
enum WeatherKind : uint8_t { WX_NONE = 0, WX_CLEAR, WX_CLOUDY, WX_FOG, WX_RAIN, WX_SNOW, WX_STORM,
                             WX_SNOW_NY };   // it is snowing in New York and nothing is happening here
// Snow from New York: for places where it never snows, the sky borrows New
// York's snow on days when the weather at home is only clear or cloudy.
#define NY_LAT "40.71"
#define NY_LON "-74.01"
static inline bool wxIsSnow(uint8_t k) { return k == WX_SNOW || k == WX_SNOW_NY; }

struct WeatherNow {
  uint8_t kind = WX_NONE;
  int8_t tempC = 0;
  bool hasOffset = false;
  int32_t utcOffset = 0;      // seconds east of UTC at the place, DST included
  uint8_t nyKind = WX_NONE;   // New York's weather, when the reply carried it
  // what the model said before the rules below (phase 29), for STATS
  int16_t rawCode = -1;
  uint16_t precip10 = 0;      // precipitation in the last 15 min, tenths of a mm
  uint8_t cloud = 255;        // cloud cover %, 255 = not in the reply
};
// Phase 29: models flag drizzle and showers with nothing actually falling,
// especially over mountain and tropical cities, which kept the sky raining
// while the sun was out. Rain, snow and storms need real precipitation;
// "cloudy" needs real cloud cover.
#define WX_MIN_PRECIP10 3       // 0.3 mm in 15 min: real rain, not a model's drizzle (0.1 mm showed rain on a sunny day)
static inline bool wmoDrizzle(int code) { return code >= 51 && code <= 57; }   // never drawn as rain
#define WX_MIN_CLOUD 60

#define WEATHER_FRESH_MS (75UL * 60 * 1000)   // older than this and the sky goes back to normal (2.5 refreshes)

uint8_t wmoKind(int code);                     // a WMO weather code -> WeatherKind
uint8_t wxHonest(uint8_t kind, bool hasPrecip, uint16_t precip10, bool hasCloud, uint8_t cloud);
bool parseOpenMeteo(const char *body, WeatherNow &out);   // false if the reply has no current weather
                                                          // (one place, or an array: here first, New York second)

// The last weather fetched, and when (millis). The scene reads weatherShown().
void weatherSet(const WeatherNow &w, uint32_t atMs);
const WeatherNow &weatherLast();
uint8_t weatherShown(uint32_t nowMs);          // WX_NONE when unknown or stale
void weatherSetNySnow(bool on);                // the "Snow from New York" switch (on by default)
bool weatherNySnow();

// The same network name, ignoring letter case ("alejandro" is "Alejandro").
// Wi-Fi names are case-sensitive, and the first real setup failed on exactly that.
bool ssidSameIgnoringCase(const char *a, const char *b);
