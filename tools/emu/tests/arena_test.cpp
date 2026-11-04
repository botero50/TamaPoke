// Gym arenas and the way out of a game. Drives the real sketch: every trainer in
// all 7 regions must get the right arena, and the marked X must be the ONLY way
// out of every game, with what was earned kept.
#include "Arduino.h"
#include "Arduino_GFX_Library.h"
#include "Preferences.h"
#include "pet.h"
#include "audio.h"
#include "party.h"
#include "moves.h"
#include "battle.h"
#include "dex.h"
#include "backs.h"
#include "trainers.h"
#include <chrono>
#include <thread>

uint32_t g_seed = 0xA7E4A;
FakeSerial Serial;
FakeESP ESP;
FakeWire Wire;
volatile int g_touchX = 0, g_touchY = 0;
volatile bool g_touchDown = false;
void FakeESP::restart() { exit(0); }
int FakeSerial::available() { return 0; }
String FakeSerial::readStringUntil(char) { return String(""); }

void setup(); void loop();
void onSwipe(int dir); void onSwipeV(int dir);
extern Pet pet;
extern int8_t btlTrainer; extern uint8_t btlRegion; extern bool battleOpen;
void startBattle(int16_t dex, uint8_t lvl);
uint8_t battleArena();
bool exitHit(int16_t x, int16_t y);

extern bool gameOpen, sackOpen, spdOpen, fpOpen, scrubOpen;
extern uint8_t gameScore, fpGame;
extern float ballX, ballY;
extern uint16_t sackHits, spdHits, fpScore;
extern int16_t spdX, spdY;
void startGame(); void startSack(); void startSpeedGame(); void startFunPackGame(uint8_t g);
void gameTap(int16_t x, int16_t y); void spdTap(int16_t x, int16_t y); void fpTap(int16_t x, int16_t y);

static int bad = 0;
static void ck(bool ok, const char *what) {
  printf("%s  %s\n", ok ? "PASS" : "FAIL", what);
  if (!ok) bad++;
}

static void pump(int times) {
  for (int i = 0; i < times; i++) {
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    loop();
  }
}
static void click(int x, int y) {
  g_touchX = x; g_touchY = y; g_touchDown = true;
  emuFireInterrupt();
  pump(5);
  g_touchDown = false;
  emuFireInterrupt();
  pump(3);
}
static void ready() {
  pet.fullness = pet.joy = pet.energy = pet.hygiene = 80;
  pet.poops = 0; pet.sickness = SICK_NONE; pet.sleeping = false;
  while (pet.hasLearnOffer()) pet.declineLearn();
}

static const int XX = 233, XY = 24;     // the middle of the X
static const int STRIP_X = 100, STRIP_Y = 50;   // on the old quit strip, away from the X

static void arenaTests() {
  // What each speciality should fight in, restated here so a slip in the table fails.
  static const uint8_t WANT[TYPE_COUNT] = {
    ARENA_ROCK, ARENA_FIRE, ARENA_WATER, ARENA_ELECTRIC, ARENA_GRASS, ARENA_ICE,
    ARENA_ROCK, ARENA_GHOST, ARENA_ROCK, ARENA_GRASS, ARENA_GHOST, ARENA_GRASS,
    ARENA_ROCK, ARENA_GHOST, ARENA_DRAGON, ARENA_GHOST, ARENA_ELECTRIC, ARENA_GHOST };
  bool types = true, halls = true, inRange = true;
  uint32_t used = 0;
  for (uint8_t r = 0; r < GYM_REGIONS; r++) {
    btlRegion = r;
    for (uint8_t i = 0; i < TRAINER_COUNT; i++) {
      btlTrainer = (int8_t)i;
      uint8_t a = battleArena();
      if (a >= ARENA_COUNT) { inRange = false; continue; }
      used |= 1u << a;
      if (i == TRAINER_COUNT - 1) { if (a != ARENA_DRAGON) halls = false; }
      else if (a != WANT[TRAINER_SETS[r].list[i].type]) {
        printf("  region %u trainer %u (%s): arena %u\n", r, i, TRAINER_SETS[r].list[i].name, a);
        types = false;
      }
    }
  }
  ck(inRange, "every gym, Elite 4 and Champion in all 7 regions has an arena");
  ck(types, "each leader fights in the arena of their type");
  ck(halls, "every Champion fights in the grand hall");
  ck(used == (1u << ARENA_COUNT) - 1, "all 8 arenas get used");

  // A one-off fight keeps today's background, even straight after a gym fight.
  btlRegion = 0; btlTrainer = 0;
  ck(battleArena() < ARENA_COUNT, "a leader's fight has an arena");
  ready();
  startBattle(25, 5);
  ck(btlTrainer == -1 && battleArena() == ARENA_COUNT, "a one-off fight after it does not");
  battleOpen = false;

  bool sized = true;
  for (int a = 0; a < ARENA_COUNT; a++) {
    const BackScene &b = ARENAS[a];
    if (b.w != 240 || b.h != 112 || b.palCount == 0 || b.palCount > 256) sized = false;
    for (uint32_t i = 0; i < (uint32_t)b.w * b.h; i++) if (b.idx[i] >= b.palCount) { sized = false; break; }
  }
  ck(sized, "every arena is 240x112 and its colours are all in its palette");
}

static void exitTests() {
  ck(exitHit(XX, XY) && exitHit(XX - 30, XY) && exitHit(XX + 30, XY) && exitHit(XX, 55),
     "the X has a finger-sized touch area");
  ck(!exitHit(STRIP_X, STRIP_Y) && !exitHit(366, STRIP_Y) && !exitHit(XX, 72),
     "the rest of the strip is not the X");

  // --- the ball game
  ready();
  startGame();
  ballX = 233; ballY = 320;
  uint8_t joy0 = pet.joy;
  gameTap(STRIP_X, STRIP_Y); ck(gameOpen, "ball game: a tap on the strip does not quit");
  onSwipe(1); onSwipeV(1);   ck(gameOpen, "ball game: a swipe does not quit");
  ballX = 233; ballY = 28; gameScore = 6;
  gameTap(XX, XY);
  ck(gameOpen && gameScore == 7, "ball game: a high ball on the X counts as a hit");
  ballX = 233; ballY = 320;
  gameTap(XX, XY);
  ck(!gameOpen, "ball game: the X quits");
  ck(pet.joy > joy0, "ball game: and keeps what was earned");

  // --- the bag, through the real touch path: it taps on press
  ready();
  startSack();
  click(STRIP_X, STRIP_Y);
  ck(sackOpen && sackHits == 0, "bag: a tap on the strip neither quits nor counts");
  click(233, 240);
  ck(sackOpen && sackHits == 1, "bag: a tap below it is a hit");
  onSwipe(1); onSwipeV(1);
  ck(sackOpen, "bag: a swipe does not quit");
  uint8_t st0 = pet.strHi;
  sackHits = 40;
  click(XX, XY);
  ck(!sackOpen, "bag: the X quits");
  ck(pet.strHi >= 40 && pet.strHi > st0, "bag: and keeps what was earned");

  // --- the reaction test
  ready();
  startSpeedGame();
  spdX = 233; spdY = 300;
  spdTap(STRIP_X, STRIP_Y); ck(spdOpen, "speed: a tap on the strip does not quit");
  onSwipe(1); onSwipeV(1);  ck(spdOpen, "speed: a swipe does not quit");
  spdX = XX; spdY = XY; spdHits = 0;
  spdTap(XX, XY);
  ck(spdOpen && spdHits == 1, "speed: a target on the X counts as a hit");
  spdX = 233; spdY = 300;
  spdTap(XX, XY);
  ck(!spdOpen, "speed: the X quits");

  // --- every other game
  bool opened = true, strip = true, swipe = true, xQuits = true;
  for (uint8_t g = 0; g < MINI_COUNT; g++) {
    ready();
    startFunPackGame(g);
    if (!fpOpen || fpGame != g) { printf("  game %u did not open\n", g); opened = false; continue; }
    fpTap(STRIP_X, STRIP_Y);
    if (!fpOpen) { printf("  game %u quit on the strip\n", g); strip = false; startFunPackGame(g); }
    onSwipe(1); onSwipe(-1); onSwipeV(1); onSwipeV(-1);
    if (!fpOpen) { printf("  game %u quit on a swipe\n", g); swipe = false; startFunPackGame(g); }
    fpTap(XX, XY);
    if (fpOpen) { printf("  game %u ignored the X\n", g); xQuits = false; fpOpen = false; }
  }
  ck(opened, "all the other games open");
  ck(strip, "none of them quits on a tap on the strip");
  ck(swipe, "none of them quits on a swipe");
  ck(xQuits, "the X quits every one of them");

  ready();
  startFunPackGame(MINI_MOLE);
  uint16_t hi0 = pet.miniHi[MINI_MOLE];
  fpScore = (uint16_t)(hi0 + 5);
  click(XX, XY);                       // the real touch path, for a game that taps on press
  ck(!fpOpen && pet.miniHi[MINI_MOLE] == hi0 + 5, "mole: the X quits and keeps the record");

  // --- the scrub game rubs by dragging, so it has its own path
  ready();
  scrubOpen = true;
  click(STRIP_X, STRIP_Y); ck(scrubOpen, "scrub: a rub on the strip does not quit");
  click(XX, XY);           ck(!scrubOpen, "scrub: the X quits");
}

int main() {
  setup();
  pump(4);
  click(233, 108 + 30);               // first boot: Kanto
  pump(2);
  click(233, 145);                    // its first starter
  if (pet.awaitingStarter()) { printf("FAIL: no starter\n"); return 1; }
  if (pet.isEgg()) pet.dbgHatchAs(6, false);
  pump(4);
  arenaTests();
  exitTests();
  if (bad) { printf("FAILURES: %d\n", bad); return 1; }
  printf("arena test passed\n");
  return 0;
}
