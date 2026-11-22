#pragma once
#include <stdint.h>
#include <stddef.h>

// The QMI8658's own sample FIFO, and the rule for sleeping the chip while the screen is off.
// Pure helpers, kept out of imu.cpp so the host tests reach them (power_test.cpp).
//
// Why a FIFO: the accelerometer task (imu.cpp) polls the sensor every 10 ms, which keeps the CPU
// awake. With the screen off the sensor can store the 62.5 Hz samples itself (128 of them, ~2 s) and
// the loop collects them in a batch, so the chip can sleep in between. Reading it the way
// Mu-Muu/TamaPoke does (MIT, see CREDITS.md): a CTRL9 "read mode" request, then at most 5 samples
// (30 bytes) per I2C read -- a longer read can crash the I2C interrupt when Wi-Fi writes flash, and
// SensorLib's one 1536-byte read is also what returned garbage on this board before.

#define IMU_FIFO_CHUNK_SAMPLES 5       // 6 bytes each: 30 bytes, never more than 32 per I2C read
#define IMU_FIFO_MAX_SAMPLES 128
#define IMU_SAMPLE_MS 16               // 62.5 Hz

// Samples waiting: FIFO_SMPL_CNT (reg 0x15) and FIFO_STATUS bits 1:0 (reg 0x16) count 16-bit words.
static inline uint16_t imuFifoSamples(uint8_t cntLsb, uint8_t statusByte) {
  uint16_t words = (uint16_t)(((statusByte & 0x03) << 8) | cntLsb);
  uint16_t n = (uint16_t)(words * 2 / 6);          // 3 words (x, y, z) per sample
  return n > IMU_FIFO_MAX_SAMPLES ? IMU_FIFO_MAX_SAMPLES : n;
}

// One 6-byte sample (x, y, z as little-endian int16) to g, at the +-4 g range imu.cpp configures.
static inline void imuDecodeSample(const uint8_t *b, float *xyz) {
  const float k = 4.0f / 32768.0f;
  xyz[0] = (int16_t)(b[0] | (b[1] << 8)) * k;
  xyz[1] = (int16_t)(b[2] | (b[3] << 8)) * k;
  xyz[2] = (int16_t)(b[4] | (b[5] << 8)) * k;
}

// The time of sample i (0 = oldest) of n read together at `now`: the newest is `now`, each earlier
// one IMU_SAMPLE_MS older, and never at or before the previous sample's time (`last`).
static inline uint32_t imuSampleTime(uint32_t now, uint16_t n, uint16_t i, uint32_t last) {
  uint32_t t = now - (uint32_t)(n - 1 - i) * IMU_SAMPLE_MS;
  if ((int32_t)(t - last) <= 0) t = last + 1;
  return t;
}

// May the chip go into light sleep this pass? Only with the screen off, on battery, the saver switch
// on, the sensor storing its own samples (otherwise steps would be lost) and nothing that needs the
// CPU: no Wi-Fi sync, no link game, no sound, no alarm.
struct SleepCtx {
  bool screenOff, saverOn, onBattery, fifoOn;
  bool radioBusy, linkLive, audioBusy, alarm;
};
static inline bool lightSleepAllowed(const SleepCtx &c) {
  return c.screenOff && c.saverOn && c.onBattery && c.fifoOn &&
         !c.radioBusy && !c.linkLive && !c.audioBusy && !c.alarm;
}

// After the FIFO is switched on or off the sensor's output can ring for a moment; samples older than
// this many ms after the switch are dropped (0 = keep everything).
#define IMU_SETTLE_MS 0

#define LIGHT_SLEEP_MS 200      // the timer wake: step samples wait in the FIFO (2 s), PWR is polled
