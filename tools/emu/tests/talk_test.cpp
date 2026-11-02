// Phase 29: what the creature says -- the groups, what comes first, no repeats,
// and every line present in every language.
#include "Arduino.h"
#include "Preferences.h"
#include "pet.h"
#include "talk.h"
#include "talk_groups.h"
#include "weather.h"
#include "i18n.h"
#include <cstdio>
#include <cstring>
uint32_t g_seed=29; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
volatile int g_touchX=0,g_touchY=0; volatile bool g_touchDown=false; bool wasPressed=false;
uint32_t millis(){return 0;} void FakeESP::restart(){exit(0);}
int FakeSerial::available(){return 0;} String FakeSerial::readStringUntil(char){return String("");}
void sfxPlay(uint8_t){}
static int bad=0;
static void ck(bool ok,const char*w){printf("%s  %s\n",ok?"PASS":"FAIL",w); if(!ok)bad++;}
static uint32_t rnd(uint32_t n){ return n ? (uint32_t)random(n) : 0; }

int main(){
  // every group has lines, and every line is written in all eight languages
  {
    int total = 0, empty = 0, groups = 0;
    for (uint8_t g = 0; g < TG_COUNT; g++) {
      if (talkGroupSize(g)) groups++;
      for (uint8_t i = 0; i < talkGroupSize(g); i++, total++)
        for (uint8_t l = 0; l < LANG_COUNT; l++) {
          setLang((Lang)l);
          const char *s = T(talkGroupLine(g, i));
          if (!s || !*s) empty++;
        }
    }
    setLang(LANG_EN);
    printf("      %d lines in %d groups\n", total, groups);
    ck(groups == TG_COUNT, "every group has lines");
    ck(total >= 130, "about 130 lines or more, not 13");
    ck(empty == 0, "and none is missing in any language");
  }

  // what it needs comes first
  {
    TalkCtx c;
    c.hungry = true; c.wx = WX_RAIN; c.garden = true;
    talkForget();
    ck(talkGroupOf(talkPick(c, rnd)) == TG_HUNGRY, "hungry beats the rain and the garden");
    c.sick = true;
    ck(talkGroupOf(talkPick(c, rnd)) == TG_SICK, "sick beats hungry");
    TalkCtx l; l.lonely = true; l.hungry = true;
    ck(talkGroupOf(talkPick(l, rnd)) == TG_LONELY, "lonely beats hungry");
  }

  // otherwise it fits the moment, and never repeats one of the last six
  {
    TalkCtx c;
    c.wx = WX_STORM; c.hour = 21; c.event = EVENT_HALLOWEEN;
    talkForget();
    StrId last[TALK_RECENT] = {};
    int at = 0, repeats = 0, offTopic = 0, storm = 0, halloween = 0;
    for (int i = 0; i < 60; i++) {
      StrId s = talkPick(c, rnd);
      for (int k = 0; k < TALK_RECENT; k++) if (last[k] == s && i >= 1) repeats++;
      last[at] = s; at = (at + 1) % TALK_RECENT;
      uint8_t g = talkGroupOf(s);
      if (g == TG_STORM) storm++;
      if (g == TG_HALLOWEEN) halloween++;
      if (g != TG_STORM && g != TG_EVENING && g != TG_HALLOWEEN && g != TG_HAPPY && g != TG_LOVE &&
          g != TG_ARCADE && g != TG_SUN) offTopic++;
    }
    ck(repeats == 0, "60 lines in a row, never one of the last six");
    ck(offTopic == 0, "every line fits the moment (a stormy Halloween evening)");
    ck(storm > 0 && halloween > 0, "the storm and Halloween both get mentioned");
  }

  // the weather it talks about is the one it shows
  {
    TalkCtx c; c.wx = WX_CLEAR; c.hour = 11;
    talkForget();
    bool rainTalk = false;
    for (int i = 0; i < 80; i++) { uint8_t g = talkGroupOf(talkPick(c, rnd)); if (g == TG_RAIN || g == TG_STORM || g == TG_SNOW) rainTalk = true; }
    ck(!rainTalk, "a sunny morning never talks about rain");
    TalkCtx n; n.wx = WX_CLEAR; n.hour = 23;
    talkForget();
    bool sunAtNight = false;
    for (int i = 0; i < 80; i++) if (talkGroupOf(talkPick(n, rnd)) == TG_SUN) sunAtNight = true;
    ck(!sunAtNight, "and a clear night does not talk about the sun");
  }

  if (bad) printf("FAILURES\n");
  return bad?1:0;
}
