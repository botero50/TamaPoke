#pragma once
#include <stdint.h>
#include <stdio.h>

// The standby clock: when the ball is plugged in and nobody touches it, the screen turns into
// a big clock (black background: the AMOLED draws nothing for black) with the date, the
// weather and the companion. The ideas are from Mu-Muu/TamaPoke (MIT, see CREDITS.md); this is
// our own code, kept apart from the sketch so the host tests can reach it.

#define STANDBY_IDLE_MS 90000UL     // untouched this long, on USB: the clock
#define SELFCARE_IDLE_MS 600000UL   // untouched this long, on USB: it sees to its own needs

struct StandbyCtx {
  bool usb;          // the USB power is connected
  bool screenOn;     // not turned off with PWR
  bool mainScreen;   // the pet screen, no menu or game open
  bool modal;        // a ceremony, evolution, alarm, starter choice, Wi-Fi portal, an egg...
  uint32_t idleMs;   // time since the last touch
};

static inline bool standbyDue(const StandbyCtx &c) {
  return c.usb && c.screenOn && c.mainScreen && !c.modal && c.idleMs >= STANDBY_IDLE_MS;
}

static inline bool selfCareDue(bool usb, uint32_t idleMs) {
  return usb && idleMs >= SELFCARE_IDLE_MS;
}

// 0 = Sunday .. 6 = Saturday, from days since 1970-01-01 (a Thursday)
static inline uint8_t weekdayOf(uint32_t days) { return (uint8_t)((days + 4) % 7); }

// A different small offset every minute (-4..+4 px each way) so no pixel stays lit in the same
// place for hours.
static inline void standbyShift(uint32_t minuteStamp, int &dx, int &dy) {
  dx = (int)((minuteStamp * 7) % 9) - 4;
  dy = (int)((minuteStamp * 5 + 3) % 9) - 4;
}

// "1:05" in 12 h (no leading zero), "13:05" in 24 h; *pm tells AM/PM for the 12 h form.
static inline void standbyTimeText(char *out, size_t n, int hour, int minute, bool h24, bool *pm) {
  if (pm) *pm = hour >= 12;
  if (h24) snprintf(out, n, "%02d:%02d", hour, minute);
  else snprintf(out, n, "%d:%02d", hour % 12 == 0 ? 12 : hour % 12, minute);
}

// Big digits, 5 columns x 7 rows, one row per byte, the left column in bit 4.
static const uint8_t STANDBY_DIGIT[10][7] = {
  { 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e },  // 0
  { 0x04, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x0e },  // 1
  { 0x0e, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1f },  // 2
  { 0x1f, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0e },  // 3
  { 0x02, 0x06, 0x0a, 0x12, 0x1f, 0x02, 0x02 },  // 4
  { 0x1f, 0x10, 0x1e, 0x01, 0x01, 0x11, 0x0e },  // 5
  { 0x06, 0x08, 0x10, 0x1e, 0x11, 0x11, 0x0e },  // 6
  { 0x1f, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 },  // 7
  { 0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e },  // 8
  { 0x0e, 0x11, 0x11, 0x0f, 0x01, 0x02, 0x0c },  // 9
};

// Width in blocks of the text "0-9 and :" (a digit is 5 + 1 of gap, a colon 2).
static inline int standbyTextBlocks(const char *t) {
  int w = 0;
  for (; *t; t++) w += (*t == ':') ? 2 : 6;
  return w > 0 ? w - 1 : 0;   // no gap after the last glyph
}
