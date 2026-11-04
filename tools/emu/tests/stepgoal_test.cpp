// Phase 43: the three daily step goals and the lifetime ranks, on the real sketch.
#include "Arduino.h"
#include "Arduino_GFX_Library.h"
#include "Preferences.h"
#include "pet.h"
#include "audio.h"
#include "party.h"
#include "moves.h"
#include "battle.h"
#include "i18n.h"
#include <chrono>
#include <thread>

uint32_t g_seed = 0x57E9;
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
extern uint32_t fpToastUntil;
extern StrId fpToast;
extern char fpToastText[48];
extern uint32_t lastInteract;
const char *walkRankName(uint8_t r);

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

static const uint32_t DAY = 86400, D0 = 20000 * DAY + 12 * 3600;

static void fresh(Pet &p) {
  p.begin();
  p.dbgHatchAs(1, false);
  p.lastSeenEpoch = D0;
  p.ceremony = CER_NONE;
  p.stepDay = 0; p.stepCount = 0; p.stepTotal = 0; p.stepGoalsDone = 0; p.stepGoalNew = 0;
  p.walkPrize = WALK_PRIZES;     // the walking prizes have their own test
  p.fullness = p.joy = p.energy = p.hygiene = 50;
  p.bond = 10;
  p.trAtk = p.trDef = p.trSpe = 0;
}

static void goalTests() {
  { Pet p; fresh(p);
    p.addSteps(2499);
    ck(p.stepGoalsDone == 0 && p.fullness == 50, "2499 steps: no goal yet");
    p.addSteps(1);
    ck(p.stepGoalsDone == 1 && p.fullness == 65 && p.joy == 60, "2500: a snack, +15 food and +10 joy");
    p.addSteps(100);
    ck(p.fullness == 65 && p.joy == 60, "and it pays only once");
    p.addSteps(2400);
    ck(p.stepGoalsDone == 3 && p.joy == 80 && p.energy == 60 && p.bond == 13, "5000: +20 joy, +10 energy, +3 bond");
    p.addSteps(4999);
    ck(p.stepGoalsDone == 3, "9999: not the third yet");
    p.addSteps(1);
    ck(p.stepGoalsDone == 7 && p.joy == 100 && p.bond == 18, "10000: +30 joy and +5 bond");
    ck(p.trAtk + p.trDef + p.trSpe == 3, "and a training reward");
    uint8_t j = p.joy, tr = p.trAtk + p.trDef + p.trSpe;
    p.addSteps(50000);
    ck(p.joy == j && p.trAtk + p.trDef + p.trSpe == tr, "nothing more that day, however far");
    ck(p.stepGoalNew == 7, "all three wait to be shown");
    p.lastSeenEpoch = D0 + DAY;
    p.fullness = p.joy = 50;
    p.addSteps(2500);
    ck(p.stepGoalsDone == 1 && p.fullness == 65, "the next day they pay again");
    ck(p.stepCoinK < 3 && p.coins >= 0, "(steps still pay their usual coins)");
  }
  { Pet p; fresh(p);
    uint32_t c0 = p.coins;
    p.addSteps(1000);
    uint32_t perK = p.coins - c0;
    p.addSteps(9000);
    ck(p.stepGoalsDone == 7, "one big jump pays all three");
    ck(p.coins - c0 == perK * 10, "goals add no coins of their own");
  }
  { Pet p; fresh(p);        // the clock is wrong: an earlier date
    p.addSteps(2500);
    uint8_t f = p.fullness;
    p.lastSeenEpoch = D0 - 3 * DAY;
    p.addSteps(10);
    ck(p.fullness == f && (p.stepGoalsDone & 1), "a clock going back never pays a goal again");
  }
  { Pet p; fresh(p);        // unplugged right after the goal, before any full save
    p.addSteps(2500);
    uint8_t f = p.fullness;
    Pet q; q.begin();
    q.lastSeenEpoch = D0;
    ck(q.stepGoalsDone == 1, "a goal paid survives an unplug");
    q.fullness = f;
    q.addSteps(1);
    ck(q.fullness == f, "and is not paid twice after it");
  }
  { Pet p; fresh(p);        // an egg waits
    p.speciesId = -1;
    p.addSteps(2500);
    ck(p.stepGoalsDone == 0 && p.fullness == 50, "an egg holds the goal back");
    p.dbgHatchAs(1, false);
    p.lastSeenEpoch = D0;
    p.fullness = 50;
    p.claimStepGoals();
    ck(p.stepGoalsDone == 1 && p.fullness == 65, "and it pays the same day it hatches");
  }
  { Pet p; fresh(p);        // the training ceiling holds
    p.trAtk = p.trMaxAtk(); p.trDef = p.trMaxDef(); p.trSpe = p.trMaxSpe();
    uint16_t t = p.trAtk + p.trDef + p.trSpe;
    p.addSteps(10000);
    ck(p.trAtk + p.trDef + p.trSpe == t && p.stepGoalsDone == 7, "the training ceiling holds");
  }
  { Pet p; fresh(p);        // a backup from before this version has no "stpc"
    p.addSteps(3000);
    p.saveSteps();
    { Preferences pr; pr.begin("tamapoke", false); pr.remove("stpc"); pr.end(); }
    Pet q; q.begin();
    q.lastSeenEpoch = D0;
    ck(q.stepGoalsDone == 0 && q.stepsToday() == 3000, "an old backup starts with no goals claimed");
    q.fullness = 40;
    q.claimStepGoals();
    q.claimStepGoals();
    ck(q.fullness == 55 && q.stepGoalsDone == 1, "and pays what it has already walked, once");
  }
}

static void rankTests() {
  ck(Pet::walkRankOf(0) == 0 && Pet::walkRankOf(4999) == 0, "under 5000: the first rank");
  ck(Pet::walkRankOf(5000) == 1 && Pet::walkRankOf(9999) == 1, "5000: the second");
  ck(Pet::walkRankOf(10000) == 2 && Pet::walkRankOf(25000) == 3 && Pet::walkRankOf(50000) == 4, "10000, 25000, 50000");
  ck(Pet::walkRankOf(100000) == 5 && Pet::walkRankOf(250000) == 6 && Pet::walkRankOf(500000) == 7, "100000, 250000, 500000");
  ck(Pet::walkRankOf(999999) == 7 && Pet::walkRankOf(1000000) == 8, "1000000: the top");
  ck(Pet::walkRankOf(0xFFFFFFF0UL) == WALK_RANKS - 1, "and it stays the top");
  setLang(LANG_ES);
  ck(!strcmp(walkRankName(8), "Leyenda"), "the names follow the language");
  setLang(LANG_EN);
  ck(!strcmp(walkRankName(0), "Beginner") && !strcmp(walkRankName(99), "Legend"), "and stay in range");
}

static void toastTests() {
  // A prize and a goal landing on the same step both show, one after the other.
  setLang(LANG_EN);
  pet.dbgHatchAs(1, false);
  pet.ceremony = CER_NONE;
  pet.sleeping = false;
  while (pet.hasLearnOffer()) pet.declineLearn();
  pet.stepDay = 0; pet.stepCount = 0; pet.stepGoalsDone = 0; pet.stepGoalNew = 0;
  pet.walkPrize = 0; pet.walkPrizeGain = 0; pet.stepTotal = 2490;
  pet.addSteps(2500);        // the total reaches 4990, today 2500: the goal alone
  fpToastUntil = 0; lastInteract = millis();
  pumpMs(150);
  ck(fpToastUntil != 0 && fpToast == S_GOAL_SNACK, "the goal shows a toast");
  ck(pet.stepGoalNew == 0, "and is marked as shown");
  pumpMs(2700);

  pet.stepCount = 4990; pet.stepHist[0] = 4990; pet.stepTotal = 4990; lastInteract = millis();
  pet.stepGoalsDone = 1;
  pet.addSteps(10);          // today 5000 and the total 5000: both at once
  pumpMs(150);
  ck(!strcmp(fpToastText, "Stroller! +10 coins"), "the prize toast names the rank");
  ck(pet.stepGoalNew == 2, "the goal waits its turn");
  lastInteract = millis();
  // Another toast (a trophy, say) may go first: the goal waits for a free moment.
  for (int i = 0; i < 400 && pet.stepGoalNew; i++) pumpMs(30);
  ck(fpToast == S_GOAL_WALK && fpToastText[0] == 0 && pet.stepGoalNew == 0, "then the goal shows");
}

int main() {
  rankTests();
  goalTests();
  setup();
  pumpMs(120);
  click(233, 108 + 30);
  pumpMs(60);
  click(233, 145);
  if (pet.awaitingStarter()) { printf("FAIL: no starter\n"); return 1; }
  pumpMs(120);
  toastTests();
  if (bad) { printf("FAILURES: %d\n", bad); return 1; }
  printf("step goal test passed\n");
  return 0;
}
