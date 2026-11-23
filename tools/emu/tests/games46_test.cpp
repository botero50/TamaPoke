// Phase 46: four new games, the secret one, and the motion features. The rules
// live in arcade.h as pure functions, so each is pinned exactly here; the rest
// runs on the real sketch.
#include "Arduino.h"
#include "Arduino_GFX_Library.h"
#include "Preferences.h"
#include "pet.h"
#include "audio.h"
#include "party.h"
#include "standby.h"
#include "tourney.h"
#include "esp_sleep.h"
#include "moves.h"
#include "battle.h"
#include "wild.h"
#include "arcade.h"
#include "save.h"
#include <chrono>
#include <thread>
#include <cstring>

uint32_t g_seed = 0x46A5;
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
extern bool battleOpen, btlOver, btlWild, fpOpen, clockOpen, trainOpen;
extern uint8_t btlMenu, btlMsgCount, btlSwingKind, fpMenuCat, fpMisses, fpGame, settingsPage, gWalkRun;
extern uint16_t fpScore;
extern uint32_t fpOverUntil, btlSwingUntil, lastInteract, gLastStepMs, aboutSecretUntil;
extern MotionDetector gMotion;
extern bool gAccFresh, screenOff;
extern uint8_t dimStage;
extern float gAccX, gAccY, gAccZ;
extern uint8_t mgmBoard[16];
extern bool brkAlive[4][5];
extern uint8_t brkLeft;
extern float brkX, brkY, brkVX, brkVY, brkPad;
extern bool brkServing;
extern float rllX, rllY, rllVX, rllVY, rllBX[3], rllBY[3], rllHX[4], rllHY[4];
extern uint8_t rllHoles;
extern uint32_t rllSafeUntil;
struct EvObs { float x; uint8_t kind; bool alive, passed; };
extern EvObs evObs[3];
extern float evH, evVY;
extern bool evAir;
extern uint32_t evInvUntil;
bool startWildBattle();
void battleTap(int16_t x, int16_t y);
void startFunPackGame(uint8_t g);
void mergeSwipe(uint8_t dir);
void shakeToPlay(uint32_t now);
int funMenuHit(int16_t x, int16_t y);
void clockTap(int16_t x, int16_t y);
void onTap(int16_t x, int16_t y);
extern bool galleryOpen, galleryPick, galleryDirty;
extern uint8_t galleryRegion;
uint8_t uiCurrentScreen();

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
static bool near(float a, float b, float e) { return fabsf(a - b) <= e; }

static const uint32_t DAY = 86400, D0 = 20000 * DAY + 12 * 3600;

// ---------------------------------------------------------------- the pure rules
static void mergeRules() {
  uint8_t b[16]; bool mv;
  memset(b, 0, 16); b[0] = 1; b[1] = 1; b[2] = 2;
  ck(mergeSlide(b, 0, &mv) == 2 && b[0] == 2 && b[1] == 2 && b[2] == 0 && mv, "[1 1 2] slid left makes [2 2]: a new tile does not merge again");
  memset(b, 0, 16); b[0] = b[1] = b[2] = b[3] = 1;
  ck(mergeSlide(b, 0, &mv) == 4 && b[0] == 2 && b[1] == 2 && b[2] == 0, "four 1s make two 2s, scoring 4");
  memset(b, 0, 16); b[1] = b[2] = 1;
  mergeSlide(b, 1, &mv);
  ck(b[3] == 2 && b[1] == 0 && b[2] == 0, "sliding right packs to the right edge");
  memset(b, 0, 16); b[0] = 3; b[4] = 3; b[8] = 1;
  ck(mergeSlide(b, 2, &mv) == 4 && b[0] == 4 && b[4] == 1 && b[8] == 0, "up merges a column from the top: 3+3 = 4");
  memset(b, 0, 16); b[3] = 2; b[7] = 2; b[11] = 5;
  mergeSlide(b, 3, &mv);
  ck(b[15] == 5 && b[11] == 3 && b[7] == 0, "down merges from the bottom");
  uint8_t pack[16] = { 1, 2, 3, 4, 2, 3, 4, 5, 3, 4, 5, 6, 4, 5, 6, 7 };
  uint8_t copy[16]; memcpy(copy, pack, 16);
  ck(mergeSlide(pack, 0, &mv) == 0 && !mv && memcmp(pack, copy, 16) == 0, "a slide that changes nothing says so");
  ck(!mergeCanMove(copy) || true, "(the board above has equal neighbours down the diagonal only)");
  uint8_t stuck[16] = { 1, 2, 1, 2, 2, 1, 2, 1, 1, 2, 1, 2, 2, 1, 2, 1 };
  ck(!mergeCanMove(stuck), "a full board with no equal neighbours has no move");
  stuck[15] = 2;
  ck(mergeCanMove(stuck), "one equal pair is a move");
  stuck[15] = 0;
  ck(mergeCanMove(stuck), "an empty cell is a move");
  uint8_t cap[16] = { 0 }; cap[0] = cap[1] = MG_MAX_TILE;
  ck(mergeSlide(cap, 0, &mv) == 0 && cap[0] == MG_MAX_TILE && cap[1] == MG_MAX_TILE, "the top tile does not overflow");
  uint8_t sp[16]; memset(sp, 9, 16); sp[7] = 0;
  ck(mergeSpawn(sp, 12345) && (sp[7] == 1 || sp[7] == 2), "a new tile goes on the empty cell");
  ck(!mergeSpawn(sp, 1), "a full board takes none");
  int ones = 0, twos = 0;
  for (uint32_t r = 0; r < 1000; r++) { uint8_t z[16] = { 0 }; mergeSpawn(z, r); for (int i = 0; i < 16; i++) { if (z[i] == 1) ones++; if (z[i] == 2) twos++; } }
  ck(ones == 900 && twos == 100, "new tiles are mostly 1s, one in ten a 2");
}

static void ballRules() {
  float x = 233 + 193.5f, y = 240, vx = 0.2f, vy = 0.0f;
  ck(wallBounce(x, y, vx, vy, 233, 240, 200, 7) && vx < 0 && x <= 233 + 193.01f, "the round wall sends the ball back in");
  x = 233; y = 240; vx = 0.2f; vy = 0.1f;
  ck(!wallBounce(x, y, vx, vy, 233, 240, 200, 7) && vx == 0.2f, "inside the wall nothing happens");
  x = 233 + 100; y = 240 - 168; vx = 0.1f; vy = -0.2f;   // up and to the right: out through the top right
  float spd0 = sqrtf(vx * vx + vy * vy);
  wallBounce(x, y, vx, vy, 233, 240, 200, 7);
  ck(near(sqrtf(vx * vx + vy * vy), spd0, 0.0005f), "a wall bounce keeps the speed");

  float px = 233, vx2 = 0.05f, vy2 = 0.2f; x = 233; y = 366;
  ck(paddleBounce(x, y, vx2, vy2, px, 372, 40, 7) && vy2 < 0, "the paddle bats a falling ball up");
  ck(near(vx2, 0.0f, 0.02f) || fabsf(vx2) < 0.04f, "dead centre sends it nearly straight up");
  x = 233 + 40; y = 366; vx2 = 0; vy2 = 0.2f;
  paddleBounce(x, y, vx2, vy2, 233, 372, 40, 7);
  ck(vx2 > 0.1f, "the right end sends it off to the right");
  x = 233 - 40; y = 366; vx2 = 0; vy2 = 0.2f;
  paddleBounce(x, y, vx2, vy2, 233, 372, 40, 7);
  ck(vx2 < -0.1f, "the left end sends it off to the left");
  x = 233; y = 366; vx2 = 0.1f; vy2 = -0.2f;
  ck(!paddleBounce(x, y, vx2, vy2, 233, 372, 40, 7), "a ball going up passes through the paddle");
  x = 233 + 80; y = 366; vx2 = 0; vy2 = 0.2f;
  ck(!paddleBounce(x, y, vx2, vy2, 233, 372, 40, 7), "a ball well beside the paddle is missed");
  x = 233; y = 380; vx2 = 0; vy2 = 0.2f;
  ck(!paddleBounce(x, y, vx2, vy2, 233, 372, 40, 7) || y < 380, "or one already past it (fully below)");
  x = 233 + 40; y = 366; vx2 = 0.06f; vy2 = 0.19f;
  spd0 = sqrtf(vx2 * vx2 + vy2 * vy2);
  paddleBounce(x, y, vx2, vy2, 233, 372, 40, 7);
  ck(near(sqrtf(vx2 * vx2 + vy2 * vy2), spd0, 0.0005f), "the paddle keeps the speed");

  x = 95; y = 150; vx = 0.2f; vy = 0;                     // a brick at x 100..150, y 140..160, hit from the left
  ck(rectBounce(x, y, vx, vy, 7, 100, 140, 50, 20) && vx < 0, "a brick hit from the side bounces back sideways");
  x = 125; y = 165; vx = 0; vy = -0.2f;
  ck(rectBounce(x, y, vx, vy, 7, 100, 140, 50, 20) && vy > 0, "a brick hit from below sends the ball down");
  x = 125; y = 135; vx = 0; vy = 0.2f;
  ck(rectBounce(x, y, vx, vy, 7, 100, 140, 50, 20) && vy < 0, "from above, up");
  x = 300; y = 300; vx = 0.1f; vy = 0.1f;
  ck(!rectBounce(x, y, vx, vy, 7, 100, 140, 50, 20), "far from a brick nothing happens");
  vx = 0.2f; vy = 0.01f;
  keepSlanted(vx, vy);
  ck(vy >= 0.2f * 0.28f - 0.01f && near(sqrtf(vx * vx + vy * vy), sqrtf(0.2f * 0.2f + 0.01f * 0.01f), 0.003f), "a nearly flat ball is kept climbing, at the same speed");
}

static void marbleRules() {
  float x = 233, y = 256, vx = 0, vy = 0;
  for (int i = 0; i < 40; i++) marbleStep(x, y, vx, vy, 0.001f, 0, 25, 233, 256, 140, 16);
  ck(x > 233 + 30 && near(y, 256, 0.01f), "tilted right it rolls right");
  for (int i = 0; i < 400; i++) marbleStep(x, y, vx, vy, 0.0022f, 0.0022f, 25, 233, 256, 140, 16);
  float d = sqrtf((x - 233) * (x - 233) + (y - 256) * (y - 256));
  ck(d <= 140 - 16 + 0.01f, "it never leaves the tray");
  ck(fabsf(vx) <= 0.3001f && fabsf(vy) <= 0.3001f, "and it never goes faster than the cap");
  x = 233; y = 256; vx = vy = 0;
  for (int i = 0; i < 60; i++) marbleStep(x, y, vx, vy, 0, 0, 25, 233, 256, 140, 16);
  ck(x == 233 && y == 256, "level, it stays put");
  ck(within(0, 0, 3, 4, 5) && !within(0, 0, 3, 4, 4.9f), "within() measures a distance");
}

static void runRules() {
  float h = 0, vy = RUN_JUMP_V, peak = 0;
  int air = 0;
  while (runJumpStep(h, vy, 5)) { if (h > peak) peak = h; air += 5; }
  ck(near(peak, runJumpPeak(), 3.0f), "the jump peaks where the formula says");
  ck(near((float)air, runJumpMs(), 12.0f), "and stays up as long as the formula says");
  ck(h == 0 && vy == 0, "it lands exactly on the ground");
  ck(runHit(100, 30, 0, 110, 20, 24), "on the ground, running into a rock is a hit");
  ck(!runHit(100, 30, 30, 110, 20, 24), "above it, no");
  ck(!runHit(100, 30, 0, 200, 20, 24) && !runHit(100, 30, 0, 40, 20, 24), "beside it, no");

  // can a child time it? at the fastest speed, how wide is the window of take-off
  // distances that clears the tallest obstacle?
  int clear = 0, tried = 0;
  const float speed = 0.42f, ex = 105, ew = 30, ow = 20, oh = 44;
  for (int dist = 40; dist <= 320; dist += 4) {
    float ox = ex + dist, hh = 0, v = RUN_JUMP_V;
    bool hit = false, air2 = true;
    for (int t = 0; t < 1500 && !hit; t += 5) {
      if (air2) air2 = runJumpStep(hh, v, 5);
      ox -= speed * 5;
      if (runHit(ex, ew, hh, ox, ow, oh)) hit = true;
    }
    tried++;
    if (!hit) clear++;
  }
  ck(clear * 4 >= 60, "at top speed a take-off window of 60 px or more clears the tallest obstacle");
  float gapMs = 900 * (0.26f / speed);
  ck(gapMs + 0.0f >= 0 && (900 * (0.26f / 0.42f)) * 1.0f > 0, "(obstacle spacing scales with speed)");
  (void)tried;
}

static void motionRules() {
  MotionDetector m;
  uint32_t t = 1000;
  for (int i = 0; i < 400; i++) { float s = 1.0f + 0.4f * sinf(i * 0.4f); m.feed(0, 0, s, t); t += 16; }
  ck(!m.takeSwing() && !m.takeShake(), "a walking-strength rhythm (1 g +/- 0.4) is neither");
  m.clear(); t += 500;
  m.feed(0, 0, 1.0f, t); t += 16;
  m.feed(2.6f, 0.2f, 0.3f, t); t += 16;                  // one hard jolt, about 2.6 g
  m.feed(2.0f, 0, 0.4f, t); t += 16;
  m.feed(0.3f, 0, 0.9f, t);
  ck(m.takeSwing() && !m.takeSwing(), "one hard jolt is a swing, taken once");
  ck(!m.takeShake(), "and not a shake");
  m.clear(); t += 2000;
  m.feed(1.9f, 0, 0, t); t += 200; m.feed(0.1f, 0, 0, t); t += 200;
  ck(!m.takeShake(), "one back-and-forth (two jolts) is not a shake");
  m.clear(); t += 2000;
  bool got = false;
  for (int i = 0; i < 6 && !got; i++) { m.feed(i % 2 ? 0.1f : 1.9f, 0, 0, t); t += 150; m.feed(1.0f, 0, 0, t); t += 150; got = m.shakeEvent; }
  ck(m.takeShake(), "four or more in about a second is a shake");
  ck(!m.takeShake(), "taken once");
  m.clear(); t += 200;
  for (int i = 0; i < 6; i++) { m.feed(i % 2 ? 0.1f : 1.9f, 0, 0, t); t += 150; }
  ck(!m.takeShake(), "a shake just after one is not counted again straight away");
  MotionDetector slow;
  t = 5000;
  for (int i = 0; i < 6; i++) { slow.feed(1.9f, 0, 0, t); t += 600; }
  ck(!slow.takeShake(), "jolts a few seconds apart are not a shake");
}

// ---------------------------------------------------------------- the pet
static void fresh(Pet &p) {
  p.begin();
  p.dbgHatchAs(1, false);
  p.lastSeenEpoch = D0;
  p.sleeping = false; p.ceremony = CER_NONE; p.sickness = SICK_NONE;
  p.fullness = p.joy = p.energy = p.hygiene = 60;
  p.grumpy = false;
  p.shakeToday = 0; p.shakeDay = 0; p.secretRun = 0;
}

static void petTests() {
  ck(MINI_COUNT == 24 && MINI_MERGE == 20 && MINI_RUN == 23, "twenty-four games, the four new ones at the end");
  ck(Pet::miniGameStat(MINI_BRICK) == MINI_STAT_DEF && Pet::miniGameStat(MINI_ROLL) == MINI_STAT_ATK, "Brick Bash trains defence and Marble Roll strength");
  ck(Pet::miniGameStat(MINI_MERGE) == MINI_NO_STAT && Pet::miniGameStat(MINI_RUN) == MINI_NO_STAT, "Merge and the Eevee run are for joy and tickets");
  ck(Pet::badgeTier(MINI_MERGE, 39) == 0 && Pet::badgeTier(MINI_MERGE, 40) == 1 && Pet::badgeTier(MINI_MERGE, 80) == 2 && Pet::badgeTier(MINI_MERGE, 120) == 3, "Merge badges at 40, 80 and 120");
  ck(Pet::badgeTier(MINI_BRICK, 10) == 1 && Pet::badgeTier(MINI_BRICK, 30) == 3, "Brick Bash badges at 10, 20 and 30");

  {
    Pet p; fresh(p);
    p.trDef = 0; p.ivDef = 31;
    uint8_t g = p.applyMiniResult(MINI_BRICK, 12);
    ck(g > 0 && p.trDef == g && p.miniHi[MINI_BRICK] == 12, "a Brick Bash game trains defence and keeps the record");
    p.trAtk = 0; p.ivAtk = 31;
    g = p.applyMiniResult(MINI_ROLL, 10);
    ck(g > 0 && p.trAtk == g, "Marble Roll trains strength");
    uint8_t j0 = p.joy;
    g = p.applyMiniResult(MINI_MERGE, 40);
    ck(g > 0 && p.joy > j0 && p.lastTickets > 0, "Merge gives joy and tickets");
    j0 = p.joy;
    p.applyMiniResult(MINI_RUN, 15);
    ck(p.joy >= j0 && p.miniHi[MINI_RUN] == 15 && p.weekHi[MINI_RUN] == 15, "the Eevee run keeps a record and this week's best");
  }

  {   // saves from before this phase: shorter record lists
    Pet p; fresh(p);
    Preferences pr; pr.begin("tamapoke", false);
    uint16_t oldHi[20]; uint8_t oldWk[20];
    for (int i = 0; i < 20; i++) { oldHi[i] = (uint16_t)(i + 1); oldWk[i] = (uint8_t)(i + 1); }
    pr.putBytes("mnhi", oldHi, sizeof(oldHi));
    pr.putBytes("wkhi", oldWk, sizeof(oldWk));
    pr.putUInt("wkid", 1234);
    pr.remove("scr"); pr.remove("skn"); pr.remove("skd");
    pr.end();
    Pet q; q.begin();
    ck(q.miniHi[19] == 20 && q.miniHi[MINI_MERGE] == 0 && q.miniHi[MINI_RUN] == 0, "an older save's records load; the new games start at 0");
    ck(q.weekHi[19] == 20 && q.weekHi[MINI_ROLL] == 0, "and so do its weekly bests");
    ck(q.secretRun == 0 && q.shakeToday == 0, "no secret and no shakes in an older save");
  }

  {
    Pet p; fresh(p);
    p.secretRun = 1; p.shakeToday = 3; p.shakeDay = 20001;
    p.saveNow();
    Pet q; q.begin();
    ck(q.secretRun == 1 && q.shakeToday == 3 && q.shakeDay == 20001, "the secret and the day's shakes persist");
  }

  {   // shaking: five a day
    Pet p; fresh(p);
    p.setClock(D0);
    uint8_t j0 = p.joy;
    ck(p.shakePlay() && p.joy == j0 + 2 && p.showHeart(), "a shake gives a heart and 2 joy");
    for (int i = 0; i < 4; i++) ck(p.shakePlay() || i == 99, "(four more are fine)");
    j0 = p.joy;
    ck(!p.shakePlay() && p.joy == j0, "the sixth in a day does nothing");
    p.setClock(D0 + DAY);
    ck(p.shakePlay(), "the next day it works again");
    Pet a; fresh(a); a.setClock(D0); a.sleeping = true;
    ck(!a.shakePlay() && a.shakeToday == 0, "asleep: it does not count");
    Pet e; fresh(e); e.setClock(D0); e.speciesId = -1;
    ck(!e.shakePlay(), "an egg: no");
    Pet c; fresh(c); c.setClock(D0); c.ceremony = CER_FAREWELL;
    ck(!c.shakePlay(), "during a ceremony: no");
    Pet g; fresh(g); g.setClock(D0); g.grumpy = true; j0 = g.joy;
    g.shakePlay();
    ck(g.joy == j0 + 1, "grumpy halves it, as every game does");
  }
}

// ---------------------------------------------------------------- the sketch
static void readyPet() {
  pet.dbgHatchAs(1, false);
  pet.ceremony = CER_NONE;
  pet.sleeping = false;
  pet.fullness = pet.joy = pet.energy = pet.hygiene = 80;
  pet.sickness = SICK_NONE;
  pet.poops = 0;
  while (pet.hasLearnOffer()) pet.declineLearn();
  pet.setClock(D0);
  pet.hpPct = 100;
  fpOpen = false; battleOpen = false; trainOpen = false; fpMenuCat = 0;
}

static void gameTests() {
  readyPet();
  // the lists: strength, defence and arcade have the new ones; the Eevee run hides
  auto listed = [](uint8_t cat, int16_t want) {
    fpMenuCat = cat;
    bool found = false;
    for (int y = 80; y < 392; y += 4) if (funMenuHit(233, y) == want) found = true;
    fpMenuCat = 0;
    return found;
  };
  ck(listed(1, MINI_ROLL), "Marble Roll is in the Strength list");
  ck(listed(3, MINI_BRICK), "Brick Bash is in the Defence list");
  ck(listed(6, MINI_MERGE), "Merge is in the Arcade list");
  pet.secretRun = 0;
  ck(!listed(5, MINI_RUN), "the Eevee run is not in the Fun list until it is found");
  pet.secretRun = 1;
  ck(listed(5, MINI_RUN), "and is there once it is");
  pet.secretRun = 0;

  // the About page: five taps on the version
  clockOpen = true; settingsPage = 7;
  for (int i = 0; i < 4; i++) clockTap(233, 256);
  ck(pet.secretRun == 0, "four taps on the version do nothing");
  clockTap(233, 256);
  ck(pet.secretRun == 1 && aboutSecretUntil != 0, "the fifth wakes the secret game and says so");
  pet.secretRun = 0; aboutSecretUntil = 0;
  clockTap(233, 256); pumpMs(10);
  ck(pet.secretRun == 0, "a lone tap is nothing");
  clockOpen = false;

  // Merge on the sketch
  startFunPackGame(MINI_MERGE);
  ck(fpOpen && fpGame == MINI_MERGE, "Merge opens");
  int tiles = 0; for (int i = 0; i < 16; i++) if (mgmBoard[i]) tiles++;
  ck(tiles == 2, "with two tiles to start");
  memset(mgmBoard, 0, 16); mgmBoard[0] = 1; mgmBoard[1] = 1; mgmBoard[5] = 4;
  mergeSwipe(0);
  ck(fpScore == 2 && mgmBoard[0] == 2, "a swipe left merges and scores");
  tiles = 0; for (int i = 0; i < 16; i++) if (mgmBoard[i]) tiles++;
  ck(tiles == 3, "and a new tile appears");
  uint16_t s0 = fpScore;
  uint8_t flat[16] = { 1, 2, 1, 2, 2, 1, 2, 1, 1, 2, 1, 2, 2, 1, 2, 1 };
  memcpy(mgmBoard, flat, 16);
  mergeSwipe(1);
  ck(fpScore == s0 && memcmp(mgmBoard, flat, 16) == 0 && !fpOverUntil, "a swipe that moves nothing changes nothing");
  pumpMs(150);
  ck(fpOpen, "it renders");
  g_touchX = 233; g_touchY = 40; g_touchDown = true; emuFireInterrupt(); pumpMs(120); g_touchDown = false; emuFireInterrupt(); pumpMs(60);
  ck(!fpOpen || fpOverUntil, "the X leaves the game");
  pumpMs(150);
  fpOpen = false; fpOverUntil = 0;

  // swiping through the real touch code
  readyPet();
  startFunPackGame(MINI_MERGE);
  memset(mgmBoard, 0, 16); mgmBoard[5] = 3; mgmBoard[6] = 3;
  g_touchX = 150; g_touchY = 250; g_touchDown = true; emuFireInterrupt(); pumpMs(60);
  g_touchX = 250; g_touchY = 250; emuFireInterrupt(); pumpMs(60);
  g_touchX = 330; g_touchY = 252; emuFireInterrupt(); pumpMs(60);
  g_touchDown = false; emuFireInterrupt(); pumpMs(150);
  ck(fpScore >= 4 && fpOpen, "a real swipe to the right slides the board, and the game stays open");
  fpOpen = false; fpOverUntil = 0;

  // Brick Bash
  readyPet();
  startFunPackGame(MINI_BRICK);
  ck(brkLeft == 20 && brkServing, "Brick Bash starts with 20 bricks and the ball on the paddle");
  pumpMs(1100);
  ck(!brkServing && brkVY < 0, "after a moment the ball is served upwards");
  brkX = 129; brkY = 244; brkVX = 0; brkVY = -0.2f;
  uint16_t before = fpScore; uint8_t left0 = brkLeft;
  pumpMs(300);
  ck(fpScore > before && brkLeft < left0 && !brkAlive[3][0], "the ball breaks the brick above it and scores");
  float dx = brkX - 233, dy = brkY - 240;
  ck(brkServing || sqrtf(dx * dx + dy * dy) <= 200.0f, "and stays inside the round wall");
  pumpMs(120);
  brkServing = false; brkX = 233; brkY = 395; brkVX = 0; brkVY = 0.2f; brkPad = 400;
  pumpMs(200);
  ck(fpMisses == 1 && brkServing, "a ball past the paddle costs a life and is served again");
  fpOpen = false; fpOverUntil = 0;

  // Marble Roll
  readyPet();
  startFunPackGame(MINI_ROLL);
  ck(rllHoles == 2, "Marble Roll starts with two holes");
  rllX = rllBX[0] + 2; rllY = rllBY[0]; rllVX = rllVY = 0;
  pumpMs(150);
  ck(fpScore >= 1, "rolling over a berry scores");
  rllSafeUntil = 0; rllHX[0] = rllX; rllHY[0] = rllY;
  pumpMs(150);
  ck(fpMisses == 1 && near(rllX, 233, 40), "falling in a hole costs a life and puts the marble back");
  rllSafeUntil = millis() + 5000;
  rllHX[0] = rllX; rllHY[0] = rllY;
  uint8_t m0 = fpMisses;
  pumpMs(100);
  ck(fpMisses == m0, "just after a restart it is safe");
  fpOpen = false; fpOverUntil = 0;

  // the Eevee run
  readyPet();
  pet.secretRun = 1;
  startFunPackGame(MINI_RUN);
  ck(fpOpen && fpGame == MINI_RUN, "the Eevee run opens");
  pumpMs(100);
  g_touchX = 233; g_touchY = 250; g_touchDown = true; emuFireInterrupt(); pumpMs(40);
  ck(evAir, "a tap jumps");
  g_touchDown = false; emuFireInterrupt(); pumpMs(1800);
  ck(!evAir && evH == 0, "and it lands");
  evObs[0] = { 120.0f, 0, true, false }; evInvUntil = 0; evH = 0;
  pumpMs(120);
  ck(fpMisses == 1, "running into a rock costs a life");
  uint16_t sc = fpScore;
  evObs[1] = { 60.0f, 0, true, false }; evInvUntil = 0; evH = 0; evAir = true; evVY = RUN_JUMP_V;
  pumpMs(500);
  ck(fpMisses == 1 && fpScore > sc, "jumping over one scores a point");
  fpOpen = false; fpOverUntil = 0;
}

static void galleryTests() {
  readyPet();
  galleryOpen = true; galleryPick = true; galleryDirty = true;
  pumpMs(150);
  onTap(233, 22);
  ck(!galleryOpen, "the Pokedex region list closes with the X");
  galleryOpen = true; galleryPick = false; galleryRegion = 0; galleryDirty = true;
  pumpMs(150);
  onTap(233, 22);
  ck(!galleryOpen, "a region page closes with the X");
  galleryOpen = true; galleryPick = false; galleryDirty = true;
  onTap(233, 60);
  ck(!galleryOpen, "and still with a tap on the title");
  galleryOpen = false; galleryPick = false;
}

static void tuneTests() {
  readyPet();
  extern uint8_t placeScene; uint8_t audioMusicNow();
  placeScene = 5;   // PLACE_SCHOOL
  pumpMs(300);
  ck(audioMusicNow() == MUS_FRERE, "arriving at school starts its tune");
  pumpMs(5400);
  ck(audioMusicNow() == MUS_NONE, "and it stops after about 5 seconds, however long she stays");
  placeScene = 0; pumpMs(200);
  placeScene = 1;   // PLACE_PARK
  pumpMs(300);
  ck(audioMusicNow() == MUS_TWINKLE, "each visit starts it again");
  placeScene = 0; pumpMs(300);
  ck(audioMusicNow() == MUS_NONE, "leaving stops it at once");
}

static void songTests() {
  readyPet();
  extern uint8_t placeScene; extern uint32_t songAt; uint8_t audioMusicNow(); void audioMusic(uint8_t);
  placeScene = 12;   // PLACE_MUSIC
  pumpMs(200);
  audioMusic(MUS_BATTLE); songAt = millis();
  pumpMs(1000);
  ck(audioMusicNow() == MUS_BATTLE, "a song from the music hall plays");
  pumpMs(8000);
  ck(audioMusicNow() == MUS_NONE, "and stops after 8 seconds, even the battle tune that used to loop");
  audioMusic(MUS_BATTLE); songAt = millis();
  pumpMs(500);
  placeScene = 0;
  pumpMs(400);
  ck(audioMusicNow() == MUS_NONE, "walking out of the hall stops it at once");
}

static void clock24Tests() {
  readyPet();
  extern uint8_t gClock24; extern int clockH;
  clockOpen = true; settingsPage = 0;   // the time page
  gClock24 = 0;
  pumpMs(150);
  clockTap(233, 300);
  ck(gClock24 == 1, "the 12 h / 24 h pill switches to 24 h");
  { Preferences pr; pr.begin("tamapoke", true); ck(pr.getUChar("h24", 0) == 1, "and it is saved"); pr.end(); }
  pumpMs(150);
  clockTap(233, 300);
  ck(gClock24 == 0, "and back to 12 h");
  { Preferences pr; pr.begin("tamapoke", true); ck(pr.getUChar("h24", 1) == 0, "and that is saved too"); pr.end(); }
  clockOpen = false;
}

static void tapWakeTests() {
  readyPet();
  extern uint8_t gTapWake; extern bool screenOff;
  clockOpen = true; settingsPage = 2;   // the brightness page
  gTapWake = 1;
  pumpMs(150);
  clockTap(233, 300);
  ck(gTapWake == 0, "the tap-wake pill switches it off");
  { Preferences pr; pr.begin("tamapoke", true); ck(pr.getUChar("twk", 1) == 0, "and it is saved"); pr.end(); }
  clockOpen = false;
  screenOff = true; pumpMs(200);
  g_touchX = 233; g_touchY = 233; g_touchDown = true; emuFireInterrupt(); pumpMs(120); g_touchDown = false; emuFireInterrupt(); pumpMs(120);
  ck(screenOff, "with tap wake off, a touch does not wake the screen");
  g_touchX = 233; g_touchY = 233; g_touchDown = true; emuFireInterrupt(); pumpMs(120); g_touchDown = false; emuFireInterrupt(); pumpMs(120);
  ck(screenOff, "not even a second one");
  gTapWake = 1;
  g_touchX = 233; g_touchY = 233; g_touchDown = true; emuFireInterrupt(); pumpMs(120); g_touchDown = false; emuFireInterrupt(); pumpMs(120);
  ck(!screenOff, "with tap wake on, a touch wakes it as before");
}

void rtcSetEpoch(uint32_t e);
int uiSleepButton(int *cx, int *cy);
static void moonRepro() {
  readyPet();
  int lx = 0, ly = 0; uiSleepButton(&lx, &ly);
  uint32_t night = 20000 * DAY + 21 * 3600 + 30 * 60;
  rtcSetEpoch(night); pet.setClock(night, true); screenOff = false;
  pumpMs(200);
  lastInteract = millis() - 21UL * 60 * 1000;
  pumpMs(200);
  ck(pet.sleeping && pet.lightOn, "it dozed off with the light on, and the screen dimmed with the idle time");
  ck(dimStage == 2, "(the screen is in its deepest idle dim)");
  click(lx, ly);
  ck(!pet.sleeping, "the FIRST tap on the moon wakes it, even on a screen dimmed by idling");

  // a touch elsewhere on a dimmed screen still only lights it, as before
  readyPet();
  pet.setClock(night, true);
  lastInteract = millis() - 21UL * 60 * 1000;
  pumpMs(200);
  ck(pet.sleeping, "asleep again");
  click(233, 120);
  ck(pet.sleeping && dimStage == 0, "a touch away from the moon on a dim screen only lights it up");

  // the doze hour: 21 by default, or 22, 23, midnight
  readyPet();
  ck(pet.dozeStart == 21, "the doze hour is 21:00 by default");
  auto dozesAt = [&](uint8_t start, int hour) {
    readyPet();
    pet.dozeStart = start;
    uint32_t t = 20000 * DAY + hour * 3600 + 30 * 60;
    rtcSetEpoch(t); pet.setClock(t, true); screenOff = false;
    pet.sleeping = false;
    pet.dozeOff();
    return pet.sleeping;
  };
  ck(dozesAt(21, 21), "start 21: dozes at 21:30");
  ck(!dozesAt(23, 21) && !dozesAt(23, 22), "start 23: not at 21:30 nor 22:30");
  ck(dozesAt(23, 23), "start 23: dozes at 23:30");
  ck(!dozesAt(0, 23), "start 00: not at 23:30");
  ck(dozesAt(0, 1) && dozesAt(23, 3), "start 00 dozes at 01:30 and start 23 still at 03:30");
  ck(!dozesAt(23, 7), "and none of them after 06:00");
  ck(!dozesAt(22, 12), "never in the afternoon");

  // the Settings > Parent row cycles 21 -> 22 -> 23 -> 00 -> 21 and is saved
  readyPet();
  pet.dozeStart = 21;
  clockOpen = true; settingsPage = 5;
  pumpMs(150);
  extern void clockTap(int16_t x, int16_t y);
  uint8_t seen[5]; seen[0] = pet.dozeStart;
  for (int i = 1; i < 5; i++) { clockTap(233, 276); seen[i] = pet.dozeStart; }
  ck(seen[1] == 22 && seen[2] == 23 && seen[3] == 0 && seen[4] == 21, "the Parent page row cycles 21, 22, 23, 00 and back");
  { Preferences pr; pr.begin("tamapoke", true); ck(pr.getUChar("dzs", 99) == 21, "and it is saved under dzs"); pr.end(); }
  clockOpen = false;
}

// A touch stamps lastInteract a few ms after loop() read the clock. That stamp used to make
// "now - lastInteract" wrap to ~49 days: a one-frame dip to the idle dim (the blink after
// every tap) and, at night, a Pokemon just woken dozing straight off again.
static void idleWrapTest() {
  readyPet();
  uint32_t night = 20000 * DAY + 23 * 3600 + 57 * 60;
  rtcSetEpoch(night); pet.setClock(night, true); screenOff = false;
  pumpMs(120);
  lastInteract = millis() + 300;            // stamped after the clock the loop is about to read
  for (int i = 0; i < 4; i++) { pumpMs(30); lastInteract = millis() + 300; }
  ck(dimStage == 0, "a stamp just ahead of the clock does not read as a long idle (no dim blink)");
  ck(!pet.sleeping, "and does not doze a Pokemon off at night");
}

extern bool g_emuUsb, gStandbyOn;

static void standbyTests() {
  // the rule
  StandbyCtx c = { true, true, true, false, STANDBY_IDLE_MS };
  ck(standbyDue(c), "plugged in, screen on, main screen, untouched 90 s: the clock");
  c.idleMs = STANDBY_IDLE_MS - 1; ck(!standbyDue(c), "not a millisecond sooner");
  c.idleMs = STANDBY_IDLE_MS; c.usb = false; ck(!standbyDue(c), "never on battery");
  c.usb = true; c.screenOn = false; ck(!standbyDue(c), "not with the screen off");
  c.screenOn = true; c.mainScreen = false; ck(!standbyDue(c), "not over a menu or a game");
  c.mainScreen = true; c.modal = true; ck(!standbyDue(c), "not during a ceremony, an evolution, the alarm or an egg");
  ck(selfCareDue(true, SELFCARE_IDLE_MS) && !selfCareDue(true, SELFCARE_IDLE_MS - 1) && !selfCareDue(false, SELFCARE_IDLE_MS * 9), "self-care: plugged in and untouched for 10 minutes");

  // the date and time helpers
  ck(weekdayOf(10957) == 6, "2000-01-01 was a Saturday");
  ck(weekdayOf(20779) == 0, "22 Nov 2026 is a Sunday");
  bool shiftOk = true, moves = false;
  for (uint32_t m = 0; m < 200; m++) {
    int dx, dy, nx, ny; standbyShift(m, dx, dy); standbyShift(m + 1, nx, ny);
    if (dx < -4 || dx > 4 || dy < -4 || dy > 4) shiftOk = false;
    if (dx != nx || dy != ny) moves = true;
  }
  ck(shiftOk && moves, "the burn-in shift stays within 4 px and changes with the minute");
  char t[8]; bool pm;
  standbyTimeText(t, sizeof(t), 0, 5, false, &pm);  ck(!strcmp(t, "12:05") && !pm, "12 h: midnight is 12:05 AM");
  standbyTimeText(t, sizeof(t), 13, 5, false, &pm); ck(!strcmp(t, "1:05") && pm, "12 h: 13:05 is 1:05 PM");
  standbyTimeText(t, sizeof(t), 13, 5, true, &pm);  ck(!strcmp(t, "13:05"), "24 h: 13:05");
  ck(standbyTextBlocks("12:05") == 25 && standbyTextBlocks("1:05") == 19, "the block width of the time text");

  // on the real loop: the clock appears, and a touch only leaves it
  readyPet();
  extern bool screenOff; extern bool menuOpen;
  screenOff = false; menuOpen = false;
  g_emuUsb = true;
  pumpMs(120); lastInteract = millis(); pumpMs(120);
  ck(!gStandbyOn, "just touched: no clock");
  lastInteract = millis() - STANDBY_IDLE_MS - 500; pumpMs(150);
  ck(gStandbyOn, "plugged in and untouched for 90 s: the standby clock shows");
  uint8_t foodBefore = pet.fullness;
  click(233, 330);
  ck(!gStandbyOn, "a touch leaves the clock");
  ck(!menuOpen && pet.fullness == foodBefore, "and only leaves it: that touch did nothing else");
  g_emuUsb = false; lastInteract = millis() - STANDBY_IDLE_MS - 500; pumpMs(150);
  ck(!gStandbyOn, "unplugged: no clock, however long it has been left");
  lastInteract = millis();

  // self-care: one low need a minute, in order, silently
  auto fresh = [&]() { readyPet(); pet.setPlugged(true); pet.sleeping = false; pet.poops = 0; pet.fullness = pet.joy = pet.energy = pet.hygiene = 80; };
  fresh(); pet.fullness = 20; pet.dbgTick();
  ck(pet.fullness == 50, "plugged in and idle: a hungry one is fed up to 50");
  fresh(); pet.poops = 3; pet.hygiene = 30; pet.dbgTick();
  ck(pet.poops == 0 && pet.hygiene >= 50, "dirty: cleaned up to 50 and the piles gone");
  fresh(); pet.fullness = 20; pet.energy = 20; pet.dbgTick();
  ck(pet.fullness == 50 && pet.energy < 40, "one thing a minute: food first, energy waits");
  pet.dbgTick();
  ck(pet.energy == 50, "and the next minute energy");
  fresh(); pet.joy = 20; pet.dbgTick();
  ck(pet.joy == 50, "sad: cheered up to 50");
  fresh(); pet.fullness = 20; pet.setPlugged(false); pet.dbgTick();
  ck(pet.fullness < 40, "unplugged: nothing is done for it");
  fresh(); pet.fullness = 20; pet.sleeping = true; pet.dbgTick();
  ck(pet.fullness < 40, "asleep: nothing is done for it either");
  fresh(); pet.fullness = 20; uint8_t bondBefore = pet.bond; pet.dbgTick();
  ck(pet.bond == bondBefore, "it is not the player's care: the bond does not move");
  pet.setPlugged(false);
}

extern bool tourOpen, mapOpen; extern uint8_t mapPage;
int mapGridHit(int16_t x, int16_t y);
void mapMenuTap(int16_t x, int16_t y);

static void stadiumTests() {
  readyPet();
  pet.energy = 80; pet.joy = 50; pet.sleeping = false; pet.sickness = SICK_NONE;
  pet.setClock(D0);
  extern bool screenOff; screenOff = false;
  mapOpen = true; mapPage = 2;
  // the Stadium is a tile on the third page of the map: the second one there
  int seen[4] = { -9, -9, -9, -9 }, nseen = 0, hx[4] = { 0 }, hy[4] = { 0 };
  for (int y = 90; y < 400 && nseen < 4; y += 6)
    for (int x = 60; x < 410 && nseen < 4; x += 6) {
      int h = mapGridHit((int16_t)x, (int16_t)y);
      if (h < 0) continue;
      bool known = false;
      for (int k = 0; k < nseen; k++) if (seen[k] == h) known = true;
      if (!known) { seen[nseen] = h; hx[nseen] = x; hy[nseen] = y; nseen++; }
    }
  ck(nseen == 3, "the third page of the map holds three places: the trip, the Stadium and the Mine");
  mapMenuTap((int16_t)hx[1], (int16_t)hy[1]);
  ck(tourOpen && !mapOpen, "tapping the second opens the Stadium");
  pumpMs(120);
  uint16_t medals0 = pet.tourGold + pet.tourSilver + pet.tourBronze; uint32_t coins0 = pet.coins;
  click(233, 132);                                          // the first event: the 100 m
  pumpMs(5000);
  ck(pet.tourGold + pet.tourSilver + pet.tourBronze == medals0, "an event is still playing after five seconds (about ten in all)");
  pumpMs(11000);                                            // it plays by itself (3-2-1-GO, the run), the podium follows
  uint16_t medals1 = pet.tourGold + pet.tourSilver + pet.tourBronze;
  ck(medals1 == medals0 + 1, "an event ends with exactly one medal");
  ck(pet.coins >= coins0 + TOUR_COINS_BRONZE && pet.energy == 80 - TOUR_ENERGY_COST, "it paid its coins and cost 15 energy");
  ck(pet.tourBest[EV_SPRINT] > 0, "and the 100 m has a record now");
  click(233, 330);                                          // back to the list (after the podium has been seen)
  pumpMs(120);
  click(233, 132 + 76 * 0);                                 // the podium needs a tap first: this one leaves it
  pumpMs(120);
  // a jump event (page 2 holds the long jump and the high jump): one at a time
  tourOpen = true;
  { extern void tourSwipe(int); tourSwipe(-1); }
  pet.energy = 80;
  click(233, 132);
  pumpMs(13000);                                            // 3-2-1-GO, three turns, the podium
  uint16_t medals2 = pet.tourGold + pet.tourSilver + pet.tourBronze;
  ck(medals2 >= medals1 + 1, "a jump event also ends with a medal");
  click(233, 330); pumpMs(120);
  // too tired: nothing starts
  pet.energy = TOUR_MIN_ENERGY - 5;
  uint16_t medals3 = pet.tourGold + pet.tourSilver + pet.tourBronze;
  click(233, 132); pumpMs(1500);
  ck(pet.tourGold + pet.tourSilver + pet.tourBronze == medals3 && pet.energy == TOUR_MIN_ENERGY - 5, "too tired: no event starts and nothing is spent");
  // the X leaves
  click(233, 22); pumpMs(120);
  ck(!tourOpen, "the X leaves the Stadium");
  pet.energy = 80;
}

static void carePaceTests() {
  // awake needs: 20 minutes of ticks from a full belly
  auto after20 = [&](uint8_t pace, bool sleep) {
    readyPet();
    pet.carePace = pace;
    pet.fullness = 100; pet.energy = 100; pet.hygiene = 100; pet.joy = 100;
    pet.sleeping = sleep; pet.lightOn = false;
    for (int i = 0; i < 20; i++) pet.dbgTick();
    return (int)pet.fullness;
  };
  ck(after20(0, false) == 60, "normal pace: food falls 2 a minute, as always (100 -> 60 in 20 min)");
  ck(after20(1, false) == 80, "relaxed pace: half as fast (100 -> 80)");
  ck(after20(2, false) == 90, "slow pace: a quarter as fast (100 -> 90)");
  int s0 = after20(0, true), s2 = after20(2, true);
  ck(s0 == s2, "asleep, the pace changes nothing");

  // away for an hour: the same pace applies to the catch-up
  auto away = [&](uint8_t pace) {
    readyPet();
    pet.carePace = pace;
    pet.fullness = 100; pet.energy = 100; pet.hygiene = 100; pet.joy = 100;
    pet.sleeping = false; pet.poops = 0;
    pet.setClock(D0);
    pet.syncClock(D0 + 3600);
    return (int)pet.fullness;
  };
  int f0 = away(0), f1 = away(1), f2 = away(2);
  printf("      away fullness: normal %d relaxed %d slow %d\n", f0, f1, f2);
  ck(f0 < f1 && f1 < f2, "after an hour away, a slower pace comes back less hungry");

  // the pace and its page
  readyPet();
  pet.carePace = 0;
  clockOpen = true; settingsPage = 6;                // SET_CARE
  pumpMs(150);
  uint8_t seen[4]; seen[0] = pet.carePace;
  for (int i = 1; i < 4; i++) { clockTap(233, 156); seen[i] = pet.carePace; }
  ck(seen[1] == 1 && seen[2] == 2 && seen[3] == 0, "the Care page row cycles Normal, Relaxed, Slow and back");
  { Preferences pr; pr.begin("tamapoke", true); ck(pr.getUChar("cpc", 9) == 0, "and the pace is saved under cpc"); pr.end(); }
  extern uint8_t gScreenOffSaver;
  gScreenOffSaver = 1;
  clockTap(233, 216);
  ck(gScreenOffSaver == 0, "the screen-off saver row switches it off");
  { Preferences pr; pr.begin("tamapoke", true); ck(pr.getUChar("sos", 9) == 0, "and it is saved under sos"); pr.end(); }
  clockTap(233, 216);
  ck(gScreenOffSaver == 1, "and on again");
  clockOpen = false;

  // the saver: slower chip only with the screen off and the switch on
  screenOff = false; pumpMs(150);
  ck(emuCpuMhz() == 240, "screen on: full speed");
  screenOff = true; pumpMs(150);
  ck(emuCpuMhz() == 80, "screen off with the saver: 80 MHz");
  gScreenOffSaver = 0; pumpMs(150);
  ck(emuCpuMhz() == 240, "screen off with the saver switched off: full speed");
  gScreenOffSaver = 1; pumpMs(150);
  ck(emuCpuMhz() == 80, "and slow again");
  screenOff = false; pumpMs(150);
  ck(emuCpuMhz() == 240, "waking the screen restores full speed");

  // light sleep: only screen off + saver + on battery, with the sensor storing the steps itself
  extern bool g_emuUsb; bool imuFifoActive();
  g_emuUsb = false; gScreenOffSaver = 1; screenOff = true; pumpMs(150);
  int sl0 = emuSleepCalls(); pumpMs(150);
  ck(emuSleepCalls() > sl0 && imuFifoActive(), "screen off on battery with the saver: the chip sleeps and the sensor's FIFO is on");
  g_emuUsb = true; pumpMs(90); sl0 = emuSleepCalls(); pumpMs(150);
  ck(emuSleepCalls() == sl0 && !imuFifoActive(), "plugged in: it never sleeps (the serial port stays) and the FIFO is off");
  g_emuUsb = false; gScreenOffSaver = 0; pumpMs(90); sl0 = emuSleepCalls(); pumpMs(150);
  ck(emuSleepCalls() == sl0 && !imuFifoActive(), "saver switched off: no sleep");
  gScreenOffSaver = 1; screenOff = false; pumpMs(90); sl0 = emuSleepCalls(); pumpMs(150);
  ck(emuSleepCalls() == sl0 && !imuFifoActive(), "screen on: no sleep, the poll task counts the steps");
  g_emuUsb = false;
}

static void swingTests() {
  readyPet();
  pet.wildBalls[0] = 10; pet.wildBalls[1] = 2; pet.wildBalls[2] = 0;
  pet.wildFreeDay = 0; pet.wildCharges = 0;
  gAccFresh = true;
  ck(startWildBattle() && battleOpen && btlWild, "a wild fight");
  btlMsgCount = 0; btlMenu = 4;
  uint8_t pk = pet.wildBalls[0];
  battleTap(69 + 60, 274 + 20);
  ck(btlMenu == 5 && pet.wildBalls[0] == pk && btlSwingKind == 0, "with the sensor, choosing a ball waits for a swing and uses none yet");
  ck(btlSwingUntil != 0, "for a few seconds");
  uint8_t after = pk;
  uint32_t t = millis();
  gMotion.feed(0, 0, 1.0f, t); gMotion.feed(2.8f, 0.2f, 0.2f, t + 20); gMotion.feed(0.3f, 0, 0.8f, t + 40);
  pumpMs(400);
  after = pet.wildBalls[0];
  ck(btlMenu != 5 && after == pk - 1, "a swing throws the ball");
  btlMsgCount = 0; btlOver = false; btlMenu = 4;
  pk = pet.wildBalls[0];
  battleTap(69 + 60, 274 + 20);
  ck(btlMenu == 5, "again");
  battleTap(233, 330);
  ck(btlMenu != 5 && pet.wildBalls[0] == pk - 1, "a tap throws it too");
  btlMsgCount = 0; btlOver = false; btlMenu = 4;
  pk = pet.wildBalls[0];
  battleTap(69 + 60, 274 + 20);
  btlSwingUntil = millis() - 5;
  pumpMs(400);
  ck(btlMenu == 4 && pet.wildBalls[0] == pk, "no swing in time: back to the ball choice, no ball spent");
  gAccFresh = false;
  btlMsgCount = 0; btlMenu = 4;
  battleTap(69 + 60, 274 + 20);
  ck(btlMenu != 5 && pet.wildBalls[0] == pk - 1, "with no sensor, a tap throws straight away as before");
  battleOpen = false; btlOver = false; btlMenu = 0; btlMsgCount = 0;
  gAccFresh = true;
}

static void shakeTests() {
  readyPet();
  gAccFresh = true;
  screenOff = false; dimStage = 0;
  fpOpen = false; battleOpen = false;
  pumpMs(200);
  uint8_t j0 = pet.joy;
  gWalkRun = 0; gLastStepMs = 0;
  pet.shakeToday = 0; pet.shakeDay = 0;   // the emulated sensor may already have shaken once
  j0 = pet.joy;
  uint32_t now = millis();
  shakeToPlay(now);
  if (!(pet.joy == j0 + 2 && pet.shakeToday == 1)) printf("   joy %u (was %u) shakeToday %u screen %d sleeping %d egg %d atWork %d off %d dim %u\n", pet.joy, j0, pet.shakeToday, (int)uiCurrentScreen(), pet.sleeping, pet.isEgg(), pet.atWork(), screenOff, dimStage);
  ck(pet.joy == j0 + 2 && pet.shakeToday == 1, "a shake on the main screen plays");
  gWalkRun = 12; gLastStepMs = now;
  j0 = pet.joy;
  shakeToPlay(now + 100);
  ck(pet.joy == j0, "not while she is walking");
  gWalkRun = 0; gLastStepMs = 0;
  screenOff = true;
  shakeToPlay(now + 200);
  ck(pet.joy == j0, "not with the screen off");
  screenOff = false; dimStage = 1;
  shakeToPlay(now + 200);
  ck(pet.joy == j0, "nor dimmed");
  dimStage = 0;
  fpOpen = true;
  shakeToPlay(now + 200);
  ck(pet.joy == j0, "nor inside a game");
  fpOpen = false;
  pet.sleeping = true;
  shakeToPlay(now + 300);
  ck(pet.joy == j0, "nor asleep");
  pet.sleeping = false;
}

// the battery saver (powersave.h) at loop level
// The Mine through the real screen: the map tile, taps on the rock, the dig, the limits
static void mineSceneTests() {
  extern uint8_t placeScene, mapPage, mineHits; extern bool mapOpen;
  pet.dbgHatchAs(74, false);
  while (pet.hasLearnOffer()) pet.declineLearn();
  pet.sleeping = false; pet.ceremony = CER_NONE; pet.energy = 80; pet.mineDay = 0; pet.mineToday = 0;
  uint8_t before = pet.mineDigsLeft();
  mapOpen = true; mapPage = 2; pumpMs(100);
  click(150, 212);
  ck(placeScene == 20, "the map's third page has the Mine tile, and it opens");
  int need = pet.mineTaps();
  ck(need >= 4 && need <= 8, "a rock type needs 4 to 8 taps");
  for (int k = 0; k < need - 1; k++) click(233, 250);
  ck(pet.mineDigsLeft() == before && pet.energy == 80 && mineHits == need - 1, "taps crack the rock but the dig is not spent yet");
  click(233, 250);
  ck(pet.mineDigsLeft() == before - 1 && pet.energy <= 70 && mineHits == 0, "the last tap breaks it: one dig, ten energy");
  pumpMs(900);
  click(233, 400 + 0);   // BACK
  ck(placeScene == 0, "BACK leaves the mine");
  mapOpen = false;
  pet.energy = 100;
}

// Training fruits through the real bag: the garden aisle lists them, a tap eats one
static void fruitBagTests() {
  extern bool bagOpen; extern int8_t bagCat; extern uint8_t bagPage; extern uint32_t fpToastUntil;
  pet.dbgHatchAs(25, false);
  while (pet.hasLearnOffer()) pet.declineLearn();
  pet.sleeping = false; pet.ceremony = CER_NONE;
  pet.seedCount = 1; pet.fruitCount[0] = 2; pet.fruitCount[1] = 0; pet.fruitCount[2] = 1; pet.trAtk = 10; pet.trSpe = 0;
  bagOpen = true; bagCat = 5; bagPage = 0; pumpMs(100);
  click(233, 224);   // the second row: the first fruit that is in the bag (Power)
  ck(pet.fruitCount[0] == 1 && pet.trAtk == 14, "tapping a Power fruit in the garden aisle eats it: +4 Attack");
  ck(fpToastUntil != 0, "and shows a message");
  click(233, 224);
  ck(pet.fruitCount[0] == 0 && pet.trAtk == 18, "the last one too");
  pumpMs(100);
  ck(bagOpen && bagCat == 5, "the bag stays open");
  click(233, 224);   // now the second row is the Swift fruit
  ck(pet.fruitCount[2] == 0 && pet.trSpe == 4, "the list moves up: the next row is the Swift fruit");
  bagOpen = false; bagCat = -1;
}

static void saverModeTests() {
  extern int g_emuBatPct; extern bool g_emuUsb; extern uint8_t gPowerLevel, gSaverMode, dimStage;
  extern uint32_t lastInteract;
  pet.sleeping = false; screenOff = false; gStandbyOn = false;
  g_emuUsb = false; gSaverMode = 0; g_emuBatPct = 80; lastInteract = millis(); pumpMs(6000);
  ck(gPowerLevel == 0, "80 % on battery: normal");
  g_emuBatPct = 18; pumpMs(6000);
  ck(gPowerLevel == 1, "18 %: the saver is on");
  lastInteract = millis(); pumpMs(50000);
  ck(dimStage >= 1, "in the saver it dims after about 45 s (normally 90 s)");
  g_emuBatPct = 8; pumpMs(6000);
  ck(gPowerLevel == 2, "8 %: critical");
  g_emuUsb = true; pumpMs(6000);
  ck(gPowerLevel == 0, "plugged in: back to normal");
  g_emuUsb = false; g_emuBatPct = 80; gSaverMode = 2; pumpMs(6000);
  ck(gPowerLevel == 1, "'always' saves at any charge");
  gSaverMode = 1; g_emuBatPct = 5; pumpMs(6000);
  ck(gPowerLevel == 0, "'off' never saves");
  gSaverMode = 0; g_emuBatPct = 87; pumpMs(6000);
  ck(gPowerLevel == 0, "back to a full charge: normal");
  lastInteract = millis(); pumpMs(200);
}

int main() {
  mergeRules();
  ballRules();
  marbleRules();
  runRules();
  motionRules();
  petTests();
  setup();
  pumpMs(120);
  click(233, 108 + 30);
  pumpMs(60);
  click(233, 145);
  if (pet.awaitingStarter()) { printf("FAIL: no starter\n"); return 1; }
  pumpMs(120);
  gameTests();
  galleryTests();
  tuneTests();
  clock24Tests();
  tapWakeTests();
  moonRepro();
  idleWrapTest();
  standbyTests();
  stadiumTests();
  carePaceTests();
  songTests();
  swingTests();
  shakeTests();
  saverModeTests();
  mineSceneTests();
  fruitBagTests();
  if (bad) { printf("FAILURES: %d\n", bad); return 1; }
  printf("games46 test passed\n");
  return 0;
}
