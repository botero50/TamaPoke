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

// one minute of the pet clock, through the public update()
static void tickOnce(Pet &p){ g_ms += PET_TICK_MS; p.update(g_ms); }

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

  // --- the minigame records: a save from before the four new games still loads
  {
    Pet p; fresh(p, 20000);
    p.setClock(p.lastSeenEpoch);
    Preferences pr; pr.begin("tamapoke", false);
    uint16_t old[8] = { 1, 2, 3, 44, 5, 6, 7, 8 };
    pr.putBytes("mnhi", old, sizeof(old));
    pr.end();
    Pet q; q.begin();
    ck(q.miniHi[3] == 44 && q.miniHi[7] == 8 && q.miniHi[MINI_MAZE] == 0, "old records load; the new games start at 0");
    q.applyMiniResult(MINI_SIMON, 6);
    ck(q.miniHi[MINI_SIMON] == 6, "a new game keeps its record");
  }

  // --- phase 13: the new hats need the high bits; the room's furniture
  {
    Pet p; fresh(p, 20000);
    p.hatsOwned = 0; p.coins = 1000;
    ck(p.buyHat(HAT_VIKING) && p.ownsHat(HAT_VIKING) && !p.ownsHat(HAT_CAP), "a new hat is bought into the high bits");
    p.wearHat(HAT_VIKING);
    p.furnOwned = 0; p.furnWallL = p.furnWallR = p.furnBackL = p.furnBackR = -1; p.rugOn = false; p.wallpaper = -1; p.sceneryPick = -1;
    ck(p.buyFurn(FURN_SOFA) && p.coins == 1000 - Pet::hatPrice(HAT_VIKING) - Pet::furnPrice(FURN_SOFA), "furniture is bought");
    ck(!p.buyFurn(FURN_SOFA), "once");
    p.placeFurn(FURN_SOFA);
    ck(p.furnBackL == FURN_SOFA && p.sceneryPick == SCENERY_ROOM, "the sofa stands at the back, and the room is shown");
    p.placeFurn(FURN_SOFA);
    ck(p.furnBackL == -1 && !p.furnPlaced(FURN_SOFA), "tapped again: put away");
    p.furnOwned = 0xFFFF & ((1 << FURN_COUNT) - 1);
    p.placeFurn(FURN_WINDOW); p.placeFurn(FURN_CLOCK); p.placeFurn(FURN_PAINTING);
    ck(p.furnWallL == FURN_PAINTING && p.furnWallR == FURN_CLOCK, "a third wall piece replaces the left one");
    p.placeFurn(FURN_TV); p.placeFurn(FURN_RUG); p.placeFurn(FURN_PAPER_PINK);
    ck(p.furnBackL == FURN_TV && p.rugOn && p.wallpaper == FURN_PAPER_PINK, "floor, rug and wallpaper each have their place");
    ck(p.ownsScenery(SCENERY_ROOM) && Pet::sceneryPrice(SCENERY_ROOM) == 0, "the room is free");
    p.setClock(p.lastSeenEpoch);
    Pet q; q.begin();
    ck(q.ownsHat(HAT_VIKING) && q.hatOn == HAT_VIKING, "the new hat persists");
    ck(q.furnWallL == FURN_PAINTING && q.furnBackL == FURN_TV && q.rugOn && q.wallpaper == FURN_PAPER_PINK && q.sceneryPick == SCENERY_ROOM, "the room persists");
  }

  // --- phase 14: wishes
  {
    Pet p; fresh(p, 20000);
    p.wishKind = WISH_NONE; p.wishNextAt = 0; p.wishDay = 0; p.wishesToday = 0;
    int made = 0;
    for (int m = 0; m < 24 * 60; m++) {    // a whole day of awake minutes, granting every wish at once
      p.lastSeenEpoch = atDay(20000, 0) + m * 60;
      p.sleeping = false;
      p.wishTick();
      if (p.wishKind) { made++; p.wishKind = WISH_NONE; }
    }
    ck(made >= 1 && made <= WISHES_PER_DAY, "a few wishes a day, never more than three");
    p.lastSeenEpoch = atDay(20001, 10);
    p.wishKind = WISH_FOOD; p.wishArg = FOOD_CURRY; p.wishUntil = p.lastSeenEpoch + 3600;
    p.foodCount[FOOD_SOUP] = 1; p.foodCount[FOOD_CURRY] = 1; p.fullness = 20;
    uint32_t c = p.coins;
    p.eatFood(FOOD_SOUP);
    ck(p.wishKind == WISH_FOOD, "the wrong food does not grant a food wish");
    p.eatFood(FOOD_CURRY);
    ck(p.wishKind == WISH_NONE && p.coins >= c + WISH_COINS && p.wishGrantedFlash, "the right one does: coins and a flash");
    p.wishKind = WISH_PET; p.wishUntil = p.lastSeenEpoch + 3600;
    p.caress();
    ck(p.wishKind == WISH_NONE, "a cuddle grants a cuddle wish");
    p.wishKind = WISH_BATH; p.wishUntil = p.lastSeenEpoch + 3600;
    p.lastSeenEpoch += 3601; p.wishTick();
    ck(p.wishKind == WISH_NONE, "an old wish fades away");
  }
  // --- personality, lonely, excited
  {
    Pet p; fresh(p, 20000);
    Personality a = p.personality();
    ck(a < PERS_COUNT && p.personality() == a, "the personality is stable");
    p.lastCareEpoch = p.lastSeenEpoch;
    p.lastSeenEpoch += LONELY_SECS + 60;
    ck(p.isLonely(), "three hours with nobody: lonely");
    p.caress();
    ck(!p.isLonely(), "a cuddle fixes it");
    p.missionDone = (1 << MISSIONS_PER_DAY) - 1; p.coinDay = p.lastSeenEpoch / 86400;
    ck(p.isExcited(), "every mission done: excited");
  }
  // --- birthdays
  {
    Pet p; fresh(p, 20000);
    p.bornDay = 20000; p.bdayDay = 0;
    p.lastSeenEpoch = atDay(20000 + BIRTHDAY_DAYS - 1, 12);
    ck(p.birthdayMonths() == 0 && !p.claimBirthday(), "the day before: no party");
    p.lastSeenEpoch = atDay(20000 + BIRTHDAY_DAYS, 12);
    uint8_t cakes = p.foodCount[FOOD_CAKE];
    ck(p.birthdayMonths() == 1 && p.claimBirthday() && p.foodCount[FOOD_CAKE] == cakes + 1, "one month: a party with cake");
    ck(!p.claimBirthday(), "once on the day");
    p.lastSeenEpoch = atDay(20000 + 2 * BIRTHDAY_DAYS, 12);
    ck(p.birthdayMonths() == 2, "two months");
  }
  // --- events: Easter and the summer festival
  {
    int m, d;
    Pet::easterDate(2026, m, d);
    ck(m == 4 && d == 5, "Easter 2026 is 5 April");
    Pet::easterDate(2027, m, d);
    ck(m == 3 && d == 28, "Easter 2027 is 28 March");
    Pet p; fresh(p, 20548);              // 2026-04-05
    int y; Pet::civilDate(20548, y, m, d);
    ck(m == 4 && d == 5 && p.eventToday() == EVENT_EASTER, "Easter Sunday is egg-hunt week");
    p.eggDay = 0; p.eggsFound = 0;
    int eggs = 0;
    for (int i = 0; i < 100; i++) eggs += p.findEasterEgg();
    ck(eggs == EASTER_EGGS_PER_DAY, "five eggs a day at most");
    fresh(p, 20638);                     // 2026-07-04
    ck(p.eventToday() == EVENT_SUMMER, "July is the summer festival");
    p.festDay = 0;
    ck(p.claimFestivalTreat() && !p.claimFestivalTreat(), "a free ice cream, once a day");
    fresh(p, 20758);
    ck(p.eventToday() == EVENT_NONE, "1 November: no event");
  }

  // --- phase 16: the wardrobe
  {
    Pet p; fresh(p, 20000);
    p.accOwned = 0; p.accOn[0] = p.accOn[1] = p.accOn[2] = -1; p.hatsOwned = 0; p.hatOn = -1;
    p.coins = 2000;
    ck(p.buyAcc(ACC_SUNGLASSES) && p.wearingAcc(ACC_SUNGLASSES), "bought glasses are worn");
    ck(p.buyAcc(ACC_STARGLASSES) && p.wearingAcc(ACC_STARGLASSES) && !p.wearingAcc(ACC_SUNGLASSES), "one pair at a time: the new one replaces it");
    ck(p.buyAcc(ACC_CAPE) && p.wearingAcc(ACC_CAPE) && p.wearingAcc(ACC_STARGLASSES), "a cape goes on the back, the glasses stay");
    ck(p.wearingSet() == 1, "cape + star glasses: the superhero set");
    p.wearAcc(ACC_CAPE);
    ck(!p.wearingAcc(ACC_CAPE) && p.wearingSet() < 0, "tapped again: off, and no set");
    p.buyHat(HAT_PIRATE); p.wearHat(HAT_PIRATE); p.buyAcc(ACC_BANDANA); p.buyAcc(ACC_MONOCLE);
    ck(p.wearingSet() == 0, "pirate hat + bandana + monocle: the pirate set");
    p.setClock(p.lastSeenEpoch);
    Pet q; q.begin();
    ck(q.ownsAcc(ACC_CAPE) && q.wearingAcc(ACC_MONOCLE) && q.wearingAcc(ACC_BANDANA) && q.wearingSet() == 0, "the wardrobe persists");
  }

  // --- phase 17: town life
  {
    Pet p; fresh(p, 20000);
    p.shellDay = 0; p.castleDay = 0; p.bookDay = 0; p.songDay = 0; p.contestWeek = 0;
    p.ribbons[0] = p.ribbons[1] = p.ribbons[2] = 0; p.jobKind = JOB_NONE; p.jobDay = 0; p.mailDay = 0; p.mailKind = 0; p.mailRead = true;
    int shells = 0;
    for (int i = 0; i < 10; i++) shells += p.findShell();
    ck(shells == SHELLS_PER_DAY, "five shells a day");
    ck(p.buildCastle() && !p.buildCastle(), "one sandcastle a day");
    int books = 0, songs = 0;
    for (int i = 0; i < 6; i++) { books += p.readBook(); songs += p.playSong(); }
    ck(books == BOOKS_PER_DAY && songs == SONGS_PER_DAY, "three books and three songs a day");
    uint32_t c = p.coins;
    ck(p.enterContest(120) == RIB_GOLD && p.ribbons[2] == 1 && p.coins == c + 20, "120 points: gold, 20 coins");
    ck(p.contestDoneThisWeek() && p.enterContest(120) == RIB_NONE, "once a week");
    p.lastSeenEpoch = atDay(20007, 12);
    ck(!p.contestDoneThisWeek() && p.enterContest(50) == RIB_BRONZE, "next week: 50 points is bronze");
    p.hatOn = -1; p.accOn[0] = p.accOn[1] = p.accOn[2] = -1; p.bond = 0; p.joy = 0;
    ck(p.contestScore() == 0, "plain and unloved: no points");
    // a job: away for three hours, nothing drains, paid on return
    p.lastSeenEpoch = atDay(20010, 9); p.fullness = 60; p.joy = 60;
    ck(p.startJob(JOB_CAFE) && p.atWork(), "off to the cafe");
    ck(!p.startJob(JOB_POST), "one job a day");
    uint8_t food = p.fullness;
    for (int m = 0; m < 60; m++) { p.lastSeenEpoch += 60; tickOnce(p); }
    ck(p.fullness == food, "nothing drains while it works");
    p.feedBerry(0);
    ck(p.fullness == food, "and it cannot be fed while it is out");
    c = p.coins;
    p.lastSeenEpoch = atDay(20010, 12) + 60; tickOnce(p);
    ck(!p.atWork() && p.coins >= c + JOB_COINS && p.jobDoneFlash, "home after three hours with its pay");
    // the mail: once a day, with a gift
    p.mailDay = 0; tickOnce(p);
    ck(p.hasMail(), "a letter arrives");
    uint8_t k = p.openMail();
    ck(k >= 1 && k <= LETTER_KINDS && !p.hasMail(), "read once");
    p.setClock(p.lastSeenEpoch);
    Pet q; q.begin();
    ck(q.ribbons[2] == 1 && q.ribbons[0] == 1 && q.jobDay == 20010 && q.mailDay == p.mailDay, "town life persists");
  }

  // --- phase 19: the weekly quest, Halloween and winter, the anniversary
  {
    Pet p; fresh(p, 20003);                // week 2857: quest (20003/7) % 4
    p.questWeek = 0; p.questStep = 0; p.questStepDay = 0; p.hatsOwned = 0;
    uint8_t q = p.questToday();
    QuestKind k0 = Pet::questStepKind(q, 0), k1 = Pet::questStepKind(q, 1), k2 = Pet::questStepKind(q, 2);
    ck(!p.questNote(k1) && p.questStep == 0, "out of order: nothing");
    ck(p.questNote(k0) && p.questStep == 1, "the first step");
    ck(!p.questNote(k1) && p.questStep == 1, "the next step waits for another day");
    p.lastSeenEpoch = atDay(20004, 12);
    ck(p.questNote(k1) && p.questStep == 2, "the second step, the next day");
    p.lastSeenEpoch = atDay(20005, 12);
    ck(p.questNote(k2) && p.questStep == 3 && p.questDoneFlash && p.hatsOwned != 0, "the third: done, and a hat");
    p.lastSeenEpoch = atDay(20003 + 7, 12);
    p.questRoll();
    ck(p.questStep == 0 && p.questToday() != q, "a new week: a new quest from the start");
    fresh(p, 20750);                       // 2026-10-24
    ck(p.eventToday() == EVENT_HALLOWEEN, "24 October starts Halloween week");
    p.festDay = 0; uint8_t ck0 = p.foodCount[FOOD_COOKIE];
    ck(p.claimFestivalTreat() == EVENT_HALLOWEEN && p.foodCount[FOOD_COOKIE] == ck0 + 1, "trick or treat: a cookie");
    fresh(p, 20807);                       // 2026-12-20
    ck(p.eventToday() == EVENT_WINTER, "20 December: the winter holidays");
    fresh(p, 20824);                       // 2027-01-06
    ck(p.eventToday() == EVENT_WINTER, "until 6 January");
    fresh(p, 20000);
    p.bornDay = 20000 - 360; p.bdayDay = 0; uint32_t c = p.coins;
    ck(p.birthdayMonths() == 12 && p.claimBirthday() && p.coins == c + 50, "a year old: a bigger party");
  }

  if (bad) printf("FAILURES\n");
  return bad?1:0;
}
