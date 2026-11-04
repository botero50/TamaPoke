// The road to the gym (phases 34-36): walking fills gymSteps up to the next
// leader's 10 000 + 2 000 x its place and stops there; a first easy win empties it
// and starts a week before the next challenge; rematches, losses and hard cost nothing.
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
static void fresh(Pet &p){
  p.begin();
  p.dbgHatchAs(1,false);
  p.lastSeenEpoch = 20000 * DAY + 12 * 3600;
  p.badges = p.badgesHard = 0;
  for (int r = 0; r < GYM_REGIONS - 1; r++) p.badgesX[r] = p.badgesHardX[r] = 0;
  p.region = 0;
  p.gymSteps = 0;
  p.gymLastWinDay = 0;
}

int main(){
  // --- phase 36: more steps, rising along the ladder; the road stops at its leader
  {
    Pet p; fresh(p);
    ck(Pet::gymStepsNeed(0) == 10000 && Pet::gymStepsNeed(7) == 24000 && Pet::gymStepsNeed(12) == 34000, "Brock 10 000, Giovanni 24 000, the Champion 34 000");
    ck(p.gymChallengeRegion() == 0 && p.gymChallengeIdx() == 0, "the first challenge: Kanto's first leader");
    ck(!p.gymChallengeReady(), "not before walking");
    p.addSteps(9999);
    ck(p.gymSteps == 9999 && !p.gymChallengeReady() && !p.gymWalked(), "9 999 steps: not yet");
    p.addSteps(1);
    ck(p.gymChallengeReady() && p.gymWaitDays() == 0, "10 000 steps: Brock is ready, the first one has no week to wait");
    p.addSteps(25000);
    ck(p.gymSteps == 10000, "walking on does not fill the road past Brock: nothing piles up for Misty");
    ck(p.stepTotal >= 35000, "...though every step still counts in the total");
    ck(!p.gymStepsOk(0, 1, false), "Misty needs her own steps");
    ck(p.gymStepsOk(0, 1, true), "hard is not gated");
  }

  // --- a win empties the road and starts a week; rematches, hard and losses cost nothing
  {
    Pet p; fresh(p);
    p.addSteps(10000);
    p.winBadge(0, 0, false);
    ck(p.gymSteps == 0 && p.gymChallengeIdx() == 1 && p.gymLastWinDay == 20000, "beating Brock empties the road and starts the week");
    p.addSteps(12000);
    ck(p.gymWalked() && !p.gymChallengeReady() && p.gymWaitDays() == 7, "Misty walked to the same day: 7 days to wait");
    ck(!p.gymStepsOk(0, 1, false), "and the ladder agrees");
    {                                    // the date falls behind the last win: still only a week
      uint32_t keep = p.lastSeenEpoch;
      p.lastSeenEpoch = 19950 * DAY + 9 * 3600;
      ck(p.gymWaitDays() == 7, "a date before the last win never asks for more than 7 days");
      p.lastSeenEpoch = keep;
    }
    p.winBadge(0, 0, false);
    p.winBadge(0, 3, true);
    ck(p.gymSteps == 12000 && p.gymLastWinDay == 20000, "a rematch and a hard badge change nothing");
    p.lastSeenEpoch = 20006 * DAY + 9 * 3600;
    ck(p.gymWaitDays() == 1 && !p.gymChallengeReady(), "6 days later: tomorrow");
    p.lastSeenEpoch = 20007 * DAY + 9 * 3600;
    ck(p.gymWaitDays() == 0 && p.gymChallengeReady() && p.gymStepsOk(0, 1, false), "a week later: Misty is ready");
    ck(p.gymSteps == 12000, "losing leaves the road full, to try again");
    p.region = 1; p.badges = 0;
    p.winBadge(0, 0, false);                        // the first new badge somewhere else
    ck(p.gymLastWinDay == 20007 && p.gymSteps == 0, "any first easy win starts the week again");
  }

  // --- the challenge region: its own first, then the first unfinished
  {
    Pet p; fresh(p);
    p.region = 2;
    ck(p.gymChallengeRegion() == 2 && p.gymChallengeIdx() == 0, "a Hoenn pet is challenged in Hoenn");
    p.badgesX[1] = 0x1FFF;                       // Hoenn (region 2) finished
    ck(p.gymChallengeRegion() == 0, "Hoenn done: back to Kanto");
    p.badges = 0x1FFF;
    ck(p.gymChallengeRegion() == 1, "Kanto done too: Johto");
    for (int r = 0; r < GYM_REGIONS - 1; r++) p.badgesX[r] = 0x1FFF;
    ck(p.gymChallengeRegion() == 0xFF && p.gymChallengeIdx() == -1 && !p.gymChallengeReady(), "everything beaten: no challenge");
    p.region = REGION_ALL; p.badges = 0; p.badgesX[0] = 0;
    ck(p.gymChallengeRegion() == 0, "a pet from every region starts in Kanto");
  }

  // --- an egg is never challenged
  {
    Pet p; fresh(p);
    p.addSteps(5000);
    p.newEgg();
    ck(!p.gymChallengeReady(), "an egg: no challenge");
  }

  // --- it survives a reboot; no free challenge for a save from before phase 35
  {
    Pet p; fresh(p);
    p.badges = 0x7;                              // three badges
    p.gymSteps = 1234;
    p.saveNow();
    Pet q; q.begin();
    ck(q.gymSteps == 1234, "the walk survives a reboot");
    nvs().erase("gymV");                         // phase 34's save: 1000 given away for free
    Pet r; r.begin();
    ck(r.gymChallengeIdx() == 3 && r.gymSteps == 0 && !r.gymChallengeReady(), "an old save starts the road at 0: Erika is not free");
    r.addSteps(Pet::gymStepsNeed(3));
    ck(r.gymChallengeReady(), "and her real steps bring her");
  }

  // --- phase 35: today resets at midnight; the 7 days, the total and the roads do not
  {
    Pet p; fresh(p);
    p.stepCount = 0; p.stepDay = 0; p.stepHistDay = 0; p.stepTotal = 0; p.advSteps = 0;
    for (int k = 0; k < 7; k++) p.stepHist[k] = 0;
    p.lastSeenEpoch = 20000 * DAY + 10 * 3600;
    p.addSteps(1200);
    ck(p.stepsToday() == 1200 && p.stepsDaysAgo(0) == 1200 && p.stepTotal == 1200, "1200 steps today, in the history and the total");
    p.lastSeenEpoch = 20001 * DAY + 9 * 3600;      // the next morning
    ck(p.stepsToday() == 0 && p.stepsDaysAgo(1) == 1200, "next morning: today is 0, yesterday kept");
    p.addSteps(300);
    ck(p.stepsToday() == 300 && p.stepsDaysAgo(0) == 300 && p.stepsDaysAgo(1) == 1200, "and today counts from 0 again");
    ck(p.stepTotal == 1500 && p.gymSteps == 1500 && p.advSteps == 1500, "the total and both roads carry on across the night");
    p.lastSeenEpoch = 20004 * DAY + 9 * 3600;      // three days without the ball
    p.addSteps(50);
    ck(p.stepsDaysAgo(0) == 50 && p.stepsDaysAgo(1) == 0 && p.stepsDaysAgo(2) == 0 && p.stepsDaysAgo(3) == 300 && p.stepsDaysAgo(4) == 1200,
       "days without walking come in as 0");
    p.lastSeenEpoch = 20012 * DAY + 9 * 3600;      // more than a week later
    ck(p.stepsDaysAgo(0) == 0 && p.stepsDaysAgo(6) == 0, "a week later the old days have gone");
    p.addSteps(10);
    ck(p.stepsDaysAgo(0) == 10 && p.stepTotal == 1560, "and the total still has everything");
    p.saveNow();
    Pet q; q.begin();
    q.lastSeenEpoch = 20012 * DAY + 10 * 3600;
    ck(q.stepsDaysAgo(0) == 10 && q.stepTotal == 1560 && q.gymSteps == 1560, "the history and the total survive a reboot");
  }

  // --- a clock glitch (a bad RTC read for a moment) never resets today or wipes the week
  {
    Pet p; fresh(p);
    p.stepCount = 0; p.stepDay = 0; p.stepHistDay = 0; p.stepTotal = 0;
    for (int k = 0; k < 7; k++) p.stepHist[k] = 0;
    p.lastSeenEpoch = 20009 * DAY + 9 * 3600;
    p.addSteps(800);
    p.lastSeenEpoch = 20010 * DAY + 9 * 3600;
    p.addSteps(400);
    ck(p.stepsToday() == 400 && p.stepsDaysAgo(1) == 800, "two days: 800 yesterday, 400 today");
    p.lastSeenEpoch = 20003 * DAY + 9 * 3600;      // a glitch: a week back for a moment
    p.addSteps(5);
    p.lastSeenEpoch = 20010 * DAY + 9 * 3610;      // and back to the real time
    p.addSteps(5);
    ck(p.stepsToday() == 410 && p.stepsDaysAgo(1) == 800 && p.stepTotal == 1210, "after the glitch: today 410, yesterday still 800, nothing lost");
    p.setClock(20008 * DAY + 9 * 3600);           // a deliberate change to an earlier date (TIME, a sync)
    ck(p.stepsToday() == 0, "a deliberate earlier date starts today again");
    p.addSteps(20);
    ck(p.stepsToday() == 20 && p.stepsDaysAgo(0) == 20, "and counts from there");
  }

  if (bad) printf("FAILURES\n");
  return bad?1:0;
}
