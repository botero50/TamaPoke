// Phase 44: wild encounters and catching, on the real sketch. The rules in wild.h
// take their rolls as arguments, so each curve is pinned exactly here.
#include "Arduino.h"
#include "Arduino_GFX_Library.h"
#include "Preferences.h"
#include "pet.h"
#include "audio.h"
#include "party.h"
#include "moves.h"
#include "battle.h"
#include "wild.h"
#include "save.h"
#include <chrono>
#include <thread>

uint32_t g_seed = 0x3117D;
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
extern uint8_t btlMenu, btlMsgCount, btlMsgPage, mapPage, gymRegion;
extern Combatant btlFoe, btlYou;
extern char btlMsg[12][40];
extern PartyMon btlWildMon;
extern uint32_t fpToastUntil, lastInteract;
bool startWildBattle();
uint8_t audioMusicNow();
void btlThrowWith(uint8_t kind, uint8_t roll);
void battleTap(int16_t x, int16_t y);
void startTrainerBattle(uint8_t idx, bool hard);
int mapGridHit(int16_t x, int16_t y);

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

static bool everyone(int16_t) { return true; }
static bool noOdd(int16_t d) { return d % 2 == 0; }
static bool nobody(int16_t) { return false; }

static const uint32_t DAY = 86400, D0 = 20000 * DAY + 12 * 3600;

static void ruleTests() {
  ck(wildCaptureChance(R_COMUN, 100, 100, 100, 0) == 30, "common at full health: 30%");
  ck(wildCaptureChance(R_EVO, 100, 100, 100, 0) == 24, "evolved: 24%");
  ck(wildCaptureChance(R_RARO, 100, 100, 100, 0) == 18, "rare: 18%");
  ck(wildCaptureChance(R_LEGENDARIO, 100, 100, 100, 0) == 5, "legendary: 5%");
  ck(wildCaptureChance(R_COMUN, 50, 100, 100, 0) == 42, "half health: 1.4 times");
  ck(wildCaptureChance(R_COMUN, 1, 100, 100, 0) == 53, "nearly fainted: about 1.8 times");
  ck(wildCaptureChance(R_COMUN, 100, 100, 150, 0) == 45, "a Great Ball: 1.5 times");
  ck(wildCaptureChance(R_COMUN, 100, 100, 200, 0) == 60, "an Ultra Ball: 2 times");
  ck(wildCaptureChance(R_COMUN, 1, 100, 200, 0) == 95, "never above 95%");
  ck(wildCaptureChance(R_LEGENDARIO, 1, 100, 200, 8) == 25, "a legendary stays hard, even at the best");
  ck(wildCaptureChance(R_COMUN, 100, 100, 100, 8) == 38, "each rank adds 1%");
  ck(wildCaptureChance(R_COMUN, 0, 100, 100, 0) == 0 && wildCaptureChance(R_COMUN, 5, 0, 100, 0) == 0, "no health or no maximum: 0");
  ck(wildCaptureChance(R_COMUN, 200, 100, 100, 0) == 30, "health over the maximum is safe");

  ck(wildShinyForRoll(0, 0) && !wildShinyForRoll(1, 0), "a beginner's shiny is 1 in 4096");
  ck(wildShinyForRoll(8, 8) && !wildShinyForRoll(9, 8), "a legend's is 9 in 4096, about 1 in 455");
  ck(wildShinyForRoll(4096, 0), "the roll wraps at 4096");

  ck(wildLevelMin(20) == 15 && wildLevelMax(20, 100) == 25, "level 20 meets 15 to 25");
  ck(wildLevelMin(3) == 1 && wildLevelMin(5) == 1 && wildLevelMin(6) == 1, "never under level 1");
  ck(wildLevelMax(98, 100) == 100, "never over the maximum");

  ck(wildTierForRoll(0) == R_LEGENDARIO, "roll 0: legendary (1%)");
  ck(wildTierForRoll(1) == R_RARO && wildTierForRoll(7) == R_RARO, "rolls 1 to 7: rare (7%)");
  ck(wildTierForRoll(8) == R_EVO && wildTierForRoll(29) == R_EVO, "rolls 8 to 29: evolved (22%)");
  ck(wildTierForRoll(30) == R_COMUN && wildTierForRoll(99) == R_COMUN, "the rest: common (70%)");

  // every region has someone to meet
  bool allFound = true, inRegion = true, rightTier = true;
  for (uint8_t r = 0; r < REGION_COUNT; r++) {
    if (r == REGION_ALL) continue;
    for (uint32_t roll : { 0u, 7u, 1000u, 99999u }) {
      int16_t d = wildPickSpecies(REGIONS[r].lo, REGIONS[r].hi, R_COMUN, roll, everyone);
      if (!d) { allFound = false; continue; }
      if (d < (int16_t)REGIONS[r].lo || d > (int16_t)REGIONS[r].hi) inRegion = false;
      if (DEX_TBL[d].rarity != R_COMUN) rightTier = false;
    }
  }
  ck(allFound, "every region has common Pokemon");
  ck(inRegion, "and the pick stays in its region");
  ck(rightTier, "and of the rarity asked for");
  bool skip = true;
  for (uint32_t roll = 0; roll < 300; roll++)
    if (wildPickSpecies(1, 151, R_COMUN, roll, noOdd) % 2) skip = false;
  ck(skip, "a species without sprites is never picked");
  ck(wildPickSpecies(1, 151, R_COMUN, 5, nobody) == 0, "nobody available: no encounter");
  ck(wildPickSpecies(1, 9999, R_COMUN, 5, everyone) > 0, "a range past the Pokedex is safe");
}

static void fresh(Pet &p) {
  p.begin();
  p.dbgHatchAs(1, false);
  p.lastSeenEpoch = D0;
  p.wildCharges = 0; p.wildStepBank = 0; p.wildFreeDay = 0; p.wildEarned = false;
  p.stepDay = 0; p.stepCount = 0;
}

static void encounterTests() {
  { Pet p; fresh(p);
    ck(p.wildFreeLeft() && p.wildCount() == 1, "one free encounter to start with");
    ck(p.wildSpend() && p.wildCount() == 0, "using it leaves none");
    ck(!p.wildSpend(), "and with none there is nothing to use");
    p.lastSeenEpoch = D0 + DAY;
    ck(p.wildCount() == 1, "the next day it is back");
    p.wildSpend();
    p.addSteps(499);
    ck(p.wildCount() == 0 && !p.wildEarned, "499 steps: not yet");
    ck(p.wildStepsLeft() == 1, "and it knows how far is left");
    p.addSteps(1);
    ck(p.wildCharges == 1 && p.wildEarned, "500 steps earn one, and it is announced");
    p.addSteps(5000);
    ck(p.wildCharges == 3, "steps hold at most 3");
    p.lastSeenEpoch = D0 + 2 * DAY;
    ck(p.wildCount() == 4, "the free one comes on top");
    p.wildSpend();
    ck(p.wildCharges == 3 && p.wildCount() == 3, "and it is used first");
    p.wildSpend();
    ck(p.wildCharges == 2, "then the walked ones");
  }
  { Pet p; fresh(p);        // an unplug
    p.addSteps(620);
    p.saveSteps();
    Pet q; q.begin();
    q.lastSeenEpoch = D0;
    ck(q.wildCharges == 1 && q.wildStepBank == 120, "an unplug neither loses nor repeats what was walked");
  }
  { Pet p; fresh(p);        // the clock is 0 on a board that lost it
    p.lastSeenEpoch = 0;
    p.wildFreeDay = 0;
    ck(p.wildFreeLeft(), "with no clock the free one still works");
    p.wildSpend();
    ck(!p.wildFreeLeft(), "and is still used up");
  }
  { Pet p; fresh(p);        // the starter balls, once
    { Preferences pr; pr.begin("tamapoke", false); pr.remove("wkit"); pr.remove("wbal"); pr.end(); }
    Pet q; q.begin();
    ck(q.wildBalls[0] == WILD_STARTER_BALLS && q.wildKit == 1, "a save without them gets 5 Poke Balls");
    q.wildBalls[0] = 2; q.lastSeenEpoch = D0; q.saveSteps();
    q.coins = 100;
    ck(q.buyBall(0) && q.wildBalls[0] == 3, "buying adds a ball");
    Pet r; r.begin();
    ck(r.wildKit == 1 && r.wildBalls[0] == 3, "and a later boot gives no more");
  }
  { Pet p; fresh(p);        // prices
    p.coins = 50;
    uint32_t c = p.coins;
    p.buyBall(0); ck(c - p.coins == 5, "a Poke Ball costs 5"); c = p.coins;
    p.buyBall(1); ck(c - p.coins == 15, "a Great Ball costs 15"); c = p.coins;
    p.buyBall(2); ck(c - p.coins == 30, "an Ultra Ball costs 30");
    p.coins = 1;
    uint8_t b = p.wildBalls[0];
    ck(!p.buyBall(0) && p.wildBalls[0] == b && p.coins == 1, "no coins, no ball");
    p.coins = 1000; p.wildBalls[0] = BALL_STACK_MAX;
    ck(!p.buyBall(0) && p.wildBalls[0] == BALL_STACK_MAX, "and a stack stops at 99");
    ck(!p.useBall(1 + 5), "an unknown kind is refused");
  }
}

static void readyBall() {
  pet.dbgHatchAs(1, false);
  pet.ceremony = CER_NONE;
  pet.sleeping = false;
  pet.fullness = pet.joy = pet.energy = pet.hygiene = 80;
  pet.sickness = SICK_NONE;
  pet.poops = 0;
  while (pet.hasLearnOffer()) pet.declineLearn();
  pet.wildBalls[0] = 10; pet.wildBalls[1] = 2; pet.wildBalls[2] = 0;
  pet.wildFreeDay = 0; pet.wildCharges = 0;
  battleOpen = false;
  fpToastUntil = 0;
}
static void closeBattle() { battleOpen = false; btlOver = false; btlMenu = 0; btlMsgCount = 0; }

static void flowTests() {
  // blocked: nothing is used
  readyBall();
  pet.sleeping = true;
  ck(!startWildBattle() && pet.wildCount() == 1 && !battleOpen, "asleep: no encounter, nothing used");
  ck(fpToastUntil != 0, "and it says why");
  readyBall();
  pet.sickness = SICK_COLD;
  ck(!startWildBattle() && pet.wildCount() == 1, "too sick: the same");
  readyBall();
  pet.wildSpend();
  ck(!startWildBattle() && !battleOpen && fpToastUntil != 0, "none left: it says how far to walk");

  // a fight
  readyBall();
  ck(startWildBattle() && battleOpen && btlWild && btlTrainer == -1, "the tile starts a wild fight");
  ck(pet.wildCount() == 0, "and uses the encounter");
  uint8_t lv = pet.level();
  ck(btlFoe.level >= wildLevelMin(lv) && btlFoe.level <= wildLevelMax(lv, MAX_LEVEL), "the foe is near her level");
  ck(btlMsgCount >= 1, "it announces the foe");
  ck(btlWildMon.dex == btlFoe.dex && btlWildMon.level == btlFoe.level, "a catch would add this foe");
  bool anyMove = false;
  for (int i = 0; i < MOVE_SLOTS; i++) if (btlWildMon.moves[i]) anyMove = true;
  ck(anyMove, "with its moves");
  btlMsgCount = 0;

  // through the real menu
  btlMenu = 0;
  battleTap(240 + 60, 274 + 20);
  ck(btlMenu == 4, "BALLS opens the ball menu");
  pet.wildBalls[2] = 0;
  uint8_t pk = pet.wildBalls[0];
  battleTap(69 + 60, 326 + 20);    // the third cell: Ultra, none left
  ck(btlMenu == 4 && pet.wildBalls[0] == pk, "a kind she has none of does nothing");
  battleTap(233, 400);             // BACK
  ck(btlMenu == 0, "BACK returns");
  battleTap(69 + 60, 274 + 20);
  ck(btlMenu == 1, "FIGHT still opens the moves");
  btlMenu = 0;

  // a miss
  uint16_t foeHp = btlFoe.hp;
  pk = pet.wildBalls[0];
  btlThrowWith(0, 99);
  ck(pet.wildBalls[0] == pk - 1, "a throw uses the ball");
  ck(!btlOver || btlYou.fainted(), "a miss leaves the fight going");
  ck(btlMsgCount >= 2, "and the foe takes its turn");
  ck(btlFoe.hp == foeHp, "which the throw did not hurt");
  btlMsgCount = 0;

  // a catch, to the team
  btlYou.hp = btlYou.maxHp;
  uint8_t team = party.count(), boxN = party.boxCount();
  int16_t dex = btlFoe.dex;
  pet.dexReg[(dex - 1) >> 3] &= ~(1 << ((dex - 1) & 7));
  btlThrowWith(0, 0);
  ck(btlOver && btlWon, "a catch ends the fight");
  ck(party.count() == team + 1 && party.boxCount() == boxN, "it joins the team");
  ck(pet.isRegistered(dex), "and the Pokedex");
  ck(audioMusicNow() == MUS_VICTORY, "the win tune plays");
  battleTap(233, 330);
  while (btlMsgCount && battleOpen) battleTap(233, 330);
  ck(!battleOpen && audioMusicNow() == MUS_NONE, "tapping out of a won fight stops the music");
  closeBattle();

  // team full: to the box, and shiny in the Pokedex
  readyBall();
  startWildBattle();
  while (!party.isFull()) { PartyMon m; m.dex = 1; m.level = 5; party.add(m); }
  boxN = party.boxCount();
  btlWildMon.shiny = 1;
  dex = btlWildMon.dex;
  pet.dexShinyReg[(dex - 1) >> 3] &= ~(1 << ((dex - 1) & 7));
  btlMsgCount = 0;
  btlThrowWith(1, 0);
  ck(party.boxCount() == boxN + 1, "a full team sends it to the box");
  ck(btlMsgCount == 2, "and it says so");
  ck(pet.isShinyRegistered(dex), "a shiny registers as shiny");
  closeBattle();

  // both full: the ball is safe
  readyBall();
  startWildBattle();
  while (party.boxCount() < BOX_SLOTS) { PartyMon m; m.dex = 1; m.level = 5; party.boxAdd(m); }
  pk = pet.wildBalls[0];
  btlMsgCount = 0;
  btlThrowWith(0, 0);
  ck(pet.wildBalls[0] == pk && !btlOver && btlMsgCount == 1, "team and box full: the ball is not spent");
  closeBattle();

  // winning it pays no coins, and says so
  readyBall();
  startWildBattle();
  btlMsgCount = 0;
  btlFoe.hp = 1;
  uint32_t coins = pet.coins;
  uint8_t mv = 0;
  for (int i = 0; i < MOVE_SLOTS; i++) if (btlYou.moves[i]) { mv = (uint8_t)i; break; }
  btlMenu = 1;
  for (int tries = 0; tries < 30 && !btlOver; tries++) {
    btlYou.hp = btlYou.maxHp; btlFoe.hp = 1; btlMsgCount = 0; btlMenu = 1;
    battleTap(69 + 60 + (mv % 2) * 168, 274 + 20 + (mv / 2) * 52);
  }
  ck(btlOver && btlWon, "a win ends the fight");
  ck(pet.coins == coins, "with no coins");
  closeBattle();

  // a foe always has something to attack with, even at the lowest levels
  {
    bool allAttack = true;
    int fights = 0;
    uint8_t keepAge = 0;
    (void)keepAge;
    for (uint8_t r = 0; r < 7; r++) {
      pet.region = r;
      for (int i = 0; i < 40; i++) {
        readyBall();
        pet.wildFreeDay = 0;
        if (!startWildBattle()) continue;
        fights++;
        bool attacks = false;
        for (int k = 0; k < MOVE_SLOTS; k++)
          if (btlFoe.moves[k] && MOVE_TBL[btlFoe.moves[k]].cat != MC_STATUS) attacks = true;
        if (!attacks) allAttack = false;
        closeBattle();
      }
    }
    pet.region = 0;
    ck(fights > 200 && allAttack, "every wild foe has an attack, across all 7 regions");
  }

  // a long turn is shown in full: 4 lines a page, and a tap turns the page
  {
    readyBall();
    startWildBattle();
    btlMsgCount = 0; btlMsgPage = 0;
    for (int i = 0; i < 9; i++) { snprintf(btlMsg[i], sizeof(btlMsg[0]), "line %d", i); btlMsgCount++; }
    battleTap(233, 300);
    ck(btlMsgCount == 9 && btlMsgPage == 1, "a tap turns to the second page, and keeps the lines");
    battleTap(233, 300);
    ck(btlMsgCount == 9 && btlMsgPage == 2, "and the third");
    battleTap(233, 300);
    ck(btlMsgCount == 0 && btlMsgPage == 0, "the last page clears the narration, and starts over");
    btlMsgCount = 3; btlMsgPage = 0;
    battleTap(233, 300);
    ck(btlMsgCount == 0, "a short turn still clears on one tap");
    closeBattle();
  }

  // gym fights keep the plain menu
  readyBall();
  gymRegion = 0;
  startTrainerBattle(0, false);
  ck(!btlWild, "a gym fight is not wild");
  btlMsgCount = 0; btlMenu = 0;
  battleTap(240 + 60, 274 + 20);
  ck(btlMenu == 1, "there the whole top row is FIGHT");
  closeBattle();

  // the tile
  readyBall();
  mapPage = 1;
  mapOpen = true;
  lastInteract = millis();
  click(233 + 4 + 74, 76 + 52 + 3 * 58 + 26);    // the last tile on page 2, through the real touch path
  ck(battleOpen && btlWild, "tapping the Tall Grass tile starts it");
  ck(!mapOpen, "and closes the map");
  closeBattle();
  pumpMs(90);
  mapPage = 0;

  // a save and restore keep the balls
  {
    readyBall();
    pet.wildBalls[0] = 7; pet.wildBalls[1] = 3; pet.wildBalls[2] = 1; pet.wildCharges = 2; pet.wildStepBank = 321;
    pet.saveSteps();
    Preferences pr; pr.begin("tamapoke", false);
    pr.putBytes("wbal", pet.wildBalls, sizeof(pet.wildBalls));
    pr.end();
    Pet q; q.begin();
    ck(q.wildBalls[0] == 7 && q.wildBalls[1] == 3 && q.wildBalls[2] == 1, "balls are saved");
    ck(q.wildCharges == 2 && q.wildStepBank == 321, "and encounters");
  }
}

int main() {
  ruleTests();
  encounterTests();
  setup();
  pumpMs(120);
  click(233, 108 + 30);
  pumpMs(60);
  click(233, 145);
  if (pet.awaitingStarter()) { printf("FAIL: no starter\n"); return 1; }
  pumpMs(120);
  flowTests();
  if (bad) { printf("FAILURES: %d\n", bad); return 1; }
  printf("wild test passed\n");
  return 0;
}
