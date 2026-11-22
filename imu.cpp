#include "imu.h"
#include "imu_fifo.h"
#include "pin_config.h"
#include <Wire.h>
#include <SensorQMI8658.hpp>

static SensorQMI8658 qmi;
static bool imuOk = false;

// Phase 36: every sample reaches the step detector. The main loop only gets
// round once a render frame (100-300 ms), so one read a pass threw most of the
// 62.5 Hz samples away and walks came out ~25% short. The chip's FIFO looked
// like the answer but on this board it returned garbage (0..2.5 g at rest) and
// each read took about a second. So a small task polls the plain data registers
// every 10 ms -- the same read that was always clean -- and queues each new
// sample with its own time. Wire's own locking was not enough here: the task
// garbled the loop's RTC and PMU reads, so every bus user holds i2cLock().
struct ImuSample { float x, y, z; uint32_t ms; };
static QueueHandle_t imuQ = nullptr;
static SemaphoreHandle_t i2cMx = nullptr;

void i2cLock() {
  if (!i2cMx) i2cMx = xSemaphoreCreateRecursiveMutex();   // first use is in setup(), one task
  if (i2cMx) xSemaphoreTakeRecursive(i2cMx, portMAX_DELAY);
}
void i2cUnlock() { if (i2cMx) xSemaphoreGiveRecursive(i2cMx); }
static volatile uint32_t imuDropped = 0;
static TaskHandle_t imuTaskH = nullptr;
static bool fifoOn = false;                 // FIFO regime: the sensor stores the samples, the poll task is suspended
static uint8_t fifoCtrl = 0;                // FIFO_CTRL as configured, with the read-mode bit clear
static uint32_t fifoLastT = 0;              // time given to the last FIFO sample (never goes back)
static uint32_t settleUntil = 0;            // samples before this time follow a FIFO switch: dropped (IMU_SETTLE_MS)
static volatile uint32_t fifoFails = 0, fifoTotal = 0;

static void imuTask(void *) {
  for (;;) {
    float x, y, z;
    i2cLock();
    bool got = qmi.getDataReady() && qmi.getAccelerometer(x, y, z);
    i2cUnlock();
    if (got) {
      ImuSample s = { x, y, z, (uint32_t)millis() };
      if (xQueueSend(imuQ, &s, 0) != pdTRUE) imuDropped++;   // the loop stalled for ~2 s: drop, do not block
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

bool imuBegin() {
  i2cLock();                      // creates the lock before the task exists
  imuOk = qmi.begin(Wire, QMI8658_L_SLAVE_ADDRESS, IIC_SDA, IIC_SCL);
  i2cUnlock();
  if (!imuOk) {
    Serial.println("QMI8658 not detected");
    return false;
  }
  // 62.5 Hz is plenty for a walking gait (~2 Hz). 4 g covers a heavy footfall.
  i2cLock();
  qmi.configAccelerometer(SensorQMI8658::ACC_RANGE_4G, SensorQMI8658::ACC_ODR_62_5Hz);
  qmi.enableAccelerometer();
  i2cUnlock();
  imuQ = xQueueCreate(IMU_BATCH_MAX, sizeof(ImuSample));
  if (imuQ) xTaskCreatePinnedToCore(imuTask, "imu", 3072, nullptr, 1, &imuTaskH, 0);
  return true;
}

bool imuReadAccel(float &x, float &y, float &z) {
  if (!imuOk) return false;
  i2cLock();
  bool ok = qmi.getDataReady() && qmi.getAccelerometer(x, y, z);
  i2cUnlock();
  return ok;
}

// ---- the sensor's FIFO (all of it under i2cLock) ----
// Raw register access: the library's reads are too long for the I2C controller's 32-byte FIFO.
static bool rawReg(uint8_t reg, uint8_t *buf, uint8_t n) {
  Wire.beginTransmission(QMI8658_L_SLAVE_ADDRESS);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)QMI8658_L_SLAVE_ADDRESS, (int)n) != n) return false;
  for (uint8_t k = 0; k < n; k++) buf[k] = Wire.read();
  return true;
}
static bool rawPut(uint8_t reg, uint8_t v) {
  Wire.beginTransmission(QMI8658_L_SLAVE_ADDRESS);
  Wire.write(reg);
  Wire.write(v);
  return Wire.endTransmission() == 0;
}
// a CTRL9 command with its handshake: STATUSINT bit 7 says done, then ACK
static bool ctrl9(uint8_t cmd) {
  if (!rawPut(0x0A, cmd)) return false;
  bool done = false;
  uint8_t st;
  for (int k = 0; k < 50 && !done; k++) {
    if (rawReg(0x2D, &st, 1) && (st & 0x80)) done = true;
    else delayMicroseconds(100);
  }
  rawPut(0x0A, 0x00);
  for (int k = 0; k < 50; k++) {
    if (rawReg(0x2D, &st, 1) && !(st & 0x80)) break;
    delayMicroseconds(100);
  }
  return done;
}

bool imuFifoActive() { return fifoOn; }
uint32_t imuFifoFailCount() { return fifoFails; }
uint32_t imuFifoSampleCount() { return fifoTotal; }

bool imuFifoEnter() {
  if (fifoOn) return true;
  if (!imuOk || !imuTaskH || fifoFails >= 8) return false;   // too many failures: stay on the poll task
  i2cLock();
  vTaskSuspend(imuTaskH);            // it cannot hold the lock while we do: it takes it only for a read
  // Set the FIFO mode straight in its register with the accelerometer left running: the library's
  // configFIFO() switches the accelerometer off and on, and its output rings for about a second after.
  bool ok = ctrl9(0x04) && rawPut(0x14, 0x0E);          // CTRL_CMD_RST_FIFO, then stream mode with 128 samples
  uint8_t c = 0;
  if (ok) ok = rawReg(0x14, &c, 1) && ((c & 0x03) == 0x02);
  if (ok) {
    fifoCtrl = c & 0x7F;
    fifoOn = true;
    fifoLastT = millis();
    settleUntil = fifoLastT + IMU_SETTLE_MS;
  } else {
    fifoFails++;
    rawPut(0x14, 0x00);
    vTaskResume(imuTaskH);
  }
  i2cUnlock();
  return fifoOn;
}

void imuFifoExit() {
  if (!fifoOn) return;
  i2cLock();
  rawPut(0x14, 0x00);                                // bypass: the plain data registers again
  vTaskResume(imuTaskH);
  fifoOn = false;
  settleUntil = millis() + IMU_SETTLE_MS;
  i2cUnlock();
}

// Everything the sensor stored since the last drain, oldest first. A failed read counts and returns
// what was decoded so far; the loop keeps going.
static uint8_t fifoDrain(float *xyz, uint32_t *ms, uint8_t max) {
  uint32_t now = millis();
  uint8_t got = 0;
  bool ok = true;
  i2cLock();
  uint8_t c[2];
  uint16_t n = 0;
  if (rawReg(0x15, c, 2)) n = imuFifoSamples(c[0], c[1]); else ok = false;
  if (n > max) n = max;
  if (ok && n) {
    ok = ctrl9(0x05);                                   // CTRL_CMD_REQ_FIFO: read mode (no samples enter meanwhile)
    uint8_t buf[IMU_FIFO_CHUNK_SAMPLES * 6];
    for (uint16_t i = 0; ok && i < n;) {
      uint8_t k = (n - i > IMU_FIFO_CHUNK_SAMPLES) ? IMU_FIFO_CHUNK_SAMPLES : (uint8_t)(n - i);
      if (!rawReg(0x17, buf, (uint8_t)(k * 6))) { ok = false; break; }
      for (uint8_t j = 0; j < k; j++, i++) {
        imuDecodeSample(buf + j * 6, xyz + got * 3);
        ms[got] = imuSampleTime(now, n, i, fifoLastT);
        fifoLastT = ms[got];
        got++;
      }
    }
    rawPut(0x14, fifoCtrl);                             // out of read mode: samples enter again
  }
  i2cUnlock();
  if (!ok) fifoFails++;
  fifoTotal += got;
  return got;
}

// drops the samples taken before settleUntil (a FIFO switch can make the output ring for a moment)
static uint8_t settleFilter(float *xyz, uint32_t *ms, uint8_t n) {
  if (!IMU_SETTLE_MS) return n;
  uint8_t w = 0;
  for (uint8_t i = 0; i < n; i++) {
    if ((int32_t)(ms[i] - settleUntil) < 0) continue;
    if (w != i) { xyz[w * 3] = xyz[i * 3]; xyz[w * 3 + 1] = xyz[i * 3 + 1]; xyz[w * 3 + 2] = xyz[i * 3 + 2]; ms[w] = ms[i]; }
    w++;
  }
  return w;
}

uint8_t imuReadBatch(float *xyz, uint32_t *ms, uint8_t max) {
  if (!imuOk || !max) return 0;
  if (fifoOn) return settleFilter(xyz, ms, fifoDrain(xyz, ms, max));
  if (!imuQ) {                                     // no task: one sample a pass, as before
    float x, y, z;
    if (!imuReadAccel(x, y, z)) return 0;
    xyz[0] = x; xyz[1] = y; xyz[2] = z; ms[0] = millis();
    return 1;
  }
  uint8_t n = 0;
  ImuSample s;
  while (n < max && xQueueReceive(imuQ, &s, 0) == pdTRUE) {
    xyz[n * 3] = s.x; xyz[n * 3 + 1] = s.y; xyz[n * 3 + 2] = s.z; ms[n] = s.ms;
    n++;
  }
  return settleFilter(xyz, ms, n);
}

bool imuFifoOn() { return imuQ != nullptr; }
uint32_t imuDroppedCount() { return imuDropped; }
