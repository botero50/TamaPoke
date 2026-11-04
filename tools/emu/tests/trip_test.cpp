// Phase 45: the Pokemon trip, on the real sketch.
#include "Arduino.h"
#include "Arduino_GFX_Library.h"
#include "Preferences.h"
#include "pet.h"
#include "audio.h"
#include "party.h"
#include "moves.h"
#include "battle.h"
#include <chrono>
#include <thread>

uint32_t g_seed = 0x7819;
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
extern bool mapOpen, tripCallAsk, sackOpen, fpOpen, battleOpen;
extern uint8_t mapPage;
extern uint32_t fpToastUntil, lastInteract;
extern char fpToastText[48];
bool startTripFromMap();
bool fpBlockedBySick();
bool startWildBattle();
void onTap(int16_t x, int16_t y);

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

static const uint32_t DAY = 86400, D0 = 20000 * DAY + 10 * 3600;   // 10:00

static void fresh(Pet &p) {
  p.begin();
  p.dbgHatchAs(1, false);
  p.lastSeenEpoch = D0;
  p.sleeping = false; p.ceremony = CER_NONE; p.sickness = SICK_NONE;
  p.jobKind = JOB_NONE; p.jobUntil = 0; p.jobDay = 0;
  p.tripsToday = 0; p.tripDay = 0; p.tripTale = 0; p.tripFriend = 0; p.tripCoins = 0; p.tripBackFlash = false;
  p.fullness = p.joy = p.energy = p.hygiene = 60;
  p.wildBalls[0] = p.wildBalls[1] = 0; p.wildCharges = 0;
  p.foodCount[FOOD_COOKIE] = 0; p.seedCount = 0; p.soapCount = 0;
  p.bond = 10; p.poops = 0;
  p.stepDay = 0; p.stepCount = 0; p.stepGoalsDone = 0; p.stepGoalNew = 0;
}

static void storyTests() {
  uint8_t count[TALE_KINDS] = { 0 };
  for (int r = 0; r < 100; r++) count[Pet::tripTaleForRoll((uint8_t)r)]++;
  ck(count[TALE_COINS] == 25 && count[TALE_BALL] == 15 && count[TALE_FRIEND] == 20, "coins 25%, a ball 15%, a friend 20%");
  ck(count[TALE_COOKIE] == 10 && count[TALE_SEED] == 10 && count[TALE_SOAP] == 5, "cookie 10%, seed 10%, soap 5%");
  ck(count[TALE_GRASS] == 10 && count[TALE_GREAT] == 5, "an encounter 10%, a Great Ball 5%");
  ck(count[TALE_NONE] == 0, "and every roll tells a story");
  ck(Pet::tripCoinsForRoll(0) == 5 && Pet::tripCoinsForRoll(5) == 10 && Pet::tripCoinsForRoll(6) == 5, "5 to 10 coins");
  ck(Pet::tripSecsForRoll(0) == 1800 && Pet::tripSecsForRoll(1800) == 3600 && Pet::tripSecsForRoll(1801) == 1800, "30 to 60 minutes");

  Pet p; fresh(p);
  uint32_t c = p.coins;
  p.payTale(TALE_COINS, 3, 0);
  ck(p.coins >= c + 8 && p.tripCoins == 8 && p.tripTale == TALE_COINS, "coins pay what the story says");
  ck(p.joy == 65, "every trip gives +5 joy");
  ck(p.tripBackFlash, "and it announces itself");
  p.payTale(TALE_BALL, 0, 0);    ck(p.wildBalls[0] == 1 && p.tripCoins == 0, "a Poke Ball");
  p.payTale(TALE_GREAT, 0, 0);   ck(p.wildBalls[1] == 1, "a Great Ball");
  p.payTale(TALE_COOKIE, 0, 0);  ck(p.foodCount[FOOD_COOKIE] == 1, "a cookie");
  p.payTale(TALE_SEED, 0, 0);    ck(p.seedCount == 1, "a seed");
  p.payTale(TALE_SOAP, 0, 0);    ck(p.soapCount == 1, "a soap");
  p.wildCharges = 3;
  p.payTale(TALE_GRASS, 0, 0);   ck(p.wildCharges == 4, "an encounter, above the limit of 3");
  p.joy = 50; p.bond = 10;
  memset(p.dexReg, 0, sizeof(p.dexReg));
  p.payTale(TALE_FRIEND, 0, 1234);
  ck(p.joy == 65 && p.bond == 13 && p.tripFriend > 0, "a friend: +10 joy and +3 bond, on top");
  ck(!p.isRegistered(p.tripFriend), "a friend it only met stays out of the Pokedex");
  p.wildBalls[0] = BALL_STACK_MAX; p.wildBalls[1] = BALL_STACK_MAX;
  p.foodCount[FOOD_COOKIE] = FOOD_STACK_MAX; p.seedCount = FOOD_STACK_MAX; p.soapCount = FOOD_STACK_MAX;
  p.payTale(TALE_BALL, 0, 0); p.payTale(TALE_GREAT, 0, 0); p.payTale(TALE_COOKIE, 0, 0); p.payTale(TALE_SEED, 0, 0); p.payTale(TALE_SOAP, 0, 0);
  ck(p.wildBalls[0] == BALL_STACK_MAX && p.wildBalls[1] == BALL_STACK_MAX && p.foodCount[FOOD_COOKIE] == FOOD_STACK_MAX
     && p.seedCount == FOOD_STACK_MAX && p.soapCount == FOOD_STACK_MAX, "full stacks stay at 99");
}

static void limitTests() {
  { Pet p; fresh(p);
    ck(p.tripsLeft() == 2, "2 trips a day");
    ck(p.startTrip(0) && p.onTrip() && p.jobUntil == D0 + 1800, "it leaves");
    ck(p.tripsLeft() == 1, "one left");
    ck(!p.startTrip(0), "not while it is away");
    p.lastSeenEpoch = p.jobUntil; p.callBackTrip();
    ck(p.startTrip(1800) && p.jobUntil == p.lastSeenEpoch + 3600, "the second");
    p.callBackTrip();
    ck(p.tripsLeft() == 0 && !p.startTrip(0) && !p.onTrip(), "no third");
    p.lastSeenEpoch = D0 + DAY;
    ck(p.tripsLeft() == 2 && p.startTrip(0), "the next day there are 2 again");
  }
  { Pet p; fresh(p); p.sleeping = true; ck(!p.startTrip(0), "asleep: it stays"); }
  { Pet p; fresh(p); p.sickness = SICK_COLD; ck(!p.startTrip(0), "sick: it stays"); }
  { Pet p; fresh(p); p.ceremony = CER_FAREWELL; ck(!p.startTrip(0), "in a ceremony: it stays"); }
  { Pet p; fresh(p); p.speciesId = -1; ck(!p.startTrip(0), "an egg: it stays"); }
  { Pet p; fresh(p); p.startJob(JOB_CAFE);
    ck(p.atWork() && !p.startTrip(0) && p.tripsLeft() == 2, "at work: it stays, and no trip is used");
    ck(!p.onTrip(), "a job is not a trip");
  }
  { Pet p; fresh(p); p.startTrip(0);
    ck(!p.startJob(JOB_CAFE), "away on a trip: no job");
    ck(!p.jobDoneToday(), "and the job is still free today");
  }
  { Pet p; fresh(p); p.lastSeenEpoch = 20000 * DAY + 19 * 3600 + 59 * 60;
    ck(p.startTrip(0), "19:59: it can still leave");
  }
  { Pet p; fresh(p); p.lastSeenEpoch = 20000 * DAY + 20 * 3600;
    ck(!p.startTrip(0) && p.tripsLeft() == 2, "20:00: too late, and nothing is used");
  }
  { Pet p; fresh(p); p.lastSeenEpoch = 20000 * DAY + 6 * 3600 + 59 * 60;
    ck(!p.startTrip(0), "06:59: too early");
  }
  { Pet p; fresh(p); p.lastSeenEpoch = 20000 * DAY + 7 * 3600;
    ck(p.startTrip(0), "07:00: it can leave");
  }
  { Pet p; fresh(p); p.lastSeenEpoch = 0;
    ck(!p.startTrip(0), "no clock: it stays");
  }
}

static void awayTests() {
  { Pet p; fresh(p);        // needs pause
    p.startTrip(0);
    p.lastSeenEpoch += 300;
    for (int i = 0; i < 5; i++) p.dbgTick();
    ck(p.fullness == 60 && p.joy == 60 && p.energy == 60 && p.hygiene == 60, "needs do not drop while away");
    ck(!p.needBeepDue(1000000), "and it does not beep for them");
  }
  { Pet p; fresh(p);        // home
    p.startTrip(0);
    uint32_t back = p.jobUntil;
    p.lastSeenEpoch = back;
    p.dbgTick();
    ck(!p.onTrip() && p.jobKind == JOB_NONE, "it comes home on time");
    ck(p.tripTale != TALE_NONE, "with a story");
    ck(!p.jobDoneToday(), "which does not use up the job");
    uint8_t t = p.tripTale;
    int16_t f = p.tripFriend;
    p.tripBackFlash = false;
    { Pet q; q.begin();     // unplugged right after
      ck(q.tripTale == t && q.tripFriend == f, "the story survives an unplug");
      ck(!q.tripBackFlash, "(the chime is only for the moment it arrives)");
      q.lastSeenEpoch = back + 60;
      uint8_t j = q.joy;
      q.dbgTick();
      ck(q.tripTale == t && q.joy <= j, "and is not paid twice");
    }
    p.readTale();
    Pet r; r.begin();
    ck(r.tripTale == TALE_NONE, "reading it clears it, for good");
  }
  { Pet p; fresh(p);        // the ball was off when it was due
    p.startTrip(0);
    Pet q; q.begin();
    ck(q.jobKind == JOB_TRIP && q.onTrip() == (q.lastSeenEpoch < q.jobUntil), "a trip under way survives a restart");
    q.lastSeenEpoch = q.jobUntil + 3 * 3600;
    q.dbgTick();
    ck(q.jobKind == JOB_NONE && q.tripTale != TALE_NONE, "and if it is overdue it comes home at the next start");
  }
  { Pet p; fresh(p);        // call back
    p.startTrip(0);
    uint32_t c = p.coins;
    uint8_t j = p.joy;
    p.callBackTrip();
    ck(!p.atWork() && p.tripTale == TALE_NONE && p.coins == c && p.joy == j, "called back: no story and no reward");
    ck(p.tripsLeft() == 1, "and it still counts");
    p.lastSeenEpoch += 7200;
    p.dbgTick();
    ck(p.tripTale == TALE_NONE, "and no story turns up later");
  }
  { Pet p; fresh(p);        // the clock moves
    p.startTrip(1800);
    p.lastSeenEpoch -= 10 * DAY;
    p.dbgTick();
    ck(!p.atWork() && p.jobKind == JOB_NONE, "a clock moved back cannot keep it away");
  }
  { Pet p; fresh(p);
    p.startTrip(0);
    p.lastSeenEpoch += 10 * DAY;
    p.dbgTick();
    ck(!p.atWork() && p.tripTale != TALE_NONE, "a clock moved forward brings it home");
  }
  { Pet p; fresh(p);        // step goals wait
    p.startTrip(0);
    p.fullness = 50;
    p.addSteps(2500);
    ck(p.stepGoalsDone == 0 && p.fullness == 50, "a step goal waits while it is away");
    ck(p.stepsToday() == 2500, "though the steps still count");
    p.lastSeenEpoch = p.jobUntil;
    p.dbgTick();
    p.claimStepGoals();
    ck(p.stepGoalsDone == 1, "and it pays once it is home");
  }
  { Pet p; fresh(p);        // a job still pays as it did
    p.startJob(JOB_CAFE);
    uint32_t c = p.coins;
    p.lastSeenEpoch = p.jobUntil;
    p.dbgTick();
    ck(p.coins > c && p.jobDoneFlash && p.tripTale == TALE_NONE, "a job still pays, with no story");
  }
  { Pet p; fresh(p);        // a save from before trips
    p.startJob(JOB_POST);
    { Preferences pr; pr.begin("tamapoke", false);
      pr.remove("tpn"); pr.remove("tpd"); pr.remove("tpt"); pr.remove("tpf"); pr.remove("tpc"); pr.end(); }
    Pet q; q.begin();
    q.lastSeenEpoch = D0;
    ck(q.tripTale == TALE_NONE && q.tripsLeft() == 2, "an old save has no trips taken");
    ck(q.jobKind == JOB_POST, "and keeps a job under way");
  }
}

static void flowTests() {
  pet.dbgHatchAs(1, false);
  pet.ceremony = CER_NONE; pet.sleeping = false; pet.sickness = SICK_NONE;
  pet.lastSeenEpoch = D0;
  pet.jobKind = JOB_NONE; pet.jobUntil = 0; pet.tripsToday = 0; pet.tripDay = 0; pet.tripTale = 0;
  while (pet.hasLearnOffer()) pet.declineLearn();
  pumpMs(120);
  mapPage = 2;
  mapOpen = true;
  lastInteract = millis();
  click(233 - 152 + 74, 76 + 52 + 26);   // the first tile on page 3
  ck(pet.onTrip(), "the Trip tile sends it off");
  ck(!mapOpen, "and closes the map");
  ck(fpToastUntil != 0 && fpToastText[0] != 0, "and says so");
  mapPage = 0;

  ck(fpBlockedBySick(), "games are blocked while it is away");
  ck(!startWildBattle() && !battleOpen, "and wild fights");
  fpToastUntil = 0; pumpMs(60);

  lastInteract = millis();
  click(233, 266 + 24);                  // Call back
  ck(tripCallAsk && pet.onTrip(), "Call back asks first");
  lastInteract = millis();
  click(298, 278);                       // NO
  ck(!tripCallAsk && pet.onTrip(), "NO leaves it away");
  lastInteract = millis();
  click(233, 266 + 24);
  lastInteract = millis();
  click(168, 278);                       // YES
  ck(!pet.onTrip() && !tripCallAsk && pet.tripTale == TALE_NONE, "YES brings it home, with no story");

  // the story card
  pet.payTale(TALE_COINS, 2, 0);
  pumpMs(60);
  ck(pet.tripTale == TALE_COINS, "the story is waiting");
  pumpMs(120);
  ck(!pet.tripBackFlash, "the chime has played");
  lastInteract = millis();
  click(233, 200);
  ck(pet.tripTale == TALE_NONE, "a tap closes the story");
  ck(!pet.onTrip(), "and does nothing else");

  // too late, through the tile
  pet.lastSeenEpoch = 20000 * DAY + 20 * 3600 + 60;
  pet.tripsToday = 0; pet.tripDay = 0;
  ck(!startTripFromMap() && !pet.onTrip(), "the tile says it is too late at night");
  pet.lastSeenEpoch = D0;
}

int main() {
  storyTests();
  limitTests();
  awayTests();
  setup();
  pumpMs(120);
  click(233, 108 + 30);
  pumpMs(60);
  click(233, 145);
  if (pet.awaitingStarter()) { printf("FAIL: no starter\n"); return 1; }
  pumpMs(120);
  flowTests();
  if (bad) { printf("FAILURES: %d\n", bad); return 1; }
  printf("trip test passed\n");
  return 0;
}
