#pragma once
// Emulator stand-ins for the ESP32 sleep API: the host never sleeps, it counts what was asked.
#include <stdint.h>
inline int &emuSleepCalls() { static int n = 0; return n; }
inline int esp_sleep_enable_timer_wakeup(uint64_t) { return 0; }
inline int esp_sleep_enable_gpio_wakeup() { return 0; }
inline int esp_light_sleep_start() { emuSleepCalls()++; return 0; }
