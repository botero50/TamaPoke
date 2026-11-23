// The battery saver policy and the power log (powersave.h).
#include "powersave.h"
#include <cstdio>
#include <cstdlib>
static int bad = 0;
static void ck(bool ok, const char *w) { printf("%s  %s\n", ok ? "PASS" : "FAIL", w); if (!ok) bad++; }

int main() {
  // levels with hysteresis
  ck(powerLevel(80, false, BSM_AUTO, PL_NORMAL) == PL_NORMAL, "80 %: normal");
  ck(powerLevel(20, false, BSM_AUTO, PL_NORMAL) == PL_LOW, "20 %: low");
  ck(powerLevel(21, false, BSM_AUTO, PL_NORMAL) == PL_NORMAL, "21 %: still normal");
  ck(powerLevel(22, false, BSM_AUTO, PL_LOW) == PL_LOW && powerLevel(24, false, BSM_AUTO, PL_LOW) == PL_LOW, "once low it stays low until 25 %");
  ck(powerLevel(25, false, BSM_AUTO, PL_LOW) == PL_NORMAL, "25 %: back to normal");
  ck(powerLevel(10, false, BSM_AUTO, PL_LOW) == PL_CRITICAL, "10 %: critical");
  ck(powerLevel(12, false, BSM_AUTO, PL_CRITICAL) == PL_CRITICAL && powerLevel(13, false, BSM_AUTO, PL_CRITICAL) == PL_CRITICAL, "once critical it stays until 14 %");
  ck(powerLevel(14, false, BSM_AUTO, PL_CRITICAL) == PL_LOW, "14 %: back to low");
  ck(powerLevel(5, true, BSM_AUTO, PL_NORMAL) == PL_NORMAL, "on USB: always normal");
  ck(powerLevel(-1, false, BSM_AUTO, PL_NORMAL) == PL_NORMAL, "no battery reading: normal");
  ck(powerLevel(5, false, BSM_OFF, PL_NORMAL) == PL_NORMAL, "saver off: normal at any charge");
  ck(powerLevel(90, false, BSM_ALWAYS, PL_NORMAL) == PL_LOW && powerLevel(90, true, BSM_ALWAYS, PL_NORMAL) == PL_LOW, "always: at least low, even on USB");
  ck(powerLevel(5, false, BSM_ALWAYS, PL_LOW) == PL_CRITICAL, "always still goes critical when it is");
  // no flapping: a percentage dancing on the limit changes the level at most once each way
  { uint8_t lv = PL_NORMAL; int changes = 0;
    for (int i = 0; i < 40; i++) { int pct = 20 + (i % 2) * 2 + (i / 20); uint8_t n = powerLevel(pct, false, BSM_AUTO, lv); if (n != lv) changes++; lv = n; }
    ck(changes <= 1, "hovering around 20-22 % does not flap"); }
  // the policy numbers
  ck(powerRenderGap(PL_NORMAL, 100) == 100 && powerRenderGap(PL_LOW, 100) == 200 && powerRenderGap(PL_CRITICAL, 100) == 300 && powerRenderGap(PL_LOW, 500) == 500, "frames never faster than the level allows");
  ck(powerBrightCap(PL_NORMAL, 255) == 255 && powerBrightCap(PL_LOW, 255) == 120 && powerBrightCap(PL_CRITICAL, 255) == 60 && powerBrightCap(PL_LOW, 40) == 40, "brightness is capped, never raised");
  ck(powerDimMs(PL_NORMAL, 1) == 90000 && powerDimMs(PL_LOW, 1) < 90000 && powerDimMs(PL_CRITICAL, 1) < powerDimMs(PL_LOW, 1) && powerDimMs(PL_CRITICAL, 2) < powerDimMs(PL_LOW, 2), "it dims sooner the lower the battery");
  ck(powerFxAllowed(PL_NORMAL) && !powerFxAllowed(PL_LOW) && !powerFxAllowed(PL_CRITICAL), "ambient effects only at normal");
  ck(powerCpuMhz(PL_NORMAL, 2) == 240 && powerCpuMhz(PL_LOW, 0) == 240 && powerCpuMhz(PL_LOW, 1) == 160, "slower chip only while dimmed in a saver level");
  ck(powerLoopDelayMs(PL_LOW, 0) == 1 && powerLoopDelayMs(PL_CRITICAL, 2) > 1, "the loop rests longer only while dimmed in a saver level");

  // the log
  static PowerLog lg;
  uint32_t t = 0; int pct = 90;
  for (int i = 0; i < 20; i++) { lg.add(t, 4000, (uint8_t)pct, PST_AWAKE); t += POWERLOG_EVERY_S; if (i % 4 == 3) pct--; }   // awake: 1 % per 8 min = 7.5 %/h
  for (int i = 0; i < 30; i++) { lg.add(t, 3900, (uint8_t)pct, PST_OFF); t += POWERLOG_EVERY_S; if (i % 15 == 14) pct--; }  // off: 1 % per 30 min = 2 %/h
  PowerRate r[PST_COUNT]; powerRates(lg, r);
  ck(r[PST_AWAKE].minutes > 30 && r[PST_OFF].minutes > 50 && r[PST_DIM].minutes == 0, "minutes per state");
  uint32_t a = powerTenthsPerHour(r[PST_AWAKE]), o = powerTenthsPerHour(r[PST_OFF]);
  ck(a > 60 && a < 90, "awake: about 7.5 % an hour");
  ck(o >= 5 && o < 30, "off: a couple of percent an hour at most");
  ck(powerTenthsPerHour(r[PST_DIM]) == 0, "no data: 0");
  int h = powerHoursLeft(r, 50);
  ck(h > 5 && h < 25, "hours left from the measured mix");
  ck(powerHoursLeft(r, -1) == -1, "no battery: unknown");
  // a charge in the middle and a long gap are skipped
  static PowerLog l2;
  l2.add(0, 4000, 50, PST_AWAKE); l2.add(120, 4000, 60, PST_AWAKE); l2.add(240, 4000, 59, PST_AWAKE); l2.add(5000, 4000, 40, PST_AWAKE);
  PowerRate r2[PST_COUNT]; powerRates(l2, r2);
  ck(r2[PST_AWAKE].minutes == 2 && r2[PST_AWAKE].dropTenths == 10, "a rise and a long gap are skipped");
  // the ring wraps
  static PowerLog l3; for (int i = 0; i < POWERLOG_N + 50; i++) l3.add(i * 120, 3800, 50, PST_AWAKE);
  ck(l3.count == POWERLOG_N && l3.at(0).sec == 50 * 120 && l3.at(POWERLOG_N - 1).sec == (POWERLOG_N + 49) * 120, "the ring keeps the newest samples in order");

  if (bad) { printf("FAILURES: %d\n", bad); return 1; }
  printf("powersave test passed\n");
  return 0;
}
