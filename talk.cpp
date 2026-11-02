#include "talk.h"
#include "talk_groups.h"
#include "weather.h"
#include "pet.h"

static StrId gRecent[TALK_RECENT];
static uint8_t gRecentN = 0, gRecentAt = 0;

static bool isRecent(StrId s) {
  for (uint8_t i = 0; i < gRecentN; i++) if (gRecent[i] == s) return true;
  return false;
}

static bool groupFresh(uint8_t g) {             // still has a line not said lately
  for (uint8_t i = 0; i < TALK_SETS[g].n; i++) if (!isRecent(TALK_SETS[g].lines[i])) return true;
  return false;
}

static StrId fromGroup(uint8_t g, uint32_t (*rnd)(uint32_t)) {
  const TalkSet &t = TALK_SETS[g];
  StrId s = t.lines[rnd(t.n)];
  for (int tries = 0; tries < 12 && isRecent(s); tries++) s = t.lines[rnd(t.n)];
  if (isRecent(s))                                   // a small group: the first one not said lately
    for (uint8_t i = 0; i < t.n; i++) if (!isRecent(t.lines[i])) { s = t.lines[i]; break; }
  gRecent[gRecentAt] = s;
  gRecentAt = (gRecentAt + 1) % TALK_RECENT;
  if (gRecentN < TALK_RECENT) gRecentN++;
  return s;
}

StrId talkPick(const TalkCtx &c, uint32_t (*rnd)(uint32_t)) {
  // something it needs comes first, as it always did
  if (c.sick) return fromGroup(TG_SICK, rnd);
  if (c.lonely) return fromGroup(TG_LONELY, rnd);
  if (c.hungry) return fromGroup(TG_HUNGRY, rnd);
  if (c.dirty) return fromGroup(TG_DIRTY, rnd);
  if (c.sleepy) return fromGroup(TG_SLEEPY, rnd);
  if (c.bored) return fromGroup(TG_BORED, rnd);
  // otherwise whatever fits the moment; things to go and do count double
  uint8_t cand[32], n = 0;
  auto add = [&](uint8_t g, uint8_t w) { while (w-- && n < sizeof(cand)) cand[n++] = g; };
  switch (c.wx) {
    case WX_CLEAR: if (c.hour >= 7 && c.hour < 18) add(TG_SUN, 1); break;
    case WX_CLOUDY: add(TG_CLOUD, 1); break;
    case WX_FOG: add(TG_FOG, 1); break;
    case WX_RAIN: add(TG_RAIN, 1); break;
    case WX_STORM: add(TG_STORM, 2); break;
    case WX_SNOW: add(TG_SNOW, 2); break;
    case WX_SNOW_NY: add(TG_NYSNOW, 2); break;
  }
  if (c.hasTemp && c.tempC >= 28) add(TG_HOT, 1);
  if (c.hasTemp && c.tempC <= 8) add(TG_COLD, 1);
  add(c.hour >= 5 && c.hour < 12 ? TG_MORNING : c.hour < 18 && c.hour >= 12 ? TG_AFTERNOON
      : c.hour >= 18 && c.hour < 22 ? TG_EVENING : TG_NIGHT, 1);
  if (c.garden) add(TG_GARDEN, 2);
  if (c.mail) add(TG_MAIL, 2);
  if (c.quest) add(TG_QUEST, 1);
  if (c.nursery) add(TG_NURSERY, 2);
  if (c.arcade) add(TG_ARCADE, 1);
  if (c.study) add(TG_STUDY, 1);
  if (c.job) add(TG_JOB, 2);
  if (c.birthday) add(TG_BDAY, 2);
  if (c.event == EVENT_HALLOWEEN) add(TG_HALLOWEEN, 2);
  if (c.event == EVENT_WINTER) add(TG_WINTER, 2);
  if (c.event == EVENT_EASTER) add(TG_EASTER, 2);
  if (c.event == EVENT_SUMMER) add(TG_SUMMER, 2);
  if (c.bondHigh) add(TG_BOND, 1);
  if (c.evolve) add(TG_EVOLVE, 2);
  add(TG_HAPPY, 2);
  if (c.joy >= 50) add(TG_LOVE, 1);
  // a small group can be used up (the storm has three lines): then another that fits
  uint8_t g = cand[rnd(n)];
  for (int tries = 0; tries < 16 && !groupFresh(g); tries++) g = cand[rnd(n)];
  if (!groupFresh(g)) g = TG_HAPPY;                 // the biggest group, never used up
  return fromGroup(g, rnd);
}

uint8_t talkGroupOf(StrId s) {
  for (uint8_t g = 0; g < TG_COUNT; g++)
    for (uint8_t i = 0; i < TALK_SETS[g].n; i++) if (TALK_SETS[g].lines[i] == s) return g;
  return 255;
}
uint8_t talkGroupSize(uint8_t g) { return g < TG_COUNT ? TALK_SETS[g].n : 0; }
StrId talkGroupLine(uint8_t g, uint8_t i) { return TALK_SETS[g].lines[i]; }
void talkForget() { gRecentN = 0; gRecentAt = 0; }
