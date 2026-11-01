// Phases 8 and 9: the capsule machine, scenery, trophies, brushing teeth,
// homework time, the math quiz and the parent limits -- plus persistence.
#include "Arduino.h"
#include "Preferences.h"
#include "pet.h"
#include "party.h"
#include <cstdio>
uint32_t g_seed=57; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
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
  p.coins = 0; p.coinsToday = 0; p.coinDay = day;
  p.lastCareDay = day;
  p.lastSeenEpoch = atDay(day, 12);
  p.hatsOwned = 0; p.decorOwned = 0; p.soapCount = 0; p.seedCount = 0;
  for (int i = 0; i < FOOD_COUNT; i++) p.foodCount[i] = 0;
  p.capDay = 0; p.capSpins = 0; p.sceneryOwned = 0; p.sceneryPick = -1;
  for (int i = 0; i < TALLY_COUNT; i++) p.tally[i] = 0;
  p.coinsEarned = 0; p.trophies = 0;
  p.teethDay = 0; p.teethToday = 0; p.studyDay = 0; p.studyToday = 0; p.quizHi = 0;
  p.parentBed = 0; p.parentGames = 0; p.gamesToday = 0;
}

int main(){
  // --- capsules: cost, daily limit, and a fair return on average
  {
    Pet p; fresh(p, 20000);
    ck(p.spinCapsule() == CAP_NO_COINS && p.capsulesLeft() == CAPSULES_PER_DAY, "no coins, no capsule (and none used up)");
    p.coins = CAPSULE_PRICE * 3;
    int n = 0;
    for (int i = 0; i < 3; i++) if (p.spinCapsule() >= CAP_HAT) n++;
    ck(n == 3 && p.capsulesLeft() == 0, "three a day");
    uint32_t c = p.coins;
    p.coins += 100;
    ck(p.spinCapsule() == CAP_LIMIT && p.coins == c + 100, "the fourth is refused and costs nothing");
    p.lastSeenEpoch = atDay(20001, 12);
    ck(p.capsulesLeft() == CAPSULES_PER_DAY, "three more tomorrow");
  }
  {
    // value: average over many spins, items counted at shop price
    Pet p; fresh(p, 20000);
    double value = 0; int spins = 0, rare = 0;
    for (uint32_t d = 20000; d < 21000; d++) {
      p.lastSeenEpoch = atDay(d, 12);
      p.coins = 1000; p.hatsOwned = 0; p.decorOwned = 0;
      for (int i = 0; i < FOOD_COUNT; i++) p.foodCount[i] = 0;
      p.soapCount = p.seedCount = 0;
      for (int k = 0; k < CAPSULES_PER_DAY; k++) {
        uint32_t before = p.coins;
        CapsuleResult r = p.spinCapsule();
        spins++;
        if (r == CAP_HAT) { value += Pet::hatPrice((uint8_t)p.capWhat); rare++; }
        else if (r == CAP_DECOR) { value += Pet::decorPrice((uint8_t)p.capWhat); rare++; }
        else if (r == CAP_FOOD) value += Pet::foodInfo((uint8_t)p.capWhat).price;
        else if (r == CAP_SOAP) value += 2 * SOAP_PRICE;
        else if (r == CAP_SEED) value += 2 * SEED_PRICE;
        else if (r == CAP_COINS) value += p.coins - (before - CAPSULE_PRICE);
      }
    }
    double avg = value / spins;
    printf("      capsule: average value %.1f for %d coins, rare %.1f%%\n", avg, CAPSULE_PRICE, 100.0 * rare / spins);
    ck(avg > CAPSULE_PRICE * 0.9 && avg < CAPSULE_PRICE * 2.5, "a capsule is worth about its price (with a lucky streak now and then)");
    ck(rare > spins / 20 && rare < spins / 6, "about one in ten is a hat or decoration");
  }
  {
    Pet p; fresh(p, 20000);
    p.hatsOwned = (1 << HAT_COUNT) - 1; p.decorOwned = (1 << DEC_COUNT) - 1;
    bool sawBig = false;
    for (int d = 0; d < 400 && !sawBig; d++) {
      p.lastSeenEpoch = atDay(20000 + d, 12); p.coins = 100;
      if (p.spinCapsule() == CAP_COINS && p.capWhat == 50) sawBig = true;
    }
    ck(sawBig, "owning everything, the rare prize becomes 50 coins");
  }

  // --- scenery
  {
    Pet p; fresh(p, 20000);
    p.pickScenery(1);
    ck(p.sceneryPick == -1, "an unowned backdrop cannot be picked");
    ck(!p.buyScenery(1), "not without the coins");
    p.coins = Pet::sceneryPrice(1);
    ck(p.buyScenery(1) && p.coins == 0 && p.ownsScenery(1), "bought");
    ck(!p.buyScenery(1), "only once");
    p.pickScenery(1);
    ck(p.sceneryPick == 1, "picked");
    p.pickScenery(1);
    ck(p.sceneryPick == -1, "picking it again goes back to its own");
  }

  // --- trophies: tallies count, and each pays once
  {
    Pet p; fresh(p, 20000);
    ck(p.checkTrophies() == -1, "nothing yet");
    p.fullness = 0;
    for (int i = 0; i < 100; i++) { p.fullness = 0; p.feedBerry(0); }
    ck(p.tally[TALLY_MEALS] == 100, "meals are counted");
    uint32_t c = p.coins;
    ck(p.checkTrophies() == TRO_MEALS && p.coins == c + TROPHY_COINS, "100 meals: a trophy and its coins");
    ck(p.checkTrophies() == -1 && p.coins == c + TROPHY_COINS, "only once");
    ck(p.coinsEarned >= TROPHY_COINS, "earned coins are counted");
    p.hatsOwned = 0x1F;
    ck(p.checkTrophies() == TRO_HATS, "five hats");
    uint32_t cur, goal;
    p.trophyProgress(TRO_WALKS, cur, goal);
    ck(cur == 0 && goal == 25, "progress reads cur / goal");
    for (int i = 0; i < 3; i++) { p.energy = 100; p.parkWalk(); }
    p.trophyProgress(TRO_WALKS, cur, goal);
    ck(cur == 3, "walks are counted");
    p.clean();
    ck(p.tally[TALLY_BATHS] == 1, "baths are counted");
  }

  // --- teeth: twice a day
  {
    Pet p; fresh(p, 20000);
    p.hygiene = 50;
    ck(p.teethDone() && p.hygiene == 55, "brushing cleans a little");
    ck(p.teethDone(), "twice a day");
    ck(!p.teethDone() && p.tally[TALLY_TEETH] == 2, "not a third time");
    p.lastSeenEpoch = atDay(20001, 8);
    ck(p.teethDone(), "again the next day");
    p.sleeping = true;
    ck(!p.teethDone(), "not while it sleeps");
  }

  // --- homework: coins for the minutes, three sessions a day
  {
    Pet p; fresh(p, 20000);
    ck(p.studyDone(20) == 4 && p.coins == 4, "20 minutes: 4 coins");
    ck(p.studyDone(30) == 6, "30 minutes: 6 coins");
    ck(p.studyDone(10) == 2, "10 minutes: 2 coins");
    ck(p.studyDone(30) == 0 && p.tally[TALLY_STUDY] == 4, "a fourth still counts, but pays nothing");
    p.coinsToday = COIN_DAILY_CAP;
    p.lastSeenEpoch = atDay(20001, 12); p.coinDay = 20001; p.coinsToday = COIN_DAILY_CAP;
    ck(p.studyDone(20) == 4, "homework is outside the daily game cap");
  }

  // --- the quiz
  {
    Pet p; fresh(p, 20000);
    uint8_t c = p.quizDone(QUIZ_QUESTIONS);
    ck(c == COIN_GAME_MAX && p.quizHi == QUIZ_QUESTIONS && p.tally[TALLY_PERFECT] == 1, "a perfect quiz: best score, top game pay, a perfect counted");
    ck(p.quizDone(3) == 2 && p.quizHi == QUIZ_QUESTIONS, "a weak round pays a little, the best stays");
    ck(p.gamesToday == 2 && p.tally[TALLY_GAMES] == 2, "it counts as a game");
  }

  // --- parent limits
  {
    Pet p; fresh(p, 20000);
    ck(p.parentBlock() == PB_NONE, "off by default");
    p.parentBed = 21;
    p.lastSeenEpoch = atDay(20000, 20);
    ck(p.parentBlock() == PB_NONE, "before bedtime: fine");
    p.lastSeenEpoch = atDay(20000, 21);
    ck(p.parentBlock() == PB_BEDTIME, "at bedtime: no games");
    p.lastSeenEpoch = atDay(20001, 3);
    ck(p.parentBlock() == PB_BEDTIME, "still bedtime in the small hours");
    p.lastSeenEpoch = atDay(20001, 7); p.earnCoins(0, true);
    ck(p.parentBlock() == PB_NONE, "morning: fine again");
    p.parentGames = 2;
    p.quizDone(2);
    ck(p.parentBlock() == PB_NONE, "one game of two");
    p.quizDone(2);
    ck(p.parentBlock() == PB_LIMIT, "the limit reached");
    p.lastSeenEpoch = atDay(20002, 9); p.earnCoins(0, true);
    ck(p.parentBlock() == PB_NONE && p.gamesToday == 0, "a new day, a new allowance");
  }

  // --- persistence
  {
    Pet p; fresh(p, 20000);
    p.capSpins = 2; p.capDay = 20000; p.sceneryOwned = 0x12; p.sceneryPick = 4;
    p.tally[TALLY_WALKS] = 7; p.coinsEarned = 345; p.trophies = 0x0105;
    p.teethToday = 1; p.teethDay = 20000; p.studyToday = 2; p.studyDay = 20000; p.quizHi = 8;
    p.parentBed = 21; p.parentGames = 20; p.gamesToday = 5;
    p.setClock(p.lastSeenEpoch);   // saves
    Pet q; q.begin();
    ck(q.capSpins == 2 && q.capDay == 20000, "capsules persist");
    ck(q.sceneryOwned == 0x12 && q.sceneryPick == 4, "scenery persists");
    ck(q.tally[TALLY_WALKS] == 7 && q.coinsEarned == 345 && q.trophies == 0x0105, "trophies persist");
    ck(q.teethToday == 1 && q.studyToday == 2 && q.quizHi == 8, "teeth, study, quiz persist");
    ck(q.parentBed == 21 && q.parentGames == 20 && q.gamesToday == 5, "parent limits persist");
  }

  if (bad) printf("FAILURES\n");
  return bad?1:0;
}
