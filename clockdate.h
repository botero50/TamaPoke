#pragma once
#include <stdint.h>

// Setting the date by hand (Settings, time page), for a ball that never sees Wi-Fi.
// Pure, so the host tests reach it.

#define DATE_YEAR_MIN 2025
#define DATE_YEAR_MAX 2099

static inline int daysInMonth(int y, int m) {
  static const uint8_t len[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  if (m == 2 && y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) return 29;
  return len[m - 1];
}

// Howard Hinnant's days_from_civil: days since 1970-01-01
static inline uint32_t daysFromCivil(int y, int m, int d) {
  y -= m <= 2;
  const int era = y / 400;
  const int yoe = y - era * 400;
  const int doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
  const int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return (uint32_t)(era * 146097 + doe - 719468);
}

// One press of - or + (dir) on a field: 0 day, 1 month, 2 year. Each wraps around on its
// own, and the day is pulled back when the month it lands in is shorter.
static inline void dateStep(int &y, int &m, int &d, int field, int dir) {
  if (field == 0) {
    const int n = daysInMonth(y, m);
    d = (d - 1 + dir + n) % n + 1;
    return;
  }
  if (field == 1) m = (m - 1 + dir + 12) % 12 + 1;
  else {
    y += dir;
    if (y > DATE_YEAR_MAX) y = DATE_YEAR_MIN;
    if (y < DATE_YEAR_MIN) y = DATE_YEAR_MAX;
  }
  if (d > daysInMonth(y, m)) d = daysInMonth(y, m);
}
