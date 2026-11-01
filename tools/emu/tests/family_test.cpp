// Phases 10 and 11: step adventures, fishing, cooking, the alarm, chores, the
// piggy bank and the diary -- plus persistence.
#include "Arduino.h"
#include "Preferences.h"
#include "pet.h"
#include "party.h"
#include <cstdio>
uint32_t g_seed=73; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
volatile int g_touchX=0,g_touchY=0; volatile bool g_touchDown=false; bool wasPressed=false;
static uint32_t g_ms=0;
uint32_t millis(){return g_ms;} void FakeESP::restart(){exit(0);}
int FakeSerial::available(){return 0;} String FakeSerial::readStringUntil(char){return String("");}
void sfxPlay(uint8_t){}
static int bad=0;
static void ck(bool ok,const char*w){printf("%s  %s\n",ok?"PASS":"FAIL",w); if(!ok)bad++;}

static const uint32_t DAY = 86400;
static uint32_t atDay(uint32_t d, int h, int m = 0){ return d * DAY + (uint32_t)h * 3600 + (uint32_t)m * 60 + 30; }

static void fresh(Pet &p, uint32_t day){
  p.begin();
  p.dbgHatchAs(1,false);
  p.sleeping = false;
  p.sleepAuto = SLEEP_NONE;
  p.fullness = p.joy = p.energy = p.hygiene = 60;
  p.sickness = SICK_NONE;
  p.coins = 0; p.coinsToday = 0; p.coinDay = day;
  p.lastCareDay = day;
  p.lastSeenEpoch = atDay(day, 12);
  for (int i = 0; i < FOOD_COUNT; i++) p.foodCount[i] = 0;
  p.seedCount = 0;
  for (int i = 0; i < TALLY_COUNT; i++) p.tally[i] = 0;
  p.coinsEarned = 0; p.trophies = 0;
  p.advRoute = 0; p.advStop = 0; p.advSteps = 0;
  p.fishDay = 0; p.fishToday = 0;
  p.alarmOn = false; p.alarmDay = 0; p.bedRemDay = 0; p.parentBed = 0;
  p.choreDay = 0; p.choresDone = 0; p.piggy = 0; p.piggyDay = 0;
  for (int i = 0; i < DIARY_DAYS; i++) p.diary[i] = DiaryDay();
  p.diaryCoinBase = 0;
  p.stepCount = 0; p.stepDay = day; p.stepCoinK = 0;
}

int main(){
  // --- adventures: steps walk the route, stops are claimed in order
  {
    Pet p; fresh(p, 20000);
    ck(!p.advReady() && p.advClaim() == ADV_NONE, "no steps, nothing to claim");
    p.addSteps(299);
    ck(!p.advReady(), "299 steps: not at the first stop yet");
    p.addSteps(1);
    ck(p.advReady(), "300 steps: the first stop");
    AdvResult r = p.advClaim();
    ck(r >= ADV_COINS && r <= ADV_FRIEND && p.advStop == 1, "the first stop gives something and moves on");
    ck(!p.advReady(), "the next stop needs more walking");
    p.addSteps(4000);
    int claims = 0;
    while (p.advReady() && p.advStop < ROUTE_STOPS - 1) { p.advClaim(); claims++; }
    ck(claims == 3 && p.advStop == ROUTE_STOPS - 1 && p.advReady(), "the middle stops, one at a time");
    uint32_t c = p.coins;
    ck(p.advClaim() == ADV_DONE && p.advRoute == 1 && p.advStop == 0 && p.coins == c + 10, "the last stop finishes the route: 10 coins, the next route");
    ck(p.advSteps == 300 && p.tally[TALLY_ROUTE] == 1, "extra steps carry on; the route is counted");
    ck(p.checkTrophies() == TRO_ROUTE, "the first route is a trophy");
  }

  // --- fishing: five paying catches a day, then just for fun
  {
    Pet p; fresh(p, 20000);
    int coinsCatches = 0;
    for (int i = 0; i < FISH_PER_DAY; i++) { p.fishCatch(); if (p.lastFishCoins) coinsCatches++; }
    ck(p.fishPaidLeft() == 0 && p.tally[TALLY_FISH] == FISH_PER_DAY, "five catches counted");
    uint32_t c = p.coins; uint8_t on = p.foodCount[FOOD_ONIGIRI], sd = p.seedCount;
    for (int i = 0; i < 20; i++) p.fishCatch();
    ck(p.coins == c && p.foodCount[FOOD_ONIGIRI] == on && p.seedCount == sd, "past five, catches pay nothing");
    p.lastSeenEpoch = atDay(20001, 12);
    ck(p.fishPaidLeft() == FISH_PER_DAY, "five more tomorrow");
    // value: average coins for a paying catch
    Pet q; fresh(q, 20000);
    uint32_t total = 0; int n = 0;
    for (uint32_t d = 20000; d < 20400; d++) { q.lastSeenEpoch = atDay(d, 12); q.lastCareDay = d; /* no daily care bonus in the sum */ uint32_t b = q.coins; for (int i = 0; i < FISH_PER_DAY; i++) q.fishCatch(); total += q.coins - b; n++; }
    printf("      fishing: %.1f coins a day from the paying catches\n", (double)total / n);
    ck(total / n >= 4 && total / n <= 12, "fishing pays a little, not a fortune");
  }

  // --- cooking
  {
    Pet p; fresh(p, 20000);
    ck(!p.canCook(0) && !p.cook(0), "no ingredients, no pie");
    p.foodCount[FOOD_COOKIE] = 1; p.foodCount[FOOD_JUICE] = 1;
    ck(p.canCook(0) && p.cook(0), "a cookie and a juice make a pie");
    ck(p.foodCount[FOOD_PIE] == 1 && !p.foodCount[FOOD_COOKIE] && !p.foodCount[FOOD_JUICE], "the ingredients are used up");
    p.coins = 1000;
    ck(!p.buyFood(FOOD_PIE) && !p.buyFood(FOOD_STEW), "cooked dishes are not sold");
    uint8_t bond = p.bond; p.fullness = 20;
    ck(p.eatFood(FOOD_PIE) && p.bond > bond, "home-cooked food strengthens the bond");
    ck(p.diary[0].flags & DF_COOKED, "the diary remembers the cooking");
  }

  // --- alarm and the bedtime reminder
  {
    Pet p; fresh(p, 20000);
    p.alarmH = 7; p.alarmM = 30;
    p.lastSeenEpoch = atDay(20000, 7, 30);
    ck(!p.alarmDue(), "off: never rings");
    p.alarmOn = true;
    p.lastSeenEpoch = atDay(20000, 7, 29);
    ck(!p.alarmDue(), "a minute early: not yet");
    p.lastSeenEpoch = atDay(20000, 7, 30);
    ck(p.alarmDue(), "7:30: rings");
    p.alarmRang();
    ck(!p.alarmDue(), "once a day");
    p.lastSeenEpoch = atDay(20001, 9, 0);
    ck(!p.alarmDue(), "switched on hours later: it does not ring late");
    p.lastSeenEpoch = atDay(20002, 8, 10);
    ck(p.alarmDue(), "40 minutes late (it was off): still rings");
    p.parentBed = 21;
    p.lastSeenEpoch = atDay(20002, 21, 5);
    ck(p.bedReminderDue(), "bedtime reminder at the parents' hour");
    p.bedReminded();
    ck(!p.bedReminderDue(), "once");
  }

  // --- chores
  {
    Pet p; fresh(p, 20000);
    ck(p.doChore(0) && p.coins == CHORE_COINS, "a chore pays");
    ck(!p.doChore(0) && p.coins == CHORE_COINS, "once a day");
    for (int c = 1; c < CHORE_COUNT; c++) p.doChore(c);
    ck(p.coins == CHORE_COINS * CHORE_COUNT && p.diary[0].chores == CHORE_COUNT, "all six: counted in the diary");
    p.lastSeenEpoch = atDay(20001, 12);
    ck(!p.choreDoneToday(0) && p.doChore(0), "a new day, the chart is clear");
  }

  // --- piggy bank: deposits, weekly interest, withdrawing
  {
    Pet p; fresh(p, 20000);
    ck(!p.piggyDeposit(10), "no coins to save");
    p.coins = 200;
    ck(p.piggyDeposit(100) && p.coins == 100 && p.piggy == 100, "saved 100");
    ck(p.piggyDaysLeft() == 7 && p.piggyNextInterest() == 5, "interest in 7 days: 5 coins");
    p.lastSeenEpoch = atDay(20006, 12); p.earnCoins(0, true);
    ck(p.piggy == 100, "six days: nothing yet");
    p.lastSeenEpoch = atDay(20007, 12); p.earnCoins(0, true);
    ck(p.piggy == 105 && p.piggyLastInterest == 5, "a week: 5% interest");
    p.piggy = 10000;
    p.lastSeenEpoch = atDay(20014, 12); p.earnCoins(0, true);
    ck(p.piggy == 10000 + PIGGY_INTEREST_MAX, "interest is capped");
    uint32_t c = p.coins;
    ck(p.piggyWithdraw() == 10000 + PIGGY_INTEREST_MAX && p.coins == c + 10000 + PIGGY_INTEREST_MAX && p.piggy == 0, "taking it all out");
  }

  // --- the diary: a page a day, the last week kept
  {
    Pet p; fresh(p, 20000);
    p.fullness = 0; p.feedBerry(0); p.fullness = 0; p.feedBerry(0);
    p.clean();
    ck(p.diary[0].day == 20000 && p.diary[0].meals == 2 && p.diary[0].baths == 1, "today's page counts meals and baths");
    ck(p.diaryCoins(0) > 0, "and the coins earned today");
    uint16_t today = p.diaryCoins(0);
    p.lastSeenEpoch = atDay(20001, 9);
    p.energy = 100; p.parkWalk();
    ck(p.diary[0].day == 20001 && p.diary[0].walks == 1 && p.diary[1].day == 20000 && p.diary[1].coins == today, "the page turns at midnight");
    for (int d = 2; d < 12; d++) { p.lastSeenEpoch = atDay(20000 + d, 9); p.clean(); }
    ck(p.diary[0].day == 20011 && p.diary[DIARY_DAYS - 1].day == 20005, "only the last week is kept");
  }

  // --- persistence
  {
    Pet p; fresh(p, 20000);
    p.advRoute = 2; p.advStop = 3; p.advSteps = 1234;
    p.fishDay = 20000; p.fishToday = 4;
    p.alarmOn = true; p.alarmH = 6; p.alarmM = 45; p.alarmDay = 19999;
    p.choreDay = 20000; p.choresDone = 0x15; p.piggy = 77; p.piggyDay = 19998;
    p.clean();
    p.setClock(p.lastSeenEpoch);
    Pet q; q.begin();
    ck(q.advRoute == 2 && q.advStop == 3 && q.advSteps == 1234, "the adventure persists");
    ck(q.fishToday == 4 && q.alarmOn && q.alarmH == 6 && q.alarmM == 45, "fishing and the alarm persist");
    ck(q.choresDone == 0x15 && q.piggy == 77 && q.piggyDay == 19998, "chores and the piggy bank persist");
    ck(q.diary[0].day == 20000 && q.diary[0].baths == 1, "the diary persists");
  }

  if (bad) printf("FAILURES\n");
  return bad?1:0;
}
