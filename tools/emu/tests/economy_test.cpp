// The coin economy, played rather than restated: thirty days of three kinds of
// player through the real Pet (missions, caps, steps and all), then how long
// the shop takes. It fails if a light player cannot feed their pet from the
// shop, or if a normal one could not reach the crown in about two weeks.
#include "Arduino.h"
#include "Preferences.h"
#include "pet.h"
#include "party.h"
#include <cstdio>
uint32_t g_seed=37; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
volatile int g_touchX=0,g_touchY=0; volatile bool g_touchDown=false; bool wasPressed=false;
static uint32_t g_ms=0;
uint32_t millis(){return g_ms;} void FakeESP::restart(){exit(0);}
int FakeSerial::available(){return 0;} String FakeSerial::readStringUntil(char){return String("");}
void sfxPlay(uint8_t){}
static int bad=0;
static void ck(bool ok,const char*w){printf("%s  %s\n",ok?"PASS":"FAIL",w); if(!ok)bad++;}

static const uint32_t DAY = 86400;
static uint32_t at(uint32_t d, int h){ return d * DAY + (uint32_t)h * 3600 + 90; }

struct Player { const char *name; int feeds, pets, baths, games, score, steps; bool bedtime, missions; };

// One day of play. Stats are topped up between actions: this measures coins,
// not whether the creature survives the player.
static uint32_t playDay(Pet &p, uint32_t d, const Player &pl) {
  uint32_t before = p.coins;
  // This budgets DAILY income. The walking prizes are 640 coins once in a lifetime
  // of walking, about 10 a day over a first month of normal play, and not part of it.
  p.walkPrize = WALK_PRIZES;
  p.lastSeenEpoch = at(d, 9);
  for (int i = 0; i < pl.feeds; i++) { p.fullness = 40; g_ms += 5000; p.feedBerry((uint8_t)(i % 3)); }
  for (int i = 0; i < pl.pets; i++) p.caress();
  for (int i = 0; i < pl.baths; i++) { p.poops = 1; p.clean(); }
  for (int i = 0; i < pl.games; i++) {
    p.energy = 100; p.fullness = 80;
    if (i % 4 == 0) p.trainStrength(pl.score * 3);        // the bag counts hits
    else if (i % 4 == 1) p.trainSpeed(pl.score);
    else p.applyMiniResult((uint8_t)(i % MINI_COUNT), (uint8_t)pl.score);
  }
  p.addSteps((uint32_t)pl.steps);
  // a normal player also goes where the day's missions point (the park, the lake, a book...):
  // their rewards are counted here, what the places themselves pay or cost is not
  if (pl.missions)
    for (uint8_t i = 0; i < MISSIONS_PER_DAY; i++) {
      uint8_t m = p.missionAt(i);
      if (m != MIS_STEPS && m != MIS_STEPS_BIG && !p.missionComplete(i)) p.noteMission(m, Pet::missionGoal(m));
    }
  if (pl.bedtime) { p.lastSeenEpoch = at(d, 21); p.toggleLight(); p.toggleLight(); }
  return p.coins - before;
}

int main(){
  const Player PLAYERS[3] = {
    // name          feeds pets baths games score steps  bed    missions
    { "light day",      3,   2,    0,    2,    8,  1000, false, false },
    { "normal day",     6,   5,    1,    5,   12,  3000, true,  true  },
    { "busy day",      10,  10,    2,   15,   25,  8000, true,  true  },
  };
  uint32_t avg[3];
  for (int k = 0; k < 3; k++) {
    Pet p;
    p.begin();
    p.dbgHatchAs(1, false);
    p.coins = 0; p.streak = 0; p.lastCareDay = 0; p.coinDay = 0;
    // Each player starts walking from nothing. They share one store, so the last
    // player's step day, weeks ahead of this one's first day, used to carry over and
    // make every step read as 0: step missions never paid and this budget read low.
    p.stepDay = 0; p.stepCount = 0; p.stepHistDay = 0; p.stepCoinK = 0;
    uint32_t total = 0, lo = 0xFFFFFFFF, hi = 0;
    for (uint32_t d = 20000; d < 20030; d++) {
      uint32_t c = playDay(p, d, PLAYERS[k]);
      total += c;
      if (c < lo) lo = c;
      if (c > hi) hi = c;
    }
    avg[k] = total / 30;
    printf("      %-10s  %3lu coins a day on average (%lu-%lu), %lu in 30 days\n",
           PLAYERS[k].name, (unsigned long)avg[k], (unsigned long)lo, (unsigned long)hi, (unsigned long)total);
  }

  // what things cost against that
  uint32_t hats = 0, toys = 0, decor = 0;
  for (int h = 0; h < HAT_COUNT; h++) hats += Pet::hatPrice(h);
  for (int d = 0; d < DEC_COUNT; d++) (Pet::isToy(d) ? toys : decor) += Pet::decorPrice(d);
  uint32_t all = hats + toys + decor + BRUSH_PRICE;
  uint32_t cheapFood = 255, dearFood = 0;
  for (int f = 0; f < FOOD_COUNT; f++) {
    uint8_t pr = Pet::foodInfo(f).price;
    if (pr < cheapFood) cheapFood = pr;
    if (pr > dearFood) dearFood = pr;
  }
  printf("      prices: food %lu-%lu, cheapest hat %u, crown %u, everything once %lu\n",
         (unsigned long)cheapFood, (unsigned long)dearFood, Pet::hatPrice(HAT_CAP), Pet::hatPrice(HAT_CROWN), (unsigned long)all);
  // a normal player who also buys two treats a day (about 20 coins)
  uint32_t save = avg[1] > 20 ? avg[1] - 20 : 0;
  printf("      normal day, spending ~20 on food: saves %lu a day -> cap in %.1f days, crown in %.1f, everything in %.0f\n",
         (unsigned long)save, save ? 80.0 / save : 0.0, save ? 500.0 / save : 0.0, save ? (double)all / save : 0.0);

  ck(avg[0] >= 2 * cheapFood, "a light day still buys a couple of servings of food");
  ck(avg[0] >= 20, "a light day earns at least 20");
  ck(avg[1] >= 50 && avg[1] <= 100, "a normal day earns 50-100");
  ck(avg[2] <= 130, "even a very busy day stays under ~130");
  ck(save && 80 / save <= 3, "a normal player saving for a cap gets it within 3 days");
  ck(save && 500 / save <= 15, "and the crown within about two weeks");
  ck(avg[1] * 60 >= all, "everything in the shop is reachable in about two months of normal play");
  if (bad) printf("FAILURES\n");
  return bad?1:0;
}
