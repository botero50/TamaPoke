#pragma once
#include <Arduino.h>

// QMI8658 6-axis IMU. Only the accelerometer is used (steps, the tilt games).
#define IMU_BATCH_MAX 128
// The I2C bus is shared by the IMU (read from its own task), the touch panel,
// the RTC and the PMU (read from the loop). Every user holds this lock.
void i2cLock();
void i2cUnlock();
bool imuBegin();                              // false if the chip is not found
bool imuReadAccel(float &x, float &y, float &z);   // in g; false when no new sample is ready
// Every sample since the last call, oldest first: x,y,z triples in g and the
// millis() each was read at. A background task reads the sensor every 10 ms;
// without it, one sample a pass as before.
uint8_t imuReadBatch(float *xyz, uint32_t *ms, uint8_t max);
bool imuFifoOn();                             // the background reader is running (STATS: "imu fifo=")
uint32_t imuDroppedCount();                   // samples lost to a stalled loop
