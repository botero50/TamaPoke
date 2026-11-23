#pragma once
#include <stdint.h>

// Battery saver policy and the power log. Pure logic with host tests (powersave_test.cpp).
//
// Three levels: NORMAL; LOW (on battery at 20 % or less, or "Always"): dimmer screen, slower frames,
// no ambient landscape particles, a quicker dim and a slower chip while dimmed; CRITICAL (10 % or less):
// the same, harder. Plugged in is always NORMAL (except "Always"). A hysteresis stops it flapping when
// the percentage hovers on a limit.

enum PowerLevel : uint8_t { PL_NORMAL = 0, PL_LOW, PL_CRITICAL };
enum : uint8_t { BSM_AUTO = 0, BSM_OFF, BSM_ALWAYS };

#define PL_LOW_ENTER 20
#define PL_LOW_LEAVE 25
#define PL_CRIT_ENTER 10
#define PL_CRIT_LEAVE 14

static inline uint8_t powerLevel(int pct, bool onUsb, uint8_t mode, uint8_t prev) {
  if (mode == BSM_OFF) return PL_NORMAL;
  uint8_t lvl = PL_NORMAL;
  if (!onUsb && pct >= 0) {
    // enter at the lower limits, leave only above the higher ones
    if (pct <= PL_CRIT_ENTER || (prev == PL_CRITICAL && pct < PL_CRIT_LEAVE)) lvl = PL_CRITICAL;
    else if (pct <= PL_LOW_ENTER || (prev >= PL_LOW && pct < PL_LOW_LEAVE)) lvl = PL_LOW;
  }
  if (mode == BSM_ALWAYS && lvl < PL_LOW) lvl = PL_LOW;
  return lvl;
}

// at least this many ms between frames
static inline uint32_t powerRenderGap(uint8_t lvl, uint32_t gap) {
  uint32_t min = lvl == PL_CRITICAL ? 300 : lvl == PL_LOW ? 200 : 0;
  return gap < min ? min : gap;
}
// the brightest the screen may be (0..255)
static inline uint8_t powerBrightCap(uint8_t lvl, uint8_t target) {
  uint8_t cap = lvl == PL_CRITICAL ? 60 : lvl == PL_LOW ? 120 : 255;
  return target > cap ? cap : target;
}
// idle time before the screen dims (stage 1) and nearly goes out (stage 2)
static inline uint32_t powerDimMs(uint8_t lvl, uint8_t stage) {
  if (lvl == PL_CRITICAL) return stage == 2 ? 60000UL : 20000UL;
  if (lvl == PL_LOW) return stage == 2 ? 120000UL : 45000UL;
  return stage == 2 ? 300000UL : 90000UL;
}
static inline bool powerFxAllowed(uint8_t lvl) { return lvl == PL_NORMAL; }
// the chip's speed with the screen on: slower only while dimmed in a saver level
static inline uint8_t powerCpuMhz(uint8_t lvl, uint8_t dimStage) { return (lvl != PL_NORMAL && dimStage >= 1) ? 160 : 240; }
static inline uint8_t powerLoopDelayMs(uint8_t lvl, uint8_t dimStage) { return (lvl != PL_NORMAL && dimStage >= 1) ? 12 : 1; }

// ---- the power log: one sample every POWERLOG_EVERY_S while discharging, kept in RAM
#define POWERLOG_N 360
#define POWERLOG_EVERY_S 120
enum : uint8_t { PST_AWAKE = 0, PST_DIM, PST_OFF, PST_STANDBY, PST_COUNT };
struct PowerSample { uint32_t sec; uint16_t mv; uint8_t pct; uint8_t state; };

struct PowerLog {
  PowerSample s[POWERLOG_N];
  uint16_t head = 0, count = 0;
  void add(uint32_t sec, uint16_t mv, uint8_t pct, uint8_t state) {
    s[head] = { sec, mv, pct, state };
    head = (uint16_t)((head + 1) % POWERLOG_N);
    if (count < POWERLOG_N) count++;
  }
  const PowerSample &at(uint16_t i) const { return s[(head + POWERLOG_N - count + i) % POWERLOG_N]; }   // 0 = oldest
};

// Per state: the minutes spent and the percent lost between two samples in the same state, and so
// the rate in tenths of a percent per hour. A pair where the percentage rose (a charger, a bump) or
// the gap is longer than three sample intervals is skipped.
struct PowerRate { uint32_t minutes; uint32_t dropTenths; };   // dropTenths: tenths of a percent lost
static inline void powerRates(const PowerLog &lg, PowerRate out[PST_COUNT]) {
  for (int i = 0; i < PST_COUNT; i++) out[i] = { 0, 0 };
  for (uint16_t i = 1; i < lg.count; i++) {
    const PowerSample &a = lg.at(i - 1), &b = lg.at(i);
    if (b.sec <= a.sec || b.sec - a.sec > 3UL * POWERLOG_EVERY_S) continue;
    if (a.state != b.state || a.state >= PST_COUNT || b.pct > a.pct) continue;
    out[a.state].minutes += (b.sec - a.sec) / 60;
    out[a.state].dropTenths += (uint32_t)(a.pct - b.pct) * 10;
  }
}
// tenths of a percent an hour for a state, 0 when there is not enough to say (under 10 minutes)
static inline uint32_t powerTenthsPerHour(const PowerRate &r) { return r.minutes < 10 ? 0 : r.dropTenths * 60 / r.minutes; }
// hours left at the measured average of the states weighted by the time spent in them; -1 when unknown
static inline int powerHoursLeft(const PowerRate r[PST_COUNT], int pct) {
  uint32_t mins = 0, drop = 0;
  for (int i = 0; i < PST_COUNT; i++) { mins += r[i].minutes; drop += r[i].dropTenths; }
  if (mins < 30 || drop == 0 || pct < 0) return -1;
  return (int)((uint32_t)pct * 10 * mins / (drop * 60));
}
