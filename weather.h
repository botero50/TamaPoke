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
};

#define WEATHER_FRESH_MS (3UL * 3600 * 1000)   // older than this and the sky goes back to normal

uint8_t wmoKind(int code);                     // a WMO weather code -> WeatherKind
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
