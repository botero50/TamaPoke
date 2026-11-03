#include "imu.h"
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
  if (imuQ) xTaskCreatePinnedToCore(imuTask, "imu", 3072, nullptr, 1, nullptr, 0);
  return true;
}

bool imuReadAccel(float &x, float &y, float &z) {
  if (!imuOk) return false;
  i2cLock();
  bool ok = qmi.getDataReady() && qmi.getAccelerometer(x, y, z);
  i2cUnlock();
  return ok;
}

uint8_t imuReadBatch(float *xyz, uint32_t *ms, uint8_t max) {
  if (!imuOk || !max) return 0;
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
  return n;
}

bool imuFifoOn() { return imuQ != nullptr; }
uint32_t imuDroppedCount() { return imuDropped; }
