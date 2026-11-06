// Health that stays until the Pokemon Center, on the real sketch.
#include "Arduino.h"
#include "Arduino_GFX_Library.h"
#include "Preferences.h"
#include "pet.h"
#include "audio.h"
#include "party.h"
#include "moves.h"
#include "battle.h"
#include "save.h"
#include <chrono>
#include <thread>
#include <vector>

uint32_t g_seed = 0x4811;
FakeSerial Serial;
FakeESP ESP;
FakeWire Wire;
volatile int g_touchX = 0, g_touchY = 0;
volatile bool g_touchDown = false;
void FakeESP::restart() { exit(0); }
int FakeSerial::available() { return 0; }
String FakeSerial::readStringUntil(char) { return String(""); }

void setup(); void loop();
extern Pet pet;
extern Party party;
extern bool battleOpen, btlOver, btlWon, btlWild, mapOpen;
extern int8_t btlTrainer;
extern uint8_t btlMenu, btlMsgCount, btlSquadN, btlSquadAt, gymRegion, pickTrainer;
extern uint16_t squadMask;
extern Combatant btlFoe, btlYou, btlSquad[7];
extern bool btlPetIn;
extern uint32_t fpToastUntil, lastInteract;
bool startWildBattle();
void btlThrowWith(uint8_t kind, uint8_t roll);
void battleTap(int16_t x, int16_t y);
void startTrainerBattle(uint8_t idx, bool hard);
bool fitToFight();
bool pickExists(uint8_t n);

static int bad = 0;
static void ck(bool ok, const char *what) {
  printf("%s  %s\n", ok ? "PASS" : "FAIL", what);
  if (!ok) bad++;
}
static void pumpMs(int ms) {
  for (int i = 0; i < ms / 30; i++) {
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    loop();
  }
}
static void click(int x, int y) {
  g_touchX = x; g_touchY = y; g_touchDown = true;
  emuFireInterrupt();
  pumpMs(150);
  g_touchDown = false;
  emuFireInterrupt();
  pumpMs(90);
}

static PartyMon mon(int16_t dex, uint16_t lvl) {
  PartyMon m;
  m.dex = dex; m.level = lvl;
  m.ivAtk = m.ivDef = m.ivSpe = m.ivHp = 15;
  m.moves[0] = MV_TACKLE;
  return m;
}
static int pctNow(const Combatant &c) { return c.maxHp ? (int)((c.hp * 100u + c.maxHp - 1) / c.maxHp) : 0; }

static void storageTests() {
  Party p;
  for (int i = 0; i < PARTY_SLOTS; i++) p.hpPct[i] = 100;
  ck(p.hpPct[0] == 100 && p.boxHpPct[17] == 100, "everyone starts at full");
  p.add(mon(4, 10));
  p.hpPct[0] = 40;
  p.boxAdd(mon(7, 12));
  p.boxHpPct[0] = 70;
  p.swapPartyBox(0, 0);
  ck(p.slots[0].dex == 7 && p.hpPct[0] == 70 && p.box[0].dex == 4 && p.boxHpPct[0] == 40, "a Pokemon takes its health between the team and the box");
  p.swapPartyBox(0, 0);
  ck(p.hpPct[0] == 40 && p.boxHpPct[0] == 70, "and back again");
  p.releaseAt(0);
  ck(p.hpPct[0] == 100, "releasing a slot resets it");
  p.replaceAt(1, mon(25, 5)); p.hpPct[1] = 10; p.replaceAt(1, mon(26, 5));
  ck(p.hpPct[1] == 100, "replacing one resets it");
  p.add(mon(1, 5));
  p.hpPct[2] = 5; p.boxHpPct[3] = 5;
  ck(p.anyHurt(), "anyone hurt is noticed");
  p.healAll();
  ck(!p.anyHurt() && p.hpPct[2] == 100 && p.boxHpPct[3] == 100, "healing everyone covers the team and the box");

  // saved and loaded
  Party a; a.begin();
  a.releaseAt(0); a.releaseAt(1); a.releaseAt(2);
  a.add(mon(6, 20)); a.hpPct[0] = 35; a.save();
  a.boxAdd(mon(9, 20)); a.boxHpPct[0] = 55; a.boxSave();
  Party b; b.begin();
  ck(b.hpPct[0] == 35 && b.boxHpPct[0] == 55, "health is saved and loaded");
  { Preferences pr; pr.begin("tamapoke", false); pr.remove("phpp"); pr.remove("phbx"); pr.end(); }
  Party c; c.begin();
  ck(c.hpPct[0] == 100 && c.boxHpPct[0] == 100, "a save from before this loads everyone at full");
  { Preferences pr; pr.begin("tamapoke", false); uint8_t junk[PARTY_SLOTS] = { 250, 3, 3, 3, 3, 3 }; pr.putBytes("phpp", junk, sizeof(junk)); pr.end(); }
  Party d; d.begin();
  ck(d.hpPct[0] == 100 && d.hpPct[1] == 3, "a damaged value is capped at 100");
  a.releaseAt(0); a.boxReleaseAt(0);
  { Preferences pr; pr.begin("tamapoke", false); pr.remove("phpp"); pr.end(); }
}

static void centerTests() {
  Pet p; p.begin();
  p.dbgHatchAs(1, false);
  p.lastSeenEpoch = 20000u * 86400u + 12 * 3600;
  party.begin();
  for (int i = 0; i < PARTY_SLOTS; i++) party.releaseAt(i);
  for (int i = 0; i < BOX_SLOTS; i++) party.boxReleaseAt(i);
  party.add(mon(4, 10)); party.boxAdd(mon(7, 10));
  p.hpPct = 20; party.hpPct[0] = 30; party.boxHpPct[0] = 40;
  p.centerDay = 0; p.coins = 0;
  ck(p.centerHeal() == CENTER_FREE, "the first visit of the day is free");
  ck(p.hpPct == 100 && party.hpPct[0] == 100 && party.boxHpPct[0] == 100, "and heals the live Pokemon, the team and the box");

  p.hpPct = 10; party.hpPct[0] = 10;
  p.coins = 0;
  ck(p.centerHeal() == CENTER_HP_ONLY, "after the free visit, with no coins, it still heals the health");
  ck(p.hpPct == 100 && party.hpPct[0] == 100, "everyone, completely");
  ck(p.centerHeal() == CENTER_NO_COINS, "with nothing to heal and no coins, it asks for coins as before");

  p.sickness = SICK_COLD;
  p.hpPct = 50; p.coins = 0;
  ck(p.centerHeal() == CENTER_HP_ONLY && p.isSick() && p.hpPct == 100, "health without coins leaves the sickness alone");
  p.hpPct = 50; p.coins = 100;
  ck(p.centerHeal() == CENTER_PAID && !p.isSick() && p.hpPct == 100, "with coins it does everything, as it always has");
  Pet q; q.begin();
  ck(q.hpPct == 100, "and that is saved");
}

static void readyBall() {
  pet.dbgHatchAs(1, false);
  pet.ceremony = CER_NONE; pet.sleeping = false; pet.sickness = SICK_NONE; pet.poops = 0;
  pet.fullness = pet.joy = pet.energy = pet.hygiene = 80;
  pet.lastSeenEpoch = 20000u * 86400u + 12 * 3600;
  while (pet.hasLearnOffer()) pet.declineLearn();
  pet.wildBalls[0] = 20; pet.wildBalls[1] = 5;
  pet.wildFreeDay = 0; pet.wildCharges = 0;
  pet.hpPct = 100;
  for (int i = 0; i < PARTY_SLOTS; i++) party.releaseAt(i);
  for (int i = 0; i < BOX_SLOTS; i++) party.boxReleaseAt(i);
  battleOpen = false; btlOver = false; btlMenu = 0; btlMsgCount = 0;
  squadMask = 0xFFFF;
  fpToastUntil = 0;
}
static void closeBattle() { battleOpen = false; btlOver = false; btlMenu = 0; btlMsgCount = 0; }
static void runAway() {
  btlMenu = 0; btlMsgCount = 0;
  battleTap(240 + 60, 326 + 20);      // RUN
  battleTap(69 + 60, 326 + 20);       // YES
}

static void fightTests() {
  // damage stays after running away
  readyBall();
  ck(startWildBattle(), "a wild fight starts");
  btlMsgCount = 0;
  btlYou.hp = (uint16_t)(btlYou.maxHp * 50 / 100);
  runAway();
  ck(!battleOpen, "she runs");
  ck(pet.hpPct >= 49 && pet.hpPct <= 51, "and her Pokemon stays at about half health");
  Pet saved; saved.begin();
  ck(saved.hpPct == pet.hpPct, "which is saved at once");

  // next fight starts hurt
  closeBattle();
  pet.wildFreeDay = 0;
  startWildBattle();
  int pct = pctNow(btlYou);
  ck(pct >= 49 && pct <= 52, "the next fight starts with that health");
  closeBattle();

  // a Pokemon nobody touched keeps exactly what it had, with no drift
  readyBall();
  pet.hpPct = 33;
  startWildBattle();
  btlMsgCount = 0;
  runAway();
  ck(pet.hpPct == 33, "an untouched one keeps exactly its health");
  closeBattle();

  // growing a level keeps it as hurt as before
  readyBall();
  pet.hpPct = 40;
  pet.ageMinutes += 6 * MINUTES_PER_LEVEL;
  startWildBattle();
  pct = pctNow(btlYou);
  ck(pct >= 38 && pct <= 42, "after growing, it is still about 40% hurt");
  closeBattle();

  // a fainted one sits out and the next leads
  readyBall();
  party.add(mon(4, 12));
  pet.hpPct = 0;
  ck(fitToFight(), "a team member who is fine is enough");
  ck(startWildBattle() && !btlPetIn && btlSquadN == 1 && btlYou.dex == 4, "the live Pokemon sits out and the team member leads");
  closeBattle();

  // a team member's damage is kept, through a catch
  readyBall();
  party.add(mon(4, 12));
  pet.hpPct = 0;
  startWildBattle();
  btlMsgCount = 0;
  btlYou.hp = (uint16_t)(btlYou.maxHp * 30 / 100);
  btlThrowWith(0, 0);
  ck(btlOver && party.hpPct[0] >= 29 && party.hpPct[0] <= 31, "a team member's damage is kept after a catch");
  ck(party.hpPct[1] == 100, "and what it caught arrives at full health");
  closeBattle();

  // fainted in the fight: kept at 0
  readyBall();
  startWildBattle();
  btlMsgCount = 0;
  btlYou.hp = 0;
  btlThrowWith(0, 99);          // the commit happens when the fight ends: here, by a catch
  closeBattle();
  readyBall();
  pet.hpPct = 100;
  startWildBattle();
  btlMsgCount = 0;
  btlYou.hp = 0;
  btlThrowWith(0, 0);
  ck(pet.hpPct == 0, "a Pokemon that faints in a fight stays fainted");
  closeBattle();

  // nobody can fight
  readyBall();
  party.add(mon(4, 12));
  pet.hpPct = 0; party.hpPct[0] = 0;
  uint8_t before = pet.wildCount();
  ck(!fitToFight(), "nobody fit");
  ck(!startWildBattle() && !battleOpen, "a wild fight is refused");
  ck(pet.wildCount() == before, "and no encounter is used");
  ck(fpToastUntil != 0, "and it says why");
  fpToastUntil = 0;
  squadMask = 0xFFFF;
  gymRegion = 0;
  startTrainerBattle(0, false);
  ck(!battleOpen && fpToastUntil != 0, "a gym fight is refused too");
  fpToastUntil = 0;

  // some fit: the gym fight starts without the fainted one
  readyBall();
  party.add(mon(4, 12));
  pet.hpPct = 0;
  startTrainerBattle(0, false);
  ck(battleOpen && btlTrainer == 0 && btlSquadN == 1 && !btlPetIn, "a gym fight starts with whoever can fight");
  btlMsgCount = 0;
  btlYou.hp = (uint16_t)(btlYou.maxHp / 2);
  runAway();
  ck(party.hpPct[0] >= 49 && party.hpPct[0] <= 51, "and its damage stays too");
  closeBattle();

  // the picker
  extern uint8_t pickTrainer;
  readyBall();
  party.add(mon(4, 12));
  pet.hpPct = 0; party.hpPct[0] = 0;
  pickTrainer = 0;
  ck(!pickExists(0) && !pickExists(1), "the team picker does not offer a fainted Pokemon");
  pickTrainer = 0xFF;
  ck(pickExists(0) && pickExists(1), "but a friendly fight over the radio still does");
  pickTrainer = 0;

  // a switch in the fight keeps both
  readyBall();
  party.add(mon(4, 12));
  startWildBattle();
  btlMsgCount = 0;
  ck(btlSquadN == 2, "a team of two");
  btlYou.hp = (uint16_t)(btlYou.maxHp * 60 / 100);
  btlMenu = 0;
  battleTap(69 + 60 + 168, 274 + 20 + 52 - 52 + 52);   // POKEMON (the switch menu)
  btlMenu = 2;
  battleTap(69 + 168 + 60, 274 + 20);                  // the second one
  btlMsgCount = 0;
  ck(btlSquadAt == 1, "she switched");
  btlYou.hp = (uint16_t)(btlYou.maxHp * 20 / 100);
  runAway();
  ck(pet.hpPct >= 59 && pet.hpPct <= 61 && party.hpPct[0] >= 19 && party.hpPct[0] <= 21, "both keep their own damage");
  closeBattle();
}

static void saveTests() {
  readyBall();
  party.add(mon(4, 12)); party.hpPct[0] = 45;
  pet.hpPct = 66;
  party.save(); pet.saveNow();
  static uint8_t buf[8192];
  size_t n = saveExport(buf, sizeof(buf));
  ck(n > 0, "a backup is made");
  pet.hpPct = 100; party.hpPct[0] = 100;
  party.save(); pet.saveNow();
  ck(saveImport(buf, n), "and restored");
  Pet q; q.begin();
  Party r; r.begin();
  ck(q.hpPct == 66 && r.hpPct[0] == 45, "the damage comes back with the backup");
  pet.hpPct = 100;
}

int main() {
  storageTests();
  centerTests();
  setup();
  pumpMs(120);
  click(233, 108 + 30);
  pumpMs(60);
  click(233, 145);
  if (pet.awaitingStarter()) { printf("FAIL: no starter\n"); return 1; }
  pumpMs(120);
  party.begin();
  fightTests();
  saveTests();
  if (bad) { printf("FAILURES: %d\n", bad); return 1; }
  printf("hp test passed\n");
  return 0;
}
