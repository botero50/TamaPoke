// The road to the gym (phase 34): walking fills gymSteps, the next leader not yet
// beaten on an easy ladder needs 1000 + 500 x its place, the first win spends
// them (the rest carries on), and rematches, losses and hard cost nothing.
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
}

int main(){
  // --- walking fills the road, and the steps needed rise along the ladder
  {
    Pet p; fresh(p);
    ck(Pet::gymStepsNeed(0) == 1000 && Pet::gymStepsNeed(7) == 4500 && Pet::gymStepsNeed(12) == 7000, "Brock 1000, Giovanni 4500, the Champion 7000");
    ck(p.gymChallengeRegion() == 0 && p.gymChallengeIdx() == 0, "the first challenge: Kanto's first leader");
    ck(!p.gymChallengeReady(), "not before walking");
    p.addSteps(999);
    ck(p.gymSteps == 999 && !p.gymChallengeReady(), "999 steps: not yet");
    p.addSteps(1);
    ck(p.gymChallengeReady(), "1000 steps: Brock is ready");
    ck(!p.gymStepsOk(0, 1, false), "Misty needs her own steps");
    ck(p.gymStepsOk(0, 1, true), "hard is not gated by steps");
  }

  // --- the first win spends the walk, the rest carries on; rematches and losses cost nothing
  {
    Pet p; fresh(p);
    p.addSteps(1300);
    p.winBadge(0, 0, false);
    ck(p.gymSteps == 300 && p.gymChallengeIdx() == 1, "beating Brock spends 1000, 300 carry on to Misty");
    p.winBadge(0, 0, false);
    ck(p.gymSteps == 300, "a rematch costs nothing");
    ck(p.gymStepsOk(0, 0, false), "and a beaten leader is always open");
    p.winBadge(0, 1, true);
    ck(p.gymSteps == 300, "a hard badge costs nothing");
    ck(!p.gymChallengeReady(), "300 of 1500: Misty waits");
    p.addSteps(1200);
    ck(p.gymChallengeReady(), "1500: Misty is ready");
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

  // --- it survives a reboot; a save from before phase 34 starts with its leader ready
  {
    Pet p; fresh(p);
    p.badges = 0x7;                              // three badges
    p.gymSteps = 1234;
    p.saveNow();
    Pet q; q.begin();
    ck(q.gymSteps == 1234, "the walk survives a reboot");
    nvs().erase("gymS");                         // what an old save looks like
    Pet r; r.begin();
    ck(r.gymChallengeIdx() == 3 && r.gymSteps == Pet::gymStepsNeed(3) && r.gymChallengeReady(), "an old save: the next leader (Erika) is ready at once");
  }

  if (bad) printf("FAILURES\n");
  return bad?1:0;
}
