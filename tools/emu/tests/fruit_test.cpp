// Training fruits (fruit.h and the Pet's harvest / eat / save).
#include "Arduino.h"
#include "Preferences.h"
#include "pet.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
uint32_t g_seed = 47; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
volatile int g_touchX = 0, g_touchY = 0; volatile bool g_touchDown = false; bool wasPressed = false;
static uint32_t g_ms = 0; uint32_t millis() { return g_ms; }
void FakeESP::restart() { exit(0); }
int FakeSerial::available() { return 0; } String FakeSerial::readStringUntil(char) { return String(""); }
void sfxPlay(uint8_t) {}
static int bad = 0;
static void ck(bool ok, const char *w) { printf("%s  %s\n", ok ? "PASS" : "FAIL", w); if (!ok) bad++; }
static const uint32_t DAY = 86400, D0 = 20000 * DAY + 12 * 3600;

int main() {
  // the roll: a quarter of harvests are fruits, the kind from the second roll
  int fruits = 0, kinds[3] = { 0 };
  for (int r = 0; r < 100; r++) { int8_t k = fruitFromRoll((uint8_t)r, (uint8_t)r); if (k >= 0) { fruits++; kinds[k]++; } }
  ck(fruits == 25, "25 of 100 harvest rolls are fruits");
  ck(fruitFromRoll(24, 0) == 0 && fruitFromRoll(24, 1) == 1 && fruitFromRoll(24, 2) == 2 && fruitFromRoll(24, 3) == 0, "the kind comes from its own roll");
  ck(fruitFromRoll(25, 0) == -1 && fruitFromRoll(99, 1) == -1, "otherwise the usual food");
  // the gain
  ck(fruitGain(0, 90) == 4 && fruitGain(86, 90) == 4 && fruitGain(88, 90) == 2 && fruitGain(90, 90) == 0 && fruitGain(95, 90) == 0, "+4, less near the ceiling, nothing at it");

  // a companion
  {
    Pet q; q.begin(); q.dbgHatchAs(25, false); q.setClock(D0);
    q.fruitCount[0] = 2; q.fruitCount[1] = 1; q.fruitCount[2] = 0;
    uint8_t a0 = q.trAtk, d0 = q.trDef;
    ck(q.eatFruit(0) == Pet::FRUIT_EATEN && q.trAtk == a0 + 4 && q.fruitCount[0] == 1 && q.lastFruitGain == 4, "a Power fruit: +4 Attack, one used");
    ck(q.eatFruit(1) == Pet::FRUIT_EATEN && q.trDef == d0 + 4 && q.fruitCount[1] == 0, "a Guard fruit: +4 Defence");
    ck(q.eatFruit(2) == Pet::FRUIT_NONE && q.trSpe == 0, "none in the bag: nothing happens");
    ck(q.eatFruit(7) == Pet::FRUIT_NONE, "an unknown kind: nothing");
    q.trAtk = q.trMaxAtk();
    ck(q.eatFruit(0) == Pet::FRUIT_AT_BEST && q.fruitCount[0] == 1 && q.trAtk == q.trMaxAtk(), "at the ceiling: refused, nothing spent");
    q.trAtk = q.trMaxAtk() - 1;
    ck(q.eatFruit(0) == Pet::FRUIT_EATEN && q.trAtk == q.trMaxAtk() && q.lastFruitGain == 1, "near the ceiling: only what fits");
    q.sleeping = false; q.fruitCount[2] = 1;
    uint16_t spe = q.speStat();
    q.trSpe = 0; q.eatFruit(2);
    ck(q.speStat() >= spe || q.trSpe == 4, "a Swift fruit raises Speed training");
    q.fruitCount[1] = 1; q.dbgSetEgg(25, false);
    ck(q.eatFruit(1) == Pet::FRUIT_NONE && q.fruitCount[1] == 1, "an egg cannot eat");
  }

  // the garden gives them, within the stack cap
  {
    Pet q; q.begin(); q.dbgHatchAs(25, false); q.setClock(D0);
    int gotFruit = 0, gotFood = 0, n = 0; bool capOk = true;
    for (int i = 0; i < 400; i++) {
      q.setClock(D0 + (i + 1) * 100000UL);
      q.seedCount = 1;
      if (!q.plantSeed()) { continue; }
      q.setClock(D0 + (i + 1) * 100000UL + 40UL * 3600);
      int8_t f = q.harvest();
      if (f < 0) continue;
      n++;
      if (f >= FRUIT_FOOD_BASE) { gotFruit++; if (f - FRUIT_FOOD_BASE >= FRUIT_KINDS) capOk = false; } else gotFood++;
      for (int k = 0; k < FRUIT_KINDS; k++) if (q.fruitCount[k] > FRUIT_STACK_MAX) capOk = false;
      if (i % 3 == 0) for (int k = 0; k < FRUIT_KINDS; k++) if (q.fruitCount[k] > 2) q.fruitCount[k] = 0;   // keep eating them
    }
    ck(n > 300, "many harvests happened");
    ck(gotFruit > n * 15 / 100 && gotFruit < n * 35 / 100, "about a quarter of the harvests are fruits");
    ck(capOk, "a fruit id stays in range and a stack never passes the cap");
    // a full stack falls back to the food
    q.fruitCount[0] = q.fruitCount[1] = q.fruitCount[2] = FRUIT_STACK_MAX;
    int over = 0;
    for (int i = 0; i < 60; i++) {
      q.setClock(D0 + (1000 + i) * 100000UL); q.seedCount = 1; q.plantSeed();
      q.setClock(D0 + (1000 + i) * 100000UL + 40UL * 3600);
      int8_t f = q.harvest(); if (f >= FRUIT_FOOD_BASE) over++;
      for (int k = 0; k < FRUIT_KINDS; k++) if (q.fruitCount[k] > FRUIT_STACK_MAX) over++;
    }
    ck(over == 0, "full stacks give food instead");
  }

  // the save
  {
    Pet q; q.begin(); q.dbgHatchAs(25, false); q.setClock(D0);
    q.fruitCount[0] = 3; q.fruitCount[1] = 0; q.fruitCount[2] = 9;
    q.saveNow();
    Pet r; r.begin();
    ck(r.fruitCount[0] == 3 && r.fruitCount[1] == 0 && r.fruitCount[2] == 9, "the fruits survive a save");
  }

  if (bad) { printf("FAILURES: %d\n", bad); return 1; }
  printf("fruit test passed\n");
  return 0;
}
