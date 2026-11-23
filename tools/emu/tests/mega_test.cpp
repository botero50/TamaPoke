// Mega Evolution (mega.h and the Pet's stone/timer state) and the long jump's path (tourney.h).
#include "Arduino.h"
#include "Preferences.h"
#include "pet.h"
#include "tourney.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
uint32_t g_seed = 22; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
volatile int g_touchX = 0, g_touchY = 0; volatile bool g_touchDown = false; bool wasPressed = false;
static uint32_t g_ms = 0; uint32_t millis() { return g_ms; }
void FakeESP::restart() { exit(0); }
int FakeSerial::available() { return 0; } String FakeSerial::readStringUntil(char) { return String(""); }
void sfxPlay(uint8_t) {}
static int bad = 0;
static void ck(bool ok, const char *w) { printf("%s  %s\n", ok ? "PASS" : "FAIL", w); if (!ok) bad++; }
static const uint32_t DAY = 86400, D0 = 20000 * DAY + 12 * 3600;

int main() {
  // the table
  ck(MEGA_COUNT == 41, "41 species have Mega art");
  bool sorted = true, inDex = true;
  for (int i = 0; i < MEGA_COUNT; i++) {
    if (i && MEGA_DEX[i] <= MEGA_DEX[i - 1]) sorted = false;
    if (MEGA_DEX[i] < 1 || MEGA_DEX[i] > DEX_COUNT) inDex = false;
  }
  ck(sorted && inDex, "the table is sorted and inside the dex");
  ck(megaHas(6) && megaHas(384) && megaHas(807) && !megaHas(1) && !megaHas(25) && !megaHas(0) && !megaHas(-1), "Charizard, Rayquaza and Zeraora yes; Bulbasaur, Pikachu no");
  char p[32]; megaPath(p, sizeof(p), 6);   ck(!strcmp(p, "/mons/m006.bin"), "the Mega sprite is /mons/m006.bin");
  megaPath(p, sizeof(p), 718);             ck(!strcmp(p, "/mons/m718.bin"), "and /mons/m718.bin");

  // the timer
  ck(megaRunning(1000, 999) && !megaRunning(1000, 1000) && !megaRunning(0, 5), "it runs up to the last second, 0 means none");
  ck(megaRemaining(1000, 400) == 600 && megaRemaining(1000, 1000) == 0 && megaRemaining(1000, 5000) == 0, "remaining time, never negative");

  // the stones: the first gold of three different days
  MegaProgress m = { 0, 0, false };
  m = megaGoldDay(m.days, m.stones); ck(m.days == 1 && m.stones == 0 && !m.earned, "day 1: no stone yet");
  m = megaGoldDay(m.days, m.stones); ck(m.days == 2 && m.stones == 0 && !m.earned, "day 2: no stone yet");
  m = megaGoldDay(m.days, m.stones); ck(m.days == 0 && m.stones == 1 && m.earned, "day 3: a stone, the count starts again");
  for (int i = 0; i < 6; i++) m = megaGoldDay(m.days, m.stones);
  ck(m.stones == 3, "stones stop at 3");
  m = megaGoldDay(m.days, m.stones); m = megaGoldDay(m.days, m.stones); ck(m.stones == 3 && m.days == MEGA_STONE_DAYS - 1 && !m.earned, "full: the count waits at its last day");

  // a companion
  {
    Pet q; q.begin(); q.dbgHatchAs(6, false); q.setClock(D0);
    ck(!q.megaActive() && !q.canMega(), "no stone: it cannot");
    // three days of first golds
    for (int d = 0; d < 3; d++) {
      q.setClock(D0 + d * DAY);
      Pet::TourPay a = q.tourFinish(EV_SPRINT, 1, 1100 + d);
      Pet::TourPay b = q.tourFinish(EV_SWIM, 1, 2000 + d);
      ck(a.bonus && !b.bonus, "only the day's first gold counts");
      ck(a.stone == (d == 2), d == 2 ? "the third day pays a stone" : "no stone before the third day");
      q.energy = 90;
    }
    ck(q.megaStones == 1, "one stone kept");
    q.setClock(D0 + 5 * DAY);
    ck(q.canMega(), "with a stone and a Mega species it can");
    ck(q.startMega(), "starting spends the stone");
    ck(q.megaStones == 0 && q.megaActive() && q.evolving(), "now Mega, with the blink");
    ck(q.megaLeft() == MEGA_SECS, "24 hours left");
    ck(q.megaWasSeen(6) && !q.megaWasSeen(7), "the Pokedex remembers Charizard was Mega");
    ck(!q.startMega(), "no second start while it lasts");
    q.setClock(D0 + 5 * DAY + MEGA_SECS - 1); ck(q.megaActive(), "still Mega one second before the end");
    q.saveNow();
    Pet r; r.begin();
    ck(r.megaActive() && r.megaDex == 6 && r.megaUntil == q.megaUntil && r.megaWasSeen(6), "it survives a save");
    r.setClock(D0 + 5 * DAY + MEGA_SECS);
    ck(!r.megaActive() && r.megaUntil == 0 && r.megaEnded, "after 24 hours it wears off, with the message");
    // other cases
    Pet e; e.begin(); e.dbgHatchAs(1, false); e.megaStones = 2;
    ck(!e.canMega() && !e.startMega() && e.megaStones == 2, "Bulbasaur has no Mega form: nothing spent");
    Pet s; s.begin(); s.dbgHatchAs(6, false); s.setClock(D0); s.megaStones = 1; s.sleeping = true;
    ck(!s.canMega(), "not while asleep");
    s.sleeping = false; s.startMega();
    s.newEgg();
    ck(!s.megaActive() && s.megaUntil == 0, "a new egg ends it");
    Pet t; t.begin(); t.dbgHatchAs(6, false); t.setClock(D0); t.megaStones = 1; t.startMega();
    t.speciesId = 9; ck(!t.megaActive(), "a different companion is never Mega");
    Pet u; u.begin(); u.dbgHatchAs(6, false); u.setClock(D0); u.megaStones = 9; u.megaGoldDays = 7; u.saveNow();
    Pet v; v.begin(); ck(v.megaStones == MEGA_STONE_MAX && v.megaGoldDays == 0, "a damaged save is clamped");
  }

  // Mega is +15 % in the Tournament
  {
    Athlete a = athleteOf(6, 30), b = a; b.mega = true;
    float ra = athleteRating(a, EV_SPRINT), rb = athleteRating(b, EV_SPRINT);
    ck(rb > ra * 1.1499f && rb < ra * 1.1501f, "Mega raises the rating by exactly 15 %");
  }

  // the long jump: run to the board, jump, land in the pit
  {
    const uint32_t T = 2000;
    JumpPose p0 = longJumpPose(0, T, 5.0f);
    ck(p0.x == LJ_START_X && p0.lift == 0 && p0.phase == 0, "waiting: at the start of the run-up");
    int16_t last = -1; bool mono = true, behind = true, air = false;
    for (uint32_t t = 1; t < T; t += 20) {
      JumpPose j = longJumpPose(t, T, 6.0f);
      if (j.x < last) mono = false;
      last = j.x;
      if (j.phase == 1 && j.x > LJ_BOARD_X) behind = false;      // before the board it stays behind it
      if (j.phase == 2) { air = true; if (j.lift <= 0 && t > 1000 && t < 1400) behind = false; }
    }
    ck(mono, "it only ever moves forward");
    ck(behind && air, "it runs behind the board, then is in the air");
    JumpPose fin = longJumpPose(T, T, 6.0f);
    ck(fin.x >= LJ_PIT_X0 && fin.x <= LJ_PIT_X1 && fin.lift == 0, "it ends in the sand pit, on the ground");
    ck(longJumpLandX(8.0f) > longJumpLandX(4.0f), "a longer jump lands farther in");
    ck(longJumpLandX(0.5f) == LJ_PIT_X0 && longJumpLandX(99.0f) == LJ_PIT_X1, "marks outside the range stay inside the pit");
    ck(longJumpPose(T + 5000, T, 6.0f).x == fin.x, "after its turn it stays where it landed");
    ck(fin.x < 466, "and on the screen");
  }

  if (bad) { printf("FAILURES: %d\n", bad); return 1; }
  printf("mega test passed\n");
  return 0;
}
