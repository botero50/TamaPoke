// Eggs never hold a Pokemon the Pokedex already has, unless one of its evolutions is missing.
// One rule (Pet::eggRoll) serves random eggs, the region switch, the check just before an egg
// cracks, and the Nursery's eggs.
#include "Arduino.h"
#include "Preferences.h"
#include "pet.h"
#include "party.h"
#include <cstdio>
uint32_t g_seed = 53; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
volatile int g_touchX = 0, g_touchY = 0; volatile bool g_touchDown = false; bool wasPressed = false;
static uint32_t g_ms = 0;
uint32_t millis() { return g_ms; } void FakeESP::restart() { exit(0); }
int FakeSerial::available() { return 0; } String FakeSerial::readStringUntil(char) { return String(""); }
void sfxPlay(uint8_t) {}
static int bad = 0;
static void ck(bool ok, const char *w) { printf("%s  %s\n", ok ? "PASS" : "FAIL", w); if (!ok) bad++; }

static const uint32_t DAY = 86400;
static void reg(Pet &p, int d) { p.dexReg[(d - 1) >> 3] |= (uint8_t)(1 << ((d - 1) & 7)); }
static void unreg(Pet &p, int d) { p.dexReg[(d - 1) >> 3] &= (uint8_t)~(1 << ((d - 1) & 7)); }
static void fresh(Pet &p) {
  p.begin();
  p.dbgHatchAs(1, false);
  p.sleeping = false;
  p.lastSeenEpoch = 20100 * DAY + 12 * 3600;
  p.lastEnd = CER_FAREWELL;
  for (int d = 1; d <= DEX_COUNT; d++) reg(p, d);      // start from a complete Pokedex
}
// a repeat the rule allows: not in the Pokedex, or its line still has an evolution missing
static bool allowed(const Pet &p, int d) { return !p.isRegistered(d) || p.lineHasUnregistered(d); }

int main() {
  // the rule is respected by every random egg
  {
    Pet p; fresh(p);
    const int missing[3] = { 1, 4, 129 };              // Bulbasaur, Charmander, Magikarp
    for (int d : missing) unreg(p, d);
    bool ok = true; int hits = 0;
    for (int i = 0; i < 2000; i++) {
      int16_t e = p.pickEggSpecies();
      if (!allowed(p, e)) ok = false;
      for (int d : missing) if (e == d) hits++;
    }
    ck(ok, "2000 random eggs: never a species the Pokedex has with a finished line");
    ck(hits > 1500, "and nearly all of them are the few that are missing (common tier)");
  }
  // new species come before a repeat with a missing evolution
  {
    Pet p; fresh(p);
    unreg(p, 1);                                       // Bulbasaur: new, its line is finished
    unreg(p, 8);                                       // Wartortle: Squirtle (7) is a repeat with a missing evolution
    bool onlyNew = true;
    for (int i = 0; i < 400; i++) if (p.eggRoll(0, R_COMUN) != 1) onlyNew = false;
    ck(onlyNew, "a species that is new beats a repeat that merely has an evolution to find");
    reg(p, 1);                                         // nothing new is left in the tier
    bool then7 = true;
    for (int i = 0; i < 400; i++) if (p.eggRoll(0, R_COMUN) != 7) then7 = false;
    ck(then7, "with nothing new left, the repeat with a missing evolution is the egg");
  }
  // a finished Pokedex still hatches something
  {
    Pet p; fresh(p);
    bool ok = true;
    for (int i = 0; i < 300; i++) { int16_t e = p.pickEggSpecies(); if (e < 1 || e > DEX_COUNT) ok = false; }
    ck(ok, "a complete Pokedex still gives an egg");
  }
  // changing region while an egg waits follows the same rule
  {
    Pet p; fresh(p);
    unreg(p, 152);                                     // Chikorita, the one new Johto species
    uint8_t johto = regionOfDex(152);
    bool ok = true;
    for (int i = 0; i < 300; i++) if (p.rollInRegion(johto, R_RARO) != 152) ok = false;
    ck(ok, "a region switch rolls the species the Pokedex lacks, not a repeat");
  }
  // just before it cracks, an egg that has become a repeat is rolled again
  {
    Pet p; fresh(p);
    unreg(p, 1);
    p.dbgSetEgg(19, false);                            // Rattata: it and Raticate are both in the Pokedex
    p.recheckEgg();
    ck(p.eggPeek() == 1, "a repeat with a finished line becomes the species that is new");
    p.dbgSetEgg(7, false); unreg(p, 8);                // Squirtle with an evolution missing
    p.recheckEgg();
    ck(p.eggPeek() == 7 || p.eggPeek() == 1, "a repeat with a missing evolution may stay");
    Pet q; fresh(q);
    q.dbgSetEgg(19, false);
    q.recheckEgg();
    ck(q.eggPeek() == 19, "with the whole Pokedex complete an egg is left as it is");
    Pet s; fresh(s);
    unreg(s, 1);
    s.dbgSetEgg(19, true);
    s.recheckEgg();
    ck(s.eggPeek() == 19, "the first game's starter pick is never changed");
  }
  // the Nursery: a random species with the same rule, never a legendary
  {
    bool ok = true, notLeg = true; int sawNew = 0;
    for (int i = 0; i < 300; i++) {
      Pet p; fresh(p);
      unreg(p, 1); unreg(p, 4); unreg(p, 129);
      p.nursery = NurseryState();
      p.nursery.aDex = 3; p.nursery.bDex = 26;          // Venusaur and Raichu
      p.nursery.aIv[0] = 30; p.nursery.bIv[0] = 11;
      p.nursery.since = p.lastSeenEpoch - NURSERY_SECS;
      if (!p.nurseryCollect()) { ok = false; break; }
      int16_t e = p.nursery.eggDex;
      if (!allowed(p, e)) ok = false;
      if (DEX_TBL[e].rarity == R_LEGENDARIO || DEX_TBL[e].rarity == R_EVO) notLeg = false;
      if (e == 1 || e == 4 || e == 129) sawNew++;
    }
    ck(ok, "a Nursery egg follows the same rule");
    ck(notLeg, "and is a common or rare first form, never a legendary");
    ck(sawNew > 250, "and is nearly always one of the species still missing");
  }
  if (bad) { printf("FAILURES: %d\n", bad); return 1; }
  printf("egg test passed\n");
  return 0;
}
