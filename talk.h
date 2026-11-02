#pragma once
#include <stdint.h>
#include "i18n.h"

// What the creature says (phase 29). It was 13 lines picked in a fixed order, so
// the same few came back all day. Now about 150 lines in groups: anything urgent
// first (sick, lonely, hungry, dirty, sleepy, bored), otherwise a random group that
// fits the moment (the weather, the time of day, an event, something to go and
// do, plain chatter), and never one of the last few lines said.
//
// Pure: the sketch fills a TalkCtx from the pet and the clock, so the desktop
// tests can drive every case. A wish is still the sketch's own line, first.
struct TalkCtx {
  bool sick = false, lonely = false, hungry = false, dirty = false, sleepy = false, bored = false;
  uint8_t wx = 0;              // a WeatherKind (weather.h), or 0
  bool hasTemp = false;
  int8_t tempC = 0;
  uint8_t hour = 12;
  bool garden = false, mail = false, quest = false, nursery = false, arcade = false, study = false, job = false;
  bool birthday = false;
  uint8_t event = 0;           // an EventId (pet.h)
  bool bondHigh = false, evolve = false;
  uint8_t joy = 80;
};

#define TALK_RECENT 6          // none of the last this-many lines comes back

StrId talkPick(const TalkCtx &c, uint32_t (*rnd)(uint32_t));   // rnd(n): 0..n-1
uint8_t talkGroupOf(StrId s);   // which group a line belongs to (tests), 255 = none
uint8_t talkGroupSize(uint8_t g);
StrId talkGroupLine(uint8_t g, uint8_t i);
void talkForget();              // clears the recent lines (tests)
