// Battery and the screen-off sleep, the parts that are pure logic: the LiPo curve and the power-off
// rule (batcurve.h), the sensor FIFO decoding and timing (imu_fifo.h), the light-sleep rule, and
// that steps counted from FIFO batches match steps counted sample by sample. What this cannot say is
// whether the real sensor, the real battery and a real sleep behave: that is the ball.
#include "Arduino.h"
#include "Preferences.h"
#include "pet.h"
#include "steps.h"
#include "batcurve.h"
#include "imu_fifo.h"
#include <cstdio>
#include <cmath>
uint32_t g_seed = 11; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
volatile int g_touchX = 0, g_touchY = 0; volatile bool g_touchDown = false; bool wasPressed = false;
static uint32_t g_ms = 0; uint32_t millis() { return g_ms; }
void FakeESP::restart() { exit(0); }
int FakeSerial::available() { return 0; } String FakeSerial::readStringUntil(char) { return String(""); }
void sfxPlay(uint8_t) {}
static int bad = 0;
static void ck(bool ok, const char *w) { printf("%s  %s\n", ok ? "PASS" : "FAIL", w); if (!ok) bad++; }

int main() {
  // ---- the curve
  ck(batPercentFromMv(4200) == 100 && batPercentFromMv(4300) == 100, "4.2 V and above is 100 %");
  ck(batPercentFromMv(3300) == 0 && batPercentFromMv(3000) == 0, "the cut-off and below is 0 %");
  ck(batPercentFromMv(3840) == 50 && batPercentFromMv(3770) == 30, "points on the curve read as their percentage");
  ck(batPercentFromMv(3725) == 18, "between two points it interpolates (3.725 V is 18 %)");
  bool mono = true;
  for (int mv = 3300; mv < 4200; mv++) if (batPercentFromMv(mv + 1) < batPercentFromMv(mv)) mono = false;
  ck(mono, "a higher voltage never reads lower");

  // ---- the monitor
  {
    BatMonitor m;
    m.seed(80);
    ck(m.sample(4150, 1000) == 80, "right after unplugging, a charge-inflated 4.15 V does not raise the 80 % the PMU showed");
    int last = 80; bool never_up = true;
    for (int mv = 4000; mv >= 3700; mv -= 20) { int p = m.sample(mv, 2000); if (p > last) never_up = false; last = p; }
    ck(never_up && last < 80, "while discharging the percentage only falls");
    int before = m.pct;
    m.sample(4100, 3000);
    ck(m.pct == before, "a bump back up does not raise it");
    ck(m.sample(0, 3500) == -1, "no reading (no battery) says so");
  }
  {
    BatMonitor m;
    for (int i = 0; i < 6; i++) m.sample(3250, 10000 + i * 2000);    // under 3.30 V from t = 10 s
    ck(!m.shutdownNeeded(25000), "not in the first 30 s after boot");
    ck(!m.shutdownNeeded(39999) && m.shutdownNeeded(40000), "off only after the smoothed voltage stayed under 3.30 V for 30 s");
    m.sample(3700, 41000);
    ck(!m.shutdownNeeded(60000), "a reading back up ends the countdown");
    BatMonitor n;
    n.sample(3250, 31000); n.sample(3250, 62000);
    ck(!n.shutdownNeeded(62000), "two low readings are not enough, whatever the time");
    BatMonitor q;
    for (int i = 0; i < 4; i++) q.sample(3600, 40000 + i * 10000);
    ck(!q.shutdownNeeded(80000), "3.6 V is not low enough to switch off");
  }

  // ---- the sensor FIFO
  ck(imuFifoSamples(0x80, 0x01) == 128, "a full FIFO (384 words) is 128 samples");
  ck(imuFifoSamples(3, 0) == 1 && imuFifoSamples(0, 0) == 0, "3 words are one sample, none is none");
  ck(imuFifoSamples(0xFF, 0x03) == 128, "a count past the FIFO size is clamped");
  ck(imuFifoSamples(0x40, 0xFC) == imuFifoSamples(0x40, 0x00), "only the two low bits of the status byte count");
  {
    const uint8_t s1[6] = { 0x00, 0x20, 0x00, 0x00, 0x00, 0xE0 };   // x = +8192, y = 0, z = -8192
    float v[3]; imuDecodeSample(s1, v);
    ck(fabsf(v[0] - 1.0f) < 1e-4f && fabsf(v[1]) < 1e-6f && fabsf(v[2] + 1.0f) < 1e-4f, "a sample decodes to g at the +-4 g range");
    const uint8_t s2[6] = { 0xFF, 0x7F, 0x00, 0x80, 0x01, 0x00 };   // +32767, -32768, +1
    imuDecodeSample(s2, v);
    ck(v[0] > 3.99f && v[1] < -3.99f && v[2] > 0.0f && v[2] < 0.001f, "extremes and signs decode right");
  }
  {
    uint32_t last = 0; bool ok = true; uint32_t prev = 0;
    for (uint16_t i = 0; i < 20; i++) {
      uint32_t t = imuSampleTime(100000, 20, i, last);
      if (i && t - prev != IMU_SAMPLE_MS) ok = false;
      prev = t; last = t;
    }
    ck(ok && last == 100000, "a batch is spaced 16 ms apart and its newest sample is 'now'");
    ck(imuSampleTime(1000, 10, 0, 950) > 950, "a timestamp never goes back past the previous sample");
  }

  // ---- the light-sleep rule
  SleepCtx c = { true, true, true, true, false, false, false, false };
  ck(lightSleepAllowed(c), "screen off, saver on, on battery, FIFO on and nothing busy: it may sleep");
  { SleepCtx x = c; x.screenOff = false; ck(!lightSleepAllowed(x), "never with the screen on"); }
  { SleepCtx x = c; x.saverOn = false; ck(!lightSleepAllowed(x), "never with the saver switched off"); }
  { SleepCtx x = c; x.onBattery = false; ck(!lightSleepAllowed(x), "never on USB (the serial port and the installer stay available)"); }
  { SleepCtx x = c; x.fifoOn = false; ck(!lightSleepAllowed(x), "never unless the sensor is storing the steps itself"); }
  { SleepCtx x = c; x.radioBusy = true; ck(!lightSleepAllowed(x), "not during a Wi-Fi sync"); }
  { SleepCtx x = c; x.linkLive = true; ck(!lightSleepAllowed(x), "not during a link game"); }
  { SleepCtx x = c; x.audioBusy = true; ck(!lightSleepAllowed(x), "not while sound plays"); }
  { SleepCtx x = c; x.alarm = true; ck(!lightSleepAllowed(x), "not while the alarm rings"); }

  // ---- steps from FIFO batches match steps sample by sample
  {
    const float hz = 1.8f, amp = 0.4f;
    const int N = 62 * 25;                       // 25 s at 62.5 Hz (16 ms apart)
    static float ax[2000], ay[2000], az[2000]; static uint32_t tt[2000];
    for (int k = 0; k < N; k++) {
      tt[k] = 5000 + (uint32_t)k * IMU_SAMPLE_MS;
      float s = amp * sinf(2.0f * 3.14159265f * hz * (tt[k] - 5000) / 1000.0f);
      ax[k] = 0; ay[k] = 0; az[k] = 1.0f + s;
    }
    StepDetector a; int exact = 0;
    for (int k = 0; k < N; k++) exact += a.feed(ax[k], ay[k], az[k], tt[k]);
    ck(exact > 20, "the synthetic walk is counted (the reference run)");
    StepDetector b; int batched = 0; uint32_t last = 0;
    for (int k = 0; k < N;) {                                         // read every ~200 ms, 13 samples at a time
      int n = (N - k > 13) ? 13 : (N - k);
      uint32_t now = tt[k + n - 1];
      for (int i = 0; i < n; i++) {
        uint32_t t = imuSampleTime(now, (uint16_t)n, (uint16_t)i, last);
        last = t;
        batched += b.feed(ax[k + i], ay[k + i], az[k + i], t);
      }
      k += n;
    }
    ck(batched == exact, "the same walk read in 200 ms FIFO batches counts exactly the same steps");
    StepDetector d2; int jit = 0; last = 0;
    for (int k = 0; k < N;) {                                         // the loop wakes a few ms late each time
      int n = (N - k > 13) ? 13 : (N - k);
      uint32_t now = tt[k + n - 1] + (uint32_t)(k % 7);
      for (int i = 0; i < n; i++) {
        uint32_t t = imuSampleTime(now, (uint16_t)n, (uint16_t)i, last);
        last = t;
        jit += d2.feed(ax[k + i], ay[k + i], az[k + i], t);
      }
      k += n;
    }
    ck(abs(jit - exact) <= 1, "and a few ms of wake-up jitter changes the count by at most one");
    StepDetector s2; int still = 0; last = 0;
    for (int k = 0; k < N;) {
      int n = (N - k > 13) ? 13 : (N - k);
      uint32_t now = 5000 + (uint32_t)(k + n) * IMU_SAMPLE_MS;
      for (int i = 0; i < n; i++) { uint32_t t = imuSampleTime(now, (uint16_t)n, (uint16_t)i, last); last = t; still += s2.feed(0, 0, 1.0f, t); }
      k += n;
    }
    ck(still == 0, "a ball lying still counts no phantom steps in batches");
  }

  if (bad) { printf("FAILURES: %d\n", bad); return 1; }
  printf("power test passed\n");
  return 0;
}
