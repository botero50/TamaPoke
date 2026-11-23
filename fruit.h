#pragma once
#include <stdint.h>

// Training fruits: the garden's harvest sometimes gives one; eating it adds to one training stat
// (never over the individual's ceiling, see Pet::trMaxFor). The idea follows Mu-Muu/TamaPoke (MIT,
// see CREDITS.md); the numbers are ours. Pure logic with host tests (fruit_test.cpp).

#define FRUIT_KINDS 3                // Power (Attack), Guard (Defence), Swift (Speed)
#define FRUIT_STACK_MAX 9
#define FRUIT_GAIN 4                 // training points one fruit gives
#define FRUIT_HARVEST_IN 4           // one harvest in this many is a fruit
#define FRUIT_FOOD_BASE 100          // Pet::harvest returns FRUIT_FOOD_BASE + kind for a fruit

// roll 0..99 of a harvest: the kind of fruit, or -1 for the usual food
static inline int8_t fruitFromRoll(uint8_t roll, uint8_t kindRoll) {
  if (roll >= 100 / FRUIT_HARVEST_IN) return -1;
  return (int8_t)(kindRoll % FRUIT_KINDS);
}

// how much a fruit adds to a training stat at `tr` with the ceiling `cap`
static inline uint8_t fruitGain(uint8_t tr, uint8_t cap) {
  if (tr >= cap) return 0;
  uint16_t room = (uint16_t)(cap - tr);
  return room < FRUIT_GAIN ? (uint8_t)room : (uint8_t)FRUIT_GAIN;
}
