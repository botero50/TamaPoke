// Coins and daily missions: what pays, the daily cap on activity coins, the
// missions' stable daily pick and their one-time rewards, and persistence.
#include "Arduino.h"
#include "Preferences.h"
#include "pet.h"
#include "party.h"
#include <cstdio>
uint32_t g_seed=31; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
volatile int g_touchX=0,g_touchY=0; volatile bool g_touchDown=false; bool wasPressed=false;
static uint32_t g_ms=0;
uint32_t millis(){return g_ms;} void FakeESP::restart(){exit(0);}
int FakeSerial::available(){return 0;} String FakeSerial::readStringUntil(char){return String("");}
void sfxPlay(uint8_t){}
static int bad=0;
static void ck(bool ok,const char*w){printf("%s  %s\n",ok?"PASS":"FAIL",w); if(!ok)bad++;}

static const uint32_t DAY = 86400;
static uint32_t atDay(uint32_t d, int h){ return d * DAY + (uint32_t)h * 3600 + 90; }

static void fresh(Pet &p, uint32_t day){
  p.begin();
  p.dbgHatchAs(1,false);
  p.sleeping = false;
  p.sleepAuto = SLEEP_NONE;
  p.fullness = p.joy = p.energy = p.hygiene = 60;
  p.sickness = SICK_NONE;
  p.coins = 0; p.coinsToday = 0; p.coinDay = 0; p.stepCoinK = 0;
  memset(p.missionProg, 0, sizeof(p.missionProg)); p.missionDone = 0; p.missionKindDay = 0;
  p.bedtimeDay = 0; p.stepCount = 0; p.stepDay = 0;
  p.lastCareDay = day;          // today's care already counted: no daily bonus noise
  p.lastSeenEpoch = atDay(day, 12);
}

// the slot holding mission m today, or -1
static int slotOf(Pet &p, uint8_t m){
  for (int i = 0; i < MISSIONS_PER_DAY; i++) if (p.missionAt(i) == m) return i;
  return -1;
}
// a day (from 20000 on) whose missions include m
static uint32_t dayWith(Pet &p, uint8_t m){
  for (uint32_t d = 20000; d < 20400; d++) {
    p.lastSeenEpoch = atDay(d, 12);
    if (slotOf(p, m) >= 0) return d;
  }
  return 0;
}

int main(){
  // --- the day's six missions are distinct and stable (missions_test covers the pick)
  {
    Pet p; fresh(p, 20000);
    uint8_t a = p.missionAt(0), b = p.missionAt(1), c = p.missionAt(5);
    bool distinct = true;
    for (int i = 0; i < MISSIONS_PER_DAY; i++) for (int j = i + 1; j < MISSIONS_PER_DAY; j++) distinct = distinct && p.missionAt(i) != p.missionAt(j);
    ck(distinct, "six different missions");
    ck(a < MIS_COUNT && b < MIS_COUNT && c < MIS_COUNT, "all from the list");
    p.lastSeenEpoch = atDay(20000, 20);
    ck(p.missionAt(0) == a && p.missionAt(5) == c, "the same all day");
    bool changes = false;
    for (uint32_t d = 20001; d < 20010 && !changes; d++) {
      p.lastSeenEpoch = atDay(d, 12);
      changes = p.missionAt(0) != a || p.missionAt(1) != b;
    }
    ck(changes, "and a different set on other days");
  }

  // --- the first care of a new day pays the daily bonus; the 7th day more
  {
    Pet p; fresh(p, 20000);
    p.lastCareDay = 19999; p.streak = 5;
    p.caress();
    ck(p.coins >= COIN_DAILY_BONUS, "daily bonus on the first care");
    uint32_t c = p.coins;
    p.caress();
    ck(p.coins - c <= 1 + COIN_MISSION + COIN_ALL_MISSIONS, "only once a day");
    fresh(p, 20000);
    p.lastCareDay = 19999; p.streak = 6;
    p.caress();
    ck(p.streak == 7 && p.coins >= COIN_DAILY_BONUS + COIN_STREAK7_BONUS, "day 7 of a streak pays the streak bonus");
  }

  // --- games pay by score, and activity coins stop at the daily cap
  {
    Pet p; fresh(p, 20000);
    uint32_t before = p.coins;
    p.missionDone = 0xBF;   // missions out of the way: measure the game alone
    p.coinDay = 20000;
    p.applyMiniResult(MINI_LR, 10);
    ck(p.coins - before == 2, "a game pays 1 + score/10 (score 10 = 2)");
    before = p.coins;
    p.applyMiniResult(MINI_DANCE, 80);
    ck(p.coins - before == COIN_GAME_MAX, "and at most COIN_GAME_MAX");
    for (int i = 0; i < 40; i++) p.applyMiniResult(MINI_DANCE, 80);
    ck(p.coinsToday == COIN_DAILY_CAP, "activity coins stop at the daily cap");
    before = p.coins;
    p.applyMiniResult(MINI_DANCE, 80);
    ck(p.coins == before, "nothing more from games today");
    p.lastSeenEpoch = atDay(20001, 9);
    p.lastCareDay = 20001;
    p.missionDone = 0;
    p.applyMiniResult(MINI_LR, 10);
    ck(p.coinsToday > 0 && p.coinsToday < COIN_DAILY_CAP, "a new day resets the cap");
  }

  // --- steps pay per thousand, never twice, and stop at the step cap
  {
    Pet p; fresh(p, 20000);
    p.missionDone = 0xBF; p.coinDay = 20000;
    p.addSteps(999);
    ck(p.coins == 0, "999 steps: nothing yet");
    p.addSteps(1);
    ck(p.coins == COIN_PER_1000_STEPS, "1 000 steps pay");
    p.addSteps(2500);
    ck(p.coins == 3 * COIN_PER_1000_STEPS, "3 500 steps: three thousands paid");
    p.addSteps(40000);
    ck(p.coins == COIN_STEP_CAP_K * COIN_PER_1000_STEPS, "past 20 000 steps it stops");
    ck(p.coinsToday == 0, "steps are not counted against the activity cap");
  }

  // --- a mission completes once, pays once, and all three pay a bonus
  {
    Pet p;
    fresh(p, 20000);
    uint32_t d = dayWith(p, MIS_PET);
    ck(d != 0, "found a day with the petting mission");
    fresh(p, d);
    int s = slotOf(p, MIS_PET);
    for (int k = 0; k < 4; k++) p.caress();
    ck(!p.missionComplete(s) && p.missionProgress(s) == 4, "4 of 5 pets");
    uint32_t c = p.coins;
    p.caress();
    ck(p.missionComplete(s), "the fifth completes it");
    ck(p.coins - c >= COIN_MISSION, "and pays the mission reward");
    c = p.coins;
    p.caress();
    ck(p.coins - c <= 1, "it does not pay again");

    fresh(p, 20000);
    d = dayWith(p, MIS_STEPS);
    fresh(p, d);
    s = slotOf(p, MIS_STEPS);
    p.addSteps(1990);
    ck(!p.missionComplete(s), "1 990 steps: not yet");
    p.addSteps(10);
    ck(p.missionComplete(s), "2 000 steps complete the walking mission");

    // all six done -> the bonus, once
    fresh(p, 20000);
    p.coinDay = 20000;
    p.missionDone = 0x1F;
    p.missionProg[5] = Pet::missionGoal(p.missionAt(5)) - 1;
    c = p.coins;
    uint8_t m = p.missionAt(5);
    if (m == MIS_STEPS || m == MIS_STEPS_BIG) p.addSteps(5000);
    else p.noteMission(m, Pet::missionGoal(m));
    ck((p.missionDone & 0xBF) == 0xBF, "all six done");
    ck(p.coins - c >= COIN_MISSION + COIN_ALL_MISSIONS, "and the bonus is paid");
  }

  // --- badges pay the first time only
  {
    Pet p; fresh(p, 20000);
    p.winBadge(0, 3, false);
    ck(p.coins == COIN_BADGE, "a new badge pays");
    p.winBadge(0, 3, false);
    ck(p.coins == COIN_BADGE, "a rematch does not");
  }

  // --- bedtime with the light off in the evening, once a day
  {
    Pet p; fresh(p, 20000);
    p.lastSeenEpoch = atDay(20000, 21);
    p.toggleLight();
    ck(p.sleeping && p.coins == COIN_BEDTIME, "bed at 21:00 with the light off pays");
    p.toggleLight(); p.toggleLight();
    ck(p.coins == COIN_BEDTIME, "only once a night");
    fresh(p, 20000);
    p.toggleLight();
    ck(p.coins == 0, "a nap at noon pays nothing");
  }

  // --- spending, and everything persists across a reboot and a new egg
  {
    Pet p; fresh(p, 20000);
    p.coins = 50;
    ck(!p.spendCoins(60) && p.coins == 50, "cannot spend what you do not have");
    ck(p.spendCoins(20) && p.coins == 30, "spending takes the coins");
    p.coinDay = 20000; p.missionKindDay = 20000; p.missionDone = 0x01; p.missionProg[1] = 2;
    p.saveNow();
    Pet q; q.begin();
    ck(q.coins == 30 && q.missionDone == 0x01 && q.missionProg[1] == 2, "coins and missions survive a reboot");
    q.newEgg();
    ck(q.coins == 30, "coins are the player's: a new egg keeps them");
  }

  // --- hats: buying needs the coins, once; only owned hats can be worn
  {
    Pet p; fresh(p, 20000);
    p.hatsOwned = 0; p.hatOn = -1; p.hatDx = p.hatDy = 0; p.hatDex = 0;
    p.coinDay = 20000; p.missionDone = 0xBF;   // missions out of the way: buying pays one back
    p.coins = Pet::hatPrice(HAT_CROWN) - 1;
    ck(!p.buyHat(HAT_CROWN) && !p.ownsHat(HAT_CROWN) && p.coins == Pet::hatPrice(HAT_CROWN) - 1,
       "one coin short: no crown, no coins taken");
    p.coins = 1000;
    ck(p.buyHat(HAT_CAP) && p.ownsHat(HAT_CAP) && p.coins == 1000 - Pet::hatPrice(HAT_CAP), "buying takes the price");
    uint32_t c = p.coins;
    ck(!p.buyHat(HAT_CAP) && p.coins == c, "a hat you own cannot be bought twice");
    ck(!p.buyHat(HAT_COUNT), "an unknown hat cannot be bought");
    p.wearHat(HAT_WITCH);
    ck(p.hatOn == -1, "a hat you do not own cannot be worn");
    p.wearHat(HAT_CAP);
    ck(p.hatOn == HAT_CAP, "an owned hat can");
    p.wearHat(-1);
    ck(p.hatOn == -1, "and it comes off");

    // nudges belong to the species
    p.wearHat(HAT_CAP);
    p.nudgeHat(6, -9);
    ck(p.hatOffX() == 6 && p.hatOffY() == -9, "the nudge moves the hat");
    p.nudgeHat(500, 0);
    ck(p.hatOffX() == HAT_NUDGE_MAX, "but only so far");
    p.saveNow();
    Pet q; q.begin();
    ck(q.ownsHat(HAT_CAP) && q.hatOn == HAT_CAP && q.hatOffY() == -9, "hats and the nudge survive a reboot");
    q.dbgHatchAs(2, false);   // evolved / another creature
    ck(q.hatOffX() == 0 && q.hatOffY() == 0, "another species starts with the hat centred");
    q.newEgg();
    ck(q.ownsHat(HAT_CAP) && q.hatOn == HAT_CAP, "a new egg keeps the hats");
  }

  // --- toys and decorations: buying, the two floor spots, and what they do
  {
    Pet p; fresh(p, 20000);
    p.decorOwned = 0; p.slotL = p.slotR = -1;
    p.coinDay = 20000; p.missionDone = 0xBF;   // missions out of the way: buying pays one back
    p.coins = Pet::decorPrice(DEC_BED) - 1;
    ck(!p.buyDecor(DEC_BED) && !p.ownsDecor(DEC_BED), "one coin short: no bed");
    p.coins = 1000;
    ck(p.buyDecor(DEC_BALL) && p.buyDecor(DEC_PLANT) && p.buyDecor(DEC_BED), "bought a ball, a plant and a bed");
    ck(p.coins == 1000 - Pet::decorPrice(DEC_BALL) - Pet::decorPrice(DEC_PLANT) - Pet::decorPrice(DEC_BED), "each one took its price");
    ck(!p.buyDecor(DEC_BALL), "nothing can be bought twice");
    p.placeDecor(DEC_DRUM);
    ck(p.slotL == -1, "an item you do not own cannot be put out");
    p.placeDecor(DEC_BALL);
    p.placeDecor(DEC_PLANT);
    ck(p.slotL == DEC_BALL && p.slotR == DEC_PLANT, "left first, then right");
    p.placeDecor(DEC_BED);
    ck(p.slotL == DEC_BED && p.slotR == DEC_PLANT, "both full: the new one replaces the left");
    p.placeDecor(DEC_PLANT);
    ck(p.slotR == -1, "tapping one that is out picks it up");

    // a toy out halves how fast boredom builds
    p.slotL = -1; p.slotR = -1;
    p.boredTicks = 0;
    for (int i = 0; i < 100; i++) { p.fullness = p.joy = p.energy = p.hygiene = 90; p.poops = 0; p.dbgTick(); }
    uint16_t alone = p.boredTicks;
    p.placeDecor(DEC_BALL);
    p.boredTicks = 0;
    for (int i = 0; i < 100; i++) { p.fullness = p.joy = p.energy = p.hygiene = 90; p.poops = 0; p.dbgTick(); }
    ck(alone == 100 && p.boredTicks == 50, "with a toy out, boredom builds half as fast");

    // the bed: a little more energy per minute of sleep (light off)
    p.slotL = -1; p.slotR = -1;
    p.sleeping = true; p.lightOn = false; p.energy = 10; p.dbgTick();
    uint8_t plain = p.energy;
    p.placeDecor(DEC_BED);
    p.energy = 10; p.dbgTick();
    ck(plain == 16 && p.energy == 17, "it sleeps better in its bed");

    p.saveNow();
    Pet q; q.begin();
    ck(q.ownsDecor(DEC_BALL) && q.ownsDecor(DEC_BED) && q.slotL == DEC_BED, "toys, decorations and spots survive a reboot");
    q.newEgg();
    ck(q.ownsDecor(DEC_PLANT) && q.slotL == DEC_BED, "a new egg keeps them");
  }

  // --- foods: bought by the serving, eaten from the bag
  {
    Pet p; fresh(p, 20000);
    for (int i = 0; i < FOOD_COUNT; i++) p.foodCount[i] = 0;
    p.coinDay = 20000; p.missionDone = 0xBF;   // missions out of the way: buying pays one back
    p.coins = Pet::foodInfo(FOOD_CURRY).price - 1;
    ck(!p.buyFood(FOOD_CURRY) && p.foodCount[FOOD_CURRY] == 0, "one coin short: no curry");
    p.coins = 500;
    ck(p.buyFood(FOOD_CURRY) && p.buyFood(FOOD_CURRY) && p.foodCount[FOOD_CURRY] == 2, "servings stack");
    ck(p.coins == 500 - 2 * Pet::foodInfo(FOOD_CURRY).price, "each serving costs its price");
    p.foodCount[FOOD_COOKIE] = FOOD_STACK_MAX;
    ck(!p.buyFood(FOOD_COOKIE), "a full stack takes no more");
    ck(!p.buyFood(FOOD_COUNT), "an unknown food cannot be bought");

    ck(!p.eatFood(FOOD_MILK), "nothing to eat when you have none");
    p.fullness = 40; p.joy = 50; p.weight = 0;
    g_ms = 1000000;
    ck(p.eatFood(FOOD_CURRY) && p.foodCount[FOOD_CURRY] == 1, "eating uses a serving");
    ck(p.fullness == 40 + Pet::foodInfo(FOOD_CURRY).food && p.joy == 50 + Pet::foodInfo(FOOD_CURRY).joy,
       "and applies what the curry does");
    ck(p.eating(), "it plays the eating animation");
    p.foodCount[FOOD_MILK] = 1; p.energy = 50;
    p.eatFood(FOOD_MILK);
    ck(p.energy == 50 + Pet::foodInfo(FOOD_MILK).energy, "milk gives energy");
    p.sleeping = true; p.foodCount[FOOD_SOUP] = 1;
    ck(!p.eatFood(FOOD_SOUP) && p.foodCount[FOOD_SOUP] == 1, "it cannot eat asleep, and keeps the soup");
    p.sleeping = false;

    // sweets on a full stomach count double, like candy
    p.sickness = SICK_NONE; p.overfeed = 0; p.fullness = 95;
    p.foodCount[FOOD_CAKE] = 2;
    p.eatFood(FOOD_CAKE); p.fullness = 95; p.eatFood(FOOD_CAKE);
    ck(p.sickness == SICK_STOMACH, "two cakes when full: a stomachache");

    p.foodCount[FOOD_JUICE] = 7;
    p.saveNow();
    Pet q; q.begin();
    ck(q.foodCount[FOOD_JUICE] == 7 && q.foodCount[FOOD_CURRY] == 1, "the bag survives a reboot");
  }

  // --- the bath extras: soap, the brush, the scrub game
  {
    Pet p; fresh(p, 20000);
    p.soapCount = 0; p.brushOwned = false;
    p.coins = SOAP_PRICE + BRUSH_PRICE;
    ck(!p.bubbleBath(), "no soap, no bubble bath");
    ck(p.buySoap() && p.soapCount == 1, "soap is bought by the bar");
    ck(p.buyBrush() && p.brushOwned && p.coins == 0, "and the brush once");
    ck(!p.buyBrush(), "a second brush is not sold");
    p.poops = 2; p.hygiene = 20; p.joy = 50;
    g_ms = 2000000;
    ck(p.bubbleBath() && p.soapCount == 0, "a bubble bath uses the soap");
    ck(p.poops == 2 && p.hygiene == 100 && p.joy >= 60, "and fills CLEAN, happier than a plain wash; the poop stays");
    p.joy = 50;
    ck(p.brush() && p.shining() && p.joy == 55, "brushing: joy and a shine");
    p.poops = 3; p.hygiene = 10; p.joy = 40;
    p.scrubDone(6);
    ck(p.poops == 3 && p.hygiene == 10 + WASH_SCRUB && p.joy == 52, "the scrub game adds WASH_SCRUB and pays joy");
    p.hygiene = 20; p.clean(WASH_SHOWER);
    ck(p.hygiene == 20 + WASH_SHOWER, "a shower adds WASH_SHOWER");
    p.hygiene = 90; p.clean(WASH_SHOWER);
    ck(p.hygiene == 100, "never past 100");
    uint32_t c = p.coins; p.coinDay = 20000; p.coinsToday = 0; p.missionDone = 0xBF;
    p.hygiene = 40; p.dirtyTicks = 30; uint8_t h0 = p.hygiene;
    ck(p.sweep() && p.poops == 0 && p.dirtyTicks == 0 && p.coins == c + 1, "the broom: no poop, no dirt clock, a coin");
    ck(p.hygiene == (h0 + 3 * SWEEP_PER_PILE > 100 ? 100 : h0 + 3 * SWEEP_PER_PILE), "and SWEEP_PER_PILE CLEAN for each of the 3 piles");
    c = p.coins;
    ck(!p.sweep() && p.coins == c, "nothing to sweep: nothing happens");
    p.saveNow();
    Pet q; q.begin();
    ck(q.poops == 0, "the clean floor survives a reboot");
  }

  // --- the nap ends by itself, rested and not grumpy; the lullaby puts it to bed
  {
    Pet p; fresh(p, 20000);
    p.grumpy = false;
    ck(p.startNap() && p.sleeping && !p.lightOn, "a nap: asleep with the light off");
    for (int i = 0; i < NAP_MINUTES - 1; i++) p.dbgTick();
    ck(p.sleeping, "still asleep before the half hour");
    p.dbgTick();
    ck(!p.sleeping && !p.grumpy && p.napEnd == 0, "up after 30 minutes, in a good mood");
    p.startNap();
    p.toggleLight();
    ck(!p.sleeping && p.napEnd == 0, "waking it by hand ends the nap");

    fresh(p, 20000);
    p.lastSeenEpoch = atDay(20000, 21);
    p.joy = 50;
    ck(p.lullaby() && p.sleeping && !p.lightOn, "a lullaby puts it to bed with the light off");
    ck(p.joy == 55 && p.coins == COIN_BEDTIME, "happier, and it counts as bedtime");
    ck(!p.lullaby(), "not when it is already asleep");

    p.soapCount = 4; p.brushOwned = true; p.napEnd = 77;
    p.saveNow();
    Pet q; q.begin();
    ck(q.soapCount == 4 && q.brushOwned && q.napEnd == 77, "soap, brush and the nap survive a reboot");
  }

  // --- the park: tires a little, finds coins at most PARK_FINDS_PER_DAY times a day
  {
    Pet p; fresh(p, 20000);
    p.parkDay = 0; p.parkFinds = 0;
    p.energy = 10;
    ck(p.parkWalk() == PARK_TIRED, "too tired for a walk");
    int coinsFound = 0, other = 0;
    for (int i = 0; i < 60; i++) {
      p.energy = 100; p.fullness = 50;
      uint32_t c = p.coins;
      ParkResult r = p.parkWalk();
      if (r == PARK_COINS) { coinsFound++; ck(p.coins - c >= 2 && p.coins - c <= 4, "a find is 2-4 coins"); }
      else other++;
      if (i == 0) ck(p.energy == 100 - PARK_WALK_ENERGY, "a walk costs a little energy");
    }
    ck(coinsFound == PARK_FINDS_PER_DAY, "coins are found at most 3 times a day");
    ck(other > 0, "and the rest of the walks bring berries and friends");
    p.lastSeenEpoch = atDay(20001, 12);
    bool again = false;
    for (int i = 0; i < 40 && !again; i++) { p.energy = 100; again = p.parkWalk() == PARK_COINS; }
    ck(again, "a new day, new finds");
  }

  // --- the Pokemon Center: free once a day, then CENTER_PRICE
  {
    Pet p; fresh(p, 20000);
    p.centerDay = 0; p.coins = 0;
    p.sickness = SICK_COLD; p.grumpy = true; p.energy = 40;
    ck(p.centerHeal() == CENTER_FREE && !p.isSick() && !p.grumpy && p.energy == 70, "free: cured, cheered up, rested");
    p.sickness = SICK_STOMACH;
    ck(p.centerHeal() == CENTER_NO_COINS && p.isSick(), "the second visit costs, and without coins nothing happens");
    p.coins = CENTER_PRICE;
    ck(p.centerHeal() == CENTER_PAID && !p.isSick() && p.coins == 0, "paid: cured");
    p.lastSeenEpoch = atDay(20001, 12);
    ck(p.centerFreeToday(), "free again tomorrow");
  }

  // --- the garden: a seed grows in 8 h and gives a serving of food
  {
    Pet p; fresh(p, 20000);
    p.seedCount = 0; p.plantEpoch = 0;
    for (int i = 0; i < FOOD_COUNT; i++) p.foodCount[i] = 0;
    ck(!p.plantSeed(), "no seed, nothing to plant");
    p.coins = SEED_PRICE;
    ck(p.buySeed() && p.seedCount == 1 && p.coins == 0, "a seed costs SEED_PRICE");
    ck(p.plantSeed() && p.seedCount == 0 && p.plantEpoch, "planted");
    ck(!p.plantSeed(), "one plant at a time");
    p.lastSeenEpoch += GARDEN_GROW_SECS / 2;
    ck(!p.gardenReady() && p.gardenPercent() == 50 && p.harvest() == -1, "half grown: nothing to pick yet");
    p.lastSeenEpoch += GARDEN_GROW_SECS / 2;
    ck(p.gardenReady() && p.gardenPercent() == 100, "grown after 8 h");
    int8_t f = p.harvest();
    int total = 0;
    for (int i = 0; i < FOOD_COUNT; i++) total += p.foodCount[i];
    ck(f >= 0 && f < FOOD_COUNT && total == 1 && p.plantEpoch == 0, "the harvest is one serving, and the plot is free");
  }

  // --- seasonal gifts by the date, once a day
  {
    int y, m, d;
    Pet::civilDate(20757, y, m, d);
    ck(y == 2026 && m == 10 && d == 31, "day 20757 is 2026-10-31");
    Pet p; fresh(p, 20757);
    p.giftDay = 0; p.decorOwned = 0; p.coins = 0;
    ck(p.holidayToday() == GIFT_HALLOWEEN, "Halloween is a holiday");
    ck(p.claimHolidayGift() == GIFT_HALLOWEEN && p.ownsDecor(DEC_PUMPKIN), "the Halloween gift is the pumpkin");
    ck(p.claimHolidayGift() == GIFT_NONE, "only once");
    p.giftDay = 0;
    ck(p.claimHolidayGift() == GIFT_HALLOWEEN && p.coins == 20, "already owned: 20 coins instead");
    fresh(p, 20758);
    ck(p.holidayToday() == GIFT_NONE, "1 November is not");
    // Christmas 2026 is day 20812 (Dec 25)
    Pet::civilDate(20812, y, m, d);
    ck(m == 12 && d == 25, "day 20812 is Christmas");
  }

  if (bad) printf("FAILURES\n");
  return bad?1:0;
}
