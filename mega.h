#pragma once
#include <stdint.h>
#include <stdio.h>

// Mega Evolution. A Mega Stone turns a companion that has a Mega form into it for 24 hours: a new
// sprite everywhere it shows and +15 % in every Tournament event (tourney.h, Athlete.mega).
//
// Stones are earned in the Stadium: the first gold of a day on three different days gives one
// (the same gold that pays the daily bonus). The idea follows Mu-Muu/TamaPoke (MIT, see CREDITS.md);
// the sprites are SpriteCollab's Mega forms (tools/mega_forms.py has the folder of each).
//
// Pure logic with host tests (mega_test.cpp).

#define MEGA_SECS 86400UL        // how long a Mega Evolution lasts
#define MEGA_STONE_DAYS 3        // gold days for one stone
#define MEGA_STONE_MAX 3         // stones that can be kept

// The species with a usable Mega form in SpriteCollab (tools/mega_forms.py). Sorted.
static const int16_t MEGA_DEX[] = {
  6, 65, 80, 94, 115, 142, 150, 208, 227, 229, 248, 254, 282, 302, 303, 308, 310, 319, 323, 334, 354,
  358, 359, 362, 380, 381, 384, 428, 448, 475, 491, 530, 560, 604, 670, 691, 701, 718, 719, 780, 807,
};
#define MEGA_COUNT ((int)(sizeof(MEGA_DEX) / sizeof(MEGA_DEX[0])))

static inline bool megaHas(int16_t dex) {
  for (int i = 0; i < MEGA_COUNT; i++) if (MEGA_DEX[i] == dex) return true;
  return false;
}

// "/mons/m006.bin": the Mega sprite of a species, the same format as p006.bin
static inline void megaPath(char *out, unsigned n, int16_t dex) {
  snprintf(out, n, "/mons/m%03u.bin", (unsigned)dex);
}

// Is a Mega Evolution that runs until `until` (epoch seconds, 0 = none) still on at `now`?
static inline bool megaRunning(uint32_t until, uint32_t now) { return until != 0 && now < until; }
static inline uint32_t megaRemaining(uint32_t until, uint32_t now) { return megaRunning(until, now) ? until - now : 0; }

// A day's first gold: one more gold day; the third one earns a stone and starts the count again.
// With the stones full the count waits at its last day, so the next free slot is not wasted.
struct MegaProgress { uint8_t days; uint8_t stones; bool earned; };
static inline MegaProgress megaGoldDay(uint8_t days, uint8_t stones) {
  MegaProgress r = { days, stones, false };
  if (days + 1 < MEGA_STONE_DAYS) { r.days = (uint8_t)(days + 1); return r; }
  if (stones >= MEGA_STONE_MAX) { r.days = MEGA_STONE_DAYS - 1; return r; }
  r.days = 0;
  r.stones = (uint8_t)(stones + 1);
  r.earned = true;
  return r;
}
