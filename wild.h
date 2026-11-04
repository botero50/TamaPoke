#pragma once
#include <stdint.h>
#include "dex.h"

// Wild encounters: the rules, as pure functions. Every random roll is passed in, so
// wild_test can pin each curve at its limits. The formulas follow eperdeme's fork
// (MIT), which took the design from nzlov's (MIT).

#define WILD_BALL_KINDS 3

// A ball's strength, as a percentage of a Poke Ball's.
static const uint16_t WILD_BALL_PCT[WILD_BALL_KINDS] = { 100, 150, 200 };
static const uint16_t WILD_BALL_PRICE[WILD_BALL_KINDS] = { 5, 15, 30 };

#define WILD_LEVEL_SPREAD 5    // a foe stays this close to the player's level
#define WILD_CATCH_MAX 95      // never certain: a ball is always a small gamble
#define WILD_DROP_PCT 30       // the chance of finding a Poke Ball after a win
#define WILD_SHINY_FLOOR 20    // every IV of a shiny is at least this
#define WILD_SHINY_ODDS 4096

// Catch chance in percent. A full-health foe counts as 100 and a nearly fainted one
// as 180, so weakening it helps, but never as much as a better ball. `rank` is the
// lifetime walking rank, worth +1% each.
inline uint8_t wildCaptureChance(uint8_t rarity, uint16_t hp, uint16_t maxHp, uint16_t ballPct, uint8_t rank) {
  if (!maxHp || !hp || !ballPct) return 0;
  if (hp > maxHp) hp = maxHp;
  uint32_t base;
  switch (rarity) {
    case R_LEGENDARIO: base = 5; break;
    case R_RARO:       base = 18; break;
    case R_EVO:        base = 24; break;
    default:           base = 30; break;
  }
  uint32_t hpFactor = 100U + (uint32_t)(maxHp - hp) * 80U / maxHp;
  uint32_t chance = base * hpFactor * ballPct / 10000U + rank;
  if (!chance) chance = 1;
  return chance > WILD_CATCH_MAX ? WILD_CATCH_MAX : (uint8_t)chance;
}

// Each rank adds one more chance in 4096: 1 in 4096 for a beginner, 1 in 455 for a legend.
inline bool wildShinyForRoll(uint32_t roll, uint8_t rank) {
  return (roll % WILD_SHINY_ODDS) < (uint32_t)rank + 1;
}

inline uint8_t wildLevelMin(uint8_t playerLevel) {
  return playerLevel > WILD_LEVEL_SPREAD ? (uint8_t)(playerLevel - WILD_LEVEL_SPREAD) : 1;
}
inline uint8_t wildLevelMax(uint8_t playerLevel, uint8_t maxLevel) {
  uint16_t top = (uint16_t)playerLevel + WILD_LEVEL_SPREAD;
  return top > maxLevel ? maxLevel : (uint8_t)top;
}

// roll is 0..99: 1% legendary, 7% rare, 22% evolved, the rest common.
inline uint8_t wildTierForRoll(uint8_t roll) {
  if (roll < 1) return R_LEGENDARIO;
  if (roll < 8) return R_RARO;
  if (roll < 30) return R_EVO;
  return R_COMUN;
}

// A species of that rarity between dex numbers lo and hi that `ok` accepts, or 0.
// Two passes rather than a list: a list of 809 would be 1.6 KB on the stack.
inline int16_t wildPickSpecies(uint16_t lo, uint16_t hi, uint8_t tier, uint32_t roll, bool (*ok)(int16_t)) {
  if (hi > DEX_COUNT) hi = DEX_COUNT;
  if (lo < 1) lo = 1;
  uint16_t n = 0;
  for (uint16_t d = lo; d <= hi; d++)
    if (DEX_TBL[d].rarity == tier && ok((int16_t)d)) n++;
  if (!n) return 0;
  uint16_t want = (uint16_t)(roll % n);
  for (uint16_t d = lo; d <= hi; d++) {
    if (DEX_TBL[d].rarity != tier || !ok((int16_t)d)) continue;
    if (!want) return (int16_t)d;
    want--;
  }
  return 0;
}
