#pragma once
// Emulator stand-ins for the GPIO wake-up calls used before a light sleep.
#include <stdint.h>
typedef int gpio_num_t;
typedef int gpio_int_type_t;
#define GPIO_INTR_LOW_LEVEL 4
#define GPIO_INTR_NEGEDGE 2
inline int gpio_wakeup_enable(gpio_num_t, gpio_int_type_t) { return 0; }
inline int gpio_wakeup_disable(gpio_num_t) { return 0; }
inline int gpio_set_intr_type(gpio_num_t, gpio_int_type_t) { return 0; }
