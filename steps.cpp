#include "steps.h"
#include <math.h>

// Detection after Mu-Muu/TamaPoke's imu.cpp (MIT, see CREDITS): a slow baseline
// of |a|, a step is a positive impulse over PEAK_G that must fall back under
// RESET_G before the next can count. It replaced a stricter detector (0.10 g
// peak on a 40 ms smoothed signal, four peaks to confirm) that missed light
// steps -- a ball in a bag or on a lanyard -- and so counted well under a phone.
// Our own additions on top of theirs are the rattle guard and the reset after a
// hole in the data; the numbers are theirs.
// Tuned against synthetic gait in steps_test, NOT against a person.
static const float PEAK_G = 0.085f;       // impulse over the baseline that counts
static const float RESET_G = 0.025f;      // it must drop back under this before the next
static const float CALM_G = 0.08f;        // under this the baseline follows |a| fast
static const float TAU_CALM_MS = 310.0f;
static const float TAU_MOVE_MS = 1580.0f;
static const uint32_t MIN_GAP_MS = 280;   // faster than ~3.5 steps/s is not a walk
static const uint32_t MAX_GAP_MS = 1400;  // slower than this and the run is broken
static const uint32_t RATTLE_MS = 150;    // peaks this close are a vibration, not feet
static const uint32_t SETTLE_MS = 400;    // at the start only the baseline settles

void StepDetector::reset() { primed = false; run = 0; inPeak = false; lastAnyMs = 0; lastPeakMs = 0; }

uint8_t StepDetector::feed(float ax, float ay, float az, uint32_t ms) {
  float m = sqrtf(ax * ax + ay * ay + az * az);
  if (!primed || ms - lastMs > 500) {      // first sample, or a hole in the data
    primed = true;
    lastMs = ms;
    firstMs = ms;
    base = m;
    inPeak = false;
    run = 0;
    return 0;
  }
  float dt = (float)(ms - lastMs);
  if (dt <= 0.0f) return 0;
  lastMs = ms;

  float sig = m - base;
  float tau = fabsf(sig) < CALM_G ? TAU_CALM_MS : TAU_MOVE_MS;
  base += (m - base) * (1.0f - expf(-dt / tau));
  if (ms - firstMs < SETTLE_MS) return 0;

  if (inPeak) {                            // the impulse has to die away first
    if (sig <= RESET_G) inPeak = false;
    return 0;
  }
  if (sig < PEAK_G) return 0;
  inPeak = true;

  // A peak right on the heels of the last one, counted or not, is a rattle.
  uint32_t sinceAny = ms - lastAnyMs;
  bool rattle = lastAnyMs && sinceAny < RATTLE_MS;
  lastAnyMs = ms;
  if (rattle) {
    run = 0;
    return 0;
  }
  uint32_t gap = lastPeakMs ? ms - lastPeakMs : 0;
  if (lastPeakMs && gap < MIN_GAP_MS) return 0;   // a ripple on the same footfall
  lastPeakMs = ms;
  if (!gap || gap > MAX_GAP_MS) {                 // first of a run: not walking yet
    run = 1;
    return 0;
  }
  uint8_t n = run == 1 ? 2 : 1;                   // on confirming, the first one counts too
  if (run < 255) run++;
  return n;
}
