// Fun Pack: the extra minigames' rewards and the statuses (stomachache, cold,
// medicine, dozing off with the light on, grumpy, bored).
//
// Asserts against Pet itself. The doze trigger lives in the sketch (it owns
// lastInteract), so here dozeOff() is called the way the sketch calls it.
#include "Arduino.h"
#include "Preferences.h"
#include "pet.h"
#include "party.h"
#include <cstdio>
uint32_t g_seed=29; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
volatile int g_touchX=0,g_touchY=0; volatile bool g_touchDown=false; bool wasPressed=false;
static uint32_t g_ms=0;
uint32_t millis(){return g_ms;} void FakeESP::restart(){exit(0);}
int FakeSerial::available(){return 0;} String FakeSerial::readStringUntil(char){return String("");}
void sfxPlay(uint8_t){}
static int bad=0;
static void ck(bool ok,const char*w){printf("%s  %s\n",ok?"PASS":"FAIL",w); if(!ok)bad++;}

static uint32_t atHour(int h){ return (uint32_t)h * 3600 + 90; }

static void fresh(Pet &p, int hour){
  p.begin();
  p.dbgHatchAs(1,false);
  p.sleeping = false;
  p.sleepAuto = SLEEP_NONE;
  p.fullness = p.joy = p.energy = p.hygiene = 80;
  p.poops = 0;
  p.sickness = SICK_NONE; p.sickDoses = 0; p.sickTicks = 0; p.overfeed = 0; p.dirtyTicks = 0;
  p.lightOn = false; p.lightOnTicks = 0; p.grumpy = false; p.boredTicks = 0;
  p.trAtk = p.trDef = p.trSpe = 0;
  for (int i = 0; i < MINI_COUNT; i++) p.miniHi[i] = 0;
  p.lastSeenEpoch = atHour(hour);
}

int main(){
  // --- rewards: a stat game trains its stat, a joy game returns the joy, records stick
  {
    Pet p; fresh(p, 12);
    ck(Pet::miniGameStat(MINI_MOLE) == MINI_STAT_ATK, "mole pop trains attack");
    ck(Pet::miniGameStat(MINI_DANCE) == MINI_NO_STAT, "dance is about joy");
    uint8_t g = p.applyMiniResult(MINI_MOLE, 30);
    ck(g == 10 && p.trAtk == 10, "mole pop: 30 points = +10 ATK");
    ck(p.miniHi[MINI_MOLE] == 30, "and the record is kept");
    p.joy = 50;
    uint8_t j = p.applyMiniResult(MINI_DANCE, 40);
    ck(j == 36 && p.joy == 86, "dance pays its capped joy (6 + 30)");
    p.applyMiniResult(MINI_MOLE, 3);
    ck(p.miniHi[MINI_MOLE] == 30, "a worse score does not lower the record");
    p.trSpe = p.trMaxSpe();
    ck(p.applyMiniResult(MINI_ROPE, 40) == 0, "the IV ceiling still holds");
    ck(p.applyMiniResult(200, 10) == 0, "an unknown game pays nothing");
  }

  // --- grumpy halves the joy, and playing cures grumpy and bored
  {
    Pet p; fresh(p, 12);
    p.joy = 20; p.grumpy = true; p.boredTicks = BORED_TICKS;
    ck(p.isBored(), "8 h without a game is boredom");
    uint8_t j = p.applyMiniResult(MINI_LR, 5);
    ck(j == 7, "grumpy: half of 4 + 10");
    ck(!p.grumpy && !p.isBored(), "one game cures both");
    p.grumpy = true; p.boredTicks = BORED_TICKS;
    p.trainSpeed(10);
    ck(!p.grumpy && !p.isBored(), "and so do the original games");
  }

  // --- overeating: three binges on a full stomach, candy counts double
  {
    Pet p; fresh(p, 12);
    p.fullness = 95;
    p.feedBerry(1); p.feedBerry(1);
    ck(!p.isSick(), "two binges are fine");
    p.feedBerry(1);
    ck(p.sickness == SICK_STOMACH, "the third gives a stomachache");
    g_ms = 100000;   // past the eating animation
    ck(p.mood() == MOOD_SAD, "and it looks sad");
    ck(!p.canPlay(), "it cannot play while sick");
    ck(p.giveMedicine() == PILL_CURED && !p.isSick(), "one dose cures it");
    fresh(p, 12);
    p.fullness = 95;
    p.feedCandy(); p.feedCandy();
    ck(p.sickness == SICK_STOMACH, "two candies on a full stomach do it too");
    fresh(p, 12);
    p.fullness = 50;
    for (int i = 0; i < 2; i++) p.feedBerry(1);
    ck(!p.isSick() && p.overfeed == 0, "eating while hungry never counts");
  }

  // --- dirt: 45 minutes with 3 poops gives a cold that needs 2 doses and a bath
  {
    Pet p; fresh(p, 12);
    p.poops = 3;
    for (int i = 0; i < DIRTY_SICK_TICKS - 1; i++) { p.poops = 3; p.fullness = 80; p.dbgTick(); }
    ck(!p.isSick(), "44 minutes dirty is not yet a cold");
    p.poops = 3; p.dbgTick();
    ck(p.sickness == SICK_COLD, "45 minutes is");
    ck(p.giveMedicine() == PILL_MORE, "first dose: one more later");
    ck(p.giveMedicine() == PILL_WAIT, "a second dose straight away is too soon");
    p.ageMinutes += COLD_DOSE_GAP_MIN;
    ck(p.giveMedicine() == PILL_NEED_BATH, "the second dose, still dirty: needs a bath");
    ck(p.isSick(), "still sick until the bath");
    p.clean();
    ck(!p.isSick(), "the bath finishes the cure");
  }

  // --- sickness drains and, left alone for 6 h, counts a care mistake
  {
    Pet p; fresh(p, 12);
    p.sickness = SICK_STOMACH;
    uint8_t m = p.careMistakes;
    for (int i = 0; i < SICK_MISTAKE_TICKS; i++) {
      p.fullness = p.joy = p.energy = p.hygiene = 90; p.poops = 0;
      p.lastSeenEpoch = atHour(12);
      p.dbgTick();
    }
    ck(p.careMistakes == m + 1, "6 h sick = one care mistake");
  }

  // --- healthy medicine just tastes bad
  {
    Pet p; fresh(p, 12);
    p.joy = 50;
    ck(p.giveMedicine() == PILL_NOT_SICK && p.joy == 40, "healthy: -10 joy");
  }

  // --- dozing off with the light on
  {
    Pet p; fresh(p, 14);
    p.dozeOff();
    ck(!p.sleeping, "no dozing off in the afternoon");
    fresh(p, 22);
    p.setScreenOff(true);
    p.dozeOff();
    ck(!p.sleeping, "screen off counts as light off: no doze");
    p.setScreenOff(false);
    p.dozeOff();
    ck(p.sleeping && p.lightOn, "22:00, screen on, left alone: asleep with the light on");
    p.setScreenOff(false);
    ck(p.sleeping, "the screen coming back on does not wake it");
    for (int i = 0; i < LIGHT_GRUMPY_TICKS; i++) p.dbgTick();
    ck(p.lightOnTicks >= LIGHT_GRUMPY_TICKS, "it counts the time with the light on");
    p.toggleLight();
    ck(p.sleeping && !p.lightOn, "the light button turns the light off and it sleeps on");
    p.joy = 80; p.energy = 100;
    p.toggleLight();
    ck(!p.sleeping && p.grumpy, "woken after 30 min of light: grumpy");
    ck(p.joy == 65 && p.energy == 70, "-15 joy and energy capped at 70");

    fresh(p, 22);
    p.dozeOff();
    p.setScreenOff(true);
    ck(!p.lightOn, "turning the screen off switches the light off");
    for (int i = 0; i < LIGHT_GRUMPY_TICKS; i++) p.dbgTick();
    p.setScreenOff(false);
    p.toggleLight();
    ck(!p.sleeping && !p.grumpy, "a quick dark sleep is not grumpy");
  }

  // --- boredom builds while awake and drains joy
  {
    Pet p; fresh(p, 12);
    p.boredTicks = BORED_TICKS - 1;
    p.dbgTick();
    ck(p.isBored(), "8 h awake without games is bored");
  }

  // --- everything persists, and a fresh egg starts healthy
  {
    Pet p; fresh(p, 12);
    p.sickness = SICK_COLD; p.sickDoses = 1; p.grumpy = true; p.miniHi[MINI_HIDE] = 17;
    p.saveNow();
    Pet q; q.begin();
    ck(q.sickness == SICK_COLD && q.sickDoses == 1 && q.grumpy && q.miniHi[MINI_HIDE] == 17,
       "statuses and records survive a reboot");
    q.newEgg();
    ck(!q.isSick() && !q.grumpy && q.miniHi[MINI_HIDE] == 17,
       "a new egg starts healthy; records are the player's");
  }

  if (bad) printf("FAILURES\n");
  return bad?1:0;
}
