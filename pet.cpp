#include "pet.h"
#include "steps.h"
#include "avatars.h"
#include "dex.h"
#include "moves.h"
#include "noart.h"   // speciesHasArt(): the egg pool skips what cannot be drawn
#include "audio.h"

// Reads a blob that may be LONGER than the array we are reading it into.
//
// Preferences::getBytes reads the stored length FIRST and, if it exceeds the
// caller's buffer, logs and returns 0 WITHOUT COPYING ANYTHING:
//
//     if (len > maxLen) { log_e("not enough space in buffer"); return 0; }
//
// Growing the dex was always safe -- a short blob lands in the front of a bigger
// array and the rest keeps its zero initialiser, which is what every migration
// here relies on. SHRINKING was not: flashing a build with a smaller DEX_COUNT,
// GYM_REGIONS or REGION_COUNT over a newer save left dexReg, dexShinyReg, the
// badge arrays and eggByRegion entirely ZERO. The creature survived, being all
// scalars, while the Pokedex and every badge past Kanto quietly vanished --
// which from the player's side is their game rolling back.
//
// The PREFIX is the right thing to keep: dex bit n means the same species
// whatever the table grew to afterwards, and region n is the same region.
// Only the tail we have no room for is dropped, which is data about content
// this build does not have anyway.
//
// The emulator's Preferences stub used to truncate rather than refuse, so no
// test could see any of this -- see tools/emu/Preferences.h.
static void loadBlob(Preferences &p, const char *key, void *dst, size_t n) {
  size_t have = p.getBytesLength(key);
  if (!have) return;                       // absent: keep the initialiser
  if (have <= n) { p.getBytes(key, dst, n); return; }
  uint8_t *tmp = (uint8_t *)malloc(have);
  if (!tmp) return;                        // rather no read than a half one
  if (p.getBytes(key, tmp, have) == have) memcpy(dst, tmp, n);
  free(tmp);
}

void Pet::begin() {
  prefs.begin("tamapoke", false);
  opened = true;
  // Zeroed BEFORE the branch below, not inside load(): getBytes() leaves its
  // destination untouched when the key is missing, and the fresh-install path
  // returns without ever calling load(). Without this a begin() after a factory
  // reset keeps the old Pokedex alive in RAM -- the firmware reboots on WIPE so
  // it never showed there, but anything calling begin() twice would see it, and
  // Party::begin() already guards the same way for the same reason.
  memset(badgesX, 0, sizeof(badgesX));
  memset(badgesHardX, 0, sizeof(badgesHardX));
  memset(dexReg, 0, sizeof(dexReg));
  memset(dexShinyReg, 0, sizeof(dexShinyReg));
  for (int i = 0; i < REGION_COUNT; i++) eggByRegion[i] = 0;
  if (!prefs.getBool("init", false)) {
    prefs.putBool("init", true);
    newEgg();
  } else {
    load();
  }
  lastTick = millis();
}

void Pet::newEgg() {
  ceremony = CER_NONE;
  neglectTicks = 0;
  weight = 0;
  speciesId = -1;
  prevSpeciesId = -1;
  for (int i = 0; i < REGION_COUNT; i++) eggByRegion[i] = 0;
  eggTarget = pickEggSpecies();  // hidden species according to rarity and pokedex
  eggByRegion[region % REGION_COUNT] = eggTarget;
  starterPick = (registeredCount() == 0);  // first game: the player picks a starter
  // shiny roll: 1/48 base, better with a farewell and with high streak/bond
  int shinyBase = (lastEnd == CER_FAREWELL ? 24 : 48) - careBonus();
  if (shinyBase < 8) shinyBase = 8;
  eggShiny = (random(shinyBase) == 0);
  // The debt lands on the creature about to hatch, and is spent doing so --
  // it is a one-day penalty, not a running total that compounds each retire.
  evoPen = retirePending ? EVO_PENALTY_LEVELS : 0;
  retirePending = false;
  eggTaps = 0;
  fullness = 80;
  joy = 80;
  energy = 80;
  hygiene = 100;
  poops = 0;
  ageMinutes = 0;
  careMistakes = 0;
  mistakeCooldown = 0;
  sleeping = false;
  frozen = false;
  sickness = SICK_NONE; sickDoses = 0; sickTicks = 0; lastDoseMinute = 0;
  overfeed = 0; dirtyTicks = 0; lightOn = false; lightOnTicks = 0;
  grumpy = false; boredTicks = 0;
  save();
}

// offline progression: time passed even though it was switched off, but with
// mercy -- the bars fall with a floor at 15 (comes back hungry, not dead),
// no mistakes or runaways while away
static uint8_t dropTo(uint8_t v, uint8_t d, uint8_t fl) {
  if (v <= fl) return v;
  return (v - fl > d) ? v - d : fl;
}

void Pet::setClock(uint32_t nowEpoch) {
  lastSeenEpoch = nowEpoch;
  if (nowEpoch) save();  // persists already: a power cut does not lose the reference
}

void Pet::syncClock(uint32_t nowEpoch) {
  uint32_t seen = prefs.getUInt("seen", 0);
  lastSeenEpoch = nowEpoch;
  if (nowEpoch == 0) return;
  uint32_t mins = (seen && nowEpoch > seen) ? (nowEpoch - seen) / 60 : 0;
  if (mins < 2 || ceremony != CER_NONE || starterPick) {
    save();  // first time, no time to apply or still choosing a starter
    return;
  }
  if (mins > 14UL * 24 * 60) mins = 14UL * 24 * 60;  // cap: 2 weeks

  for (uint32_t i = 0; i < mins; i++) {
    ageMinutes++;
    if (isEgg()) {
      if (ageMinutes >= 3) hatch();  // hatches while you are away
      continue;
    }
    if (sleeping && napEnd && ageMinutes >= napEnd) { napEnd = 0; wakeUp(); sleepAuto = SLEEP_NONE; }
    if (sleeping) {  // rest: falls slowly and with a floor, same as live
      energy = clamp100(energy + (lightOn ? 3 : 6));
      if (lightOn && lightOnTicks < 255) lightOnTicks++;
      if (ageMinutes % 2 == 0) {
        fullness = dropTo(fullness, 1, 30);
        joy = dropTo(joy, 1, 35);
      }
      if (ageMinutes % 3 == 0) hygiene = dropTo(hygiene, 1, 45);
      continue;
    }
    fullness = dropTo(fullness, 2, 15);
    energy = dropTo(energy, 1, 15);
    hygiene = dropTo(hygiene, 1, 15);
    joy = dropTo(joy, 1, 15);
    if (boredTicks < 0xFFFF) boredTicks++;  // nobody played with it meanwhile
  }
  if (!isEgg()) {
    if (!sleeping) {  // does not get dirty while sleeping
      uint8_t p = poops + mins / 240;
      poops = p > 3 ? 3 : p;
    }
    // evolution is NOT applied offline: it is left ready and the user triggers it
    // by tapping the creature on return (so they see the transformation)
  }
  Serial.printf("offline: %u min applied (lv.%u)\n", mins, level());
  save();
}

void Pet::update(uint32_t nowMs) {
  // end of ceremony: the creature leaves and a new egg remains
  if (ceremony != CER_NONE && millis() > ceremonyUntil) {
    snapshotForParty();  // hand it over BEFORE newEgg() erases everything
    newEgg();
    return;
  }
  while (nowMs - lastTick >= PET_TICK_MS) {
    lastTick += PET_TICK_MS;
    tick();
  }
}

void Pet::tick() {
  if (ceremony != CER_NONE) return;  // time stops at the farewell
  if (starterPick) return;  // the game does not start until a starter is chosen: if
                            // time ran here, the egg would hatch by itself after
                            // 3 min with the rolled species and the player's
                            // choice would be lost
  if (!frozen) ageMinutes++;   // a revived companion does not age

  if (isEgg()) {
    if (ageMinutes >= 3) hatch();  // if you do not touch it, it hatches by itself after 3 min
    return;
  }

  applyAutoSleep();   // put down at 21:00 still goes to bed at 22:00


  // sleep is rest: energy recovers and needs fall MUCH
  // slower than awake and with a floor (wakes up asking for some cuddling, not at
  // zero, with no mistakes or runaways). awake: food -2/min, hyg/joy -1/min.
  // Weight is still burned and rest counts toward DEF (see defTick).
  if (sleeping) {
    energy = clamp100(energy + (lightOn ? 3 : (hasDecorOut(DEC_BED) ? 7 : 6)));   // light on: badly; a bed: better
    if (napEnd && ageMinutes >= napEnd) {   // the nap is over: up it gets, rested
      napEnd = 0;
      wakeUp();
      sleepAuto = SLEEP_NONE;
      pendingSave = true;
      return;
    }
    if (lightOn && lightOnTicks < 255) lightOnTicks++;
    if (weight > 0 && ageMinutes % 3 == 0) weight--;
    if (ageMinutes % 2 == 0) {                 // ~4x slower than awake
      fullness = dropTo(fullness, 1, 30);
      joy = dropTo(joy, 1, 35);
    }
    if (ageMinutes % 3 == 0) hygiene = dropTo(hygiene, 1, 45);
    defTick(true);  // resting is also wellbeing: it counts toward DEF
    checkMedals();  // can still cross a level by age while asleep
    if (++ticksSinceSave >= 5) pendingSave = true;
    return;
  }

  if (ageMinutes % MINUTES_PER_LEVEL == 0) sfxPlay(SFX_LEVEL);  // levelled up (awake)

  fullness = clamp100(fullness - 2);
  energy = clamp100(energy - 1);
  if (fullness > 40 && poops < 3 && random(100) < 15) poops++;

  hygiene = clamp100(hygiene - 1 - 4 * poops);
  // overweight makes it lazy: energy falls twice as fast
  if (weight > 50) energy = clamp100(energy - 1);
  if (weight > 0 && ageMinutes % 3 == 0) weight--;

  defTick(false);  // calm forges defence

  int dJoy = -1;
  if (fullness < 30) dJoy -= 2;
  if (hygiene < 30) dJoy -= 2;
  joy = clamp100(joy + dJoy);

  statusTick();  // Fun Pack: sickness, boredom, dozing off with the light on

  // Mistake: leaving a stat in the gutter counts as a care
  // error (with a cooldown so the same mistake is not counted every minute)
  if (mistakeCooldown > 0) mistakeCooldown--;
  if (lowestStat() <= 10 && mistakeCooldown == 0) {
    careMistakes++;
    mistakeCooldown = 60;
    if (bond > 1) bond--;  // the mistake cools the bond, but without wrecking it:
                           // at -3 every 30 min far more was lost than could be
                           // gained in a whole day and the bond got stuck
  }

  checkMedals();  // the user triggers evolution (canEvolveNow + tap), not the tick
  checkLearnGates();

  // total neglect: with EVERYTHING at zero for an hour it becomes ready to run away;
  // it does NOT leave by itself, the user triggers it with the button (sad ending, witnessed)
  if (inTotalNeglect()) {
    if (neglectTicks < RUNAWAY_TICKS) neglectTicks++;
  } else {
    neglectTicks = 0;  // a single care saves it
  }

  // full cycle (final form + 7 days): the farewell does NOT fire by itself; it becomes
  // ready (canFarewellNow) and the user triggers it with the button, so they see it

  // periodic autosave: do NOT write to flash here (runs inside the loop,
  // while animating); only mark it and let the loop flush it when dimming
  if (++ticksSinceSave >= 5) pendingSave = true;
}

// Copies the creature into endedMon so it can be offered a party slot, since
// newEgg() is about to wipe every field. Only the two endings the player CHOSE
// qualify: a runaway ran off after an hour of total neglect, and letting it
// come back on the team would remove the cost from the one ending that has any.
// Brings a banked creature back as the live pet, frozen.
void Pet::reviveFrom(const PartyMon &m) {
  if (m.empty()) return;
  ceremony = CER_NONE;
  neglectTicks = 0;
  speciesId = m.dex;
  prevSpeciesId = -1;
  eggTaps = 0;
  starterPick = false;
  shiny = m.shiny != 0;
  ivAtk = m.ivAtk; ivDef = m.ivDef; ivSpe = m.ivSpe; ivHp = m.ivHp;
  trAtk = m.trAtk; trDef = m.trDef; trSpe = m.trSpe;
  for (int i = 0; i < MOVE_SLOTS; i++) moves[i] = m.moves[i];
  // its banked level expressed as an age, so level() needs no special case
  ageMinutes = (uint32_t)(m.level ? m.level - 1 : 0) * MINUTES_PER_LEVEL;
  lastLearnLevel = level();     // do not replay every gate it already passed
  learnQCount = 0;
  medals = m.medals;
  careMistakes = 0;
  mistakeCooldown = 0;
  sleeping = false;
  bond = 0;
  bondToday = 0;
  berryKnown = false;
  weight = 0;
  fullness = joy = energy = 80;
  hygiene = 100;
  poops = 0;
  frozen = true;
  strncpy(nick, m.nick, sizeof(nick) - 1);
  nick[sizeof(nick) - 1] = 0;
  registerSpecies(speciesId);
  save();
}

void Pet::snapshotForParty() {
  endedKind = CER_NONE;
  if (isEgg()) return;
  if (ceremony != CER_FAREWELL && ceremony != CER_RELEASE) return;
  // An EARLY retire gives the creature up for good -- it is not banked at all.
  // Retiring one that has EARNED its farewell still banks it, because that is
  // simply the farewell reached by another button. retirePending is still set
  // here: update() snapshots before newEgg() spends it, and that ordering is
  // what this depends on, so retire_test drives the real update() rather than
  // calling the two halves by hand.
  if (retireIsEarly()) return;
  endedMon = PartyMon();
  endedMon.dex = speciesId;
  endedMon.level = level();
  endedMon.medals = medals;
  endedMon.ivAtk = ivAtk;
  endedMon.ivDef = ivDef;
  endedMon.ivSpe = ivSpe;
  endedMon.ivHp = ivHp;
  endedMon.trAtk = trAtk;
  endedMon.trDef = trDef;
  endedMon.trSpe = trSpe;
  endedMon.shiny = shiny ? 1 : 0;
  for (int i = 0; i < MOVE_SLOTS; i++) endedMon.moves[i] = moves[i];  // frozen too
  strncpy(endedMon.nick, nick, sizeof(endedMon.nick) - 1);
  endedMon.nick[sizeof(endedMon.nick) - 1] = 0;
  endedKind = ceremony;
}

// flushes the pending periodic save (the loop calls it at a moment with no
// animation so the stall of the flash write is not visible)
void Pet::saveNow() { save(); }

void Pet::flushSave() {
  if (pendingSave) save();
}

// Calm forges defence: every hour of wellbeing (resting, or awake
// with everything >= 40) gives +1 DEF, up to the ceiling the IV allows.
//
// It used to ask for 12 h IN A ROW and ANY slip reset the counter to zero, besides
// not counting sleep. Simulating a whole life (3 days) that gave 1 point to the
// theoretically perfect player (one acting every minute for 72 h) and 0 to
// everyone else, including one who tends it every 15 min: food falls 2/min,
// so whoever does not check on the creature every half hour is ALWAYS below
// 40 and the counter never started. DEF was, in practice, untrainable.
// Now it accumulates instead of resetting: a mistake costs the bad minutes, not
// all the progress.
void Pet::defTick(bool resting) {
  if (!resting && lowestStat() < 40) return;
  if (++goodTicks < DEF_TRAIN_TICKS) return;
  goodTicks = 0;
  if (trDef < trMaxDef()) trDef++;
}

// are there unregistered members left in this base's evolutionary line?
bool Pet::lineHasUnregistered(int16_t base) const {
  int16_t cur = base;
  for (int guard = 0; cur >= 1 && cur <= DEX_COUNT && guard < 6; guard++) {
    if (!isRegistered(cur)) return true;
    if (cur == DEX_EEVEE) {
      int16_t opts[EEVEE_EVO_COUNT];
      uint8_t n = eeveeOptions(opts);
      for (uint8_t i = 0; i < n; i++)
        if (!isRegistered(opts[i])) return true;
      return false;
    }
    cur = DEX_TBL[cur].evolvesTo;
  }
  return false;
}

uint8_t Pet::eeveeOptions(int16_t *out) const {
  uint8_t n = 0;
  for (uint8_t i = 0; i < EEVEE_EVO_COUNT; i++) {
    int16_t b = EEVEE_EVOS[i];
    if (b < 1 || b > DEX_COUNT) continue;
    if (!speciesHasArt(b)) continue;                     // no art anywhere
    if (!regionAvailable(regionOfDex(b))) continue;      // pack not on the card
    out[n++] = b;
  }
  return n;
}

uint8_t Pet::eggRarity() const {
  return (eggTarget >= 1 && eggTarget <= DEX_COUNT) ? DEX_TBL[eggTarget].rarity : R_COMUN;
}

// picks the egg's species: rarity roll (improved by a full
// farewell, penalised by a runaway) and a bias toward incomplete lines
// Room for the candidate list. A whole rarity tier of a 386-species dex is far
// more than the 80 the Kanto-only build needed.
#define CAND_MAX 260

uint16_t gRegionArt = 0xFFFF;   // everything, until the SD narrows it

bool regionAvailable(uint8_t r) {
  if (r >= REGION_COUNT) return false;
  if (r == REGION_ALL) {                       // the mixed pool: any pack will do
    for (uint8_t i = 0; i < REGION_COUNT; i++)
      if (i != REGION_ALL && (gRegionArt & (uint16_t)(1u << i))) return true;
    return false;
  }
  return (gRegionArt & (uint16_t)(1u << r)) != 0;
}

uint8_t regionOfDex(int16_t d) {
  for (uint8_t i = 0; i < REGION_COUNT; i++) {
    if (i == REGION_ALL) continue;
    if (d >= REGIONS[i].lo && d <= REGIONS[i].hi) return i;
  }
  return REGION_ALL;
}

uint8_t nextAvailableRegion(uint8_t from) {
  for (uint8_t i = 1; i <= REGION_COUNT; i++) {
    uint8_t r = (uint8_t)((from + i) % REGION_COUNT);
    if (regionAvailable(r)) return r;
  }
  return from;                      // nothing available anywhere: stay put
}

// The region to actually hatch from. Normally the player's own, but a card can
// be swapped under a save: rather than rewrite their choice (which would lose
// it silently the moment they put the right card back), the CHOICE is kept and
// only the roll falls through to somewhere playable.
static uint8_t eggRegionFallback(uint8_t want) {
  if (regionAvailable(want)) return want;
  for (uint8_t i = 0; i < REGION_COUNT; i++)
    if (i != REGION_ALL && regionAvailable(i)) return i;
  return want;                      // no art anywhere: behave as we always did
}

int16_t Pet::pickEggSpecies() {
  const uint8_t use = eggRegionFallback(region % REGION_COUNT);
  const RegionInfo &rg = REGIONS[use];
  // first game: classic starter -- from the chosen region, so a Johto run
  // starts with a Johto starter rather than a Kanto one
  if (registeredCount() == 0) {
    return rg.starters[random(rg.starterCount)];
  }

  uint8_t tier = R_COMUN;
  if (lastEnd != CER_RUNAWAY) {
    bool blessed = (lastEnd == CER_FAREWELL);
    int rare = (blessed ? 45 : 27) + careBonus();
    int leg = (registeredCount() >= 25) ? (blessed ? 10 : 3) + careBonus() / 3 : 0;
    int r = random(100);
    if (r < leg) tier = R_LEGENDARIO;
    else if (r < leg + rare) tier = R_RARO;
  }

  // candidates of the tier with an incomplete line; if none, drop a tier;
  // if the tier's pokedex is complete, any of the tier will do
  for (int pass = 0; pass < 2; pass++) {
    for (int t = tier; t >= R_COMUN; t--) {
      int16_t cand[CAND_MAX];
      int n = 0;
      for (int16_t d = rg.lo; d <= rg.hi && n < CAND_MAX; d++) {
        if (DEX_TBL[d].rarity != t) continue;
        if (pass == 0 && !lineHasUnregistered(d)) continue;
        // ALL spans every region, so filter per species: a missing Sinnoh pack
        // must not put a Sinnoh creature in a mixed egg.
        if (!regionAvailable(regionOfDex(d))) continue;
        // And a species with no art ANYWHERE never hatches, pack or no pack --
        // it would be a creature that can only ever draw as a number. It keeps
        // its dex slot; it is simply not something an egg can contain.
        if (!speciesHasArt(d)) continue;
        cand[n++] = d;
      }
      if (n > 0) return cand[random(n)];
    }
  }
  return rg.starters[random(rg.starterCount)];  // unreachable, just in case
}

// Rolls a species of a GIVEN tier inside a region. Used when the player changes
// region while an egg is waiting: the rarity they were granted is kept and only
// the region changes, so switching cannot be farmed for a legendary.
int16_t Pet::rollInRegion(uint8_t r, uint8_t tier) {
  const RegionInfo &rg = REGIONS[eggRegionFallback(r % REGION_COUNT)];
  for (int t = tier; t >= R_COMUN; t--) {
    int16_t cand[CAND_MAX];
    int n = 0;
    for (int16_t d = rg.lo; d <= rg.hi && n < CAND_MAX; d++)
      if (DEX_TBL[d].rarity == t && regionAvailable(regionOfDex(d)) &&
          speciesHasArt(d)) cand[n++] = d;
    if (n) return cand[random(n)];
  }
  return rg.starters[0];      // a region with nothing in it cannot happen
}

// Changing region swaps the WAITING egg to that region's creature.
//
// Without this the setting would look broken: you would pick Johto and still
// hatch a Rattata, because the species is rolled when the egg appears and not
// when it cracks. Two rules stop it becoming a re-roll button:
//
//   1. The rarity tier is kept. Only which species of that tier changes, so
//      toggling can never be farmed for a legendary.
//   2. Each region's answer is REMEMBERED for this egg. Switching back shows
//      the same creature again, so there is nothing to gain by flipping.
//
// A hatched creature is untouched -- this only ever moves an egg.
void Pet::setRegion(uint8_t r) {
  r %= REGION_COUNT;
  // The sprite pack is a real gate, not a hint: without it the region is not
  // selectable at all. The chooser still SHOWS it, greyed and with a reason --
  // hiding it outright is how Johto and Hoenn once came to look absent when
  // they were built and reachable all along.
  if (!regionAvailable(r)) return;
  if (r == region) return;
  uint8_t old = region;
  region = r;
  if (isEgg() && eggTarget >= 1) {
    if (old < REGION_COUNT) eggByRegion[old] = eggTarget;
    int16_t known = eggByRegion[r];
    eggTarget = known >= 1 ? known : rollInRegion(r, eggRarity());
    eggByRegion[r] = eggTarget;
  }
  save();
}

void Pet::registerSpecies(int16_t dex) {
  if (dex < 1 || dex > DEX_COUNT) return;
  dexReg[(dex - 1) >> 3] |= (1 << ((dex - 1) & 7));
  if (shiny) dexShinyReg[(dex - 1) >> 3] |= (1 << ((dex - 1) & 7));
}

// streak and bond improve the egg roll (0..~14)
int Pet::careBonus() const {
  int s = streak > 30 ? 30 : streak;
  return s / 3 + bond / 25;
}

// Every caller asks the day here, so a step is never added to yesterday's total.
// Saving is throttled: the flush runs only while the screen is dim or off, so
// marking it pending often is free, but it should not be every footfall.
void Pet::addSteps(uint32_t n) {
  if (!n) return;
  uint32_t d = today();
  if (d != stepDay) {
    stepDay = d;
    stepCount = 0;
    pendingSave = true;
  }
  stepCount += n;
  if (stepCount > STEP_MAX) stepCount = STEP_MAX;
  if (advSteps < 60000) advSteps += n;          // the adventure route moves too
  stepsUnsaved += n;
  if (stepsUnsaved >= 20) {
    stepsUnsaved = 0;
    pendingSave = true;
  }
  rollCoinDay();
  uint32_t k = stepCount / 1000;
  if (k > COIN_STEP_CAP_K) k = COIN_STEP_CAP_K;
  if (k > stepCoinK) {
    earnCoins((uint16_t)((k - stepCoinK) * COIN_PER_1000_STEPS), false);
    stepCoinK = (uint8_t)k;
    pendingSave = true;
  }
  noteMission(MIS_STEPS, 0);
}

void Pet::setStepsToday(uint32_t n) {
  stepDay = today();
  stepCount = n > STEP_MAX ? STEP_MAX : n;
  stepsUnsaved = 0;
}

// first care of the day: advances the streak and strengthens the bond
void Pet::registerCare() {
  if (isEgg() || ceremony != CER_NONE) return;
  uint32_t d = today();
  if (d == 0 || d == lastCareDay) return;  // no clock, or already counted today
  if (lastCareDay == 0 || d == lastCareDay + 1) {
    streak++;
  } else {
    streak = 1;        // there was a gap of days
    lastMilestone = 0;
  }
  lastCareDay = d;
  bondToday = 0;
  earnCoins(COIN_DAILY_BONUS, false);
  if (streak % 7 == 0) earnCoins(COIN_STREAK7_BONUS, false);
  if (streak > bestStreak) bestStreak = streak;
  bond = clamp100(bond + 4);
  uint16_t ms = (streak >= 100) ? 100 : (streak >= 30) ? 30
              : (streak >= 7)   ? 7   : (streak >= 3)  ? 3 : 0;
  if (ms > lastMilestone) {
    lastMilestone = ms;
    milestoneUntil = millis() + 4500;
  }
  checkMedals();
  save();
}

void Pet::addBond(uint8_t amt) {
  if (bondToday >= 20) return;  // daily cap: bond cannot be farmed
  bond = clamp100(bond + amt);
  bondToday += amt;
}

void Pet::checkMedals() {
  if (isEgg()) return;
  uint16_t before = medals;
  if (level() >= 10) medals |= MED_LV10;
  if (level() >= 25) medals |= MED_LV25;
  if (level() >= 50) medals |= MED_LV50;
  if (berryKnown) medals |= MED_BERRY;
  if (streak >= 7) medals |= MED_STREAK7;
  if (bond >= 100) medals |= MED_BOND;
  if (DEX_TBL[speciesId].evolvesTo == 0) medals |= MED_FINAL;
  if (weight == 0 && level() >= 5 && careMistakes == 0) medals |= MED_FIT;
  uint16_t gained = medals & ~before;
  if (gained) {
    for (uint16_t m = gained; m; m &= (m - 1)) totalMedals++;
    newMedal = gained;
    medalUntil = millis() + 4000;
    if (!sleeping) sfxPlay(SFX_MEDAL);
    save();
  }
}

void Pet::rename(const char *name) {
  strncpy(nick, name, sizeof(nick) - 1);
  nick[sizeof(nick) - 1] = 0;
  save();
}

// The IV contribution (IV x level / 100) is exactly that of the 3rd generation
// games onward: a perfect IV is worth +31 at level 100. The rest of
// the formula is TamaPoke's (flat base + level) and not the games': with
// the canonical x level/100 on the base, a newborn creature would show
// single-digit stats, which on a pet screen looks like an error.
static uint16_t calcStat(uint8_t base, uint8_t iv, uint8_t lvl, uint8_t tr) {
  return (uint16_t)base + lvl + (uint16_t)iv * lvl / 100 + tr;
}

uint16_t Pet::atkStat() const {
  return isEgg() ? 0 : calcStat(DEX_TBL[speciesId].bAtk, ivAtk, level(), trAtk);
}
uint16_t Pet::defStat() const {
  return isEgg() ? 0 : calcStat(DEX_TBL[speciesId].bDef, ivDef, level(), trDef);
}
uint16_t Pet::speStat() const {
  return isEgg() ? 0 : calcStat(DEX_TBL[speciesId].bSpe, ivSpe, level(), trSpe);
}
// vitality is not trained (nothing raises it), so it carries a fixed +10
// instead of training, like the +Level+10 of HP in the games
uint16_t Pet::vitStat() const {
  return isEgg() ? 0 : calcStat(DEX_TBL[speciesId].bHp, ivHp, level(), 10);
}
// Special reuses the physical IV and training against the species' special base
// stat, which is what keeps Alakazam (50 Atk / 135 SpA) a real attacker without
// adding IVs or migrating saves.
uint16_t Pet::spaStat() const {
  return isEgg() ? 0 : calcStat(DEX_TBL[speciesId].bSpA, ivAtk, level(), trAtk);
}
uint16_t Pet::spdStat() const {
  return isEgg() ? 0 : calcStat(DEX_TBL[speciesId].bSpD, ivDef, level(), trDef);
}

// ---------- moves ----------

uint8_t Pet::moveCount() const {
  uint8_t n = 0;
  for (int i = 0; i < MOVE_SLOTS; i++)
    if (moves[i]) n++;
  return n;
}

bool Pet::knowsMove(uint8_t mv) const {
  if (!mv) return false;
  for (int i = 0; i < MOVE_SLOTS; i++)
    if (moves[i] == mv) return true;
  return false;
}

// Picks a sensible default set, best first.
//
// NOT "the newest four": 1907 of the 2281 learnset entries sit at level 0 and
// another 237 at level 1, so for most species level orders nothing and taking
// the last four in table order is arbitrary -- it handed a level 100 Charizard
// GROWL and LEER. So score instead: attacks over status, stronger over weaker,
// STAB ahead of equal power, and a bonus for the handful of moves that really
// are gated behind a level, since those are meant to be upgrades.
// TMs unlock at one level, for everything.
//
// This replaced a power/2 curve, which was impossible for a player to predict
// (SURF at 45, ROCK SLIDE at 37) and dribbled unlocks out one at a time so none
// of them felt like anything. A single number is explainable in one sentence and
// lands on a seam the game already has: the first five leaders sit at 14-43, so
// you fight the early ladder on what your species actually learns, and TMs
// arrive as you enter the back half. A creature retires at 73 and caps at 100.
//
// It only works because dex_moves.py now carries the cheap early attacks --
// SCRATCH, PECK, POISON STING, BUBBLE and the rest. Without those, gating TMs
// this hard would leave young creatures with nothing at all, which is exactly
// what the power/2 version was papering over.
#define TM_LEVEL 40

static uint8_t tmLevelFor(const MoveEntry &m) {
  (void)m;
  return TM_LEVEL;
}

// THE single answer, used by relearnFromLevel(), by the STAB fallback and by
// the move picker in the sketch. Three call sites once had three opinions.
uint8_t moveUnlockLevel(int16_t dex, uint8_t idx) {
  uint8_t at = learnLevel(dex, idx);
  if (at > 0) return at;                 // a real level-up move
  uint8_t mv = learnMove(dex, idx);
  if (!mv || mv >= MOVE_COUNT) return 255;
  return tmLevelFor(MOVE_TBL[mv]);       // a TM: no natural level, so the gate
}

void Pet::relearnFromLevel() {
  for (int i = 0; i < MOVE_SLOTS; i++) moves[i] = 0;
  if (isEgg()) return;
  const DexEntry &d = DEX_TBL[speciesId];
  uint8_t lvl = level(), n = learnCount(speciesId);
  int16_t score[MOVE_SLOTS] = { 0, 0, 0, 0 };
  // Two passes. Level-up moves (level >= 1) are what a creature grows into, so
  // they fill the set first; TMs (level 0, no gate) only top up the slots left
  // over. Without this a just-hatched pet opens with FIRE BLAST and SOLAR BEAM,
  // because every TM is legal at level 1.
  for (int pass = 0; pass < 2; pass++) {
  bool tmPass = (pass == 1);
  for (uint8_t i = 0; i < n; i++) {
    uint8_t at = learnLevel(speciesId, i);
    if (at > lvl) continue;
    if (tmPass != (at == 0)) continue;
    uint8_t mv = learnMove(speciesId, i);
    if (!mv || mv >= MOVE_COUNT || knowsMove(mv)) continue;
    const MoveEntry &m = MOVE_TBL[mv];
    // A TM carries no level requirement in the data, which is true of the games
    // but wrong here: with only one or two level-up moves early on, the spare
    // slots were filled with the strongest TMs in the table and a NEWBORN opened
    // with SURF, BLIZZARD and OUTRAGE. That, not the damage formula, is why a
    // level 1 Squirtle could beat Brock.
    //
    // So a TM is gated by its own power: roughly power/2, which puts the 40s
    // around level 20 and the 110s out past 50 where a creature is genuinely
    // built. Level-up moves are untouched -- they already have real gates.
    if (tmPass && lvl < tmLevelFor(m)) continue;
    int16_t sc = (m.cat == MC_STATUS) ? 10 : (int16_t)m.power + 20;
    // STAB outweighs raw power, or every species defaults to the same two
    // generic sledgehammers and the roster loses its identity.
    if (m.cat != MC_STATUS && (m.type == d.type1 || m.type == d.type2)) sc += 40;
    if (m.effect == EF_RECHARGE) sc -= 35;   // a free turn for the opponent
    if (m.effect == EF_RECOIL) sc -= 20;
    sc += at;
    if (sc < 1) sc = 1;
    int slot = -1;
    for (int s = 0; s < MOVE_SLOTS; s++)
      if (sc > score[s]) { slot = s; break; }
    if (slot < 0) continue;
    for (int s = MOVE_SLOTS - 1; s > slot; s--) {
      score[s] = score[s - 1];
      moves[s] = moves[s - 1];
    }
    score[slot] = sc;
    moves[slot] = mv;
  }
  if (moveCount() >= MOVE_SLOTS) break;   // level-up moves already filled it
  }
  // Guarantee one same-type move. Machamp's only Fighting options are weak or
  // recoil-laden, so pure scoring left it with four generic attacks and nothing
  // that reads as a Machamp. If the set came out with no STAB, the weakest slot
  // gives way to the best same-type attack the species can actually learn.
  for (int i = 0; i < MOVE_SLOTS; i++) {
    if (!moves[i]) continue;
    const MoveEntry &m = MOVE_TBL[moves[i]];
    if (m.cat != MC_STATUS && (m.type == d.type1 || m.type == d.type2)) return;
  }
  uint8_t best = 0;
  int16_t bestSc = 0;
  for (uint8_t i = 0; i < n; i++) {
    uint8_t at = learnLevel(speciesId, i);
    if (at > lvl) continue;
    uint8_t mv = learnMove(speciesId, i);
    if (!mv || mv >= MOVE_COUNT) continue;
    const MoveEntry &m = MOVE_TBL[mv];
    if (m.cat == MC_STATUS || (m.type != d.type1 && m.type != d.type2)) continue;
    // The same TM gate as above. This fallback used to ignore it, which is how
    // a level 1 Squirtle ended up holding SURF: it had no Water move, so the
    // guarantee reached past every check and handed it the best one in the
    // table. A creature with no STAB it can legally use simply has none yet.
    if (at == 0 && lvl < tmLevelFor(m)) continue;
    int16_t sc = (int16_t)m.power;
    if (m.effect == EF_RECHARGE) sc -= 35;
    if (m.effect == EF_RECOIL) sc -= 20;
    if (sc > bestSc) { bestSc = sc; best = mv; }
  }
  if (best) moves[MOVE_SLOTS - 1] = best;
}

// Queues every level-up move unlocked since the last check. A free slot is
// filled silently -- the games do not ask when there is room either -- and only
// a full moveset produces an offer the player has to answer.
void Pet::checkLearnGates() {
  if (isEgg() || ceremony != CER_NONE) return;
  uint8_t lvl = level();
  if (lvl <= lastLearnLevel) return;
  uint8_t n = learnCount(speciesId);
  for (uint8_t i = 0; i < n; i++) {
    uint8_t at = learnLevel(speciesId, i);
    if (at == 0 || at <= lastLearnLevel || at > lvl) continue;  // 0 = TM, no gate
    uint8_t mv = learnMove(speciesId, i);
    if (!mv || mv >= MOVE_COUNT || knowsMove(mv)) continue;
    int freeSlot = -1;
    for (int s = 0; s < MOVE_SLOTS; s++)
      if (!moves[s]) { freeSlot = s; break; }
    if (freeSlot >= 0) { moves[freeSlot] = mv; continue; }
    if (learnQCount >= sizeof(learnQueue)) continue;
    bool dup = false;
    for (uint8_t q = 0; q < learnQCount; q++)
      if (learnQueue[q] == mv) dup = true;
    if (!dup) learnQueue[learnQCount++] = mv;
  }
  lastLearnLevel = lvl;
  pendingSave = true;
}

static void popLearn(uint8_t *q, uint8_t &n) {
  if (!n) return;
  for (uint8_t i = 0; i + 1 < n; i++) q[i] = q[i + 1];
  q[--n] = 0;
}

void Pet::acceptLearn(uint8_t slot) {
  if (!learnQCount || slot >= MOVE_SLOTS) return;
  moves[slot] = learnQueue[0];
  popLearn(learnQueue, learnQCount);
  save();
}

void Pet::declineLearn() {
  popLearn(learnQueue, learnQCount);
  save();
}

uint8_t Pet::pendingLearnables(uint8_t *out, uint8_t max) const {
  if (isEgg() || !out || !max) return 0;
  uint8_t lvl = level(), n = learnCount(speciesId), w = 0;
  for (uint8_t i = 0; i < n && w < max; i++) {
    if (learnLevel(speciesId, i) > lvl) break;
    uint8_t mv = learnMove(speciesId, i);
    if (knowsMove(mv)) continue;
    bool dup = false;                     // do not offer the same move twice
    for (uint8_t j = 0; j < w; j++)
      if (out[j] == mv) { dup = true; break; }
    if (!dup) out[w++] = mv;
  }
  return w;
}

// IV roll: 8-31. The floor at 8 is deliberate -- in the games a 0 is
// possible because you can breed hundreds of eggs, here each raising lasts 3 days
// and a throwaway individual would be a disproportionate punishment. The streak and
// bond of the PREVIOUS creature push the roll: caring well improves the litter.
uint8_t Pet::rollIV(int bonus) const {
  int v = 8 + (int)random(24) + bonus / 2;  // bonus 0..14 -> +0..7
  return (uint8_t)(v > 31 ? 31 : v);
}

// Saves with the old genes system (90-110%): converted to the range of the
// IV rolled today (8-31) so nobody loses out with the
// update. gene 0 = previous pet, genes included.
uint8_t Pet::ivFromGene(uint8_t gene) const {
  if (gene == 0) return rollIV(0);
  if (gene < 90) gene = 90;
  if (gene > 110) gene = 110;
  return 8 + (uint8_t)(((uint16_t)(gene - 90) * 23) / 20);
}

void Pet::rollIVs() {
  int bonus = careBonus();
  ivAtk = rollIV(bonus);
  ivDef = rollIV(bonus);
  ivSpe = rollIV(bonus);
  ivHp = rollIV(bonus);
  // legendaries are born with 3 of 4 perfect IVs, as in the games
  if (speciesId >= 1 && speciesId <= DEX_COUNT && DEX_TBL[speciesId].rarity == R_LEGENDARIO) {
    uint8_t *p[4] = { &ivAtk, &ivDef, &ivSpe, &ivHp };
    for (int k = 3; k > 0; k--) {  // shuffle to choose which 3
      int j = random(k + 1);
      uint8_t *t = p[k]; p[k] = p[j]; p[j] = t;
    }
    for (int k = 0; k < 3; k++) *p[k] = 31;
  }
  // in the 2nd generation shiny WAS a specific DV pattern: a shiny was never
  // mediocre. Here it translates to a floor of 20 on every IV.
  if (shiny) {
    if (ivAtk < 20) ivAtk = 20;
    if (ivDef < 20) ivDef = 20;
    if (ivSpe < 20) ivSpe = 20;
    if (ivHp < 20) ivHp = 20;
  }
}

uint16_t Pet::registeredCount() const {
  uint16_t n = 0;
  for (int i = 1; i <= DEX_COUNT; i++)
    if (isRegistered(i)) n++;
  return n;
}

// final form that has completed its cycle (7 days): ready to say goodbye. The
// farewell is triggered by the user with the button (it does not fire by itself, so they see it)
bool Pet::canFarewellNow() const {
  if (frozen) return false;     // a companion cannot be lost; that is the point
  return !isEgg() && !sleeping && ceremony == CER_NONE &&
         DEX_TBL[speciesId].evolvesTo == 0 && ageMinutes >= FAREWELL_AGE_MIN;
}

// total neglect for 1h: ready to run away. The user triggers it with the
// button (sad ending); caring for it for a single tick saves it (neglectTicks resets)
bool Pet::canRunawayNow() const {
  if (frozen) return false;
  // inTotalNeglect() as well as the counter, and NOT just the counter. The
  // sleeping branch of tick() returns before the neglect block, so neglectTicks
  // is frozen rather than cleared for the whole night: a creature that went to
  // bed at zero woke with energy back at 100 and was still one tap from
  // leaving, until the next tick 60 s later cleared it. That tap is a caress --
  // the button is drawn over the creature -- so the window really was reachable
  // and it cost somebody a DRAGONAIR.
  return !isEgg() && !sleeping && ceremony == CER_NONE &&
         neglectTicks >= RUNAWAY_TICKS && inTotalNeglect();
}

bool Pet::canRetireNow() const {
  if (frozen) return false;     // a companion is never given up
  return !isEgg() && !sleeping && ceremony == CER_NONE && !starterPick;
}

// The ceremony is the same one; only the debt differs. Marked BEFORE the
// ceremony starts and spent by newEgg(), so a reset mid-ceremony loses the
// penalty rather than applying it to a creature that never got retired.
void Pet::startRetire() {
  if (!canRetireNow()) return;
  retirePending = !canFarewellNow();
  save();
  startFarewell();
  // An early retire is NOT the good ending and must not pay like one.
  // startFarewell() sets lastEnd = CER_FAREWELL, which blesses the next egg --
  // rare 27% -> 45%, legendary 3% -> 10%, shiny 1/48 -> 1/24. Combined with the
  // creature no longer being banked, that made retiring early a pure SHINY FARM:
  // retire, check the egg, retire again, with nothing accumulating to regret.
  // Neutral instead, exactly like a release. The ceremony on screen is still the
  // farewell -- this was a choice the player made, not a neglected creature
  // walking out -- but the reward is not.
  if (retirePending) {
    lastEnd = CER_RELEASE;
    save();
  }
}

void Pet::startFarewell() {
  if (isEgg() || ceremony != CER_NONE) return;
  lastEnd = CER_FAREWELL;
  ceremony = CER_FAREWELL;
  ceremonyUntil = millis() + CEREMONY_MS;
  heartUntil = ceremonyUntil;  // hearts throughout the farewell
  sfxPlay(SFX_BYE);
  save();
}

void Pet::startRunaway() {
  if (isEgg() || ceremony != CER_NONE) return;
  lastEnd = CER_RUNAWAY;
  ceremony = CER_RUNAWAY;
  ceremonyUntil = millis() + CEREMONY_MS;
  sfxPlay(SFX_BYE);
  save();
}

void Pet::release() {
  if (isEgg() || ceremony != CER_NONE) return;
  lastEnd = CER_RELEASE;
  ceremony = CER_RELEASE;
  ceremonyUntil = millis() + CEREMONY_MS;
  heartUntil = ceremonyUntil;
  sfxPlay(SFX_BYE);
  save();
}

void Pet::hatch() {
  speciesId = eggTarget;
  shiny = eggShiny;
  // the individual's IVs (each raising is unique). They are rolled BEFORE resetting the
  // bond on purpose: the careBonus that pushes them is the previous creature's.
  rollIVs();
  trAtk = trDef = trSpe = 0;
  goodTicks = 0;
  berryKnown = false;
  bond = 0;          // bond, medals and name belong to the individual
  bondToday = 0;
  medals = 0;
  newMedal = 0;
  nick[0] = 0;
  registerSpecies(speciesId);  // raised = registered in the pokedex
  // Start empty: checkLearnGates() fills the level-1 moves. Seeding from TMs
  // instead would hand a newborn FIRE BLAST, which no level 1 creature knows.
  for (int i = 0; i < MOVE_SLOTS; i++) moves[i] = 0;
  learnQCount = 0;
  lastLearnLevel = 0;
  checkLearnGates();
  checkMedals();     // in case it is born already in final form (legendary)
  sfxPlay(SFX_HATCH);
  save();
}

// are the conditions for evolving met? Each mistake delays
// evolution by 1 level, and it must also be well cared for at that moment
// (no stat below 40). It does NOT evolve by itself: the user triggers it by
// tapping the creature (evolve()), so they see the transformation.
bool Pet::canEvolveNow() const {
  if (frozen) return false;     // frozen at the form it was banked in
  if (isEgg() || sleeping || ceremony != CER_NONE) return false;
  const DexEntry &d = DEX_TBL[speciesId];
  if (d.evolvesTo == 0) return false;
  // evoPen is the day owed for retiring the PREVIOUS creature early. It rides
  // on the same threshold careMistakes already moves, so there is one rule for
  // "this creature evolves later" rather than two that can disagree.
  return level() >= (uint16_t)(d.evolveLevel + careMistakes + evoPen) &&
         lowestStat() >= 40;
}

void Pet::evolve() {
  if (!canEvolveNow()) return;
  const DexEntry &d = DEX_TBL[speciesId];
  prevSpeciesId = speciesId;
  int16_t next = d.evolvesTo;
  if (speciesId == DEX_EEVEE) {
    // Eevee's branch: all eight, preferring one still missing from the Pokedex,
    // which is what makes raising Eevees a collection goal rather than a
    // coin flip. Only ones this player can actually be shown -- see
    // eeveeOptions(). If none qualify the table's own 134 stands, so a card with
    // no packs at all still evolves rather than freezing.
    int16_t opts[EEVEE_EVO_COUNT];
    uint8_t n = eeveeOptions(opts);
    if (n) {
      int16_t fresh[EEVEE_EVO_COUNT];
      uint8_t m = 0;
      for (uint8_t i = 0; i < n; i++)
        if (!isRegistered(opts[i])) fresh[m++] = opts[i];
      next = m ? fresh[random(m)] : opts[random(n)];
    }
  }
  speciesId = next;
  registerSpecies(speciesId);
  checkLearnGates();   // the new form may gate a move at this very level
  sfxPlay(SFX_EVOLVE);
  evolveUntil = millis() + EVOLVE_ANIM_MS;
  save();
}

void Pet::feed() {
  feedBerry(0);
}

void Pet::feedBerry(uint8_t color) {
  if (ceremony != CER_NONE) return;
  if (isEgg() || sleeping) return;
  bool wasFull = fullness >= 90;
  if (lovesBerry(color)) {
    fullness = clamp100(fullness + 35);
    joy = clamp100(joy + 10);
    heartUntil = millis() + HEART_MS;  // "loves it!"
    berryKnown = true;                 // discovered: shown on the card
    addBond(2);
  } else {
    fullness = clamp100(fullness + 25);
  }
  if (wasFull) noteOverfeed(1);  // it was already full: a binge
  if (lovesBerry(color)) noteMission(MIS_FEED_FAV, 1);
  noteMission(MIS_FEED, 1);
  bumpTally(TALLY_MEALS);
  earnCoins(1, true);
  eatUntil = millis() + EAT_ANIM_MS;
  registerCare();
  save();
}

void Pet::feedCandy() {
  if (ceremony != CER_NONE) return;
  if (isEgg() || sleeping) return;
  if (fullness >= 90) noteOverfeed(2);  // candy on a full stomach: worse
  fullness = clamp100(fullness + 10);
  joy = clamp100(joy + 12);
  weight = clamp100(weight + 12);  // treats take their toll
  noteMission(MIS_FEED, 1);
  bumpTally(TALLY_MEALS);
  earnCoins(1, true);
  eatUntil = millis() + EAT_ANIM_MS;
  registerCare();
  save();
}

uint8_t Pet::playResult(uint8_t score) {
  if (ceremony != CER_NONE || isEgg()) return 0;
  // The ball game is DEFENCE's trainer now. It used to train SPEED, which was
  // moved to its own reaction test to stop playing being a stat grind -- and
  // that left DEF with no active trainer at all, only the slow passive tick.
  // Keeping the ball on the defensive stat fits it: you are stopping something
  // from getting past you.
  uint8_t before = trDef;
  uint8_t gain = score / 2;
  if (gain > 18) gain = 18;          // the same per-session ceiling as the bag
  uint16_t v = (uint16_t)trDef + gain;
  trDef = v > trMaxDef() ? trMaxDef() : (uint8_t)v;
  gain = trDef - before;
  joy = clamp100(joy + 5 + (score > 15 ? 30 : score * 2));
  energy = dropTo(energy, 10 + score / 2, 5);
  fullness = dropTo(fullness, 5, 5);
  int burn = (int)weight - score * 2;  // exercise burns weight
  weight = burn > 0 ? burn : 0;
  if (score >= 5) heartUntil = millis() + HEART_MS;
  if (score > gameHi) gameHi = score;  // new record
  // Training bonds, and it scales with the session: a token effort is worth the
  // base, a full one is worth more. The daily cap in addBond() still stops it
  // being farmed -- this changes how fast a good session gets there, not the
  // ceiling.
  addBond((uint8_t)(2 + gain / 6));
  registerCare();
  notePlayed();
  notePlayedGame(MINI_STAT_DEF, score);
  save();
  return gain;
}

// training bag: the hits train strength. Returns the gain.
uint8_t Pet::rewardTraining(uint8_t amount, uint8_t &which) {
  which = 0;
  if (ceremony != CER_NONE || isEgg() || !amount) return 0;
  // Only the stats with headroom are candidates.
  uint8_t room[3], n = 0;
  if (trAtk < trMaxAtk()) room[n++] = 0;
  if (trDef < trMaxDef()) room[n++] = 1;
  if (trSpe < trMaxSpe()) room[n++] = 2;
  if (!n) return 0;                     // nothing left to train
  which = room[random(n)];
  uint8_t before, capped;
  switch (which) {
    case 0: before = trAtk; capped = trMaxAtk(); trAtk = (uint8_t)min<uint16_t>(before + amount, capped); amount = trAtk - before; break;
    case 1: before = trDef; capped = trMaxDef(); trDef = (uint8_t)min<uint16_t>(before + amount, capped); amount = trDef - before; break;
    default: before = trSpe; capped = trMaxSpe(); trSpe = (uint8_t)min<uint16_t>(before + amount, capped); amount = trSpe - before; break;
  }
  // The IV-bound ceiling is never crossed: a mediocre individual not reaching
  // as far is the whole point of trMaxFor().
  save();
  return amount;
}

uint8_t Pet::trainSpeed(uint16_t hits) {
  if (ceremony != CER_NONE || isEgg()) return 0;
  uint8_t gain = hits / 2;          // ~2 reactions = 1 point
  if (gain > 18) gain = 18;         // same per-session ceiling as the bag
  uint8_t before = trSpe;
  uint8_t v = trSpe + gain;
  trSpe = v > trMaxSpe() ? trMaxSpe() : v;   // the IV sets the ceiling
  gain = trSpe - before;
  energy = dropTo(energy, 10, 5);
  fullness = dropTo(fullness, 4, 5);
  int burn = (int)weight - hits / 2;
  weight = burn > 0 ? burn : 0;
  joy = clamp100(joy + 4);
  if (hits > spdHi) spdHi = hits;
  notePlayedGame(MINI_STAT_SPE, hits);
  // Training bonds, and it scales with the session: a token effort is worth the
  // base, a full one is worth more. The daily cap in addBond() still stops it
  // being farmed -- this changes how fast a good session gets there, not the
  // ceiling.
  addBond((uint8_t)(2 + gain / 6));
  registerCare();
  notePlayed();
  save();
  return gain;
}

uint8_t Pet::trainStrength(uint16_t hits) {
  if (ceremony != CER_NONE || isEgg()) return 0;
  uint8_t gain = hits / 4;          // ~4 hits = 1 training point
  if (gain > 18) gain = 18;         // per-session cap: ATK is forged slowly
  uint8_t before = trAtk;
  uint8_t v = trAtk + gain;
  trAtk = v > trMaxAtk() ? trMaxAtk() : v;  // the IV sets the ceiling
  gain = trAtk - before;            // what actually went up (it may hit the cap)
  energy = dropTo(energy, 12, 5);   // tires
  fullness = dropTo(fullness, 5, 5);
  int burn = (int)weight - hits / 3;  // also burns weight
  weight = burn > 0 ? burn : 0;
  joy = clamp100(joy + 6);
  if (hits >= 20) heartUntil = millis() + HEART_MS;
  if (hits > strHi) strHi = hits;   // record de golpes
  notePlayedGame(MINI_STAT_ATK, hits);
  // Training bonds, and it scales with the session: a token effort is worth the
  // base, a full one is worth more. The daily cap in addBond() still stops it
  // being farmed -- this changes how fast a good session gets there, not the
  // ceiling.
  addBond((uint8_t)(2 + gain / 6));
  registerCare();
  notePlayed();
  save();
  return gain;
}

// ---------- Fun Pack: the extra minigames ----------

namespace {
struct MiniReward {
  uint8_t joyBase, joyPerScore, joyMax;  // joy = base + min(score*per, max)
  uint8_t bond;
  int8_t stat;                           // MINI_STAT_* or MINI_NO_STAT
  uint8_t statDiv, statCap;              // gain = min(score/div, cap), IV ceiling still applies
  uint8_t eneBase, eneDiv;               // cost = base + score/div
  uint8_t foodDiv, foodCap;              // food = min(score/div, cap) (0 = none)
  uint8_t weightPerScore;                // weight burned per point
  uint8_t heartAt;                       // score that earns a heart
};
const MiniReward MINI_REWARDS[MINI_COUNT] = {
  // joy        bond  stat            div cap  ene    food   wgt heart
  { 4, 2, 20,   2,    MINI_NO_STAT,   1,  0,   3, 4,  0, 0,  0,  5 },  // LR
  { 4, 2, 20,   2,    MINI_NO_STAT,   1,  0,   3, 4,  0, 0,  0,  5 },  // HL
  { 4, 1, 16,   1,    MINI_STAT_DEF,  3,  8,   4, 4,  0, 0,  0,  6 },  // WHO
  { 3, 1, 15,   1,    MINI_STAT_SPE,  4, 10,   6, 3,  3, 15, 1,  8 },  // DROP
  { 3, 1, 15,   1,    MINI_STAT_ATK,  3, 12,   6, 3,  0, 0,  0,  8 },  // MOLE
  { 3, 1, 15,   1,    MINI_STAT_SPE,  3, 12,   8, 2,  0, 0,  2,  8 },  // ROPE
  { 6, 2, 30,   3,    MINI_NO_STAT,   1,  0,   6, 3,  0, 0,  1,  8 },  // DANCE
  { 4, 2, 20,   2,    MINI_NO_STAT,   1,  0,   3, 4,  0, 0,  0,  5 },  // HIDE
};
}

int8_t Pet::miniGameStat(uint8_t game) {
  return game < MINI_COUNT ? MINI_REWARDS[game].stat : MINI_NO_STAT;
}

uint8_t Pet::applyMiniResult(uint8_t game, uint8_t score) {
  if (ceremony != CER_NONE || isEgg() || game >= MINI_COUNT) return 0;
  const MiniReward &r = MINI_REWARDS[game];
  int joyGain = r.joyBase + min((int)score * r.joyPerScore, (int)r.joyMax);
  if (grumpy) joyGain /= 2;  // grumpy: harder to cheer up today
  uint8_t joyBefore = joy;
  joy = clamp100((int)joy + joyGain);
  uint8_t statGain = 0;
  if (r.stat != MINI_NO_STAT) {
    uint8_t want = score / r.statDiv;
    if (want > r.statCap) want = r.statCap;
    uint8_t *tr = r.stat == MINI_STAT_ATK ? &trAtk : (r.stat == MINI_STAT_DEF ? &trDef : &trSpe);
    uint8_t cap = r.stat == MINI_STAT_ATK ? trMaxAtk() : (r.stat == MINI_STAT_DEF ? trMaxDef() : trMaxSpe());
    uint8_t before = *tr;
    *tr = (uint8_t)min<uint16_t>((uint16_t)before + want, cap);   // the IV sets the ceiling
    statGain = *tr - before;
  }
  energy = dropTo(energy, r.eneBase + score / r.eneDiv, 5);
  if (r.foodDiv) {
    uint8_t food = score / r.foodDiv;
    fullness = clamp100((int)fullness + (food > r.foodCap ? r.foodCap : food));
  }
  int burn = (int)weight - (int)score * r.weightPerScore;
  weight = burn > 0 ? burn : 0;
  if (score >= r.heartAt) heartUntil = millis() + HEART_MS;
  if (score > miniHi[game]) miniHi[game] = score;
  notePlayedGame((uint8_t)r.stat, score);
  addBond(r.bond);
  registerCare();
  notePlayed();
  save();
  return r.stat != MINI_NO_STAT ? statGain : (uint8_t)(joy - joyBefore);
}

// ---------- Fun Pack: statuses (sickness, light, grumpy, bored) ----------

void Pet::notePlayed() {
  boredTicks = 0;
  grumpy = false;
}

void Pet::noteOverfeed(uint8_t amount) {
  uint8_t v = overfeed + amount;
  overfeed = v > 9 ? 9 : v;
  if (overfeed >= OVERFEED_SICK && !isSick()) {
    sickness = SICK_STOMACH;
    sickTicks = 0;
    sickDoses = 0;
  }
}

PillResult Pet::giveMedicine() {
  if (isEgg() || ceremony != CER_NONE) return PILL_NOT_SICK;
  PillResult res;
  if (sickness == SICK_NONE) {
    joy = clamp100((int)joy - 10);  // healthy: the medicine tastes awful
    res = PILL_NOT_SICK;
  } else if (sickness == SICK_STOMACH) {
    sickness = SICK_NONE;
    overfeed = 0;
    res = PILL_CURED;
  } else if (sickDoses >= 2) {
    res = PILL_NEED_BATH;
  } else if (sickDoses == 1 && ageMinutes - lastDoseMinute < COLD_DOSE_GAP_MIN) {
    res = PILL_WAIT;
  } else {
    sickDoses++;
    lastDoseMinute = ageMinutes;
    if (sickDoses < 2) res = PILL_MORE;
    else if (poops == 0 && hygiene >= 60) { sickness = SICK_NONE; res = PILL_CURED; }
    else res = PILL_NEED_BATH;
  }
  if (sickness == SICK_NONE) { sickTicks = 0; sickDoses = 0; dirtyTicks = 0; }
  save();
  return res;
}

void Pet::wakeUp() {
  sleeping = false;
  if (lightOnTicks >= LIGHT_GRUMPY_TICKS) {  // slept badly with the light on
    grumpy = true;
    joy = clamp100((int)joy - 15);
    if (energy > 70) energy = 70;
  }
  lightOn = false;
  lightOnTicks = 0;
}

bool Pet::isDozeHour() const {
  if (!lastSeenEpoch) return false;   // no clock, no evening
  int h = (int)((lastSeenEpoch / 3600) % 24);
  return h >= DOZE_START || h < DOZE_END;
}

void Pet::statusTick() {
  if (ageMinutes % 30 == 0 && overfeed) overfeed--;
  // dirty for too long -> a cold
  if (poops >= 3 || hygiene < 15) {
    if (dirtyTicks < 255) dirtyTicks++;
    if (dirtyTicks >= DIRTY_SICK_TICKS && !isSick()) {
      sickness = SICK_COLD;
      sickTicks = 0;
      sickDoses = 0;
    }
  } else {
    dirtyTicks = 0;
  }
  if (isSick()) {
    fullness = clamp100((int)fullness - 1);
    joy = clamp100((int)joy - 1);
    if (++sickTicks % SICK_MISTAKE_TICKS == 0) careMistakes++;
  }
  if (boredTicks < 0xFFFF && (!toyOut() || ageMinutes % 2 == 0)) boredTicks++;   // a toy keeps it company
  if (isBored()) joy = clamp100((int)joy - 1);
}

// Left alone in the evening with the screen on, it nods off by itself -- with
// the light still on. The sketch decides "left alone" (it owns lastInteract) and
// calls this. The power button turning the screen off counts as switching the
// light off (setScreenOff), so only a ball left glowing on the desk ends up here.
// SLEEP_PLAYER keeps the screen coming back on from waking it: the player has
// to deal with the light themselves.
// ---------- coins and daily missions ----------

void Pet::rollCoinDay() {
  uint32_t d = today();
  if (d == coinDay) return;
  coinDay = d;
  coinsToday = 0;
  stepCoinK = 0;
  memset(missionProg, 0, sizeof(missionProg));
  missionDone = 0;
  gamesToday = 0;
  pendingSave = true;
  // a week since the piggy bank last paid: 5% interest, a little at least, not too much
  if (piggy && piggyDay && d >= piggyDay + 7) {
    uint32_t i = piggy / PIGGY_RATE_DIV;
    if (!i && piggy >= 20) i = 1;
    if (i > PIGGY_INTEREST_MAX) i = PIGGY_INTEREST_MAX;
    piggy += i;
    piggyLastInterest = (uint16_t)i;
    piggyDay = d;
  }
}

uint16_t Pet::earnCoins(uint16_t amount, bool capped) {
  rollCoinDay();
  if (capped) {
    uint16_t left = coinsToday < COIN_DAILY_CAP ? COIN_DAILY_CAP - coinsToday : 0;
    if (amount > left) amount = left;
    coinsToday += amount;
  }
  if (!amount) return 0;
  coinsEarned += amount;
  uint32_t v = coins + amount;
  coins = v > COIN_MAX ? COIN_MAX : v;
  uint32_t now = millis();
  coinFlash = (now < coinFlashUntil ? coinFlash : 0) + amount;
  coinFlashUntil = now + 1800;
  pendingSave = true;
  return amount;
}

bool Pet::spendCoins(uint32_t amount) {
  if (coins < amount) return false;
  coins -= amount;
  save();
  return true;
}

uint16_t Pet::coinsCapLeft() const {
  if (coinDay != today()) return COIN_DAILY_CAP;
  return coinsToday < COIN_DAILY_CAP ? COIN_DAILY_CAP - coinsToday : 0;
}

uint8_t Pet::missionGoal(uint8_t m) {
  switch (m) {
    case MIS_FEED: return 3;
    case MIS_PLAY: return 2;
    case MIS_STEPS: return 20;   // in hundreds: 2 000 steps
    case MIS_PET: return 3;
    case MIS_SCORE: return 10;
    default: return 1;
  }
}

// Three different missions, the same all day: a small LCG seeded by the day.
uint8_t Pet::missionAt(uint8_t slot) const {
  uint32_t x = today() * 2654435761UL + 12345;
  uint8_t pick[MISSIONS_PER_DAY];
  uint8_t n = 0;
  while (n < MISSIONS_PER_DAY) {
    x = x * 1103515245UL + 12345;
    uint8_t m = (uint8_t)((x >> 16) % MIS_COUNT);
    bool dup = false;
    for (uint8_t i = 0; i < n; i++) dup = dup || pick[i] == m;
    if (!dup) pick[n++] = m;
  }
  return pick[slot % MISSIONS_PER_DAY];
}

uint8_t Pet::missionProgress(uint8_t slot) const {
  if (slot >= MISSIONS_PER_DAY) return 0;
  uint8_t m = missionAt(slot);
  if (m == MIS_STEPS) {
    uint32_t h = stepsToday() / 100;
    return h > missionGoal(m) ? missionGoal(m) : (uint8_t)h;
  }
  return coinDay == today() ? missionProg[slot] : 0;
}

void Pet::noteMission(uint8_t m, uint8_t amount) {
  if (isEgg() || ceremony != CER_NONE) return;
  rollCoinDay();
  for (uint8_t i = 0; i < MISSIONS_PER_DAY; i++) {
    if (missionAt(i) != m || missionComplete(i)) continue;
    uint8_t goal = missionGoal(m);
    uint32_t v = (m == MIS_STEPS) ? stepsToday() / 100 : (uint32_t)missionProg[i] + amount;
    missionProg[i] = v > goal ? goal : (uint8_t)v;
    if (missionProg[i] >= goal) {
      missionDone |= (uint8_t)(1 << i);
      earnCoins(COIN_MISSION, false);
    }
    pendingSave = true;
  }
  if ((missionDone & 7) == 7 && !(missionDone & 0x80)) {
    missionDone |= 0x80;
    earnCoins(COIN_ALL_MISSIONS, false);
  }
}

// A game finished: coins by score, and the missions it counts towards.
void Pet::notePlayedGame(uint8_t cat, uint16_t score) {
  uint16_t c = 1 + score / 10;
  earnCoins(c > COIN_GAME_MAX ? COIN_GAME_MAX : c, true);
  noteMission(MIS_PLAY, 1);
  bumpTally(TALLY_GAMES);
  if (gamesToday < 255) gamesToday++;
  if (cat == (uint8_t)MINI_STAT_ATK) noteMission(MIS_TRAIN_ATK, 1);
  else if (cat == (uint8_t)MINI_STAT_SPE) noteMission(MIS_TRAIN_SPE, 1);
  else if (cat == (uint8_t)MINI_STAT_DEF) noteMission(MIS_TRAIN_DEF, 1);
  if (score >= missionGoal(MIS_SCORE)) noteMission(MIS_SCORE, missionGoal(MIS_SCORE));
}

// ---------- hats ----------

uint16_t Pet::hatPrice(uint8_t h) {
  // About a day of good play for the simple ones, most of a week for the crown.
  static const uint16_t PRICE[HAT_COUNT] = { 80, 80, 100, 120, 150, 150, 200, 200, 300, 500 };
  return h < HAT_COUNT ? PRICE[h] : 0;
}

bool Pet::buyHat(uint8_t h) {
  if (h >= HAT_COUNT || ownsHat(h)) return false;
  if (!spendCoins(hatPrice(h))) return false;
  hatsOwned |= (uint16_t)(1 << h);
  save();
  return true;
}

void Pet::wearHat(int8_t h) {
  if (h >= HAT_COUNT) return;
  if (h >= 0 && !ownsHat((uint8_t)h)) return;
  hatOn = h < 0 ? -1 : h;
  save();
}

void Pet::nudgeHat(int dx, int dy) {
  if (hatDex != speciesId) { hatDx = hatDy = 0; hatDex = speciesId; }
  int x = hatDx + dx, y = hatDy + dy;
  hatDx = (int8_t)(x > HAT_NUDGE_MAX ? HAT_NUDGE_MAX : (x < -HAT_NUDGE_MAX ? -HAT_NUDGE_MAX : x));
  hatDy = (int8_t)(y > HAT_NUDGE_MAX ? HAT_NUDGE_MAX : (y < -HAT_NUDGE_MAX ? -HAT_NUDGE_MAX : y));
  save();
}

// ---------- toys and decorations ----------

uint16_t Pet::decorPrice(uint8_t d) {
  static const uint16_t PRICE[DEC_COUNT] = {
    40, 90, 60, 70, 110, 120,      // ball, plush, duck, blocks, drum, kite
    50, 60, 90, 150, 80, 70,       // plant, flowers, lamp, bed, gift, pumpkin
  };
  return d < DEC_COUNT ? PRICE[d] : 0;
}

bool Pet::buyDecor(uint8_t d) {
  if (d >= DEC_COUNT || ownsDecor(d)) return false;
  if (!spendCoins(decorPrice(d))) return false;
  decorOwned |= (uint16_t)(1 << d);
  save();
  return true;
}

void Pet::placeDecor(uint8_t d) {
  if (!ownsDecor(d)) return;
  if (slotL == (int8_t)d) slotL = -1;
  else if (slotR == (int8_t)d) slotR = -1;
  else if (slotL < 0) slotL = (int8_t)d;
  else if (slotR < 0) slotR = (int8_t)d;
  else slotL = (int8_t)d;
  save();
}

// ---------- foods ----------

const FoodInfo &Pet::foodInfo(uint8_t f) {
  static const FoodInfo T_[FOOD_COUNT] = {
    // food joy  ene  wgt  price sweet
    {  30,   5,   0,   2,  12, false },   // sandwich
    {  45,  10,   0,   4,  20, false },   // curry
    {  25,   5,  10,   0,  15, false },   // soup
    {  20,   0,   0,   1,   8, false },   // rice ball
    {  15,  25,   0,  15,  25, true  },   // cake
    {   5,  12,   0,   5,   8, true  },   // cookie
    {   8,  18,   0,   8,  12, true  },   // ice cream
    {  10,   0,  20,   0,  12, false },   // milk
    {   5,  10,   5,   2,   8, false },   // juice
    {  20,  30,   0,   8,   0, true  },   // berry pie (cooked)
    {  60,  15,  10,   3,   0, false },   // stew (cooked)
    {  15,  35,   5,   8,   0, true  },   // parfait (cooked)
  };
  return T_[f < FOOD_COUNT ? f : 0];
}

bool Pet::buyFood(uint8_t f) {
  if (f >= FOOD_SHOP_COUNT || foodCount[f] >= FOOD_STACK_MAX) return false;   // cooked dishes are not sold
  if (!spendCoins(foodInfo(f).price)) return false;
  foodCount[f]++;
  save();
  return true;
}

bool Pet::eatFood(uint8_t f) {
  if (f >= FOOD_COUNT || !foodCount[f]) return false;
  if (ceremony != CER_NONE || isEgg() || sleeping) return false;
  const FoodInfo &fi = foodInfo(f);
  if (fullness >= 90 && fi.food > 0) noteOverfeed(fi.sweet ? 2 : 1);   // already full: a binge
  bumpTally(TALLY_MEALS);
  if (f >= FOOD_SHOP_COUNT) addBond(2);   // home-cooked
  fullness = clamp100(fullness + fi.food);
  joy = clamp100(joy + fi.joy);
  energy = clamp100(energy + fi.energy);
  weight = clamp100(weight + fi.weight);
  foodCount[f]--;
  eatUntil = millis() + EAT_ANIM_MS;
  if (fi.joy >= 15) heartUntil = millis() + HEART_MS;
  registerCare();
  noteMission(MIS_FEED, 1);
  earnCoins(1, true);
  save();
  return true;
}

// ---------- bath and sleep extras ----------

bool Pet::buySoap() {
  if (soapCount >= FOOD_STACK_MAX || !spendCoins(SOAP_PRICE)) return false;
  soapCount++;
  save();
  return true;
}

bool Pet::buyBrush() {
  if (brushOwned || !spendCoins(BRUSH_PRICE)) return false;
  brushOwned = true;
  save();
  return true;
}

bool Pet::bubbleBath() {
  if (!soapCount || isEgg() || sleeping || ceremony != CER_NONE) return false;
  soapCount--;
  clean();                       // the bath itself: poops, hygiene, mission, coin
  joy = clamp100(joy + 10);
  addBond(2);
  heartUntil = millis() + HEART_MS;
  save();
  return true;
}

bool Pet::brush() {
  if (!brushOwned || isEgg() || sleeping || ceremony != CER_NONE) return false;
  joy = clamp100(joy + 5);
  addBond(3);
  heartUntil = millis() + HEART_MS;
  shineUntil = millis() + SHINE_MS;
  registerCare();
  save();
  return true;
}

void Pet::scrubDone(uint8_t spots) {
  if (isEgg() || ceremony != CER_NONE) return;
  clean();
  uint8_t j = spots * 2;
  joy = clamp100(joy + (j > 12 ? 12 : j));
  heartUntil = millis() + HEART_MS;
  save();
}

bool Pet::startNap() {
  if (isEgg() || sleeping || ceremony != CER_NONE) return false;
  sleeping = true;
  lightOn = false;
  lightOnTicks = 0;
  napEnd = ageMinutes + NAP_MINUTES;
  sleepAuto = SLEEP_PLAYER;
  save();
  return true;
}

bool Pet::lullaby() {
  if (isEgg() || sleeping || ceremony != CER_NONE) return false;
  toggleLight();                 // to bed with the light off (and the bedtime coins)
  joy = clamp100(joy + 5);
  save();
  return true;
}

// ---------- places: the park, the Pokemon Center, the garden ----------

ParkResult Pet::parkWalk() {
  if (isEgg() || sleeping || ceremony != CER_NONE || energy < 15) return PARK_TIRED;
  energy = dropTo(energy, PARK_WALK_ENERGY, 5);
  bumpTally(TALLY_WALKS);
  joy = clamp100(joy + 3);
  registerCare();
  if (parkDay != today()) { parkDay = today(); parkFinds = 0; }
  int r = random(100);
  ParkResult res;
  if (r < 30 && parkFinds < PARK_FINDS_PER_DAY) {       // something shiny in the grass
    parkFinds++;
    lastFind = (uint8_t)(2 + random(3));
    earnCoins(lastFind, false);
    res = PARK_COINS;
  } else if (r < 55) {                                   // a wild berry
    fullness = clamp100(fullness + 10);
    joy = clamp100(joy + 5);
    res = PARK_BERRY;
  } else if (r < 85) {                                   // a wild Pokemon says hello
    joy = clamp100(joy + 3);
    addBond(1);
    heartUntil = millis() + HEART_MS;
    res = PARK_FRIEND;
  } else {
    res = PARK_NOTHING;
  }
  save();
  return res;
}

CenterResult Pet::centerHeal() {
  CenterResult res = CENTER_FREE;
  if (!centerFreeToday()) {
    if (!spendCoins(CENTER_PRICE)) return CENTER_NO_COINS;
    res = CENTER_PAID;
  }
  centerDay = today();
  sickness = SICK_NONE; sickTicks = 0; sickDoses = 0; overfeed = 0; dirtyTicks = 0;
  grumpy = false;
  energy = clamp100(energy + 30);
  heartUntil = millis() + HEART_MS;
  save();
  return res;
}

bool Pet::buySeed() {
  if (seedCount >= FOOD_STACK_MAX || !spendCoins(SEED_PRICE)) return false;
  seedCount++;
  save();
  return true;
}

bool Pet::plantSeed() {
  if (!seedCount || plantEpoch || !lastSeenEpoch) return false;
  seedCount--;
  plantEpoch = lastSeenEpoch;
  save();
  return true;
}

bool Pet::gardenReady() const {
  return plantEpoch && lastSeenEpoch >= plantEpoch + GARDEN_GROW_SECS;
}

uint8_t Pet::gardenPercent() const {
  if (!plantEpoch) return 0;
  if (lastSeenEpoch <= plantEpoch) return 0;
  uint32_t g = (lastSeenEpoch - plantEpoch) * 100UL / GARDEN_GROW_SECS;
  return g > 100 ? 100 : (uint8_t)g;
}

int8_t Pet::harvest() {
  if (!gardenReady()) return -1;
  // fruit of the garden: mostly drinks and sweets, sometimes a meal
  static const uint8_t CROP[8] = { FOOD_JUICE, FOOD_JUICE, FOOD_COOKIE, FOOD_ICECREAM,
                                   FOOD_CAKE, FOOD_SANDWICH, FOOD_SOUP, FOOD_ONIGIRI };
  uint8_t f = CROP[random(8)];
  if (foodCount[f] < FOOD_STACK_MAX) foodCount[f]++;
  plantEpoch = 0;
  bumpTally(TALLY_HARVEST);
  registerCare();
  save();
  return (int8_t)f;
}

// ---------- the capsule machine ----------

CapsuleResult Pet::spinCapsule() {
  if (capDay != today()) { capDay = today(); capSpins = 0; }
  if (capSpins >= CAPSULES_PER_DAY) return CAP_LIMIT;
  if (!spendCoins(CAPSULE_PRICE)) return CAP_NO_COINS;
  capSpins++;
  bumpTally(TALLY_CAPSULE);
  int r = random(100);
  CapsuleResult res;
  if (r < 10) {                                          // the rare one: a hat or decoration not owned yet
    uint8_t free[HAT_COUNT + DEC_COUNT], n = 0;
    for (uint8_t h = 0; h < HAT_COUNT; h++) if (!ownsHat(h)) free[n++] = h;
    for (uint8_t d = 0; d < DEC_COUNT; d++) if (!ownsDecor(d)) free[n++] = 100 + d;
    if (n) {
      uint8_t pick = free[random(n)];
      diaryFlag(DF_RARE);
      if (pick >= 100) { decorOwned |= 1 << (pick - 100); capWhat = pick - 100; res = CAP_DECOR; }
      else { hatsOwned |= 1 << pick; capWhat = pick; res = CAP_HAT; }
    } else {                                             // owns everything: a big coin prize
      capWhat = 50; earnCoins(50, false); res = CAP_COINS;
    }
  } else if (r < 50) {                                   // a serving of food
    uint8_t f = (uint8_t)random(FOOD_SHOP_COUNT);
    if (foodCount[f] < FOOD_STACK_MAX) foodCount[f]++;
    capWhat = f; res = CAP_FOOD;
  } else if (r < 60) {
    if (soapCount < FOOD_STACK_MAX) soapCount += soapCount + 2 > FOOD_STACK_MAX ? 1 : 2;
    capWhat = 2; res = CAP_SOAP;
  } else if (r < 70) {
    if (seedCount < FOOD_STACK_MAX) seedCount += seedCount + 2 > FOOD_STACK_MAX ? 1 : 2;
    capWhat = 2; res = CAP_SEED;
  } else {                                               // coins back: 10..40
    capWhat = (uint16_t)(10 + random(31));
    earnCoins(capWhat, false);
    res = CAP_COINS;
  }
  save();
  return res;
}

// ---------- scenery ----------

uint16_t Pet::sceneryPrice(uint8_t b) {
  // meadow, beach, forest, volcano, mountain, snow, graveyard
  static const uint16_t PRICE[SCENERY_COUNT] = { 100, 120, 120, 180, 150, 150, 200 };
  return PRICE[b % SCENERY_COUNT];
}

bool Pet::buyScenery(uint8_t b) {
  if (b >= SCENERY_COUNT || ownsScenery(b)) return false;
  if (!spendCoins(sceneryPrice(b))) return false;
  sceneryOwned |= 1 << b;
  save();
  return true;
}

void Pet::pickScenery(int8_t b) {
  if (b >= 0 && !ownsScenery((uint8_t)b)) return;
  sceneryPick = (b == sceneryPick) ? -1 : b;
  save();
}

// ---------- trophies ----------

void Pet::bumpTally(uint8_t t, uint16_t n) {
  if (t >= TALLY_COUNT) return;
  uint32_t v = (uint32_t)tally[t] + n;
  tally[t] = v > 60000 ? 60000 : (uint16_t)v;
  pendingSave = true;
  diaryRoll();
  uint8_t *f = nullptr;
  switch (t) {
    case TALLY_MEALS: f = &diary[0].meals; break;
    case TALLY_GAMES: f = &diary[0].games; break;
    case TALLY_BATHS: f = &diary[0].baths; break;
    case TALLY_WALKS: f = &diary[0].walks; break;
    case TALLY_FISH:  f = &diary[0].fish; break;
    case TALLY_STUDY: f = &diary[0].study; break;
    case TALLY_CHORES: f = &diary[0].chores; break;
    case TALLY_COOK:  diary[0].flags |= DF_COOKED; break;
    default: break;
  }
  if (f) *f = (uint8_t)(*f + n > 255 ? 255 : *f + n);
}

static uint8_t bitCount16(uint16_t v) { uint8_t n = 0; while (v) { n += v & 1; v >>= 1; } return n; }

void Pet::trophyProgress(uint8_t t, uint32_t &cur, uint32_t &goal) const {
  switch (t) {
    case TRO_MEALS:   cur = tally[TALLY_MEALS];   goal = 100; break;
    case TRO_GAMES:   cur = tally[TALLY_GAMES];   goal = 50;  break;
    case TRO_BATHS:   cur = tally[TALLY_BATHS];   goal = 30;  break;
    case TRO_WALKS:   cur = tally[TALLY_WALKS];   goal = 25;  break;
    case TRO_HARVEST: cur = tally[TALLY_HARVEST]; goal = 5;   break;
    case TRO_COINS:   cur = coinsEarned;          goal = 1000; break;
    case TRO_HATS:    cur = bitCount16(hatsOwned); goal = 5;  break;
    case TRO_DECOR:   cur = bitCount16(decorOwned); goal = 6; break;
    case TRO_STREAK:  cur = bestStreak > streak ? bestStreak : streak; goal = 7; break;
    case TRO_LEVEL:   cur = isEgg() ? 0 : level(); goal = 20; break;
    case TRO_STUDY:   cur = tally[TALLY_STUDY];   goal = 10;  break;
    case TRO_TEETH:   cur = tally[TALLY_TEETH];   goal = 14;  break;
    case TRO_PERFECT: cur = tally[TALLY_PERFECT]; goal = 1;   break;
    case TRO_CAPSULE: cur = tally[TALLY_CAPSULE]; goal = 10;  break;
    case TRO_FISH:    cur = tally[TALLY_FISH];    goal = 20;  break;
    case TRO_ROUTE:   cur = tally[TALLY_ROUTE];   goal = 1;   break;
    default: cur = 0; goal = 1; break;
  }
  if (cur > goal) cur = goal;
}

int8_t Pet::checkTrophies() {
  for (uint8_t t = 0; t < TRO_COUNT; t++) {
    if (hasTrophy(t)) continue;
    uint32_t c, g;
    trophyProgress(t, c, g);
    if (c < g) continue;
    trophies |= 1 << t;
    diaryFlag(DF_TROPHY);
    earnCoins(TROPHY_COINS, false);
    save();
    return (int8_t)t;
  }
  return -1;
}

// ---------- teeth, homework, the quiz, parent limits ----------

bool Pet::teethDone() {
  if (isEgg() || sleeping || ceremony != CER_NONE) return false;
  if (teethDay != today()) { teethDay = today(); teethToday = 0; }
  if (teethToday >= TEETH_PER_DAY) return false;
  teethToday++;
  hygiene = clamp100(hygiene + 5);
  joy = clamp100(joy + 3);
  addBond(1);
  bumpTally(TALLY_TEETH);
  earnCoins(1, true);
  registerCare();
  save();
  return true;
}

uint8_t Pet::studyDone(uint8_t minutes) {
  if (isEgg() || ceremony != CER_NONE) return 0;
  if (studyDay != today()) { studyDay = today(); studyToday = 0; }
  joy = clamp100(joy + 2);
  addBond(2);
  bumpTally(TALLY_STUDY);
  uint8_t c = 0;
  if (studyToday < STUDY_PER_DAY) {      // a reward for the work, not a coin farm
    studyToday++;
    c = (uint8_t)earnCoins(minutes / 5, false);
  }
  registerCare();
  save();
  return c;
}

uint8_t Pet::quizDone(uint8_t correct) {
  if (correct > QUIZ_QUESTIONS) correct = QUIZ_QUESTIONS;
  if (correct > quizHi) quizHi = correct;
  if (correct == QUIZ_QUESTIONS) bumpTally(TALLY_PERFECT);
  joy = clamp100(joy + correct / 2);
  if (correct >= 7) addBond(1);
  uint16_t before = coinsToday;
  notePlayedGame(MINI_NO_STAT, (uint16_t)correct * 5);   // a perfect round pays like a good game
  save();
  return (uint8_t)(coinsToday - before);
}

ParentBlock Pet::parentBlock() const {
  if (parentBed && lastSeenEpoch) {
    int h = (int)((lastSeenEpoch / 3600) % 24);
    if (h >= parentBed || h < 6) return PB_BEDTIME;
  }
  if (parentGames && coinDay == today() && gamesToday >= parentGames) return PB_LIMIT;
  return PB_NONE;
}

// ---------- step adventures ----------

uint16_t Pet::advNeed(uint8_t stop) {
  // a route is about a day of a child's walking; the first stop comes quickly
  static const uint16_t NEED[ROUTE_STOPS] = { 300, 800, 1500, 2500, 4000 };
  return NEED[stop < ROUTE_STOPS ? stop : ROUTE_STOPS - 1];
}

AdvResult Pet::advClaim() {
  if (!advReady()) return ADV_NONE;
  diaryFlag(DF_ADVENTURE);
  if (advStop == ROUTE_STOPS - 1) {                      // the end of the route
    earnCoins(10, false);
    advWhat = 10;
    advSteps = advSteps > advNeed(advStop) ? advSteps - advNeed(advStop) : 0;   // the extra steps carry on
    advStop = 0;
    advRoute = (uint8_t)((advRoute + 1) % ROUTE_COUNT);
    bumpTally(TALLY_ROUTE);
    diaryFlag(DF_ROUTE);
    joy = clamp100(joy + 10);
    addBond(2);
    save();
    return ADV_DONE;
  }
  advStop++;
  AdvResult r;
  int k = random(100);
  if (k < 35) { advWhat = (uint16_t)(2 + random(4)); earnCoins(advWhat, false); r = ADV_COINS; }
  else if (k < 65) {
    uint8_t f = (uint8_t)random(FOOD_SHOP_COUNT);
    if (foodCount[f] < FOOD_STACK_MAX) foodCount[f]++;
    advWhat = f; r = ADV_FOOD;
  } else if (k < 80) {
    seedCount = seedCount + 2 > FOOD_STACK_MAX ? FOOD_STACK_MAX : seedCount + 2;
    advWhat = 2; r = ADV_SEEDS;
  } else { joy = clamp100(joy + 5); addBond(2); advWhat = 0; r = ADV_FRIEND; }   // the screen picks who
  save();
  return r;
}

// ---------- fishing ----------

FishResult Pet::fishCatch() {
  if (fishDay != today()) { fishDay = today(); fishToday = 0; }
  bool pays = fishToday < FISH_PER_DAY;
  if (fishToday < 255) fishToday++;
  bumpTally(TALLY_FISH);
  joy = clamp100(joy + 2);
  registerCare();
  int k = random(100);
  FishResult r = k < 35 ? FISH_KARP : k < 50 ? FISH_GOLDEEN : k < 65 ? FISH_BOOT
               : k < 85 ? FISH_SNACK : k < 95 ? FISH_SHELL : FISH_PEARL;
  lastFishCoins = 0;
  if (pays) {
    static const uint8_t COINS[6] = { 1, 3, 0, 0, 0, 10 };   // about 7 coins a day from the paying five
    if (COINS[r]) lastFishCoins = (uint8_t)earnCoins(COINS[r], false);
    if (r == FISH_SNACK && foodCount[FOOD_ONIGIRI] < FOOD_STACK_MAX) foodCount[FOOD_ONIGIRI]++;
    if (r == FISH_SHELL && seedCount < FOOD_STACK_MAX) seedCount++;
  }
  save();
  return r;
}

// ---------- the kitchen ----------

const Recipe &Pet::recipe(uint8_t r) {
  static const Recipe R_[RECIPE_COUNT] = {
    { FOOD_COOKIE, FOOD_JUICE, FOOD_PIE },
    { FOOD_SOUP, FOOD_ONIGIRI, FOOD_STEW },
    { FOOD_ICECREAM, FOOD_MILK, FOOD_PARFAIT },
  };
  return R_[r % RECIPE_COUNT];
}

bool Pet::canCook(uint8_t r) const {
  const Recipe &x = recipe(r);
  return r < RECIPE_COUNT && foodCount[x.a] && foodCount[x.b] && foodCount[x.out] < FOOD_STACK_MAX;
}

bool Pet::cook(uint8_t r) {
  if (!canCook(r)) return false;
  const Recipe &x = recipe(r);
  foodCount[x.a]--;
  foodCount[x.b]--;
  foodCount[x.out]++;
  bumpTally(TALLY_COOK);
  registerCare();
  save();
  return true;
}

// ---------- alarm, chores, piggy bank, diary ----------

bool Pet::alarmDue() const {
  if (!alarmOn || !lastSeenEpoch || alarmDay == today()) return false;
  int now = (int)((lastSeenEpoch / 60) % 1440), at = alarmH * 60 + alarmM;
  return now >= at && now < at + 60;
}

bool Pet::bedReminderDue() const {
  if (!parentBed || !lastSeenEpoch || bedRemDay == today()) return false;
  int h = (int)((lastSeenEpoch / 3600) % 24);
  return h == parentBed;
}

bool Pet::doChore(uint8_t c) {
  if (c >= CHORE_COUNT) return false;
  if (choreDay != today()) { choreDay = today(); choresDone = 0; }
  if (choresDone & (1 << c)) return false;
  choresDone |= 1 << c;
  earnCoins(CHORE_COINS, false);
  joy = clamp100(joy + 3);
  addBond(1);
  bumpTally(TALLY_CHORES);
  save();
  return true;
}

bool Pet::piggyDeposit(uint32_t amount) {
  if (!amount || coins < amount) return false;
  coins -= amount;
  piggy += amount;
  if (!piggyDay) piggyDay = today();   // the week starts with the first coin
  save();
  return true;
}

uint32_t Pet::piggyWithdraw() {
  uint32_t a = piggy;
  if (!a) return 0;
  uint32_t v = coins + a;
  coins = v > COIN_MAX ? COIN_MAX : v;
  piggy = 0;
  piggyDay = 0;                        // a fresh week on the next deposit
  save();
  return a;
}

uint8_t Pet::piggyDaysLeft() const {
  if (!piggyDay) return 7;
  uint32_t d = today();
  return d >= piggyDay + 7 ? 0 : (uint8_t)(piggyDay + 7 - d);
}

uint16_t Pet::piggyNextInterest() const {
  uint32_t i = piggy / PIGGY_RATE_DIV;
  if (!i && piggy >= 20) i = 1;
  return (uint16_t)(i > PIGGY_INTEREST_MAX ? PIGGY_INTEREST_MAX : i);
}

void Pet::diaryRoll() {
  uint32_t d = today();
  if (!d || diary[0].day == (uint16_t)d) return;
  if (diary[0].day) {                  // close yesterday's page and turn it
    diary[0].coins = (uint16_t)(coinsEarned - diaryCoinBase);
    for (int i = DIARY_DAYS - 1; i > 0; i--) diary[i] = diary[i - 1];
  }
  memset(&diary[0], 0, sizeof(DiaryDay));
  diary[0].day = (uint16_t)d;
  diaryCoinBase = coinsEarned;
  pendingSave = true;
}

// Howard Hinnant's days_from_civil, inverted: exact for any date after 1970.
void Pet::civilDate(uint32_t days, int &y, int &m, int &d) {
  int32_t z = (int32_t)days + 719468;
  int32_t era = z / 146097;
  uint32_t doe = (uint32_t)(z - era * 146097);
  uint32_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  int32_t yy = (int32_t)yoe + era * 400;
  uint32_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  uint32_t mp = (5 * doy + 2) / 153;
  d = (int)(doy - (153 * mp + 2) / 5 + 1);
  m = (int)(mp < 10 ? mp + 3 : mp - 9);
  y = (int)(yy + (m <= 2));
}

GiftKind Pet::holidayToday() const {
  if (!today()) return GIFT_NONE;
  int y, m, d;
  civilDate(today(), y, m, d);
  if (m == 10 && d == 31) return GIFT_HALLOWEEN;
  if (m == 12 && d >= 24 && d <= 26) return GIFT_CHRISTMAS;
  if (m == 1 && d == 1) return GIFT_NEWYEAR;
  if (m == 2 && d == 14) return GIFT_VALENTINE;
  return GIFT_NONE;
}

// The gift is the holiday's item if it is not owned yet, else 20 coins.
GiftKind Pet::claimHolidayGift() {
  if (isEgg() || ceremony != CER_NONE) return GIFT_NONE;
  GiftKind g = holidayToday();
  if (g == GIFT_NONE || giftDay == today()) return GIFT_NONE;
  giftDay = today();
  switch (g) {
    case GIFT_HALLOWEEN: if (!ownsDecor(DEC_PUMPKIN)) decorOwned |= 1 << DEC_PUMPKIN; else earnCoins(20, false); break;
    case GIFT_CHRISTMAS: if (!ownsHat(HAT_SANTA)) hatsOwned |= 1 << HAT_SANTA; else earnCoins(20, false); break;
    case GIFT_NEWYEAR:   if (!ownsHat(HAT_PARTY)) hatsOwned |= 1 << HAT_PARTY; else earnCoins(20, false); break;
    case GIFT_VALENTINE: if (!ownsDecor(DEC_FLOWERS)) decorOwned |= 1 << DEC_FLOWERS; else earnCoins(20, false); break;
    default: break;
  }
  save();
  return g;
}

void Pet::dozeOff() {
  if (isEgg() || ceremony != CER_NONE || sleeping || screenIsOff || !isDozeHour()) return;
  sleeping = true;
  sleepAuto = SLEEP_PLAYER;
  lightOn = true;
  lightOnTicks = 0;
  save();
}

void Pet::play() {
  if (ceremony != CER_NONE) return;
  if (isEgg() || sleeping) return;
  joy = clamp100(joy + 25);
  energy = clamp100(energy - 10);
  fullness = clamp100(fullness - 5);
  heartUntil = millis() + HEART_MS;
  addBond(2);
  registerCare();
  save();
}

// The hour off the RTC, the same source the scene uses. With no clock at all
// there is no night, so a board that has never been set never auto-sleeps.
bool Pet::isNightHour() const {
  if (!lastSeenEpoch) return false;
  int h = (int)((lastSeenEpoch / 3600) % 24);
  // The window may or may not cross midnight, and it must keep working either
  // way: with NIGHT_START 0 the old `h >= START || h < END` was true for every
  // hour of the day, which put the creature to sleep the moment the screen went
  // off at noon. sleep_test catches it, and did.
  if (NIGHT_START < NIGHT_END) return h >= NIGHT_START && h < NIGHT_END;
  return h >= NIGHT_START || h < NIGHT_END;
}

// Auto-sleep needs the screen off AND the night hours, and is re-checked every
// tick rather than only on the button, so a device put down at 21:00 nods off
// at 22:00 and gets up at 06:00 without anyone touching it.
//
// Both halves earn their place. Screen-off alone paused the game whenever you
// put the device down, and the creature is meant to get hungry during the day.
// The hour alone sent it to bed while you were still playing with it.
//
// Only what this put to sleep is woken by it: a creature the player sent to bed
// with the light button stays there until the player says otherwise.
void Pet::applyAutoSleep() {
  if (isEgg() || ceremony != CER_NONE) return;
  bool night = isNightHour();
  if (screenIsOff && night && !sleeping && sleepAuto != SLEEP_PLAYER) {
    sleeping = true;
    sleepAuto = SLEEP_AUTO;
    pendingSave = true;
  }
  // NOTHING wakes it here, and that is the whole point. Waking at 06:00 would
  // reopen the hole this exists to close: from the sleep floors, food is empty
  // by 06:15 and every stat by 07:40, so anyone who sleeps past eight would
  // find the creature ready to run away again. It sleeps until YOU are up,
  // which is the screen coming back on.
  if (!night && sleepAuto == SLEEP_PLAYER) sleepAuto = SLEEP_NONE;  // a new day
}

void Pet::setScreenOff(bool off) {
  screenIsOff = off;
  // Turning the screen off is turning the light off: a Pokemon that dozed off
  // with it on sleeps properly from here, and what it already lost stays lost.
  if (off && sleeping) lightOn = false;
  // Coming back to the device is what wakes it -- only if the device is what
  // put it to sleep. A creature sent to bed with the light stays there.
  if (!off && sleeping && sleepAuto == SLEEP_AUTO) {
    wakeUp();
    sleepAuto = SLEEP_NONE;
  }
  applyAutoSleep();
  save();
}

void Pet::toggleLight() {
  if (ceremony != CER_NONE) return;
  if (isEgg()) return;
  if (sleeping && lightOn) {   // dozed off by itself: light off, keeps sleeping
    lightOn = false;
  } else if (sleeping) {
    wakeUp();
    napEnd = 0;
  } else {
    sleeping = true;
    lightOn = false;
    lightOnTicks = 0;
    // put to bed properly in the evening: once a day
    if (isDozeHour() && today() && bedtimeDay != today()) {
      bedtimeDay = today();
      earnCoins(COIN_BEDTIME, false);
    }
  }
  // The player's hand beats the clock until morning: waking it at 23:00 must
  // not be undone a minute later by the auto-sleep, and neither must putting
  // it to bed early.
  sleepAuto = SLEEP_PLAYER;
  save();
}

void Pet::clean() {
  if (ceremony != CER_NONE) return;
  poops = 0;
  hygiene = 100;
  dirtyTicks = 0;
  // the bath is the last step of curing a cold
  if (sickness == SICK_COLD && sickDoses >= 2) { sickness = SICK_NONE; sickTicks = 0; sickDoses = 0; }
  noteMission(MIS_BATH, 1);
  bumpTally(TALLY_BATHS);
  earnCoins(1, true);
  addBond(1);
  registerCare();
  save();
}

void Pet::caress() {
  if (ceremony != CER_NONE) return;
  if (isEgg() || sleeping) return;
  joy = clamp100(joy + 5);
  heartUntil = millis() + HEART_MS;
  addBond(1);
  registerCare();
  noteMission(MIS_PET, 1);
  earnCoins(1, true);
}

void Pet::eggTap() {
  if (!isEgg()) return;
  if (++eggTaps >= 3) hatch();
  else save();
}

PetMood Pet::mood() const {
  if (sleeping) return MOOD_SLEEPING;
  if (eating()) return MOOD_EATING;
  if (isSick() || lowestStat() < 25) return MOOD_SAD;
  return MOOD_HAPPY;
}

void Pet::save() {
  // A Pet nobody opened is a scratch object -- the battle's opponent, a test
  // fixture, anything built to be read and thrown away. It shares the single
  // NVS namespace with the real creature, so letting it write means it
  // OVERWRITES the player's pet. See the note on `opened` in pet.h.
  if (!opened) return;
  ticksSinceSave = 0;
  pendingSave = false;
  prefs.putUChar("full", fullness);
  prefs.putUChar("joy", joy);
  prefs.putUChar("ene", energy);
  prefs.putUChar("hyg", hygiene);
  prefs.putUChar("poop", poops);
  prefs.putUChar("wgt", weight);
  prefs.putUChar("ivat", ivAtk);
  prefs.putUChar("ivdf", ivDef);
  prefs.putUChar("ivsp", ivSpe);
  prefs.putUChar("ivhp", ivHp);
  prefs.putUChar("tatk", trAtk);
  prefs.putUChar("tdef", trDef);
  prefs.putUChar("tspe", trSpe);
  prefs.putBytes("mvs", moves, sizeof(moves));
  prefs.putUChar("mvlv", lastLearnLevel);
  prefs.putUChar("avtr", avatar);
  prefs.putUChar("reg", region);
  prefs.putBytes("badgX", badgesX, sizeof(badgesX));
  prefs.putBytes("badhX", badgesHardX, sizeof(badgesHardX));
  prefs.putBytes("eggR", eggByRegion, sizeof(eggByRegion));
  prefs.putString("tnam", trainerName);
  prefs.putString("onam", ownerName);
  prefs.putBool("froz", frozen);
  prefs.putUShort("badg", badges);
  prefs.putUShort("badh", badgesHard);
  prefs.putBool("bk", berryKnown);
  prefs.putBool("shy", shiny);
  prefs.putBool("eshy", eggShiny);
  prefs.putBool("stpk", starterPick);
  prefs.putUChar("evop", evoPen);
  prefs.putUChar("slpa", sleepAuto);
  prefs.putBool("rtpn", retirePending);
  prefs.putBytes("dexsh", dexShinyReg, sizeof(dexShinyReg));
  prefs.putUInt("age", ageMinutes);
  prefs.putShort("dexn", speciesId);
  prefs.putShort("eggT2", eggTarget);
  prefs.putUChar("crack", eggTaps);
  prefs.putUChar("mist", careMistakes);
  prefs.putBool("sleep", sleeping);
  prefs.putUChar("lend", lastEnd);
  if (lastSeenEpoch) prefs.putUInt("seen", lastSeenEpoch);
  prefs.putBytes("dexreg", dexReg, sizeof(dexReg));
  prefs.putUShort("strk", streak);
  prefs.putUShort("bstrk", bestStreak);
  prefs.putUInt("cday", lastCareDay);
  prefs.putUInt("stps", stepCount);
  prefs.putUInt("sday", stepDay);
  prefs.putUChar("bond", bond);
  prefs.putUShort("medal", medals);
  prefs.putUShort("tmedal", totalMedals);
  prefs.putUShort("mstone", lastMilestone);
  prefs.putUShort("ghi", gameHi);
  prefs.putUShort("shi", strHi);
  prefs.putUShort("qhi", spdHi);
  prefs.putString("nick", nick);
  // Fun Pack: new keys only, so an older save loads healthy and with no records
  prefs.putBytes("mnhi", miniHi, sizeof(miniHi));
  prefs.putUChar("sick", sickness);
  prefs.putUChar("sdose", sickDoses);
  prefs.putUInt("sdmin", lastDoseMinute);
  prefs.putUShort("stick", sickTicks);
  prefs.putUChar("ovf", overfeed);
  prefs.putUChar("dirt", dirtyTicks);
  prefs.putBool("lon", lightOn);
  prefs.putUChar("lont", lightOnTicks);
  prefs.putBool("grmp", grumpy);
  prefs.putUShort("bord", boredTicks);
  // coins and missions
  prefs.putUInt("coin", coins);
  prefs.putUShort("cntd", coinsToday);
  prefs.putUInt("cnday", coinDay);
  prefs.putUChar("stck", stepCoinK);
  prefs.putBytes("msprg", missionProg, sizeof(missionProg));
  prefs.putUChar("msdon", missionDone);
  prefs.putUInt("bedd", bedtimeDay);
  // hats
  prefs.putUShort("hats", hatsOwned);
  prefs.putChar("hton", hatOn);
  prefs.putChar("htdx", hatDx);
  prefs.putChar("htdy", hatDy);
  prefs.putShort("htdex", hatDex);
  // toys and decorations
  prefs.putUShort("deco", decorOwned);
  prefs.putChar("dslL", slotL);
  prefs.putChar("dslR", slotR);
  prefs.putBytes("food", foodCount, sizeof(foodCount));
  prefs.putUChar("soap", soapCount);
  prefs.putBool("brsh", brushOwned);
  prefs.putUInt("napd", napEnd);
  prefs.putUInt("pkday", parkDay);
  prefs.putUChar("pkfnd", parkFinds);
  prefs.putUInt("ctday", centerDay);
  prefs.putUChar("seed", seedCount);
  prefs.putUInt("plant", plantEpoch);
  prefs.putUInt("gift", giftDay);
  prefs.putUInt("capd", capDay);
  prefs.putUChar("capn", capSpins);
  prefs.putUChar("scnO", sceneryOwned);
  prefs.putChar("scnP", sceneryPick);
  prefs.putBytes("tly", tally, sizeof(tally));
  prefs.putUInt("cnern", coinsEarned);
  prefs.putUShort("trph", trophies);
  prefs.putUInt("tthd", teethDay);
  prefs.putUChar("tthn", teethToday);
  prefs.putUInt("stdd", studyDay);
  prefs.putUChar("stdn", studyToday);
  prefs.putUChar("qzhi", quizHi);
  prefs.putUChar("pbed", parentBed);
  prefs.putUChar("pgam", parentGames);
  prefs.putUChar("gmtd", gamesToday);
  prefs.putUChar("advR", advRoute);
  prefs.putUChar("advN", advStop);
  prefs.putUInt("advS", advSteps);
  prefs.putUInt("fshd", fishDay);
  prefs.putUChar("fshn", fishToday);
  prefs.putBool("alOn", alarmOn);
  prefs.putUChar("alH", alarmH);
  prefs.putUChar("alM", alarmM);
  prefs.putUInt("alD", alarmDay);
  prefs.putUInt("brD", bedRemDay);
  prefs.putUInt("chrD", choreDay);
  prefs.putUChar("chrB", choresDone);
  prefs.putUInt("pig", piggy);
  prefs.putUInt("pigD", piggyDay);
  prefs.putBytes("diary", diary, sizeof(diary));
  prefs.putUInt("dcb", diaryCoinBase);
}

void Pet::load() {
  fullness = prefs.getUChar("full", 80);
  joy = prefs.getUChar("joy", 80);
  energy = prefs.getUChar("ene", 80);
  hygiene = prefs.getUChar("hyg", 100);
  poops = prefs.getUChar("poop", 0);
  weight = prefs.getUChar("wgt", 0);
  if (prefs.isKey("ivat")) {
    ivAtk = prefs.getUChar("ivat", 16);
    ivDef = prefs.getUChar("ivdf", 16);
    ivSpe = prefs.getUChar("ivsp", 16);
    ivHp = prefs.getUChar("ivhp", 16);
  } else {
    // migration from the genes (90-110%) to IVs (8-31) preserving relative
    // quality: whoever had a top gene keeps a top IV. The vitality IV
    // did not exist, it is rolled now.
    ivAtk = ivFromGene(prefs.getUChar("gatk", 0));
    ivDef = ivFromGene(prefs.getUChar("gdef", 0));
    ivSpe = ivFromGene(prefs.getUChar("gspe", 0));
    ivHp = rollIV(0);
  }
  trAtk = prefs.getUChar("tatk", 0);
  trDef = prefs.getUChar("tdef", 0);
  trSpe = prefs.getUChar("tspe", 0);
  // an old save may carry training above the new ceiling
  if (trAtk > trMaxAtk()) trAtk = trMaxAtk();
  if (trDef > trMaxDef()) trDef = trMaxDef();
  if (trSpe > trMaxSpe()) trSpe = trMaxSpe();
  berryKnown = prefs.getBool("bk", false);
  shiny = prefs.getBool("shy", false);
  eggShiny = prefs.getBool("eshy", false);
  starterPick = prefs.getBool("stpk", false);
  evoPen = prefs.getUChar("evop", 0);
  sleepAuto = prefs.getUChar("slpa", SLEEP_NONE);
  retirePending = prefs.getBool("rtpn", false);
  loadBlob(prefs, "dexsh", dexShinyReg, sizeof(dexShinyReg));
  ageMinutes = prefs.getUInt("age", 0);
  if (prefs.isKey("dexn")) {
    speciesId = prefs.getShort("dexn", -1);
    eggTarget = prefs.getShort("eggT2", 4);
  } else {
    // migration from the version with flash indices (0-8)
    static const uint8_t OLD2DEX[9] = { 4, 5, 6, 1, 2, 3, 7, 8, 9 };
    int8_t old = prefs.getChar("spec", -1);
    speciesId = (old >= 0 && old < 9) ? OLD2DEX[old] : -1;
    int8_t oldT = prefs.getChar("eggT", 0);
    eggTarget = (oldT >= 0 && oldT < 9) ? OLD2DEX[oldT] : 4;
  }
  eggTaps = prefs.getUChar("crack", 0);
  careMistakes = prefs.getUChar("mist", 0);
  sleeping = prefs.getBool("sleep", false);
  lastEnd = prefs.getUChar("lend", CER_NONE);
  loadBlob(prefs, "dexreg", dexReg, sizeof(dexReg));
  streak = prefs.getUShort("strk", 0);
  bestStreak = prefs.getUShort("bstrk", 0);
  lastCareDay = prefs.getUInt("cday", 0);
  stepCount = prefs.getUInt("stps", 0);
  stepDay = prefs.getUInt("sday", 0);
  bond = prefs.getUChar("bond", 0);
  medals = prefs.getUShort("medal", 0);
  totalMedals = prefs.getUShort("tmedal", 0);
  lastMilestone = prefs.getUShort("mstone", 0);
  gameHi = prefs.getUShort("ghi", 0);
  strHi = prefs.getUShort("shi", 0);
  spdHi = prefs.getUShort("qhi", 0);
  prefs.getString("nick", nick, sizeof(nick));
  // Moves load last: relearnFromLevel() needs speciesId and ageMinutes, both of
  // which are read above. A save from before moves existed has no "mvs" key and
  // leaves the array zeroed, so an established pet is handed the moveset it
  // should already have rather than walking into a battle knowing nothing.
  loadBlob(prefs, "mvs", moves, sizeof(moves));
  for (int i = 0; i < MOVE_SLOTS; i++)
    if (moves[i] >= MOVE_COUNT) moves[i] = 0;   // never index MOVE_TBL with junk
  lastLearnLevel = prefs.getUChar("mvlv", 0);
  frozen = prefs.getBool("froz", false);
  avatar = prefs.getUChar("avtr", 0);
  // Absent on a Kanto-only save, which leaves both arrays zeroed -- exactly
  // "no Johto or Hoenn badges yet".
  loadBlob(prefs, "badgX", badgesX, sizeof(badgesX));
  loadBlob(prefs, "badhX", badgesHardX, sizeof(badgesHardX));
  region = prefs.getUChar("reg", REGION_ALL);
  if (region >= REGION_COUNT) region = REGION_ALL;
  // A save from the Kanto-only build has neither key; getBytes leaves the
  // array at its zeroed initialiser, which is exactly "nothing remembered".
  loadBlob(prefs, "eggR", eggByRegion, sizeof(eggByRegion));
  prefs.getString("tnam", trainerName, sizeof(trainerName));
  prefs.getString("onam", ownerName, sizeof(ownerName));
  if (avatar >= AVATAR_COUNT) avatar = 0;   // a save from when there were four
  badges = prefs.getUShort("badg", 0);
  badgesHard = prefs.getUShort("badh", 0);
  if (!isEgg() && moveCount() == 0 && lastLearnLevel == 0) {
    // save from before moves existed: hand it the set it should already have
    // rather than a queue of every gate it ever passed
    relearnFromLevel();
    lastLearnLevel = level();
  }
  learnQCount = 0;      // rebuilt from lastLearnLevel by the next tick
  checkLearnGates();
  // Fun Pack
  memset(miniHi, 0, sizeof(miniHi));
  if (prefs.getBytesLength("mnhi") == sizeof(miniHi)) prefs.getBytes("mnhi", miniHi, sizeof(miniHi));
  sickness = prefs.getUChar("sick", SICK_NONE);
  if (sickness > SICK_COLD) sickness = SICK_NONE;
  sickDoses = prefs.getUChar("sdose", 0);
  lastDoseMinute = prefs.getUInt("sdmin", 0);
  sickTicks = prefs.getUShort("stick", 0);
  overfeed = prefs.getUChar("ovf", 0);
  dirtyTicks = prefs.getUChar("dirt", 0);
  lightOn = prefs.getBool("lon", false);
  lightOnTicks = prefs.getUChar("lont", 0);
  grumpy = prefs.getBool("grmp", false);
  boredTicks = prefs.getUShort("bord", 0);
  coins = prefs.getUInt("coin", 0);
  if (coins > COIN_MAX) coins = COIN_MAX;
  coinsToday = prefs.getUShort("cntd", 0);
  coinDay = prefs.getUInt("cnday", 0);
  stepCoinK = prefs.getUChar("stck", 0);
  memset(missionProg, 0, sizeof(missionProg));
  if (prefs.getBytesLength("msprg") == sizeof(missionProg)) prefs.getBytes("msprg", missionProg, sizeof(missionProg));
  missionDone = prefs.getUChar("msdon", 0);
  bedtimeDay = prefs.getUInt("bedd", 0);
  hatsOwned = prefs.getUShort("hats", 0) & ((1 << HAT_COUNT) - 1);
  hatOn = prefs.getChar("hton", -1);
  if (hatOn >= HAT_COUNT || (hatOn >= 0 && !ownsHat((uint8_t)hatOn))) hatOn = -1;
  hatDx = prefs.getChar("htdx", 0);
  hatDy = prefs.getChar("htdy", 0);
  hatDex = prefs.getShort("htdex", 0);
  decorOwned = prefs.getUShort("deco", 0) & ((1 << DEC_COUNT) - 1);
  slotL = prefs.getChar("dslL", -1);
  slotR = prefs.getChar("dslR", -1);
  if (slotL >= DEC_COUNT || (slotL >= 0 && !ownsDecor((uint8_t)slotL))) slotL = -1;
  if (slotR >= DEC_COUNT || (slotR >= 0 && !ownsDecor((uint8_t)slotR))) slotR = -1;
  memset(foodCount, 0, sizeof(foodCount));
  {   // shorter on a save from before a food was appended: the rest stay 0
    size_t n = prefs.getBytesLength("food");
    if (n) prefs.getBytes("food", foodCount, n < sizeof(foodCount) ? n : sizeof(foodCount));
    for (int i = 0; i < FOOD_COUNT; i++) if (foodCount[i] > FOOD_STACK_MAX) foodCount[i] = FOOD_STACK_MAX;
  }
  soapCount = prefs.getUChar("soap", 0);
  if (soapCount > FOOD_STACK_MAX) soapCount = FOOD_STACK_MAX;
  brushOwned = prefs.getBool("brsh", false);
  napEnd = prefs.getUInt("napd", 0);
  parkDay = prefs.getUInt("pkday", 0);
  parkFinds = prefs.getUChar("pkfnd", 0);
  centerDay = prefs.getUInt("ctday", 0);
  seedCount = prefs.getUChar("seed", 0);
  if (seedCount > FOOD_STACK_MAX) seedCount = FOOD_STACK_MAX;
  plantEpoch = prefs.getUInt("plant", 0);
  giftDay = prefs.getUInt("gift", 0);
  capDay = prefs.getUInt("capd", 0);
  capSpins = prefs.getUChar("capn", 0);
  sceneryOwned = prefs.getUChar("scnO", 0) & ((1 << SCENERY_COUNT) - 1);
  sceneryPick = prefs.getChar("scnP", -1);
  if (sceneryPick >= SCENERY_COUNT) sceneryPick = -1;
  {
    size_t n = prefs.getBytesLength("tly");   // shorter from an older build: the rest stay 0
    if (n) prefs.getBytes("tly", tally, n < sizeof(tally) ? n : sizeof(tally));
  }
  coinsEarned = prefs.getUInt("cnern", 0);
  trophies = prefs.getUShort("trph", 0) & ((1 << TRO_COUNT) - 1);
  teethDay = prefs.getUInt("tthd", 0);
  teethToday = prefs.getUChar("tthn", 0);
  studyDay = prefs.getUInt("stdd", 0);
  studyToday = prefs.getUChar("stdn", 0);
  quizHi = prefs.getUChar("qzhi", 0);
  parentBed = prefs.getUChar("pbed", 0);
  if (parentBed && (parentBed < PARENT_BED_MIN || parentBed > PARENT_BED_MAX)) parentBed = 0;
  parentGames = prefs.getUChar("pgam", 0);
  gamesToday = prefs.getUChar("gmtd", 0);
  advRoute = prefs.getUChar("advR", 0) % ROUTE_COUNT;
  advStop = prefs.getUChar("advN", 0);
  if (advStop > ROUTE_STOPS) advStop = 0;
  advSteps = prefs.getUInt("advS", 0);
  fishDay = prefs.getUInt("fshd", 0);
  fishToday = prefs.getUChar("fshn", 0);
  alarmOn = prefs.getBool("alOn", false);
  alarmH = prefs.getUChar("alH", 7) % 24;
  alarmM = prefs.getUChar("alM", 0) % 60;
  alarmDay = prefs.getUInt("alD", 0);
  bedRemDay = prefs.getUInt("brD", 0);
  choreDay = prefs.getUInt("chrD", 0);
  choresDone = prefs.getUChar("chrB", 0);
  piggy = prefs.getUInt("pig", 0);
  piggyDay = prefs.getUInt("pigD", 0);
  if (prefs.getBytesLength("diary") == sizeof(diary)) prefs.getBytes("diary", diary, sizeof(diary));
  diaryCoinBase = prefs.getUInt("dcb", 0);
  // seeding: the current pet counts as raised (old saves)
  if (speciesId >= 1) registerSpecies(speciesId);
}
