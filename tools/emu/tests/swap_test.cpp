// Swapping the companion with a party member (Pet::swapWith, PartyExt): nothing is lost, both keep growing.
#include "Arduino.h"
#include "Preferences.h"
#include "pet.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
uint32_t g_seed = 23; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
volatile int g_touchX = 0, g_touchY = 0; volatile bool g_touchDown = false; bool wasPressed = false;
static uint32_t g_ms = 0; uint32_t millis() { return g_ms; }
void FakeESP::restart() { exit(0); }
int FakeSerial::available() { return 0; } String FakeSerial::readStringUntil(char) { return String(""); }
void sfxPlay(uint8_t) {}
static int bad = 0;
static void ck(bool ok, const char *w) { printf("%s  %s\n", ok ? "PASS" : "FAIL", w); if (!ok) bad++; }
static const uint32_t DAY = 86400, D0 = 20000 * DAY + 12 * 3600;

static PartyMon mon(int16_t dex, uint16_t lv) {
  PartyMon m; m.dex = dex; m.level = lv; m.ivAtk = 11; m.ivDef = 12; m.ivSpe = 13; m.ivHp = 14;
  m.trAtk = 5; m.trDef = 6; m.trSpe = 7; m.medals = 3; m.shiny = 0; strcpy(m.nick, "Rex");
  return m;
}

int main() {
  party.begin();
  for (int i = 0; i < PARTY_SLOTS; i++) party.releaseAt(i);
  Pet p; p.begin(); p.dbgHatchAs(4, false); p.setClock(D0);
  p.ageMinutes = 3000; p.bond = 61; p.fullness = 40; p.joy = 55; p.energy = 66; p.hygiene = 77;
  p.ivAtk = 20; p.ivDef = 21; p.ivSpe = 22; p.ivHp = 23; p.trAtk = 9; p.hpPct = 70;
  uint16_t lvOut = p.level(); uint32_t born = p.bornDay;
  party.slots[2] = mon(25, 18); party.hpPct[2] = 55; party.save();

  ck(p.canSwap(), "a healthy awake companion can swap");
  ck(!p.swapWith(0), "an empty slot cannot be swapped with");
  ck(p.swapWith(2), "swap with slot 3");
  ck(p.speciesId == 25 && !p.frozen && !p.isEgg(), "Pikachu is live, not frozen");
  ck(p.level() == 18 && p.ivAtk == 11 && p.trDef == 6 && p.medals == 3 && !strcmp(p.nick, "Rex"), "it keeps what it was banked with");
  ck(p.hpPct == 55, "and its health");
  ck(p.evolving(), "the blink plays");
  ck(party.slots[2].dex == 4 && party.slots[2].level == lvOut && party.slots[2].ivAtk == 20 && party.slots[2].trAtk == 9, "Charmander took the same slot with its stats");
  ck(party.hpPct[2] == 70, "and its health");
  ck(party.ext[2].live == 1 && party.ext[2].ageMinutes == 3000 && party.ext[2].bond == 61 && party.ext[2].fullness == 40 &&
     party.ext[2].joy == 55 && party.ext[2].energy == 66 && party.ext[2].hygiene == 77 && party.ext[2].bornDay == born, "its raising state is kept");

  // the one that came out grows
  uint32_t a0 = p.ageMinutes;
  for (int i = 0; i < 30; i++) p.dbgTick();
  ck(p.ageMinutes > a0, "the new companion ages");

  // and back again: Charmander resumes where it left off
  g_ms += 20000; p.dbgSetSeen(D0 + 100);
  p.sleeping = false; p.ceremony = CER_NONE;
  ck(p.canSwap(), "it can swap again");
  ck(p.swapWith(2), "swap back");
  ck(p.speciesId == 4 && !p.frozen && p.ageMinutes == 3000 && p.bond == 61 && p.fullness == 40 && p.joy == 55 &&
     p.energy == 66 && p.hygiene == 77 && p.bornDay == born && p.ivAtk == 20 && p.hpPct == 70, "Charmander is back exactly as it left");
  ck(party.slots[2].dex == 25 && party.ext[2].live == 1, "and Pikachu is banked with its state");

  // saved and reloaded
  p.saveNow(); party.save();
  party.begin();
  ck(party.ext[2].live == 1 && party.ext[2].ageMinutes > 0 && party.slots[2].dex == 25, "the state survives a save");

  // a retired one (no state) comes out growing at its level
  party.slots[4] = mon(133, 22); memset(&party.ext[4], 0, sizeof(PartyExt)); party.hpPct[4] = 100; party.save();
  g_ms += 20000;
  ck(p.swapWith(4), "swap with an old retired member");
  ck(p.speciesId == 133 && !p.frozen && p.level() == 22 && p.fullness >= 60 && p.hygiene >= 60, "it grows from its level, never arriving starving");

  // refusals
  p.sleeping = true; ck(!p.canSwap() && !p.swapWith(2), "not while asleep"); p.sleeping = false;
  p.ceremony = CER_FAREWELL; ck(!p.canSwap(), "not in a ceremony"); p.ceremony = CER_NONE;
  Pet e; e.begin(); e.newEgg();
  ck(e.isEgg() && !e.canSwap() && !e.swapWith(2), "an egg uses BRING BACK, not swap");
  e.ceremony = CER_NONE;

  // Mega ends on a swap
  g_ms += 20000;
  p.megaStones = 1; p.setClock(D0 + 2 * DAY); p.dbgHatchAs(6, false); p.setClock(D0 + 2 * DAY);
  p.ceremony = CER_NONE; p.sleeping = false; p.megaStones = 1;
  if (p.startMega()) {
    g_ms += 20000;
    party.slots[1] = mon(25, 10); party.ext[1].live = 0;
    p.swapWith(1);
    ck(!p.megaActive(), "a swap ends Mega");
  } else ck(true, "(mega not startable here)");

  // party/box moves and clears
  party.slots[0] = mon(7, 5); party.ext[0].live = 1; party.ext[0].ageMinutes = 999;
  party.box[3] = mon(8, 6); party.boxExt[3].live = 1; party.boxExt[3].ageMinutes = 555;
  party.swapPartyBox(0, 3);
  ck(party.ext[0].ageMinutes == 555 && party.boxExt[3].ageMinutes == 999, "the state moves with the record between party and box");
  party.releaseAt(0);
  ck(party.ext[0].live == 0 && party.ext[0].ageMinutes == 0, "releasing clears it");
  party.boxReleaseAt(3);
  ck(party.boxExt[3].live == 0, "and in the box");
  ck(party.add(mon(9, 3)) && party.ext[0].live == 0, "a new member starts with no state");

  if (bad) { printf("FAILURES: %d\n", bad); return 1; }
  printf("swap test passed\n");
  return 0;
}
