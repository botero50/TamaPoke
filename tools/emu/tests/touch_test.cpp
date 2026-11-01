// Headless proof that a click reaches onTap(). Replaces main_sdl.cpp's main()
// and runtime globals, then drives the REAL setup()/loop()/handleTouch() by
// poking the same touch globals the SDL event loop pokes.
//
// Build twice: with and without -DNO_IRQ, to A/B the emuFireInterrupt() fix.
#include "Arduino.h"
#include "Arduino_GFX_Library.h"
#include "Preferences.h"
#include "pet.h"
#include "party.h"
#include "moves.h"
#include "battle.h"
#include <chrono>
#include <thread>
#include <string>
#include <deque>

uint32_t g_seed = 0xC0FFEE;
FakeSerial Serial;
FakeESP ESP;
FakeWire Wire;
volatile int g_touchX = 0, g_touchY = 0;
volatile bool g_touchDown = false;

// millis() deliberately NOT defined here: clock.cpp is linked in so this
// exercises the real emulator clock, --fast handling included.
void FakeESP::restart() { exit(0); }

// No stdin console in this harness: the serial path is not what is under test.
int FakeSerial::available() { return 0; }
String FakeSerial::readStringUntil(char) { return String(""); }

void setup();
void loop();
extern Pet pet;   // defined in the sketch
extern bool trainOpen, sackOpen, gameOpen, menuOpen, cardOpen, movePickOpen, spdOpen;
extern uint8_t fpMenuCat, fpGame;
extern bool fpOpen, missionsOpen, walletOpen, hubOpen, shopOpen, bagOpen, hatAdjust;
extern int8_t shopConfirm, shopCat, bagCat, foodCat;
extern bool foodOpen, bathOpen, sleepOpen, scrubOpen, mapOpen;
extern uint8_t placeScene;
extern uint32_t fishBiteAt; extern uint8_t fishPhase;
extern bool alarmRinging, choresOpen, piggyOpen, diaryOpen;
extern uint32_t bathUntil; extern uint8_t bathPops; extern bool bathPlay;
void startBath(); void bathBubblePos(int i, int *x, int *y);
extern uint8_t schoolMode, quizRight, quizCorrect, quizQ;
extern uint32_t studyEnd, teethEnd;
extern bool trophyOpen, clockOpen;
extern uint8_t settingsPage;
void openClock();
bool fpBlockedBySick();
extern uint8_t movePickSlot, movePickPage;
extern bool battleOpen, btlOver, btlWon;
extern Combatant btlYou, btlFoe;
extern uint8_t btlMsgCount;
void startBattle(int16_t dex, uint8_t lvl);
void startTrainerBattle(uint8_t idx, bool hard);
extern uint8_t btlFoeAt, btlSquadN, btlSquadAt, btlMenu;
extern Combatant btlSquad[7];
extern int8_t btlSwapWho;
extern bool btlHard;
extern uint8_t gymRegion, btlRegion;
extern bool gymOpen, gymHard; extern uint8_t gymPage;
extern bool pickOpen, pickHard; extern uint8_t pickTrainer; extern uint16_t squadMask;
uint8_t squadCap(uint8_t idx, bool hard);
uint8_t pickChosen(); uint8_t pickCandidates(); void pickDefault(uint8_t);
extern uint8_t pickPage;
#define PICK_X(i) (78 + ((i) % 2) * 160)
#define PICK_Y(i) (86 + ((i) / 2) * 80)
#define BTL_CELL_X(i) (69 + ((i) % 2) * 168)
#define BTL_CELL_Y(i) (286 + ((i) / 2) * 52)
uint8_t learnableList(uint8_t *out, uint8_t max);
#define MOVE_PICK_PER_PAGE 5
#define MOVE_PICK_Y(i) (76 + (i) * 58)
extern uint8_t cardPage;

// handleTouch() self-gates to 50 Hz off millis(), so real time must pass
// between polls; a tight loop() spin would be swallowed by that gate.
static void pump(int times) {
  for (int i = 0; i < times; i++) {
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    loop();
  }
}

static void fire() {
#ifndef NO_IRQ
  emuFireInterrupt();
#endif
}

// One press/release at (x,y), driven exactly as the SDL event loop drives it.
static void click(int x, int y) {
  g_touchX = x; g_touchY = y; g_touchDown = true;
  fire();
  pump(5);                       // finger held: gesture starts
  g_touchDown = false;
  fire();
  pump(3);                       // finger up: gesture resolves as a tap
}

void uiButtonAt(int i, int *cx, int *cy, int *half);

int main(int argc, char **argv) {
  if (argc > 1) emuSetTimeScale((uint32_t)atoi(argv[1]));
  printf("--- time scale x%u ---\n", emuTimeScale());
  setup();
  pump(4);

  if (!pet.awaitingStarter()) { printf("FAIL: not on the starter screen\n"); return 1; }
  printf("on starter screen, awaitingStarter=1\n");

  // First boot is two steps now: the region, then its starters. Pick KANTO so
  // row 0 below is Bulbasaur, exactly as it was before the region step existed.
  click(233, 108 + 30);
  pump(2);
  if (pet.region != 0) { printf("FAIL: region tap did not land\n"); return 1; }
  if (!pet.awaitingStarter()) { printf("FAIL: the region tap chose a starter\n"); return 1; }
  printf("picked KANTO, still on the starter flow\n");

  // Negative control: below the three rows (they span y 110..344). If this
  // "selects" a starter the test proves nothing about coordinate routing.
  click(233, 420);
  if (!pet.awaitingStarter()) { printf("FAIL: off-row click selected a starter\n"); return 1; }
  printf("click at (233,420) off the rows: correctly ignored\n");

  // row 0 == Kanto's first starter == Bulbasaur. Centre of the row: x 70..396, y 110..180.
  click(233, 145);
  printf("click at (233,145) on row 0: awaitingStarter=%d\n", (int)pet.awaitingStarter());

  if (pet.awaitingStarter()) { printf("FAIL: click did not register\n"); return 1; }
  printf("PASS: tap reached onTap and picked a starter\n");

  // --fast must still do its job: with no finger down, game time has to run at
  // the scale. Otherwise the touch fix would have quietly neutered the flag.
  uint32_t t0 = millis();
  auto w0 = std::chrono::steady_clock::now();
  pump(20);                      // ~600 ms of real time, idle
  uint32_t gameMs = millis() - t0;
  uint64_t realMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - w0).count();
  double ratio = realMs ? (double)gameMs / (double)realMs : 0.0;
  printf("idle: %llu ms real -> %u ms game (x%.1f, want x%u)\n",
         (unsigned long long)realMs, gameMs, ratio, emuTimeScale());
  if (ratio < emuTimeScale() * 0.8 || ratio > emuTimeScale() * 1.2) {
    printf("FAIL: --fast no longer accelerates idle game time\n");
    return 1;
  }
  printf("PASS: --fast still accelerates idle game time\n");

  // ---- new UI routing: 5th icon -> training submenu -> the two live trainers,
  // and the menu's STATS row -> the stats card page. Both changed index
  // mappings inside existing handlers, which is where an off-by-one would hide.
  if (pet.isEgg()) pet.dbgHatchAs(6, false);
  pump(4);

  // The learn prompt is MODAL -- it swallows every tap until answered -- so a
  // creature with moves waiting will ignore the icons. That is correct
  // behaviour, and it started firing here as soon as dex_moves.py gained the
  // cheap early attacks: a level 1 creature now actually has things to learn.
  while (pet.hasLearnOffer()) pet.declineLearn();
  pump(2);
  int tbx, tby; uiButtonAt(3, &tbx, &tby, nullptr);   // ask, do not hardcode:
  click(tbx, tby);                       // the dumbbell, the 4th of 4
  if (!trainOpen) { printf("FAIL: train icon did not open the submenu\n"); return 1; }
  printf("PASS: 5th icon opens the training submenu\n");

  // Each row opens its category's list; the first game there is the original
  // trainer. List rows sit at FPM_ROW_Y(i) = 132 + 64 i, so +28 is mid-row.
  click(233, 178);                       // row 0 == STRENGTH == TRAIN_ROW_Y(0)+28
  if (fpMenuCat != 1) { printf("FAIL: STRENGTH row did not open its games\n"); return 1; }
  click(233, 160);
  if (!sackOpen) { printf("FAIL: STRENGTH list did not start the sack\n"); return 1; }
  printf("PASS: STRENGTH -> list -> the punching bag\n");
  sackOpen = false;

  click(tbx, tby);
  click(233, 242);                       // row 1 == SPEED == TRAIN_ROW_Y(1)+28
  if (fpMenuCat != 2) { printf("FAIL: SPEED row did not open its games\n"); return 1; }
  click(233, 160);
  if (!spdOpen) { printf("FAIL: SPEED list did not start the reaction test\n"); return 1; }
  printf("PASS: SPEED -> list -> the reaction test\n");
  spdOpen = false;

  click(tbx, tby);
  click(233, 306);                       // row 2 == DEFENCE
  if (fpMenuCat != 3) { printf("FAIL: DEFENCE row did not open its games\n"); return 1; }
  click(233, 160);
  if (!gameOpen) { printf("FAIL: DEFENCE list did not start the ball game\n"); return 1; }
  printf("PASS: DEFENCE -> list -> the ball game\n");
  gameOpen = false;

  // Starting a game draws random numbers, and the gym fight further down is only
  // a sweep on the seed it has always had -- so put the stream back afterwards.
  uint32_t seedBefore = g_seed;
  click(tbx, tby);
  click(233, 370);                       // row 3 == FUN == TRAIN_ROW_Y(3)+28
  if (fpMenuCat != 4) { printf("FAIL: FUN row did not open its games\n"); return 1; }
  click(233, 160 + 64);                  // second entry: hide and seek
  if (!fpOpen || fpGame != MINI_HIDE) { printf("FAIL: FUN list did not start hide and seek\n"); return 1; }
  printf("PASS: FUN -> list -> hide and seek\n");
  fpOpen = false;
  g_seed = seedBefore;

  click(tbx, tby);
  click(233, 178);
  click(20, 233);                        // outside the list: back to the categories
  if (fpMenuCat || !trainOpen) { printf("FAIL: tapping outside a list did not go back\n"); return 1; }
  printf("PASS: outside a list goes back to the categories\n");
  trainOpen = false;

  // the two top buttons: missions on the left, the purse on the right
  click(100, 72);
  if (!missionsOpen) { printf("FAIL: the left top button did not open the missions\n"); return 1; }
  click(233, 233);
  if (missionsOpen) { printf("FAIL: a tap did not close the missions\n"); return 1; }
  click(366, 72);
  if (!hubOpen) { printf("FAIL: the right top button did not open the shop hub\n"); return 1; }
  click(20, 233);                        // outside the panel closes it
  if (hubOpen || menuOpen) { printf("FAIL: a tap outside did not close the hub cleanly\n"); return 1; }
  printf("PASS: the top buttons open the missions and the shop hub\n");

  // hub -> shop -> buy the cap -> it goes straight on
  pet.coins = 1000; pet.hatsOwned = 0; pet.hatOn = -1;
  click(366, 72);
  click(155, 176);                       // hub tile 0: SHOP (2x4 grid, rows 150 + 58 r)
  if (!shopOpen) { printf("FAIL: SHOP row did not open the shop\n"); return 1; }
  // shop and bag rows sit at SHOP_ROW_Y(i) = 146 + 54 i, so +24 is mid-row
  click(233, 76 + 52 + 20);              // category 0: HATS (rows CATL_ROW_Y = 128 + 44 i)
  if (shopCat != 0) { printf("FAIL: HATS did not open the hats\n"); return 1; }
  click(233, 146 + 24);                  // hats row 0: the cap
  if (shopConfirm != HAT_CAP) { printf("FAIL: tapping a hat did not ask to buy it\n"); return 1; }
  click(163, 307);                       // BUY
  if (!pet.ownsHat(HAT_CAP) || pet.hatOn != HAT_CAP || pet.coins != 1000 - Pet::hatPrice(HAT_CAP)) {
    printf("FAIL: BUY did not buy and wear the cap\n"); return 1;
  }
  printf("PASS: shop -> hats -> confirm -> the cap is bought and worn\n");
  click(20, 233);                        // outside: back to the categories
  if (!shopOpen || shopCat != -1) { printf("FAIL: tapping outside did not go back to the categories\n"); return 1; }
  pet.decorOwned = 0; pet.slotL = pet.slotR = -1;
  click(233, 76 + 52 + 44 + 20);         // category 1: TOYS
  click(233, 146 + 24);                  // toys row 0: the ball
  click(163, 307);                       // BUY
  if (!pet.ownsDecor(DEC_BALL) || pet.slotL != DEC_BALL) { printf("FAIL: buying the ball did not put it out\n"); return 1; }
  printf("PASS: shop -> toys -> the ball is bought and set down\n");
  click(20, 233);
  click(20, 233);                        // and outside again closes the shop
  if (shopOpen) { printf("FAIL: tapping outside the categories did not close the shop\n"); return 1; }

  // hub -> bag -> move the hat with the pad
  click(366, 72);
  click(311, 176);                       // hub tile 1: BAG
  if (!bagOpen) { printf("FAIL: BAG row did not open the bag\n"); return 1; }
  click(233, 76 + 52 + 20);              // category 0: HATS (rows CATL_ROW_Y = 128 + 44 i)
  click(233, 146 + 24);                  // bag row 0: move the hat
  if (!hatAdjust) { printf("FAIL: MOVE HAT did not open the pad\n"); return 1; }
  click(233, 372 - 48);                  // up arrow
  if (pet.hatOffY() != -4) { printf("FAIL: the up arrow did not move the hat (%d)\n", pet.hatOffY()); return 1; }
  click(233, 372);                       // DONE
  if (hatAdjust) { printf("FAIL: DONE did not close the pad\n"); return 1; }
  printf("PASS: bag -> move hat -> the pad nudges it and closes\n");

  // the food menu: a free berry in one tap, a curry from MEALS
  {
    int fbx, fby; uiButtonAt(0, &fbx, &fby, nullptr);   // the food button, 1st of 4
    pet.fullness = 30; pet.sleeping = false;
    click(fbx, fby);
    if (!foodOpen) { printf("FAIL: the food button did not open the food menu\n"); return 1; }
    uint8_t before = pet.fullness;
    click(81 + 76 + 32, 76 + 46 + 23);     // quick row, 2nd: the blue berry
    if (foodOpen || pet.fullness <= before) { printf("FAIL: a quick berry did not feed and close\n"); return 1; }
    printf("PASS: food -> a free berry in one tap\n");
    pump(80);                              // let the eating animation end
    pet.foodCount[FOOD_CURRY] = 1;
    click(fbx, fby);
    click(233, 76 + 46 + 52 + 23);         // row 1: MEALS
    if (foodCat != 0) { printf("FAIL: MEALS did not open the meals\n"); return 1; }
    click(233, 76 + 46 + 52 + 23);         // meals row 1: the curry
    if (foodOpen || pet.foodCount[FOOD_CURRY] != 0) { printf("FAIL: tapping the curry did not eat it\n"); return 1; }
    printf("PASS: food -> meals -> the curry is eaten from the bag\n");
  }

  // the bath and sleep menus (rows at 122 + 52 i, so +23 is mid-row)
  {
    pump(80);
    int bbx, bby; uiButtonAt(2, &bbx, &bby, nullptr);   // the bath button, 3rd of 4
    pet.poops = 2; pet.hygiene = 30;
    click(bbx, bby);
    if (!bathOpen) { printf("FAIL: the bath button did not open the bath menu\n"); return 1; }
    pet.soapCount = 0;
    click(233, 122 + 52 + 23);             // BUBBLE BATH without soap
    if (!bathOpen) { printf("FAIL: a bubble bath without soap should be refused\n"); return 1; }
    click(233, 122 + 23);                  // QUICK WASH
    if (bathOpen) { printf("FAIL: QUICK WASH did not start the bath\n"); return 1; }
    pump(130);                             // the 3 s bath plays out
    if (pet.poops != 0 || pet.hygiene != 100) { printf("FAIL: the quick wash did not clean it\n"); return 1; }
    printf("PASS: bath -> quick wash cleans it; no soap, no bubble bath\n");

    int lbx, lby; uiButtonAt(1, &lbx, &lby, nullptr);   // the light button, 2nd of 4
    pet.sleeping = false;
    click(lbx, lby);
    if (!sleepOpen || pet.sleeping) { printf("FAIL: awake, the light button should open the sleep menu\n"); return 1; }
    click(233, 122 + 52 + 23);             // the 30 min nap
    if (!pet.sleeping || pet.lightOn || !pet.napEnd) { printf("FAIL: NAP did not put it to sleep\n"); return 1; }
    click(lbx, lby);                       // asleep: the button wakes it at once
    if (pet.sleeping || sleepOpen) { printf("FAIL: asleep, the light button should wake it directly\n"); return 1; }
    printf("PASS: sleep menu -> nap; asleep the light button wakes it\n");
  }

  // places: the map button, the Pokemon Center, the garden, the store's shop
  {
    pump(10);
    uint32_t seedBefore = g_seed;
    click(394, 154);                       // the map button
    if (!mapOpen) { printf("FAIL: the map button did not open GO OUT\n"); return 1; }
    click(155, 214);                     // map tile 2: the Pokemon Center (rows 128 + 60 r)
    if (placeScene != 3) { printf("FAIL: the center row did not open the center\n"); return 1; }
    pet.sickness = SICK_COLD; pet.centerDay = 0;
    click(110 + 58, 380 + 22);             // HEAL
    if (pet.isSick()) { printf("FAIL: HEAL did not cure it\n"); return 1; }
    click(240 + 58, 380 + 22);             // BACK
    if (placeScene) { printf("FAIL: BACK did not leave the center\n"); return 1; }
    printf("PASS: map -> Pokemon Center -> heal -> back\n");

    pet.seedCount = 1; pet.plantEpoch = 0;
    click(394, 154);
    click(311, 214);                     // map tile 3: the garden
    click(110 + 58, 380 + 22);             // PLANT
    if (!pet.plantEpoch || pet.seedCount) { printf("FAIL: PLANT did not plant the seed\n"); return 1; }
    click(240 + 58, 380 + 22);
    printf("PASS: map -> garden -> a seed is planted\n");

    click(394, 154);
    click(311, 154);                     // map tile 1: the store
    click(110 + 58, 380 + 22);             // GO IN: the shop opens over the store
    if (!shopOpen || placeScene != 2) { printf("FAIL: the store did not open the shop\n"); return 1; }
    click(20, 233);                        // out of the shop: back in the store
    if (shopOpen || placeScene != 2) { printf("FAIL: leaving the shop should return to the store\n"); return 1; }
    click(240 + 58, 380 + 22);
    printf("PASS: map -> store -> the shop opens over it\n");

    // the capsule machine, left in the store
    pet.coins = 100; pet.capDay = 0; pet.capSpins = 0;
    uint16_t capsBefore = pet.tally[TALLY_CAPSULE];
    click(394, 154);
    click(311, 154);                     // the store
    click(80, 250);                        // the machine
    if (pet.tally[TALLY_CAPSULE] != capsBefore + 1 || pet.capSpins != 1) { printf("FAIL: tapping the machine did not turn a capsule\n"); return 1; }
    click(240 + 58, 380 + 22);
    printf("PASS: store -> the capsule machine turns\n");

    // the school: a full math quiz, all right; then 10 minutes of homework
    pet.sleeping = false; pet.parentBed = 0; pet.parentGames = 0;
    click(394, 154);
    click(155, 274);                     // map tile 4: the school
    if (placeScene != 5) { printf("FAIL: the school row did not open the school\n"); return 1; }
    click(233, 380 + 22);                  // MATH (middle button)
    if (schoolMode != 3) { printf("FAIL: MATH did not start the quiz\n"); return 1; }
    for (int q = 0; q < 10; q++) {
      click(233 - 110 + quizRight * 110, 280);   // the right answer's tile
      pump(25);                                  // past the green flash
    }
    if (schoolMode != 4 || quizCorrect != 10 || pet.quizHi != 10) { printf("FAIL: ten right answers did not finish a perfect quiz (%u)\n", quizCorrect); return 1; }
    printf("PASS: school -> math quiz -> 10/10\n");
    click(343, 380 + 22);                  // BACK: to the room
    click(123, 380 + 22);                  // STUDY
    click(123, 280);                       // 10 min
    if (schoolMode != 2 || !studyEnd) { printf("FAIL: STUDY 10 min did not start the timer\n"); return 1; }
    uint16_t studyBefore = pet.tally[TALLY_STUDY];
    studyEnd = 1;                          // the ten minutes are up
    pump(3);
    if (pet.tally[TALLY_STUDY] != studyBefore + 1 || studyEnd || schoolMode != 0) { printf("FAIL: the homework timer did not finish\n"); return 1; }
    click(343, 380 + 22);                  // BACK: out of the school
    if (placeScene) { printf("FAIL: BACK did not leave the school\n"); return 1; }
    printf("PASS: school -> homework -> rewarded when the time is up\n");

    // brushing teeth, from the bath menu
    pet.teethDay = 0; pet.teethToday = 0;
    { int bbx, bby; uiButtonAt(2, &bbx, &bby, nullptr); click(bbx, bby); }
    if (!bathOpen) { printf("FAIL: the bath button did not open the bath menu\n"); return 1; }
    click(233, 122 + 4 * 52 + 23);         // row 4: BRUSH TEETH
    if (placeScene != 6 || !teethEnd) { printf("FAIL: BRUSH TEETH did not start brushing\n"); return 1; }
    uint16_t teethBefore = pet.tally[TALLY_TEETH];
    teethEnd = 1;                          // two minutes later
    pump(3);
    if (pet.tally[TALLY_TEETH] != teethBefore + 1) { printf("FAIL: finishing the brushing did not count\n"); return 1; }
    click(240 - 64 + 58, 380 + 22);        // BACK
    if (placeScene) { printf("FAIL: BACK did not leave the bathroom\n"); return 1; }
    printf("PASS: bath -> brush teeth -> counted\n");

    // trophies from the purse; a backdrop from the shop
    click(366, 72);
    click(155, 292);                     // hub tile 4: TROPHIES
    if (!trophyOpen) { printf("FAIL: TROPHIES did not open the list\n"); return 1; }
    click(20, 233);
    if (trophyOpen) { printf("FAIL: tapping outside did not close the trophies\n"); return 1; }
    pet.coins = 1000; pet.sceneryOwned = 0; pet.sceneryPick = -1;
    click(366, 72);
    click(155, 176);                     // SHOP
    click(233, 76 + 52 + 5 * 44 + 20);     // category 5: SCENERY
    if (shopCat != 5) { printf("FAIL: SCENERY did not open (cat %d)\n", shopCat); return 1; }
    click(233, 146 + 54 + 24);             // row 1: the beach
    click(163, 307);                       // BUY
    if (!pet.ownsScenery(1)) { printf("FAIL: the beach was not bought\n"); return 1; }
    click(20, 233); click(20, 233);
    printf("PASS: purse -> trophies; shop -> scenery -> the beach\n");

    // the parents' page: a bedtime that stops the games
    openClock();
    settingsPage = 5;                      // SET_PARENT
    click(233, 130 + 32);                  // bedtime row: OFF -> 19:00
    if (pet.parentBed != 19) { printf("FAIL: the bedtime row did not set 19:00 (%u)\n", pet.parentBed); return 1; }
    clockOpen = false;
    uint32_t seen = pet.lastSeenEpoch;
    pet.lastSeenEpoch = (seen / 86400) * 86400 + 20 * 3600;
    bool blocked = fpBlockedBySick();
    pet.lastSeenEpoch = seen; pet.parentBed = 0;
    if (!blocked) { printf("FAIL: past the bedtime the games should be refused\n"); return 1; }
    printf("PASS: settings -> parents -> bedtime refuses games\n");
    pump(90);                              // let the toast go

    // the shower: a finger on a bubble pops it and turns the wash into a game
    pet.sleeping = false; pet.hygiene = 40;
    startBath();
    pump(2);
    int bx = 0, by = 0, pick = -1;
    for (int i = 0; i < 14 && pick < 0; i++) {   // one well inside the landscape
      bathBubblePos(i, &bx, &by);
      if (by > 130 && by < 290 && bx > 80 && bx < 386) pick = i;
    }
    if (pick < 0) { printf("FAIL: no bubble on screen to pop\n"); return 1; }
    uint8_t joyBefore = pet.joy;
    click(bx, by - 2);
    if (bathPops != 1 || !bathPlay) { printf("FAIL: tapping a bubble did not pop it (pops %u)\n", bathPops); return 1; }
    for (int k = 0; k < 120 && bathUntil; k++) pump(4);   // let the game run out
    if (bathUntil || pet.hygiene != 100) { printf("FAIL: the bubble game did not end in a clean pet\n"); return 1; }
    (void)joyBefore;   // joy drains with time too, so the small pop bonus is not asserted here
    printf("PASS: shower -> pop a bubble -> a bubble game, then clean\n");

    // the lake: cast, wait for the bite, strike
    pet.sleeping = false;
    uint16_t fishBefore = pet.tally[TALLY_FISH];
    click(394, 154);
    click(311, 274);                       // map tile 5: the lake
    if (placeScene != 7) { printf("FAIL: the lake tile did not open the lake (%u)\n", placeScene); return 1; }
    click(110 + 58, 380 + 22);             // FISH: cast
    if (fishPhase != 1) { printf("FAIL: FISH did not cast the line\n"); return 1; }
    fishBiteAt = millis();                 // a fish bites now
    for (int k = 0; k < 10 && fishPhase != 2; k++) pump(1);
    if (fishPhase != 2) { printf("FAIL: no bite came\n"); return 1; }
    click(233, 250);                       // strike
    if (pet.tally[TALLY_FISH] != fishBefore + 1) { printf("FAIL: striking on the bite did not catch anything\n"); return 1; }
    click(240 + 58, 380 + 22);             // BACK
    printf("PASS: map -> lake -> cast, bite, catch\n");

    // an adventure stop reached by walking
    pet.advRoute = 0; pet.advStop = 0; pet.advSteps = 400;
    click(394, 154);
    click(155, 334);                       // map tile 6: adventure
    if (placeScene != 8) { printf("FAIL: the adventure tile did not open the route\n"); return 1; }
    click(110 + 58, 380 + 22);             // OPEN
    if (pet.advStop != 1) { printf("FAIL: OPEN did not claim the first stop\n"); return 1; }
    click(240 + 58, 380 + 22);
    printf("PASS: map -> adventure -> the first stop opened\n");

    // the kitchen: a cookie and a juice, stirred into a pie
    pet.foodCount[FOOD_COOKIE] = 1; pet.foodCount[FOOD_JUICE] = 1; pet.foodCount[FOOD_PIE] = 0;
    click(394, 154);
    click(311, 334);                       // map tile 7: the kitchen
    if (placeScene != 9) { printf("FAIL: the kitchen tile did not open the kitchen\n"); return 1; }
    click(233, 110 + 28);                  // recipe 0: the pie
    for (int k = 0; k < 8; k++) click(233, 250);   // stir
    if (pet.foodCount[FOOD_PIE] != 1 || pet.foodCount[FOOD_COOKIE]) { printf("FAIL: stirring did not cook the pie\n"); return 1; }
    click(240 - 64 + 58, 380 + 22);        // BACK
    if (placeScene) { printf("FAIL: BACK did not leave the kitchen\n"); return 1; }
    printf("PASS: map -> kitchen -> a berry pie\n");

    // the purse: chores, the piggy bank, the diary
    pet.choreDay = 0; pet.choresDone = 0;
    uint32_t c0 = pet.coins;
    click(366, 72);
    click(155, 234);                       // hub tile 2: CHORES
    if (!choresOpen) { printf("FAIL: CHORES did not open\n"); return 1; }
    click(233, 76 + 52 + 20);              // chore 0
    if (!pet.choreDoneToday(0) || pet.coins != c0 + CHORE_COINS) { printf("FAIL: ticking a chore did not pay\n"); return 1; }
    click(20, 233);
    pet.coins = 100; pet.piggy = 0;
    click(366, 72);
    click(311, 234);                       // hub tile 3: PIGGY BANK
    if (!piggyOpen) { printf("FAIL: PIGGY BANK did not open\n"); return 1; }
    click(133, 76 + 232 + 23);             // +10
    if (pet.piggy != 10 || pet.coins != 90) { printf("FAIL: +10 did not save ten coins\n"); return 1; }
    click(20, 233);
    click(366, 72);
    click(311, 292);                       // hub tile 5: DIARY
    if (!diaryOpen) { printf("FAIL: DIARY did not open\n"); return 1; }
    click(20, 233);
    printf("PASS: purse -> chores, piggy bank, diary\n");

    // the alarm: set from the settings page; when it rings, a tap stops it
    openClock();
    settingsPage = 4;                      // SET_ALARM
    pet.alarmOn = false;
    click(199, 219);                       // hour +: sets it, and switches it on
    if (!pet.alarmOn) { printf("FAIL: setting the alarm time did not switch it on\n"); return 1; }
    clockOpen = false;
    uint32_t mins = (pet.lastSeenEpoch / 60) % 1440;
    pet.alarmH = (uint8_t)(mins / 60); pet.alarmM = (uint8_t)(mins % 60); pet.alarmDay = 0;
    pump(45);                              // the once-a-second check
    if (!alarmRinging) { printf("FAIL: the alarm did not ring at its time\n"); return 1; }
    click(233, 233);
    if (alarmRinging) { printf("FAIL: a tap did not stop the alarm\n"); return 1; }
    pet.alarmOn = false;
    printf("PASS: settings -> alarm; it rings and a tap stops it\n");
    g_seed = seedBefore;
  }

  click(233, 60);                        // name/status band opens the menu
  if (!menuOpen) { printf("FAIL: name band did not open the menu\n"); return 1; }
  click(233, 104 + 16 + 22);             // menu row 0 == STATS == MENU_ROW_Y(0)+22
  if (!cardOpen || cardPage != 1) {
    printf("FAIL: STATS row -> cardOpen=%d cardPage=%d (want 1,1)\n",
           (int)cardOpen, (int)cardPage);
    return 1;
  }
  printf("PASS: menu STATS row opens the stats card page\n");

  // ---- moves card page -> picker -> slot actually changes, with no duplicates
  cardPage = 2;
  uint8_t before = pet.moves[1];
  click(233, 154);                       // slot 1 == MOVE_ROW_Y(1)
  if (!movePickOpen) { printf("FAIL: tapping a move slot did not open the picker\n"); return 1; }
  printf("PASS: tapping a move slot opens the picker\n");

  click(233, 100);                       // first row of the picker
  if (movePickOpen) { printf("FAIL: picking a move left the picker open\n"); return 1; }
  if (pet.moves[1] == before) { printf("FAIL: slot 1 did not change\n"); return 1; }
  printf("PASS: picking a move replaces the slot (%s -> %s)\n",
         before ? MOVE_TBL[before].name : "-",
         pet.moves[1] ? MOVE_TBL[pet.moves[1]].name : "-");

  // the swap must never leave the same move in two slots
  bool dupe = false;
  for (int i = 0; i < MOVE_SLOTS; i++)
    for (int j = i + 1; j < MOVE_SLOTS; j++)
      if (pet.moves[i] && pet.moves[i] == pet.moves[j]) dupe = true;
  if (dupe) {
    printf("FAIL: a move ended up in two slots\n");
    for (int i = 0; i < MOVE_SLOTS; i++)
      printf("      slot %d: %s\n", i, pet.moves[i] ? MOVE_TBL[pet.moves[i]].name : "-");
    return 1;
  }
  printf("PASS: no duplicate move after the swap\n");

  // picking a move the pet already knows should TRADE slots, not clone it
  uint8_t s0 = pet.moves[0], s2 = pet.moves[2];
  cardPage = 2;
  click(233, 212);                       // slot 2
  uint8_t all[64];
  uint8_t n = learnableList(all, sizeof(all));
  int page = -1, row = -1;
  for (uint8_t i = 0; i < n; i++)
    if (all[i] == s0) { page = i / MOVE_PICK_PER_PAGE; row = i % MOVE_PICK_PER_PAGE; }
  if (page >= 0) {
    movePickPage = page;
    click(233, MOVE_PICK_Y(row) + 10);
    bool ok = (pet.moves[2] == s0 && pet.moves[0] == s2);
    printf("%s: picking a known move trades the two slots (0=%s 2=%s)\n",
           ok ? "PASS" : "FAIL",
           pet.moves[0] ? MOVE_TBL[pet.moves[0]].name : "-",
           pet.moves[2] ? MOVE_TBL[pet.moves[2]].name : "-");
    if (!ok) return 1;
  }
  // ---- level-up learn prompt: accept into a slot, and decline.
  // Past level 36, so LEER@15 / FLAMETHROWER@34 / WING ATTACK@36 are all in
  // range and more of them exist than there are slots.
  pet.ageMinutes = 40 * MINUTES_PER_LEVEL;
  pet.relearnFromLevel();
  pet.lastLearnLevel = 0;
  pet.learnQCount = 0;
  pet.checkLearnGates();
  if (!pet.hasLearnOffer()) { printf("FAIL: no learn offer was queued\n"); return 1; }
  printf("PASS: crossing gates with a full moveset queues an offer\n");

  uint8_t offered = pet.learnOffer(), was2 = pet.moves[2];
  click(233, 104 + 2 * 56 + 10);          // LEARN_ROW_Y(2)
  if (pet.moves[2] != offered) {
    printf("FAIL: accepting did not put %s in slot 2 (got %s)\n",
           MOVE_TBL[offered].name, pet.moves[2] ? MOVE_TBL[pet.moves[2]].name : "-");
    return 1;
  }
  printf("PASS: accepting replaces the chosen slot (%s -> %s)\n",
         was2 ? MOVE_TBL[was2].name : "-", MOVE_TBL[offered].name);

  if (pet.hasLearnOffer()) {
    uint8_t before[MOVE_SLOTS];
    for (int i = 0; i < MOVE_SLOTS; i++) before[i] = pet.moves[i];
    uint8_t skipped = pet.learnOffer();
    click(233, 334 + 20);                 // LEARN_SKIP_Y
    bool same = true;
    for (int i = 0; i < MOVE_SLOTS; i++) if (pet.moves[i] != before[i]) same = false;
    if (!same || pet.knowsMove(skipped)) {
      printf("FAIL: declining changed the moveset\n"); return 1;
    }
    printf("PASS: declining %s leaves the moveset untouched\n", MOVE_TBL[skipped].name);
  }
  // ---- battle: drive a whole fight through the real tap handler
  while (pet.hasLearnOffer()) pet.declineLearn();
  pet.ageMinutes = 50 * MINUTES_PER_LEVEL;
  pet.relearnFromLevel();
  startBattle(9, 50);
  if (!battleOpen) { printf("FAIL: battle did not start\n"); return 1; }
  printf("PASS: BATTLE opens (%s L%u vs %s L%u)\n",
         btlYou.name, btlYou.level, btlFoe.name, btlFoe.level);

  uint16_t foeHp0 = btlFoe.hp;
  int turns = 0;
  while (battleOpen && turns < 60) {
    turns++;
    if (btlMsgCount) { click(233, 320); continue; }   // clear narration
    int slot = -1;
    for (int i = 0; i < MOVE_SLOTS; i++) if (btlYou.moves[i]) { slot = i; break; }
    if (slot < 0) break;
    click(BTL_CELL_X(slot) + 40, BTL_CELL_Y(slot) + 20);
  }
  printf("     after %d taps: you %u/%u, foe %u/%u, over=%d won=%d\n",
         turns, btlYou.hp, btlYou.maxHp, btlFoe.hp, btlFoe.maxHp,
         (int)btlOver, (int)btlWon);
  if (btlFoe.hp >= foeHp0) { printf("FAIL: tapping a move did no damage\n"); return 1; }
  printf("PASS: tapping a move resolves a turn and deals damage\n");
  if (!btlOver) { printf("FAIL: fight never concluded\n"); return 1; }
  printf("PASS: the fight concludes and closes\n");

  // ---- a trainer fight: the foe's squad must chain, and winning awards a badge
  pet.ageMinutes = 100 * MINUTES_PER_LEVEL;   // strong enough to sweep Brock
  pet.relearnFromLevel();
  while (pet.hasLearnOffer()) pet.declineLearn();
  bool hadBadge = pet.hasBadge(0, 0, false);
  startTrainerBattle(0, false);
  if (!battleOpen) { printf("FAIL: trainer battle did not start\n"); return 1; }
  printf("PASS: gym battle starts (squad of %u vs BROCK)\n", btlSquadN);

  int taps = 0;
  uint8_t sawFoeAt = 0;
  while (battleOpen && taps < 200) {
    taps++;
    if (btlMsgCount) { click(233, 320); continue; }
    int slot = -1;
    for (int i = 0; i < MOVE_SLOTS; i++) if (btlYou.moves[i]) { slot = i; break; }
    if (slot < 0) break;
    click(BTL_CELL_X(slot) + 40, BTL_CELL_Y(slot) + 20);
    if (btlFoeAt > sawFoeAt) sawFoeAt = btlFoeAt;
  }
  printf("     %d taps, foe reached team index %u, won=%d\n", taps, sawFoeAt, (int)btlWon);
  if (sawFoeAt == 0) { printf("FAIL: the second gym creature never came out\n"); return 1; }
  printf("PASS: the trainer sends out its next creature on a faint\n");
  if (!btlWon) { printf("FAIL: a L100 creature lost to Brock\n"); return 1; }
  if (!pet.hasBadge(0, 0, false) || hadBadge) { printf("FAIL: no badge awarded\n"); return 1; }
  printf("PASS: beating a leader awards its badge (%u/8)\n", pet.badgeCount(false));

  // ---- hard mode caps the team to the opponent's size AND level
  battleOpen = false;
  pet.ageMinutes = 100 * MINUTES_PER_LEVEL;   // a level 100 creature
  startTrainerBattle(0, true);                // BROCK: 2 mons, top level 14
  printf("     hard vs BROCK: squad %u, your lead is L%u (pet is L%u)\n",
         btlSquadN, btlYou.level, pet.level());
  if (btlYou.level != 14) {
    printf("FAIL: level not capped to the leader's top (got %u, want 14)\n", btlYou.level);
    return 1;
  }
  printf("PASS: hard mode caps your level to the leader's best\n");
  if (btlSquadN > 2) { printf("FAIL: squad not capped to 2\n"); return 1; }
  printf("PASS: hard mode caps your team size to the leader's\n");
  if (pet.level() != 100) { printf("FAIL: the stored pet was modified\n"); return 1; }
  printf("PASS: the stored creature is untouched (still L%u)\n", pet.level());
  battleOpen = false;

  // ---- switching mid-fight
  battleOpen = false; pickOpen = false; pet.badges = 0;
  for (int i = 0; i < 3; i++) { PartyMon m; m.dex = 9 + i * 20; m.level = 40;
    m.ivAtk = m.ivDef = m.ivSpe = m.ivHp = 25; party.replaceAt(i, m); }
  squadMask = 0xFFFF;
  // The fight must read the ladder it STARTED from. btlRegion was never
  // assigned, so a Johto fight showed Falkner's Pidgey, then Brock's Onix, and
  // paid out Brock's badge.
  gymRegion = 1;
  startTrainerBattle(0, false);
  if (btlRegion != 1) { printf("FAIL: btlRegion=%u after a Johto fight started (want 1)\n", btlRegion); return 1; }
  gymRegion = 2;   // browsing another ladder mid-fight must not retarget it
  if (btlRegion != 1) { printf("FAIL: btlRegion followed gymRegion\n"); return 1; }
  printf("PASS: a Johto fight latches its own region\n");
  battleOpen = false; gymRegion = 0;
  startTrainerBattle(0, false);
  if (btlSquadN < 2) { printf("FAIL: squad too small to test switching\n"); return 1; }
  printf("PASS: battle starts with a squad of %u\n", btlSquadN);

  btlMenu = 0;
  click(150, 274 + 52 + 22);            // POKEMON: lower-LEFT cell now (RUN is right)
  if (btlMenu != 2) { printf("FAIL: POKEMON did not open the switch list\n"); return 1; }
  printf("PASS: POKEMON opens the switch list\n");

  uint8_t was = btlSquadAt;
  uint16_t swFoeHp0 = btlFoe.hp;
  click(69 + 40, 286 + 22);             // slot 0 = the one already out: inert
  if (btlSquadAt != was) { printf("FAIL: switching to the active creature was allowed\n"); return 1; }
  printf("PASS: you cannot switch to the one already out\n");

  btlMenu = 2;
  click(69 + 168 + 40, 286 + 22);       // slot 1
  if (btlSquadAt == was) { printf("FAIL: the switch did not happen\n"); return 1; }
  printf("PASS: switching brings on another creature (slot %u -> %u)\n", was, btlSquadAt);
  bool costTurn = (btlYou.hp < btlYou.maxHp) || (btlFoe.hp == swFoeHp0);
  printf("     foe hp %u -> %u, your new one %u/%u\n", swFoeHp0, btlFoe.hp, btlYou.hp, btlYou.maxHp);
  if (!costTurn) { printf("FAIL: switching was free -- the foe never acted\n"); return 1; }
  printf("PASS: switching spends the turn\n");
  battleOpen = false; btlMenu = 0;
  for (int i = 0; i < 3; i++) party.releaseAt(i);

  // ---- the gym ladder is sequential: only the next one may be entered
  battleOpen = false; pickOpen = false;
  pet.badges = 0;                       // nothing beaten yet
  gymOpen = true; gymHard = false; gymPage = 0;
  click(233, 110 + 1 * 50 + 20);        // MISTY, the second row
  if (pickOpen || battleOpen) { printf("FAIL: a locked leader was enterable\n"); return 1; }
  printf("PASS: a locked leader cannot be entered\n");
  click(233, 110 + 0 * 50 + 20);        // BROCK, the first
  if (!pickOpen) { printf("FAIL: the first leader was not enterable\n"); return 1; }
  printf("PASS: the first leader is always open\n");
  pickOpen = false;
  pet.badges = 1;                       // Brock beaten
  gymOpen = true; gymPage = 0;
  click(233, 110 + 1 * 50 + 20);        // MISTY again
  if (!pickOpen) { printf("FAIL: beating one did not unlock the next\n"); return 1; }
  printf("PASS: beating a leader unlocks the next\n");
  pickOpen = false;
  // and the hard ladder is its own run
  gymOpen = true; gymHard = true; gymPage = 0;
  click(233, 110 + 1 * 50 + 20);
  if (pickOpen) { printf("FAIL: easy progress unlocked the hard ladder\n"); return 1; }
  printf("PASS: hard mode keeps its own unlock order\n");
  gymHard = false; gymOpen = false; pet.badges = 0;

  // ---- a live pet plus a FULL party is 7 candidates against a cap of 6. The
  // 7th used to be counted but never drawn and never tappable, so FIGHT sat
  // inert with no way to fix it.
  battleOpen = false; pickOpen = false;
  for (int i = 0; i < PARTY_SLOTS; i++) { PartyMon m; m.dex = 9 + i * 10; m.level = 40;
    m.ivAtk = m.ivDef = m.ivSpe = m.ivHp = 25; party.replaceAt(i, m); }
  pickTrainer = 0; pickHard = false; pickPage = 0;
  pickDefault(squadCap(0, false));
  printf("     candidates=%u chosen=%u cap=%u\n",
         pickCandidates(), pickChosen(), squadCap(0, false));
  if (pickCandidates() != PARTY_SLOTS + 1) { printf("FAIL: expected 7 candidates\n"); return 1; }
  printf("PASS: a live pet plus a full party is 7 candidates\n");
  if (pickChosen() > squadCap(0, false)) {
    printf("FAIL: the picker opens over its own cap\n"); return 1; }
  printf("PASS: it opens with a valid selection, not everything\n");
  { uint8_t pages = (pickCandidates() + 6 - 1) / 6;
    if (pages < 2) { printf("FAIL: 7 candidates must span 2 pages\n"); return 1; }
    printf("PASS: the 7th is reachable on page %u of %u\n", pages, pages); }
  for (int i = 0; i < PARTY_SLOTS; i++) party.releaseAt(i);
  squadMask = 0xFFFF;

  // ---- team select: cap enforcement and toggling
  battleOpen = false;
  for (int i = 0; i < 5; i++) { PartyMon m; m.dex = 9 + i; m.level = 40;
    m.ivAtk = m.ivDef = m.ivSpe = m.ivHp = 25; party.replaceAt(i, m); }
  squadMask = 0xFFFF;
  pickTrainer = 0; pickHard = true; pickOpen = true;   // BROCK: cap of 2
  uint8_t cap = squadCap(0, true);
  // x=300: FIGHT sits right of centre now, with BACK to its left. Tapping 233
  // lands in the gap between them and hits nothing, which made the next check
  // pass for the wrong reason.
  click(300, 366);                                     // FIGHT with 6 chosen
  if (battleOpen || !pickOpen) { printf("FAIL: FIGHT fired while over the cap\n"); return 1; }
  printf("PASS: FIGHT is inert while over the cap (%u chosen, cap %u)\n", 6, cap);

  // deselect down to the cap, then it should start
  for (int slot = 5; slot >= (int)cap; slot--)
    click(PICK_X(slot) + 40, PICK_Y(slot) + 30);
  click(300, 366);
  if (!battleOpen) { printf("FAIL: FIGHT did not start at the cap\n"); return 1; }
  printf("PASS: trimming to the cap lets the fight start (squad %u)\n", btlSquadN);
  if (btlSquadN > cap) { printf("FAIL: squad exceeded the cap\n"); return 1; }
  printf("PASS: the squad respects the chosen team\n");
  battleOpen = false;
  squadMask = 0xFFFF;
  for (int i = 0; i < 5; i++) party.releaseAt(i);

  // easy caps the level too -- it just does not cap the team size
  startTrainerBattle(0, false);
  if (btlYou.level != 14) { printf("FAIL: easy mode did not cap the level\n"); return 1; }
  printf("PASS: easy mode caps the level as well\n");
  return 0;
}
