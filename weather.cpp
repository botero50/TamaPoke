#include "weather.h"
#include <string.h>
#include <stdlib.h>

static WeatherNow gWx;
static uint32_t gWxAt = 0;
static bool gWxHave = false;
static bool gNySnow = true;

// What to draw from what the model says: falling weather needs something falling,
// and grey skies need clouds. Without those fields (an older reply) the code stands.
uint8_t wxHonest(uint8_t kind, bool hasPrecip, uint16_t precip10, bool hasCloud, uint8_t cloud) {
  if (hasPrecip && precip10 < WX_MIN_PRECIP10 && (kind == WX_RAIN || kind == WX_SNOW || kind == WX_STORM))
    kind = WX_CLOUDY;
  if (hasCloud && kind == WX_CLOUDY && cloud < WX_MIN_CLOUD) kind = WX_CLEAR;
  return kind;
}

// WMO 4677 codes as Open-Meteo reports them
uint8_t wmoKind(int code) {
  if (code < 0) return WX_NONE;
  if (code <= 1) return WX_CLEAR;
  if (code <= 3) return WX_CLOUDY;
  if (code == 45 || code == 48) return WX_FOG;
  if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) return WX_RAIN;
  if ((code >= 71 && code <= 77) || code == 85 || code == 86) return WX_SNOW;
  if (code >= 95 && code <= 99) return WX_STORM;
  return WX_CLOUDY;
}

// The number after "key": inside `from`, or false. Small and strict on purpose:
// the reply is a handful of flat fields and pulling in a JSON library for it
// would cost more flash than the whole feature.
static bool numAfter(const char *from, const char *key, double &v) {
  if (!from) return false;
  const char *p = strstr(from, key);
  if (!p) return false;
  p += strlen(key);
  while (*p == ' ' || *p == ':' || *p == '"') p++;
  char *end = nullptr;
  v = strtod(p, &end);
  return end != p;
}

bool parseOpenMeteo(const char *body, WeatherNow &out) {
  if (!body) return false;
  // "current_units" also holds the field names, so look inside "current":{ only
  const char *cur = strstr(body, "\"current\":");
  if (!cur) return false;
  double t, c;
  if (!numAfter(cur, "\"temperature_2m\"", t) || !numAfter(cur, "\"weather_code\"", c)) return false;
  WeatherNow w;
  w.rawCode = (int16_t)c;
  w.kind = wmoDrizzle((int)c) ? WX_CLOUDY : wmoKind((int)c);   // drizzle is a grey sky at most
  double pr, cc;
  bool hasPr = numAfter(cur, "\"precipitation\"", pr), hasCc = numAfter(cur, "\"cloud_cover\"", cc);
  // only inside this place's own "current" block: stop at the next place
  const char *next = strstr(cur + 10, "\"current\":");
  if (hasPr && next && strstr(cur, "\"precipitation\"") > next) hasPr = false;
  if (hasCc && next && strstr(cur, "\"cloud_cover\"") > next) hasCc = false;
  if (hasPr) w.precip10 = pr < 0 ? 0 : (uint16_t)(pr * 10 + 0.5);
  if (hasCc) w.cloud = cc < 0 ? 0 : cc > 100 ? 100 : (uint8_t)cc;
  w.kind = wxHonest(w.kind, hasPr, w.precip10, hasCc, w.cloud);
  if (t > 60) t = 60;
  if (t < -60) t = -60;
  w.tempC = (int8_t)(t < 0 ? t - 0.5 : t + 0.5);
  // an array of two places: New York's "current" is the second one
  const char *cur2 = strstr(cur + 10, "\"current\":");
  double c2;
  if (cur2 && numAfter(cur2, "\"weather_code\"", c2)) {
    double p2;
    bool hp2 = numAfter(cur2, "\"precipitation\"", p2);
    w.nyKind = wxHonest(wmoDrizzle((int)c2) ? WX_CLOUDY : wmoKind((int)c2), hp2, hp2 ? (uint16_t)(p2 * 10 + 0.5) : 0, false, 0);
  }
  double off;
  if (numAfter(body, "\"utc_offset_seconds\"", off) && off > -14 * 3600 - 1 && off < 14 * 3600 + 1) {
    w.hasOffset = true;
    w.utcOffset = (int32_t)off;
  }
  out = w;
  return true;
}

void weatherSet(const WeatherNow &w, uint32_t atMs) { gWx = w; gWxAt = atMs; gWxHave = true; }
const WeatherNow &weatherLast() { return gWx; }
uint8_t weatherShown(uint32_t nowMs) {
  if (!gWxHave || nowMs - gWxAt > WEATHER_FRESH_MS) return WX_NONE;
  // anything really happening here wins; only a plain day borrows New York's snow
  if (gNySnow && gWx.nyKind == WX_SNOW && (gWx.kind == WX_CLEAR || gWx.kind == WX_CLOUDY)) return WX_SNOW_NY;
  return gWx.kind;
}
void weatherSetNySnow(bool on) { gNySnow = on; }
bool weatherNySnow() { return gNySnow; }

bool ssidSameIgnoringCase(const char *a, const char *b) {
  if (!a || !b) return false;
  for (; *a && *b; a++, b++) {
    char x = *a, y = *b;
    if (x >= 'A' && x <= 'Z') x += 32;
    if (y >= 'A' && y <= 'Z') y += 32;
    if (x != y) return false;
  }
  return *a == *b;
}
