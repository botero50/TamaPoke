// Phase 42's safety fixes, on the real sketch: timers across the 49.7-day wrap of
// millis(), damaged sprite and thumbnail files, and taps just outside a grid.
#include "Arduino.h"
#include "Arduino_GFX_Library.h"
#include "Preferences.h"
#include "pet.h"
#include "audio.h"
#include "party.h"
#include "moves.h"
#include "battle.h"
#include "sdmon.h"
#include <chrono>
#include <thread>
#include <vector>

uint32_t g_seed = 0xBA7C4;
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
extern bool sackOpen, galleryOpen, kbOpen;
extern uint32_t sackUntil, sackOverUntil, fpToastUntil, confirmUntil;
extern int16_t galleryDetail;
extern uint32_t lastInteract; extern bool screenOff;
extern uint8_t nameLen;
void startSack();
void galleryTap(int16_t x, int16_t y); void keyboardTap(int16_t x, int16_t y);
uint8_t pmdFrameAt(const PmdAct &a, uint32_t t, bool loop);
void sweepOldTimers(uint32_t now);
void drawFpToast();

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
static void ready() {
  pet.fullness = pet.joy = pet.energy = pet.hygiene = 80;
  pet.poops = 0; pet.sickness = SICK_NONE; pet.sleeping = false;
  while (pet.hasLearnOffer()) pet.declineLearn();
}

static void helperTests() {
  ck(!timeOn(1000, 0) && timeUp(1000, 0), "0 means off");
  ck(!timeOn(0x90000000u, 0), "0 still means off past 24.8 days");
  ck(timeOn(1000, 2000) && !timeOn(2000, 2000) && !timeOn(3000, 2000), "a plain timer runs, then ends");
  ck(timeOn(0xFFFFFF00u, 0x00000100u), "a timer crossing the wrap is still running before it");
  ck(timeOn(0x00000010u, 0x00000100u), "and after it");
  ck(!timeOn(0x00000200u, 0x00000100u), "and ends on time");
  ck(!timeOn(0x00000200u, 0xFFFFFF00u), "a timer that ended just before the wrap stays ended");
}

static void wrapTests() {
  // Start two seconds before the wrap and live through it.
  emuSetMillis(0xFFFFFFFFu - 2000);
  lastInteract = millis();    // or the jump reads as weeks idle and the screen goes off
  ready();
  pet.shineUntil = millis() + 3000;
  fpToastUntil = millis() + 1000;
  ck(pet.shining(), "a shine runs before the wrap");
  pumpMs(2400);
  ck(millis() < 5000, "the clock has wrapped");
  ck(pet.shining(), "a shine still runs just after the wrap");
  drawFpToast();
  ck(fpToastUntil == 0, "a toast that ended before the wrap has cleared");
  pumpMs(1000);
  ck(!pet.shining(), "a shine ends on time after the wrap");

  emuSetMillis(0xFFFFFFFFu - 2000);
  ready();
  pet.shineUntil = 0;
  startSack();
  uint32_t until0 = sackUntil;
  pumpMs(3000);
  ck(millis() < 5000 && sackOpen && sackOverUntil == 0, "a 10 s game keeps going across the wrap");
  pumpMs(8000);
  ck(sackOverUntil != 0 || !sackOpen, "and ends when its 10 s are up");
  ck(until0 > 0xF0000000u || until0 < 20000, "(its deadline was set near the wrap)");
  pumpMs(4000);
  ck(!sackOpen, "the results screen closes too");

  // A timer that ended long ago must not come back to life 24.8 days later.
  emuSetMillis(1000);
  pet.shineUntil = 2000;
  sweepOldTimers(1000);
  ck(pet.shineUntil == 2000, "the sweep leaves a running timer alone");
  sweepOldTimers(2000 + 3600000);
  ck(pet.shineUntil == 2000, "and one that ended an hour ago");
  ck(timeOn(2000u + 0x80000100u, 2000), "(without a sweep, a 24.8-day-old timer reads as running)");
  sweepOldTimers(2000 + 86400000u + 1000);
  ck(pet.shineUntil == 0, "the sweep clears one that ended over a day ago");
  emuSetMillis(1000);
  pet.coinFlashUntil = 2000;
  emuSetMillis(2000 + 2u * 86400000u);
  pumpMs(120);
  ck(pet.coinFlashUntil == 0, "and the main loop runs the sweep");
  emuSetMillis(5000);
}

static void fileTests() {
  PmdAct a{};
  a.frames = 3;
  a.ms[0] = a.ms[1] = a.ms[2] = 0;
  uint8_t f = pmdFrameAt(a, 12345, true);       // used to spin forever
  ck(f < 3, "a sprite with all durations 0 no longer freezes");
  a.ms[0] = a.ms[1] = a.ms[2] = 100;
  ck(pmdFrameAt(a, 150, true) == 1 && pmdFrameAt(a, 250, true) == 2 && pmdFrameAt(a, 310, true) == 0,
     "a normal sprite still steps through its frames");
  ck(pmdFrameAt(a, 5000, false) == 2, "and a one-shot still stops on its last frame");
  ck(pmdFrameMs(0) == 100 && pmdFrameMs(80) == 80, "the loader replaces only a duration of 0");

  // A little thumbs.bin: 2 entries, then one 2x2 picture with 1 colour (9 bytes).
  std::vector<uint8_t> b(6 + 8 + 9, 0);
  memcpy(b.data(), "TPTH", 4);
  b[4] = 2;
  uint32_t good = 14;
  memcpy(&b[6], &good, 4);
  memcpy(&b[10], &good, 4);
  b[14] = 2; b[15] = 2; b[16] = 1;
  uint32_t n = (uint32_t)b.size();
  ck(thumbAt(b.data(), n, 2, 1) == b.data() + 14, "a good thumbnail is found");
  ck(!thumbAt(b.data(), n, 2, 0) && !thumbAt(b.data(), n, 2, 3), "a dex number outside the file is not");
  uint32_t far = 100000;
  memcpy(&b[10], &far, 4);
  ck(!thumbAt(b.data(), n, 2, 2), "an offset past the end of the file is refused");
  uint32_t huge = 0xFFFFFFFEu;
  memcpy(&b[10], &huge, 4);
  ck(!thumbAt(b.data(), n, 2, 2), "and one near the top of the number range");
  memcpy(&b[10], &good, 4);
  b[14] = 200;
  ck(!thumbAt(b.data(), n, 2, 2), "a picture running past the end is refused");
  ck(!thumbAt(b.data(), n, 2000, 1), "a table bigger than the file is refused");
  b[14] = 2;
  SdThumbs th;
  th.data = b.data(); th.size = n; th.count = 2; th.loaded = true;
  ck(th.get(1) == b.data() + 14, "SdThumbs::get uses the same check");
  th.data = nullptr;                            // the vector owns the memory
}

static void touchTests() {
  // The Pokedex grid starts at x 73, the keyboard's at (40, 150).
  galleryOpen = true; galleryDetail = 0;
  galleryTap(70, 200);
  ck(galleryDetail == 0, "a tap just left of the Pokedex grid selects nothing");
  galleryTap(75, 74);
  ck(galleryDetail == 0, "a tap above it selects nothing");
  galleryOpen = false; galleryDetail = 0;

  kbOpen = true;
  uint8_t n0 = nameLen;
  keyboardTap(37, 170);
  ck(nameLen == n0, "a tap just left of the keyboard types nothing");
  keyboardTap(100, 148);
  ck(nameLen == n0, "a tap just above it types nothing");
  keyboardTap(41, 151);
  ck(nameLen == n0 + 1, "the first key still types");
  nameLen = n0; kbOpen = false;
}

int main() {
  helperTests();
  fileTests();
  setup();
  pumpMs(120);
  click(233, 108 + 30);               // first boot: Kanto
  pumpMs(60);
  click(233, 145);                    // its first starter
  if (pet.awaitingStarter()) { printf("FAIL: no starter\n"); return 1; }
  if (pet.isEgg()) pet.dbgHatchAs(6, false);
  pumpMs(120);
  touchTests();
  wrapTests();
  if (bad) { printf("FAILURES: %d\n", bad); return 1; }
  printf("hardening test passed\n");
  return 0;
}
