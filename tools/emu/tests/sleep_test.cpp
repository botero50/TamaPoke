// Putting the device down must not cost you the creature.
//
// A player left the board running overnight with the pet awake and came back to
// a Dratini that had run away. That was reachable because the LIVE tick is the
// only drain path with no floor: offline floors at 15, asleep floors at 30/35/45
// and skips the neglect check entirely, but a board left RUNNING took an awake
// creature to zero on all four stats in 100 minutes and to the point of running
// away in 160.
//
// Auto-sleep needs BOTH the screen off and the night window (00:00-06:00).
// Screen-off alone paused the game whenever the device was put down, and the
// creature is meant to get hungry during the day. The hour alone sent it to bed
// while somebody was still playing with it.
#include "Arduino.h"
#include "Preferences.h"
#include "pet.h"
#include "party.h"
#include <cstdio>
uint32_t g_seed=23; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
volatile int g_touchX=0,g_touchY=0; volatile bool g_touchDown=false; bool wasPressed=false;
uint32_t millis(){return 0;} void FakeESP::restart(){exit(0);}
int FakeSerial::available(){return 0;} String FakeSerial::readStringUntil(char){return String("");}
void sfxPlay(uint8_t){}
static int bad=0;
static void ck(bool ok,const char*w){printf("%s  %s\n",ok?"PASS":"FAIL",w); if(!ok)bad++;}

// an epoch whose hour is exactly h
static uint32_t atHour(int h){ return (uint32_t)h * 3600 + 90; }

// Every case starts awake and unowned. The suite shares one NVS store, so a Pet
// built after another one slept would LOAD that sleep -- which is correct
// behaviour and a wrong starting point for the next test.
static void fresh(Pet &p, int hour){
  p.begin();
  p.dbgHatchAs(147,false);
  p.sleeping = false;
  p.sleepAuto = SLEEP_NONE;
  p.fullness = p.joy = p.energy = p.hygiene = 100;
  p.lastSeenEpoch = atHour(hour);
}

// run n minutes of live ticks, moving the clock with them
static void liveMinutes(Pet &p, int startHour, int n){
  for (int i = 0; i < n; i++) {
    p.lastSeenEpoch = atHour(startHour) + (uint32_t)i * 60;
    p.dbgTick();
  }
}

int main(){
  // --- THE DAY: putting it down does NOT pause the game
  {
    Pet p; fresh(p, 13);
    p.setScreenOff(true);
    ck(!p.sleeping, "screen off in the afternoon does not put it to sleep");
    liveMinutes(p, 13, 120);
    printf("      two hours face-down at 13:00: food=%u joy=%u hyg=%u\n",
           p.fullness, p.joy, p.hygiene);
    ck(p.fullness == 0, "it still gets hungry with the screen off during the day");
  }

  // --- THE NIGHT: screen off after midnight and it sleeps
  {
    Pet p; fresh(p, 1);
    p.setScreenOff(true);
    ck(p.sleeping, "screen off at 01:00 puts it to sleep");
  }

  // --- put down BEFORE bedtime: it nods off when the hour comes (21:00, phase 24)
  {
    Pet p; fresh(p, 20);
    p.setScreenOff(true);
    ck(!p.sleeping, "put down at 20:00, still awake -- the evening is yours");
    liveMinutes(p, 20, 70);              // through 21:00
    ck(p.sleeping, "and it goes to bed by itself once 21:00 arrives");
  }

  // --- the afternoon and early evening still drain, which is the point of a window
  {
    Pet p; fresh(p, 18);
    p.setScreenOff(true);
    liveMinutes(p, 18, 120);             // 18:00 -> 20:00, screen off
    ck(!p.sleeping, "screen off at 18:00 does not sleep it");
    ck(p.fullness == 0, "and the early evening drains like any other hour");
  }

  // --- screen off late in the evening: straight to bed
  {
    Pet p; fresh(p, 21);
    p.lastSeenEpoch = atHour(21) + 30 * 60;
    p.setScreenOff(true);
    ck(p.sleeping, "screen off at 21:30 puts it to sleep");
  }

  // --- THE ONE THAT MATTERS: the whole night costs nothing (phase 24: the bars hold)
  {
    Pet p; fresh(p, 0);
    p.fullness = 80; p.joy = 70; p.hygiene = 65;
    p.setScreenOff(true);
    liveMinutes(p, 0, 6 * 60 - 5);       // 00:00 -> 05:55
    printf("      at 05:55: food=%u joy=%u ene=%u hyg=%u ready=%d\n",
           p.fullness, p.joy, p.energy, p.hygiene, (int)p.canRunawayNow());
    ck(p.joy == 70 && p.hygiene == 65, "a night asleep: joy and hygiene hold exactly");
    ck(p.fullness >= REST_FOOD_FLOOR && p.fullness < 80, "food only creeps down, never below the rest floor");
    ck(p.energy > 50, "and it wakes rested");
    ck(!p.canRunawayNow(), "NOWHERE NEAR running away");
    ck(p.speciesId == 147, "the creature is still there");

    // AND IT DOES NOT GET UP BY ITSELF. Waking at 06:00 would reopen the hole:
    // from the floors, food empties by 06:15 and everything by 07:40, so anyone
    // who sleeps past eight would find it ready to run away all over again.
    liveMinutes(p, 6, 3 * 60);           // through to 09:00, screen still off
    ck(p.sleeping, "still asleep at 09:00 -- you might be too");
    ck(!p.canRunawayNow(), "and STILL nowhere near running away");
    printf("      at 09:00 unattended: food=%u joy=%u ene=%u hyg=%u\n",
           p.fullness, p.joy, p.energy, p.hygiene);

    // it wakes when YOU do
    p.setScreenOff(false);
    ck(!p.sleeping, "turning the screen back on is what wakes it");
  }

  // --- a daytime nap still drains (slowly): only a night is a rest
  {
    Pet p; fresh(p, 13);
    p.toggleLight();
    liveMinutes(p, 13, 60);
    ck(p.sleeping && p.joy < 100 && p.fullness < 100, "a nap at 13:00 still drains a little");
    ck(p.joy >= 35 && p.fullness >= 30, "but never below the nap floors");
  }

  // --- switched off for the night while awake: it went to bed by itself
  {
    Pet p; fresh(p, 20);
    p.fullness = 80; p.joy = 70; p.hygiene = 65;
    p.dbgSetSeen(atHour(21) + 10 * 60);              // unplugged at 21:10, awake
    p.syncClock(86400 + atHour(7) + 30 * 60);        // plugged in at 07:30 next morning
    printf("      unplugged overnight: food=%u joy=%u hyg=%u asleep=%d\n", p.fullness, p.joy, p.hygiene, (int)p.sleeping);
    ck(p.sleeping, "it slept through the night while switched off");
    ck(p.joy == 70 && p.hygiene == 65 && p.fullness >= REST_FOOD_FLOOR, "and wakes in the same shape");
  }

  // --- switched off for an afternoon: the day still counts
  {
    Pet p; fresh(p, 12);
    p.dbgSetSeen(atHour(12));
    p.syncClock(atHour(16));
    ck(!p.sleeping && p.fullness == 15, "an afternoon off still drains to the offline floor");
  }

  // --- the light button still beats both
  {
    Pet p; fresh(p, 13);
    p.toggleLight();
    ck(p.sleeping, "the light puts it to bed by hand, in daylight");
    p.setScreenOff(false);
    ck(p.sleeping, "and neither the screen...");
    liveMinutes(p, 13, 30);
    ck(p.sleeping, "...nor the clock overrules that");
    p.toggleLight();
    ck(!p.sleeping, "only the light does");
  }

  // --- waking it at night keeps it awake rather than fighting the rule
  {
    Pet p; fresh(p, 1);
    p.setScreenOff(true);
    ck(p.sleeping, "asleep at 01:00");
    p.toggleLight();                     // the player wants to play
    ck(!p.sleeping, "the light wakes it");
    liveMinutes(p, 1, 30);
    ck(!p.sleeping, "and it stays awake rather than nodding straight off again");
  }

  // --- total neglect while AWAKE still ends badly: the teeth are still there
  {
    Pet p; fresh(p, 9);
    liveMinutes(p, 9, 170);
    ck(!p.sleeping, "ignored all day, still awake");
    ck(p.canRunawayNow(), "and genuine neglect still makes it ready to leave");
  }

  // --- a board whose clock was never set never auto-sleeps, and says so by
  //     still draining rather than by freezing
  {
    Pet p; fresh(p, 12);
    p.lastSeenEpoch = 0;
    p.setScreenOff(true);
    ck(!p.sleeping, "no clock, no bedtime");
  }

  // --- THE MORNING AFTER. A creature that went to bed already at zero keeps a
  //     neglectTicks of 60 all night, because the sleeping branch of tick()
  //     returns before the neglect block and so neither counts nor clears it.
  //     canRunawayNow() used to read that counter alone, so waking the screen
  //     made a creature with 100 energy instantly ready to leave -- and the
  //     runaway button is drawn over the creature, where the tap that says good
  //     morning lands. It cost a real player a DRAGONAIR.
  {
    Pet p; fresh(p, 19);
    p.fullness = p.joy = p.energy = p.hygiene = 0;   // a neglected evening
    liveMinutes(p, 19, 60);
    ck(p.canRunawayNow(), "armed by an hour of total neglect before bed");

    p.setScreenOff(true);                            // 20:00, not the window yet
    ck(!p.sleeping, "screen off before 21:00 does not sleep it yet");

    for (int i = 0; i < 720; i++) {                  // through to 08:00
      p.lastSeenEpoch = atHour(20) + (uint32_t)i * 60;
      p.dbgTick();
    }
    ck(p.sleeping, "it slept from 21:00 through the night");
    ck(p.energy == 100, "and energy recovered while it slept");
    ck(!p.canRunawayNow(), "asleep it cannot leave -- the night itself was safe");

    p.setScreenOff(false);                           // you pick the device up
    ck(!p.canRunawayNow(),
       "and waking it does NOT hand you a runaway: 100 energy is not neglect");
  }

  // --- but the counter is not the only thing that matters: a creature actually
  //     at zero on waking is still ready, or the ending would have no teeth
  {
    Pet p; fresh(p, 22);
    p.fullness = p.joy = p.energy = p.hygiene = 0;
    liveMinutes(p, 22, 60);
    p.dbgSetSeen(atHour(9));                         // morning, awake, still empty
    p.fullness = p.joy = p.energy = p.hygiene = 0;
    ck(p.canRunawayNow(), "genuinely empty on waking is still ready to leave");
  }

  // --- an egg is not something you put to sleep
  {
    Pet p; p.begin(); p.newEgg();
    p.setScreenOff(true);
    ck(!p.sleeping, "an egg does not sleep");
  }

  printf("%s\n", bad?"FAILURES":"all good");
  return bad?1:0;
}
