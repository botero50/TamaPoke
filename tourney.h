#pragma once
#include <stdint.h>
#include <math.h>
#include "dex.h"

// The Tournament (the Stadium on the town map): eleven events your companion plays against two
// random Pokemon, by itself, nobody taps and nobody fights. Who does well depends on what each one
// IS: the base stats of its species (each event weighs them its own way), its genes (IVs) and
// training, its evolution stage, its level and its type (every event favours some types and
// punishes others), plus a bit of luck, so the strongest does not always win.
//
// Pure logic with host tests (tourney_test.cpp). The rules and their numbers follow
// Mu-Muu/TamaPoke's sport.h (MIT, see CREDITS.md), re-keyed to our type enum and our stats.

enum : uint8_t {
  EV_SPRINT = 0, EV_HURDLES, EV_SWIM, EV_LONGJUMP, EV_HIGHJUMP,
  EV_LIFT, EV_CLIMB, EV_DIVE, EV_SKATE, EV_TORCH, EV_TRAIL,
  EV_COUNT
};

#define TOUR_ENERGY_COST 15     // each event tires it
#define TOUR_MIN_ENERGY 20      // under this it is too tired to compete

struct Athlete {
  int16_t dex;
  uint8_t geneAtk, geneDef, geneSpe;   // 90..110 (% of the species stat)
  uint8_t trAtk, trDef, trSpe;         // 0..100 trained
  uint16_t level;
  bool mega;
};

// 1 = a first form, 2 = it evolved from another, 3 = from one that had evolved too. Legendaries
// count as a final form. A branch evolution (Eevee's) has no single predecessor in the table: it is
// an evolved form, so 2.
static inline uint8_t evoStage(int16_t dex) {
  if (dex < 1 || dex > DEX_COUNT) return 1;
  if (DEX_TBL[dex].rarity == R_LEGENDARIO) return 3;
  uint8_t stage = 1;
  for (int turn = 0; turn < 3; turn++) {
    int16_t prev = 0;
    for (int16_t p = 1; p <= DEX_COUNT && !prev; p++)
      if (DEX_TBL[p].evolvesTo == dex) prev = p;
    if (!prev) { if (turn == 0 && DEX_TBL[dex].rarity == R_EVO) stage = 2; break; }
    stage++;
    dex = prev;
  }
  return stage > 3 ? 3 : stage;
}

static inline Athlete athleteOf(int16_t dex, uint16_t level = 1) {
  Athlete a = { dex, 100, 100, 100, 0, 0, 0, level, false };
  return a;
}

// a rival: the species given, genes and training at random (u* in [0,1))
static inline Athlete rivalAthlete(int16_t dex, uint16_t level, float uAtk, float uDef, float uSpe, float uTr) {
  Athlete a = athleteOf(dex, level);
  a.geneAtk = (uint8_t)(90 + uAtk * 21);
  a.geneDef = (uint8_t)(90 + uDef * 21);
  a.geneSpe = (uint8_t)(90 + uSpe * 21);
  a.trAtk = a.trDef = a.trSpe = (uint8_t)(uTr * 61);     // 0..60: a rival is trained, never a champion
  return a;
}

// What each event favours: some types do better (>1) and some worse (<1)
struct TypeMod { uint8_t ev, type; float mul; };
static const TypeMod TYPE_MODS[] = {
  { EV_SPRINT, T_ELECTRIC, 1.15f }, { EV_SPRINT, T_FIGHTING, 1.10f }, { EV_SPRINT, T_NORMAL, 1.05f },
  { EV_SPRINT, T_ROCK, 0.90f }, { EV_SPRINT, T_STEEL, 0.90f },
  { EV_HURDLES, T_FIGHTING, 1.15f }, { EV_HURDLES, T_BUG, 1.10f }, { EV_HURDLES, T_GRASS, 1.05f },
  { EV_HURDLES, T_ROCK, 0.90f }, { EV_HURDLES, T_STEEL, 0.90f },
  { EV_SWIM, T_WATER, 1.25f }, { EV_SWIM, T_ICE, 1.10f }, { EV_SWIM, T_DRAGON, 1.05f },
  { EV_SWIM, T_FIRE, 0.85f }, { EV_SWIM, T_ROCK, 0.85f }, { EV_SWIM, T_GROUND, 0.90f }, { EV_SWIM, T_STEEL, 0.90f },
  { EV_LONGJUMP, T_FIGHTING, 1.15f }, { EV_LONGJUMP, T_FLYING, 1.15f }, { EV_LONGJUMP, T_BUG, 1.10f },
  { EV_LONGJUMP, T_NORMAL, 1.05f }, { EV_LONGJUMP, T_ROCK, 0.90f }, { EV_LONGJUMP, T_STEEL, 0.90f },
  { EV_HIGHJUMP, T_FLYING, 1.25f }, { EV_HIGHJUMP, T_PSYCHIC, 1.15f }, { EV_HIGHJUMP, T_FIGHTING, 1.10f },
  { EV_HIGHJUMP, T_FAIRY, 1.05f }, { EV_HIGHJUMP, T_ROCK, 0.85f }, { EV_HIGHJUMP, T_STEEL, 0.85f }, { EV_HIGHJUMP, T_GROUND, 0.90f },
  { EV_LIFT, T_FIGHTING, 1.20f }, { EV_LIFT, T_STEEL, 1.15f }, { EV_LIFT, T_ROCK, 1.15f }, { EV_LIFT, T_GROUND, 1.10f },
  { EV_LIFT, T_FLYING, 0.90f }, { EV_LIFT, T_FAIRY, 0.90f }, { EV_LIFT, T_BUG, 0.90f },
  { EV_CLIMB, T_ROCK, 1.20f }, { EV_CLIMB, T_GROUND, 1.15f }, { EV_CLIMB, T_BUG, 1.10f }, { EV_CLIMB, T_GRASS, 1.05f },
  { EV_CLIMB, T_WATER, 0.90f }, { EV_CLIMB, T_STEEL, 0.90f },
  { EV_DIVE, T_WATER, 1.20f }, { EV_DIVE, T_PSYCHIC, 1.10f }, { EV_DIVE, T_FLYING, 1.10f }, { EV_DIVE, T_FAIRY, 1.10f },
  { EV_DIVE, T_FIRE, 0.85f }, { EV_DIVE, T_ROCK, 0.85f }, { EV_DIVE, T_STEEL, 0.90f },
  { EV_SKATE, T_ICE, 1.25f }, { EV_SKATE, T_FAIRY, 1.10f }, { EV_SKATE, T_PSYCHIC, 1.05f },
  { EV_SKATE, T_FIRE, 0.85f }, { EV_SKATE, T_GRASS, 0.90f },
  { EV_TORCH, T_FIRE, 1.25f }, { EV_TORCH, T_DRAGON, 1.10f }, { EV_TORCH, T_NORMAL, 1.05f },
  { EV_TORCH, T_WATER, 0.90f }, { EV_TORCH, T_ICE, 0.90f },
  { EV_TRAIL, T_DARK, 1.15f }, { EV_TRAIL, T_GHOST, 1.15f }, { EV_TRAIL, T_POISON, 1.10f }, { EV_TRAIL, T_GRASS, 1.10f },
  { EV_TRAIL, T_BUG, 1.05f }, { EV_TRAIL, T_STEEL, 0.90f }, { EV_TRAIL, T_ROCK, 0.90f },
};

static inline float typeBonusOne(uint8_t ev, uint8_t type) {
  for (unsigned i = 0; i < sizeof(TYPE_MODS) / sizeof(TYPE_MODS[0]); i++)
    if (TYPE_MODS[i].ev == ev && TYPE_MODS[i].type == type) return TYPE_MODS[i].mul;
  return 1.0f;
}
// a dual-typed Pokemon takes the average of its two types' modifiers
static inline float typeBonus(uint8_t ev, uint8_t t1, uint8_t t2) {
  float a = typeBonusOne(ev, t1);
  if (t2 == T_NONE || t2 >= TYPE_COUNT) return a;
  return (a + typeBonusOne(ev, t2)) / 2.0f;
}

// what each event looks at: the weight of HP, attack, defence and speed (they add up to 1)
static const float EV_W[EV_COUNT][4] = {
  { 0.00f, 0.25f, 0.00f, 0.75f },  // 100 m: mostly speed
  { 0.00f, 0.25f, 0.20f, 0.55f },  // hurdles: speed, spring and poise
  { 0.50f, 0.00f, 0.00f, 0.50f },  // swimming: speed and stamina (HP)
  { 0.00f, 0.50f, 0.00f, 0.50f },  // long jump: strength and speed
  { 0.00f, 0.65f, 0.00f, 0.35f },  // high jump: mostly strength
  { 0.10f, 0.60f, 0.30f, 0.00f },  // weights: strength and steadiness
  { 0.00f, 0.40f, 0.20f, 0.40f },  // climbing: strength and agility
  { 0.00f, 0.30f, 0.30f, 0.40f },  // diving: technique
  { 0.00f, 0.20f, 0.00f, 0.80f },  // speed skating
  { 0.50f, 0.00f, 0.10f, 0.40f },  // torch run: stamina
  { 0.20f, 0.30f, 0.20f, 0.30f },  // trail run: a bit of everything
};

// the capacity for an event (a good athlete is around 100)
static inline float athleteRating(const Athlete &a, uint8_t ev) {
  const DexEntry &d = DEX_TBL[(a.dex >= 1 && a.dex <= DEX_COUNT) ? a.dex : 0];
  const float *w = EV_W[ev < EV_COUNT ? ev : 0];
  float base = w[0] * d.bHp + w[1] * d.bAtk * a.geneAtk / 100.0f + w[2] * d.bDef * a.geneDef / 100.0f +
               w[3] * d.bSpe * a.geneSpe / 100.0f;
  static const float STAGE[4] = { 0.9f, 0.9f, 1.0f, 1.1f };
  // each training counts as much as its stat weighs; all three at 100 is +12 % in any event
  float tr = w[0] * a.trDef + w[1] * a.trAtk + w[2] * a.trDef + w[3] * a.trSpe;
  float training = 1.0f + 0.12f * tr / 100.0f;
  float lv = a.level > 100 ? 100 : a.level;
  float level = 1.0f + 0.002f * lv;                                  // a high level is worth up to +20 %
  return base * STAGE[evoStage(a.dex)] * training * level * (a.mega ? 1.15f : 1.0f) *
         typeBonus(ev, d.type1, d.type2);
}

// the day's performance: the capacity with +-15 % of luck (u in [0,1))
static inline float performance(float rating, float u) { return rating * (0.85f + 0.30f * u); }

// races and the climb: less time wins; jumps, weights and dives: more wins
static inline bool eventLowerWins(uint8_t ev) {
  return ev <= EV_SWIM || ev == EV_CLIMB || ev == EV_SKATE || ev == EV_TORCH || ev == EV_TRAIL;
}
enum : uint8_t { UNIT_SECS = 0, UNIT_METERS, UNIT_KG, UNIT_POINTS, UNIT_MINSEC };
static inline uint8_t eventUnit(uint8_t ev) {
  if (ev == EV_LONGJUMP || ev == EV_HIGHJUMP) return UNIT_METERS;
  if (ev == EV_LIFT) return UNIT_KG;
  if (ev == EV_DIVE) return UNIT_POINTS;
  if (ev == EV_TORCH || ev == EV_TRAIL) return UNIT_MINSEC;
  return UNIT_SECS;
}

// the mark of a performance: seconds (races), metres (jumps), kilos, points
static inline float eventMark(uint8_t ev, float perf) {
  if (perf < 20) perf = 20;
  float m;
  switch (ev) {
    case EV_SPRINT: m = 7.0f + 500.0f / perf; return m > 25 ? 25 : m;
    case EV_HURDLES: m = 9.0f + 600.0f / perf; return m > 30 ? 30 : m;
    case EV_SWIM: m = 13.0f + 700.0f / perf; return m > 45 ? 45 : m;
    case EV_LONGJUMP: m = 2.0f + 4.6f * perf / 100.0f; return m < 2.5f ? 2.5f : (m > 9.0f ? 9.0f : m);
    case EV_LIFT:  // in 5 kg steps
      m = 40.0f + 1.2f * perf;
      m = floorf(m / 5.0f) * 5.0f;
      return m < 30 ? 30 : (m > 300 ? 300 : m);
    case EV_CLIMB: m = 6.0f + 900.0f / perf; return m > 60 ? 60 : m;           // a 15 m wall
    case EV_DIVE: m = 20.0f + 0.6f * perf; return m > 100 ? 100 : m;           // points from the judges
    case EV_SKATE: m = 30.0f + 1200.0f / perf; return m > 90 ? 90 : m;         // 500 m
    case EV_TORCH: m = 480.0f + 30000.0f / perf; return m > 1800 ? 1800 : m;   // 3000 m, in seconds
    case EV_TRAIL: m = 120.0f + 9000.0f / perf; return m > 900 ? 900 : m;      // 1 km with obstacles, in seconds
    default:       // high jump: the bar goes up 5 cm at a time
      m = 0.7f + perf / 100.0f;
      m = floorf(m * 20.0f + 0.001f) / 20.0f;
      return m < 0.8f ? 0.8f : (m > 2.45f ? 2.45f : m);
  }
}

// the player's place (1..3): in races less is better, in jumps more; a tie goes to the player
static inline uint8_t playerPlace(float me, float a, float b, bool lowerWins) {
  uint8_t place = 1;
  if (lowerWins) {
    if (a < me) place++;
    if (b < me) place++;
  } else {
    if (a > me) place++;
    if (b > me) place++;
  }
  return place;
}

// What a place pays (coins), and the joy of a gold. The first gold of a day pays a bonus on top.
#define TOUR_COINS_GOLD 25
#define TOUR_COINS_SILVER 12
#define TOUR_COINS_BRONZE 5
#define TOUR_DAILY_BONUS 20
#define TOUR_JOY_GOLD 10
static inline uint16_t tourCoins(uint8_t place) {
  return place == 1 ? TOUR_COINS_GOLD : place == 2 ? TOUR_COINS_SILVER : TOUR_COINS_BRONZE;
}

// a mark stored as hundredths in a uint16 (the longest is 1800 s = 180000: seconds are kept in tenths)
static inline uint16_t eventMarkStore(uint8_t ev, float mark) {
  float v = eventUnit(ev) == UNIT_MINSEC ? mark * 10.0f : mark * 100.0f;
  if (v < 0) v = 0;
  if (v > 65535.0f) v = 65535.0f;
  return (uint16_t)(v + 0.5f);
}
// is `mark` a new record against the stored one (0 = none yet)?
static inline bool eventRecord(uint8_t ev, uint16_t stored, uint16_t now) {
  if (!stored) return true;
  return eventLowerWins(ev) ? now < stored : now > stored;
}

// Long jump: where the jumper is `lt` ms into its turn (`turnMs` long). It runs up to the board
// (x 60 -> 280), jumps (an arc, landing in the sand pit, which starts at x 300) and stays where it
// landed. A longer mark lands farther in. lift is how high above the ground the arc is.
struct JumpPose { int16_t x; int16_t lift; uint8_t phase; };   // phase 0 waiting/standing, 1 running, 2 in the air
#define LJ_START_X 60
#define LJ_BOARD_X 280
#define LJ_PIT_X0 322
#define LJ_PIT_X1 430
static inline int16_t longJumpLandX(float mark) {
  float u = (mark - 2.5f) / 6.5f;
  if (u < 0) u = 0;
  if (u > 1) u = 1;
  return (int16_t)(LJ_PIT_X0 + u * (LJ_PIT_X1 - LJ_PIT_X0));
}
static inline JumpPose longJumpPose(uint32_t lt, uint32_t turnMs, float mark) {
  JumpPose p = { LJ_START_X, 0, 0 };
  if (lt == 0 || turnMs == 0) return p;
  if (lt >= turnMs) { p.x = longJumpLandX(mark); return p; }
  float f = (float)lt / (float)turnMs;
  const float RUN = 0.45f, AIR = 0.75f;
  if (f < RUN) { p.x = (int16_t)(LJ_START_X + (LJ_BOARD_X - LJ_START_X) * (f / RUN)); p.phase = 1; return p; }
  int16_t land = longJumpLandX(mark);
  if (f < AIR) {
    float u = (f - RUN) / (AIR - RUN);
    p.x = (int16_t)(LJ_BOARD_X + (land - LJ_BOARD_X) * u);
    p.lift = (int16_t)(sinf(u * 3.14159265f) * (50 + 8 * (mark - 2.5f)));
    p.phase = 2;
    return p;
  }
  p.x = land;
  return p;
}
