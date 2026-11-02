// Daily missions and the weekly story (phase 31): six a day from five groups,
// only from what she can do, and every kind -- missions and story steps alike --
// is reached through the real place that counts it, not by calling noteMission.
#include "Arduino.h"
#include "Preferences.h"
#include "pet.h"
#include "party.h"
#include <cstdio>
#include <cstring>
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

// a pet with everything to hand: coins, soap, seeds, a letter, a toy, ingredients
static void rich(Pet &p, uint32_t day){
  p.begin();
  p.dbgHatchAs(1,false);
  p.sleeping = false; p.sleepAuto = SLEEP_NONE; p.lightOn = true;
  p.fullness = 50; p.joy = p.energy = p.hygiene = 60;
  p.sickness = SICK_NONE;
  p.coins = 1000; p.coinsToday = 0; p.coinDay = 0; p.missionKindDay = 0;
  memset(p.missionProg, 0, sizeof(p.missionProg)); p.missionDone = 0;
  p.stepCount = 0; p.stepDay = 0;
  p.lastCareDay = day;
  p.lastSeenEpoch = atDay(day, 12);
  p.soapCount = 3; p.seedCount = 2; p.plantEpoch = 0; p.brushOwned = true;
  p.mailKind = 1; p.mailRead = false;
  p.decorOwned = 1;
  for (int i = 0; i < FOOD_COUNT; i++) p.foodCount[i] = 3;
  p.contestWeek = 0; p.advStop = 0; p.advSteps = 0;
  p.ticketDay = 0; p.ticketsToday = 0; p.capDay = 0; p.capSpins = 0; p.bookDay = 0; p.songDay = 0;
  p.choreDay = 0; p.choresDone = 0; p.teethDay = 0; p.shellDay = 0; p.castleDay = 0; p.centerDay = 0; p.fishDay = 0;
  p.questWeek = day / 7; p.questStep = 0; p.questStepDay = 0;
}

static int8_t miniFor(int8_t stat){
  for (uint8_t g = 0; g < MINI_COUNT; g++) if (Pet::miniGameStat(g) == stat) return (int8_t)g;
  return -1;
}

// does mission m once, the way the player would
static void doMission(Pet &p, uint8_t m){
  switch (m) {
    case MIS_FEED: p.feed(); break;
    case MIS_FEED_FAV: for (uint8_t c = 0; c < 8; c++) if (p.lovesBerry(c)) { p.feedBerry(c); break; } break;
    case MIS_PLAY: case MIS_SCORE: case MIS_SCORE_HI: p.applyMiniResult(miniFor(MINI_NO_STAT) >= 0 ? miniFor(MINI_NO_STAT) : 0, 80); break;
    case MIS_TRAIN_ATK: p.applyMiniResult(miniFor(MINI_STAT_ATK), 50); break;
    case MIS_TRAIN_SPE: p.applyMiniResult(miniFor(MINI_STAT_SPE), 50); break;
    case MIS_TRAIN_DEF: p.applyMiniResult(miniFor(MINI_STAT_DEF), 50); break;
    case MIS_STEPS: case MIS_STEPS_BIG: p.addSteps(1000); break;
    case MIS_BATH: p.clean(); break;
    case MIS_BUBBLE: p.bubbleBath(); break;
    case MIS_PET: p.caress(); break;
    case MIS_TEETH: p.teethDone(); break;
    case MIS_NAP: p.startNap(); p.sleeping = false; p.napEnd = 0; break;
    case MIS_HOMEMEAL: p.eatFood(FOOD_SHOP_COUNT); break;
    case MIS_TICKETS: p.earnTickets(100, false); break;
    case MIS_CAPSULE: p.spinCapsule(true); break;
    case MIS_QUIZ: p.quizDone(5); break;
    case MIS_CONTEST: p.enterContest(80); break;
    case MIS_PARK: p.energy = 100; p.parkWalk(); break;
    case MIS_ADV: p.advSteps = Pet::advNeed(p.advStop); p.advClaim(); break;
    case MIS_FISH: p.fishCatch(); break;
    case MIS_BEACH: p.findShell(); break;
    case MIS_CENTER: p.centerHeal(); break;
    case MIS_COOK: p.cook(0); break;
    case MIS_GARDEN: p.plantSeed(); break;
    case MIS_CHORES: for (uint8_t c = 0; c < CHORE_COUNT; c++) if (p.doChore(c)) break; break;
    case MIS_READ: p.readBook(); break;
    case MIS_STUDY: p.studyDone(10); break;
    case MIS_SONG: p.playSong(); break;
    case MIS_MAIL: p.openMail(); break;
    case MIS_PIGGY: p.piggyDeposit(5); break;
    case MIS_BUY_FOOD: p.foodCount[0] = 0; p.buyFood(0); break;
    case MIS_SHOP: p.buySoap(); break;
    case MIS_DECOR: p.placeDecor(0); break;
  }
}

// does the story step kind k once
static void doStep(Pet &p, uint8_t k){
  switch (k) {
    case QK_PARK: doMission(p, MIS_PARK); break;
    case QK_BEACH: p.buildCastle(); break;
    case QK_READ: doMission(p, MIS_READ); break;
    case QK_FISH: doMission(p, MIS_FISH); break;
    case QK_COOK: doMission(p, MIS_COOK); break;
    case QK_MEAL: doMission(p, MIS_HOMEMEAL); break;
    case QK_ADV: doMission(p, MIS_ADV); break;
    case QK_CENTER: doMission(p, MIS_CENTER); break;
    case QK_BATH: doMission(p, MIS_BATH); break;
    case QK_SONG: doMission(p, MIS_SONG); break;
    case QK_GAME: doMission(p, MIS_PLAY); break;
    case QK_STUDY: doMission(p, MIS_STUDY); break;
    case QK_GARDEN: doMission(p, MIS_GARDEN); break;
    case QK_CHORE: doMission(p, MIS_CHORES); break;
    case QK_NAP: doMission(p, MIS_NAP); break;
    case QK_TEETH: doMission(p, MIS_TEETH); break;
    case QK_QUIZ: doMission(p, MIS_QUIZ); break;
    case QK_CAPSULE: doMission(p, MIS_CAPSULE); break;
    case QK_SHOP: doMission(p, MIS_SHOP); break;
    case QK_TICKETS: doMission(p, MIS_TICKETS); break;
    case QK_STEPS: p.addSteps(QUEST_WALK_STEPS); break;
    case QK_FAVBERRY: doMission(p, MIS_FEED_FAV); break;
  }
}

int main(){
  // --- six a day: all different, one from each group, the same all day
  {
    Pet p; rich(p, 20000);
    bool distinct = true, groups = true, stable = true;
    bool seen[MIS_COUNT] = { false };
    for (uint32_t d = 20000; d < 20400; d++) {
      p.lastSeenEpoch = atDay(d, 9);
      uint8_t k[MISSIONS_PER_DAY];
      for (int i = 0; i < MISSIONS_PER_DAY; i++) { k[i] = p.missionAt(i); seen[k[i] % MIS_COUNT] = true; }
      for (int i = 0; i < MISSIONS_PER_DAY; i++) for (int j = i + 1; j < MISSIONS_PER_DAY; j++) distinct = distinct && k[i] != k[j];
      for (int g = 0; g < MISSION_GROUPS; g++) {
        bool has = false;
        for (int i = 0; i < MISSIONS_PER_DAY; i++) has = has || Pet::missionGroup(k[i]) == g;
        groups = groups && has;
      }
      p.lastSeenEpoch = atDay(d, 21);
      for (int i = 0; i < MISSIONS_PER_DAY; i++) stable = stable && p.missionAt(i) == k[i];
    }
    ck(distinct, "six different missions every day");
    ck(groups, "one from each group: care, play, outdoors, home, shop");
    ck(stable, "the same in the morning and the evening");
    bool all = true;
    for (int m = 0; m < MIS_COUNT; m++) all = all && seen[m];
    ck(all, "every mission kind comes up some day");
  }

  // --- rolled once and kept: something unlocking at noon does not reshuffle the day
  {
    Pet p; rich(p, 20000);
    p.mailKind = 0; p.coins = 0;
    p.caress();                                 // the day starts: the pick is made and saved
    uint8_t k[MISSIONS_PER_DAY];
    for (int i = 0; i < MISSIONS_PER_DAY; i++) k[i] = p.missionAt(i);
    p.mailKind = 2; p.mailRead = false; p.coins = 1000;
    bool same = true;
    for (int i = 0; i < MISSIONS_PER_DAY; i++) same = same && p.missionAt(i) == k[i];
    ck(same && p.missionKindDay == 20000, "a letter or coins later in the day change nothing");
    p.saveNow();
    Pet q; q.begin();
    q.lastSeenEpoch = atDay(20000, 15);       // the clock, as the ball would have it
    for (int i = 0; i < MISSIONS_PER_DAY; i++) same = same && q.missionAt(i) == k[i];
    ck(same, "and the pick survives a reboot");
  }

  // --- only what she can do: broke, no soap, no letter, no toys, nothing to cook
  {
    Pet p; rich(p, 20000);
    p.coins = 0; p.soapCount = 0; p.seedCount = 0; p.mailKind = 0; p.decorOwned = 0; p.furnOwned = 0;
    for (int i = 0; i < FOOD_COUNT; i++) p.foodCount[i] = 0;
    p.contestWeek = 20000 / 7;
    bool clean = true, six = true;
    for (uint32_t d = 20000; d < 20005; d++) {      // the contest week, and the next
      p.lastSeenEpoch = atDay(d, 9);
      for (int i = 0; i < MISSIONS_PER_DAY; i++) {
        uint8_t m = p.missionAt(i);
        bool locked = m == MIS_BUBBLE || m == MIS_CAPSULE || m == MIS_SHOP || m == MIS_BUY_FOOD || m == MIS_PIGGY
                   || m == MIS_DECOR || m == MIS_MAIL || m == MIS_COOK || m == MIS_HOMEMEAL || m == MIS_GARDEN
                   || (m == MIS_CONTEST && d / 7 == 20000 / 7);
        clean = clean && !locked;
      }
      uint8_t a = p.missionAt(0);
      for (int i = 1; i < MISSIONS_PER_DAY; i++) six = six && p.missionAt(i) != a;
    }
    ck(clean, "nothing she cannot do comes up");
    ck(six, "and the day still has six (the shop's slot goes to another group)");
  }

  // --- every mission completes through its real place, and pays once
  {
    int ok = 0;
    for (uint8_t m = 0; m < MIS_COUNT; m++) {
      Pet p; rich(p, 20000);
      p.coinDay = 20000; p.missionKindDay = 20000;
      p.missionKind[0] = m;
      for (int i = 1; i < MISSIONS_PER_DAY; i++) p.missionKind[i] = (uint8_t)((m + i) % MIS_COUNT);
      p.missionDone = 0x3E;                     // the others done: m is the only one left
      for (int t = 0; t < 60 && !p.missionComplete(0); t++) { g_ms += 5000; doMission(p, m); }
      if (p.missionComplete(0)) ok++;
      else printf("  mission %u never completed (progress %u/%u)\n", m, p.missionProgress(0), Pet::missionGoal(m));
    }
    ck(ok == MIS_COUNT, "every mission kind can be completed where it happens");
  }
  {
    Pet p; rich(p, 20000);
    p.coinDay = 20000; p.missionKindDay = 20000;
    const uint8_t K[MISSIONS_PER_DAY] = { MIS_READ, MIS_SONG, MIS_FISH, MIS_PARK, MIS_TEETH, MIS_MAIL };
    memcpy(p.missionKind, K, sizeof(K));
    uint32_t c = p.coins;
    p.readBook();
    ck(p.missionComplete(0) && p.coins - c == COIN_MISSION, "a mission pays COIN_MISSION");
    c = p.coins; p.readBook();
    ck(p.coins == c, "and only once");
    p.playSong(); p.fishCatch(); p.fishCatch(); p.energy = 100; p.parkWalk(); p.teethDone();
    c = p.coins;
    uint32_t piggyBefore = p.piggy;
    p.openMail();
    ck(p.allMissionsDone() && (p.missionDone & 0x80) && p.coins - c >= COIN_MISSION + COIN_ALL_MISSIONS, "the sixth: the all-missions bonus");
    c = p.coins; p.noteMission(MIS_READ, 1);
    ck(p.coins == c && p.piggy == piggyBefore, "the bonus is paid once");
    ck(p.isExcited(), "everything done: it is excited");
  }

  // --- the upgrade: three old slots become today's six, fresh, keeping a paid bonus
  {
    Pet p; rich(p, 20000);
    p.coinDay = 20000; p.missionDone = 0x87; p.missionProg[1] = 2;
    p.missionKindDay = 0;                       // what a save from before phase 31 loads as
    p.saveNow();
    Pet q; q.begin();
    ck(q.missionKindDay == 20000 && q.missionProg[1] == 0 && q.missionDone == 0x80, "an old save: a fresh six, the bonus stays paid");
  }

  // --- the stories: 12 of 4 steps, every step reachable where it happens
  {
    bool used[QK_COUNT] = { false };
    int okSteps = 0, steps = 0;
    bool norepeat = true;
    for (uint8_t q = 0; q < QUEST_COUNT; q++) {
      uint32_t d = 20000;
      while ((d / 7) % QUEST_COUNT != q) d += 7;
      for (uint8_t s = 0; s < QUEST_STEPS; s++) {
        Pet p; rich(p, d);
        p.questWeek = d / 7; p.questStep = s; p.questStepDay = 0;
        uint8_t k = Pet::questStepKind(q, s);
        used[k] = true;
        for (uint8_t o = 0; o < s; o++) norepeat = norepeat && Pet::questStepKind(q, o) != k;
        doStep(p, k);
        steps++;
        if (p.questStep == s + 1) okSteps++;
        else printf("  story %u step %u (kind %u) did not advance\n", q, s, k);
      }
    }
    ck(steps == QUEST_COUNT * QUEST_STEPS && okSteps == steps, "every step of every story advances where it happens");
    bool all = true;
    for (int k = 0; k < QK_COUNT; k++) all = all && used[k];
    ck(all, "every step kind is used by some story");
    ck(norepeat, "no story repeats a step");
  }

  // --- the upgrade: a 3-step story already finished stays finished
  {
    Pet p; rich(p, 20000);
    p.questWeek = 20000 / 7; p.questStep = 3; p.questStepDay = 19999;
    p.saveNow();
    nvs().erase("qver");                       // a save from before phase 31
    Pet q; q.begin();
    ck(q.questStep == QUEST_STEPS && !q.questStepReady(), "an old finished story: still finished, no second hat");
    p.questStep = 2; p.saveNow();
    nvs().erase("qver");                       // a save from before phase 31
    Pet r; r.begin();
    ck(r.questStep == 2, "an unfinished one keeps its count");
  }

  if (bad) printf("FAILURES\n");
  return bad?1:0;
}
