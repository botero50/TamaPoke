#pragma once
#include <stdint.h>
#include "mega.h"

// The Mine (map page 3): tap the rock until it breaks. Strong miners break it in fewer taps.
// A dig costs energy and there are a few a day. The loot: coins, food, seeds, Mega Stone shards
// (SHARDS_PER_STONE make a stone, kept to MEGA_STONE_MAX) and, rarely, a fossil egg.
// The idea follows Mu-Muu/TamaPoke (MIT, see CREDITS.md); the rules, the numbers and the art are ours.
//
// Pure logic with host tests (mine_test.cpp). Dex numbers and type ids are plain numbers here so
// the header needs neither dex.h nor the Pet.

#define MINE_DIGS_PER_DAY 5
#define MINE_ENERGY_COST 10
#define MINE_MIN_ENERGY 10          // too tired below this
#define SHARDS_PER_STONE 10

// types as in dex.h PkType: fire 1, fighting 6, ground 8, ice 5, rock 12, steel 16
static inline uint8_t mineTapsFor(uint8_t type1, uint8_t type2, uint16_t atk) {
  auto rank = [](uint8_t t) -> int {
    if (t == 12 || t == 16 || t == 6 || t == 1) return 8;      // rock, steel, fighting, fire
    if (t == 8 || t == 5) return 10;                           // ground, ice
    return 14;
  };
  int a = rank(type1), b = type2 == 255 ? 14 : rank(type2);
  int base = a < b ? a : b;
  base -= (int)(atk / 40);                                     // a strong arm saves taps
  if (base < 4) base = 4;
  return (uint8_t)base;
}

enum MineKind : uint8_t { ML_COINS = 0, ML_SHARD, ML_SNACK, ML_SEEDS, ML_FOSSIL };

// roll 0..99
static inline MineKind mineLootKind(uint8_t roll) {
  if (roll < 3) return ML_FOSSIL;
  if (roll < 28) return ML_SHARD;
  if (roll < 73) return ML_COINS;
  if (roll < 88) return ML_SEEDS;
  return ML_SNACK;
}

// 2..5 coins for a coin find
static inline uint8_t mineCoinsFor(uint8_t roll) { return (uint8_t)(2 + roll % 4); }

// the species a fossil egg can hatch: Omanyte, Kabuto, Aerodactyl, Lileep, Anorith, Cranidos,
// Shieldon, Tirtouga, Archen, Tyrunt, Amaura
static const int16_t FOSSIL_DEX[] = { 138, 140, 142, 345, 347, 408, 410, 564, 566, 696, 698 };
#define FOSSIL_COUNT ((int)(sizeof(FOSSIL_DEX) / sizeof(FOSSIL_DEX[0])))
static inline int16_t fossilPick(uint8_t roll) { return FOSSIL_DEX[roll % FOSSIL_COUNT]; }
static inline bool isFossilDex(int16_t d) {
  for (int i = 0; i < FOSSIL_COUNT; i++) if (FOSSIL_DEX[i] == d) return true;
  return false;
}

// A shard: SHARDS_PER_STONE of them make a stone. With the stones full the shards wait at one
// short of a stone, so the next free slot is not wasted.
struct ShardResult { uint8_t shards; uint8_t stones; bool stone; };
static inline ShardResult mineAddShard(uint8_t shards, uint8_t stones) {
  ShardResult r = { shards, stones, false };
  if (shards + 1 < SHARDS_PER_STONE) { r.shards = (uint8_t)(shards + 1); return r; }
  if (stones >= MEGA_STONE_MAX) { r.shards = SHARDS_PER_STONE - 1; return r; }
  r.shards = 0;
  r.stones = (uint8_t)(stones + 1);
  r.stone = true;
  return r;
}

// digs left today: `day` is the day of the last count, `used` how many were done then
static inline uint8_t mineDigsLeft(uint32_t day, uint8_t used, uint32_t today) {
  if (day != today) return MINE_DIGS_PER_DAY;
  return used < MINE_DIGS_PER_DAY ? (uint8_t)(MINE_DIGS_PER_DAY - used) : 0;
}
