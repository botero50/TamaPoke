#pragma once
#include <stdint.h>

// One LiPo / Li-ion cell charged to 4.2 V (what the Waveshare board does): from the resting voltage
// to a percentage, and when to save and power off so the cell is not drained flat. Pure logic with
// host tests (power_test.cpp). The design follows Mu-Muu/TamaPoke's batcurve.h (MIT, see
// CREDITS.md); this is our own implementation of it.

#define BAT_CUTOFF_MV 3300      // under this (while discharging) the firmware saves and powers off
#define BAT_CUTOFF_READS 3      // readings in a row under it (as well as the time, see BatMonitor)
#define BAT_CUTOFF_MS 30000UL   // ...and the smoothed voltage under the limit for this long
#define BAT_BOOT_GRACE_MS 30000UL  // right after boot the first voltage tells nothing: never off then
#define BAT_HW_CUTOFF_MV 3100   // backstop set in the PMU (AXP2101: 2600..3300 mV in 100 mV steps)

struct BatPoint { int16_t mv; uint8_t pct; };
// a typical discharge curve of one cell, highest first
static const BatPoint BAT_CURVE[] = {
  { 4200, 100 }, { 4150, 95 }, { 4110, 90 }, { 4080, 85 }, { 4020, 80 }, { 3980, 75 }, { 3950, 70 },
  { 3910, 65 }, { 3870, 60 }, { 3850, 55 }, { 3840, 50 }, { 3820, 45 }, { 3800, 40 }, { 3790, 35 },
  { 3770, 30 }, { 3750, 25 }, { 3730, 20 }, { 3710, 15 }, { 3690, 10 }, { 3610, 5 }, { BAT_CUTOFF_MV, 0 },
};

// linear interpolation between the points: 100 above 4.2 V, 0 at the cut-off
static inline int batPercentFromMv(int mv) {
  const int n = (int)(sizeof(BAT_CURVE) / sizeof(BAT_CURVE[0]));
  if (mv >= BAT_CURVE[0].mv) return 100;
  if (mv <= BAT_CURVE[n - 1].mv) return 0;
  for (int i = 1; i < n; i++) {
    if (mv >= BAT_CURVE[i].mv) {
      int dmv = BAT_CURVE[i - 1].mv - BAT_CURVE[i].mv, dp = BAT_CURVE[i - 1].pct - BAT_CURVE[i].pct;
      return BAT_CURVE[i].pct + (mv - BAT_CURVE[i].mv) * dp / dmv;
    }
  }
  return 0;
}

// Smooths the voltage (a moving average, 1/4 of each new reading), keeps the shown percentage from
// ever going UP while discharging (the charge-inflated voltage right after unplugging, or a bump,
// must not raise it or make it dance) and decides the power-off: the smoothed voltage has to stay
// under the limit for BAT_CUTOFF_MS in a row, not just for a reading or two.
struct BatMonitor {
  int smoothMv = 0;
  int pct = -1;              // the percentage shown so far (-1 = no reading yet)
  uint32_t lowSince = 0;
  uint8_t lowReads = 0;

  void reset() { smoothMv = 0; pct = -1; lowSince = 0; lowReads = 0; }
  // the ceiling at the start of a discharge: what the PMU was showing
  void seed(int p) { if (pct < 0 && p >= 0) pct = p; }
  // One reading while discharging (no USB, not charging). Returns the percentage, -1 for no reading.
  int sample(int mv, uint32_t now) {
    if (mv <= 0) { lowReads = 0; lowSince = 0; return -1; }
    smoothMv = smoothMv ? smoothMv + (mv - smoothMv) / 4 : mv;
    if (smoothMv < BAT_CUTOFF_MV) {
      if (!lowReads) lowSince = now ? now : 1;
      if (lowReads < 255) lowReads++;
    } else {
      lowReads = 0;
      lowSince = 0;
    }
    int c = batPercentFromMv(smoothMv);
    if (pct < 0 || c < pct) pct = c;
    return pct;
  }
  bool shutdownNeeded(uint32_t now) const {
    return now >= BAT_BOOT_GRACE_MS && lowReads >= BAT_CUTOFF_READS && now - lowSince >= BAT_CUTOFF_MS;
  }
};
