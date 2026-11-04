#pragma once
#include <stdint.h>

// Timers hold a millis() deadline, with 0 meaning "off". millis() wraps after 49.7
// days and a ball with no battery can stay on that long, so a plain "now < until"
// goes wrong there. These subtract instead, which holds across the wrap.
inline bool timeOn(uint32_t now, uint32_t until) { return until && (int32_t)(until - now) > 0; }
inline bool timeUp(uint32_t now, uint32_t until) { return !timeOn(now, until); }
// A timer that ended long ago and was never cleared would read as running again
// once it is more than 24.8 days old, the limit of that subtraction. So anything
// over a day past its end goes back to 0, which leaves any flow still waiting on a
// finished timer plenty of time to see it first.
inline void sweepTimer(uint32_t &t, uint32_t now) { if (t && (int32_t)(t - now) < -86400000L) t = 0; }
#include <Arduino.h>
#include <Preferences.h>
#include "dex.h"
#include "trainers.h"   // GYM_REGIONS sizes the badge masks
#include "party.h"

// 1 tick = 1 game minute. Lower this value to test faster
// (e.g. 5000UL = stats fall 12x faster).
#define PET_TICK_MS 60000UL
// Game minutes per level. With 60, CHARMANDER evolves at ~16 h
// of play with perfect care. Drop to 1 to see evolutions instantly.
#define MINUTES_PER_LEVEL 60
#define MAX_LEVEL 100              // reached at 4d 3h; see level()
#define EAT_ANIM_MS 2500UL
#define HEART_MS 1500UL
#define EVOLVE_ANIM_MS 5200UL              // evolution animation (longer = more epic)
#define CEREMONY_MS 10000UL                // duration of the farewell on screen
#define FAREWELL_AGE_MIN (3UL * 24 * 60)   // says goodbye after 3 days of play (in final form)
// Retiring a creature BEFORE it has earned its farewell costs the NEXT one a
// day's worth of evolution. Derived from MINUTES_PER_LEVEL rather than written
// as 24, so it stays "a day" if the level rate is ever retuned.
#define EVO_PENALTY_LEVELS ((uint8_t)((24UL * 60) / MINUTES_PER_LEVEL))
#define RUNAWAY_TICKS 60                   // runs away after 1 h with EVERYTHING at zero
// Night, by the RTC: 21:00 to 06:00 (was midnight: an evening with the screen
// off drained it empty before it ever slept). Auto-sleep needs BOTH: the screen
// off AND this window.
// The screen alone would pause the game every time you put the device in a
// pocket -- the creature is supposed to get hungry during the day. The hour
// alone would send it to bed while you were playing. Set the clock in SETTINGS
// or over the console with RTCSET; an unset board reads months out and simply
// never auto-sleeps, which fails in the safe direction (it still drains, and
// the light button still works by hand).
#define NIGHT_START 21
#define NIGHT_END 6
// A night's rest (phase 24): asleep between these hours, joy and hygiene hold and
// food only creeps down to a floor -- a child cannot care for it while it sleeps,
// so waking up to low bars every morning was a punishment for going to bed.
// Daytime naps keep the slower-than-awake drain.
#define REST_START 20
#define REST_END 8
#define REST_FOOD_FLOOR 60
#define REST_FOOD_EVERY 10      // minutes per point of food while resting
enum : uint8_t { SLEEP_NONE = 0, SLEEP_AUTO, SLEEP_PLAYER };
#define DEF_TRAIN_TICKS 60                 // minutes of wellbeing per +1 DEF

// end-of-cycle ceremonies
enum : uint8_t { CER_NONE = 0, CER_FAREWELL, CER_RUNAWAY, CER_RELEASE };

// Fun Pack: the eight extra minigames (index into miniHi and the reward table)
enum MiniGameId : uint8_t {
  MINI_LR = 0,   // left or right
  MINI_HL,       // higher or lower
  MINI_WHO,      // who's that pokemon
  MINI_DROP,     // berry drop
  MINI_MOLE,     // mole pop
  MINI_ROPE,     // jump rope
  MINI_DANCE,    // dance
  MINI_HIDE,     // hide and seek
  MINI_MEMORY,   // find the pairs (phase 12)
  MINI_SIMON,    // repeat the sequence
  MINI_FLAPPY,   // fly through the gaps
  MINI_MAZE,     // tilt maze
  MINI_BALLOON,  // balloon pop (phase 15)
  MINI_SHAKE,    // shake race
  MINI_CATCH,    // catch the falling Pokemon
  MINI_BALANCE,  // keep the berry on the seesaw
  MINI_QUIZ,     // the type quiz
  MINI_SLIDE,    // slide puzzle
  MINI_BUBBLES,  // pop groups of one colour
  MINI_DRAW,     // finger painting
  MINI_COUNT
};
#define MINI_NO_STAT (-1)
enum : int8_t { MINI_STAT_ATK = 0, MINI_STAT_DEF, MINI_STAT_SPE };

// Fun Pack: sickness and medicine. PILL_* rather than MED_*, which are medals.
enum Sickness : uint8_t { SICK_NONE = 0, SICK_STOMACH, SICK_COLD };
enum PillResult : uint8_t {
  PILL_CURED = 0,   // cured
  PILL_MORE,        // a cold needs another dose
  PILL_WAIT,        // too soon for the next dose
  PILL_NEED_BATH,   // doses done, a bath finishes the cure
  PILL_NOT_SICK,    // it was healthy: tastes awful
};
#define OVERFEED_SICK 3          // binges until a stomachache
#define DIRTY_SICK_TICKS 45      // minutes dirty (3 poops or HYG<15) until a cold
#define SICK_MISTAKE_TICKS 360   // every 6 h left sick = 1 care mistake
#define COLD_DOSE_GAP_MIN 10     // minutes between the two cold doses
#define BORED_TICKS 480          // 8 h awake without a minigame
#define DOZE_IDLE_MS (20UL * 60 * 1000)  // untouched time in the evening until it dozes off
#define LIGHT_GRUMPY_TICKS 30    // minutes asleep with the light on until grumpy
// Late evening by the RTC, for dozing off with the light on: 21:00 to 06:00.
// Wider than NIGHT_START/END on purpose -- this is about a Pokemon left alone
// with the screen on, not about putting the device in a pocket.
#define DOZE_START 21
#define DOZE_END 6

// Coins: earned by caring, playing and walking, spent in the shop (phase 2).
// Activity coins (games, care) stop at a daily cap so they cannot be farmed;
// steps, missions, the daily bonus and badges are outside it.
#define COIN_DAILY_CAP 50        // coins a day from games and care actions
#define COIN_MAX 99999UL
#define COIN_DAILY_BONUS 5       // first care of the day
#define COIN_STREAK7_BONUS 15    // every 7th day of the streak
#define COIN_PER_1000_STEPS 2
#define COIN_STEP_CAP_K 10       // steps past 10 000 a day earn nothing more
#define COIN_BADGE 15            // a badge won for the first time
#define COIN_BEDTIME 3           // sent to bed with the light off in the evening
#define COIN_MISSION 6
#define COIN_ALL_MISSIONS 12     // extra, for finishing all six
// A good day is about 60-80 coins (at most ~110): a cap or bow in a day or two,
// the crown in about a week.
#define MISSIONS_PER_DAY 6
#define MISSION_GROUPS 5         // care, play, outdoors, home, shop: one of each a day, then one more
#define COIN_GAME_MAX 5          // a game pays 1 + score/10, at most this

// Hats, bought in the shop with coins and worn on the main screen. Append new
// ones at the END: ownership is a bit per hat in NVS.
enum HatId : uint8_t {
  HAT_CAP = 0, HAT_BOW, HAT_FLOWER, HAT_BEANIE, HAT_PARTY,
  HAT_STRAW, HAT_WITCH, HAT_SANTA, HAT_TOPHAT, HAT_CROWN,
  HAT_CHEF, HAT_PIRATE, HAT_CATEARS, HAT_BUNNY, HAT_HEADPHONES,   // phase 13
  HAT_GRAD, HAT_WREATH, HAT_HELMET, HAT_BERET, HAT_VIKING,
  HAT_ARCADE, HAT_VISOR,                                            // arcade prizes (phase 26)
  HAT_COUNT
};
#define HAT_SHOP_COUNT HAT_ARCADE   // the prizes are won with tickets, never sold
static_assert(HAT_COUNT <= 32, "hatsOwned needs a bit per hat");
#define HAT_NUDGE_MAX 60     // how far the player may move a hat, px each way

// Toys and decorations, bought in the shop and set down on the main screen's
// floor, one each side of the creature. The first DEC_TOY_COUNT are toys: a
// toy out keeps it company (boredom builds half as fast) and it goes over to
// play with it. Append new ones at the END: ownership is a bit each in NVS.
enum DecorId : uint8_t {
  DEC_BALL = 0, DEC_PLUSH, DEC_DUCK, DEC_BLOCKS, DEC_DRUM, DEC_KITE,
  DEC_PLANT, DEC_FLOWERS, DEC_LAMP, DEC_BED, DEC_GIFT, DEC_PUMPKIN,
  DEC_CABINET,                    // an arcade prize (phase 26)
  DEC_COUNT
};
#define DEC_SHOP_COUNT DEC_CABINET
#define DEC_TOY_COUNT 6

// Foods bought in the shop and kept in the bag, a count each (the berries and
// the candy stay free and unlimited). Grouped for the food menu: meals, then
// sweets, then drinks. Append new ones at the END: the counts are one byte
// each in NVS, in this order.
enum FoodId : uint8_t {
  FOOD_SANDWICH = 0, FOOD_CURRY, FOOD_SOUP, FOOD_ONIGIRI,   // meals
  FOOD_CAKE, FOOD_COOKIE, FOOD_ICECREAM,                     // sweets
  FOOD_MILK, FOOD_JUICE,                                     // drinks
  FOOD_PIE, FOOD_STEW, FOOD_PARFAIT,                         // cooked in the kitchen, not sold
  FOOD_COUNT
};
#define FOOD_SHOP_COUNT 9        // the shop sells the first nine; the rest come from the kitchen
#define FOOD_DRINKS 2            // the food menu lists meals and sweets with what the kitchen makes
#define FOOD_STACK_MAX 99
// Bath and sleep extras
#define SOAP_PRICE 10            // one bubble bath
#define WASH_SHOWER 35           // the quick shower adds this much CLEAN (the bubble bath fills it)
#define WASH_SCRUB 65            // the sponge game
#define SWEEP_PER_PILE 10        // the broom: CLEAN for each pile swept
#define POOP_DRAIN_PER_PILE 2    // CLEAN each pile on the floor takes per tick
#define BRUSH_PRICE 60           // bought once
#define NAP_MINUTES 30
#define SHINE_MS 60000UL         // sparkles after brushing

// Places (the map): the park, the store, the Pokemon Center and the garden.
#define PARK_FINDS_PER_DAY 3     // coins found in the park, at most this often a day
#define PARK_WALK_ENERGY 5
#define CENTER_PRICE 20          // a second visit the same day
#define SEED_PRICE 5
#define GARDEN_GROW_SECS (8UL * 3600)   // a seed is ready to harvest after 8 h
enum ParkResult : uint8_t { PARK_TIRED = 0, PARK_COINS, PARK_BERRY, PARK_FRIEND, PARK_NOTHING };
enum CenterResult : uint8_t { CENTER_FREE = 0, CENTER_PAID, CENTER_NO_COINS };
// Seasonal gifts, by the RTC date. GIFT_NONE when today is no holiday or the
// gift was already handed out today.
enum GiftKind : uint8_t { GIFT_NONE = 0, GIFT_HALLOWEEN, GIFT_CHRISTMAS, GIFT_NEWYEAR, GIFT_VALENTINE };

// The capsule machine in the store: a surprise for coins. What comes out is
// worth about the price on average (see coins_test), with a small chance of
// a hat or decoration not owned yet. A few a day, so it cannot eat the purse.
#define CAPSULE_PRICE 30
#define TICKETS_PER_DAY 30          // the arcade pays tickets, at most this many a day
#define TICKET_GAME_MAX 5
#define TICKET_RECORD_BONUS 3
enum PrizeId : uint8_t { PRIZE_CAP = 0, PRIZE_VISOR, PRIZE_CABINET, PRIZE_CAPSULE, PRIZE_CAKE, PRIZE_COUNT };
enum PrizeResult : uint8_t { PRZ_OK = 0, PRZ_NO_TICKETS, PRZ_OWNED, PRZ_FULL };
#define CAPSULES_PER_DAY 3
enum CapsuleResult : uint8_t { CAP_NO_COINS = 0, CAP_LIMIT, CAP_HAT, CAP_DECOR, CAP_FOOD, CAP_SOAP, CAP_SEED, CAP_COINS };
// Scenery: the main screen's backdrop. The creature's own biome is free; the
// others are bought once and picked from the bag. A bit per biome (0..6).
#define SCENERY_COUNT 8
#define SCENERY_ROOM 7           // not a biome: the creature's own room, always owned
// The room's furniture: wall pieces hang in two spots, floor pieces stand in two
// spots at the back, the rug lies under the creature, wallpapers recolour the wall.
// Append at the END: a bit each in NVS.
enum FurnId : uint8_t {
  FURN_WINDOW = 0, FURN_CLOCK, FURN_PAINTING, FURN_SHELF,          // wall
  FURN_SOFA, FURN_TV, FURN_AQUARIUM, FURN_BOOKCASE, FURN_FIREPLACE, // floor
  FURN_RUG,
  FURN_PAPER_BLUE, FURN_PAPER_PINK, FURN_PAPER_MINT,                 // wallpapers
  FURN_COUNT
};
#define FURN_WALL_LAST FURN_SHELF
#define FURN_FLOOR_LAST FURN_FIREPLACE
static_assert(FURN_COUNT <= 16, "furnOwned needs a bit per piece");

// Accessories (phase 16), worn with a hat: one on the face, one at the neck, one
// on the back. Append within a slot's run only by moving the runs: the slot is
// the id's group of four. A bit each in NVS.
enum AccId : uint8_t {
  ACC_SUNGLASSES = 0, ACC_STARGLASSES, ACC_HEARTGLASSES, ACC_MONOCLE,   // face
  ACC_SCARF, ACC_BOWTIE, ACC_MEDAL, ACC_BANDANA,                         // neck
  ACC_CAPE, ACC_BACKPACK, ACC_WINGS, ACC_BATWINGS,                       // back
  ACC_COUNT
};
enum AccSlot : uint8_t { ACC_FACE = 0, ACC_NECK, ACC_BACK };
#define OUTFIT_SETS 5
struct OutfitSet { uint8_t hat, a, b; };   // 255 = that part is not needed

// Town life (phase 17).
#define SHELLS_PER_DAY 5
#define BOOKS_PER_DAY 3
#define SONGS_PER_DAY 3
#define JOB_SECS (3UL * 3600)            // a shift
#define JOB_COINS 15
#define LETTER_KINDS 6
// The Nursery (phase 21): two banked creatures spend a day together and leave
// an egg. It waits for the NEXT raising -- the live pet is never replaced.
#define NURSERY_PRICE 40
#define NURSERY_SECS (24UL * 3600)       // a full day together
// Copied from the party/box at drop-off, so releasing or moving a parent later
// changes nothing: the visit already happened. Appended-only, saved as one blob.
struct NurseryState {
  int16_t aDex, bDex;                   // the parents, 0 = nobody there
  uint8_t aIv[4], bIv[4];               // atk, def, spe, hp
  uint8_t aShiny, bShiny;
  uint32_t since;                       // epoch they were dropped off
  uint32_t week;                        // today()/7 of the last drop-off
  int16_t eggDex;                       // the egg they left, 0 = none
  uint8_t eggIv[4];
  uint8_t eggShiny;
  int16_t eggA, eggB;                   // whose egg it is (for the faces on it)
};
enum NurseryResult : uint8_t { NUR_OK = 0, NUR_BAD_PAIR, NUR_BUSY, NUR_TOO_SOON, NUR_NO_COINS };
enum JobId : uint8_t { JOB_NONE = 0, JOB_CAFE, JOB_FLOWERS, JOB_POST, JOB_KINDS };
enum Ribbon : uint8_t { RIB_NONE = 0, RIB_BRONZE, RIB_SILVER, RIB_GOLD };

// Wishes: now and then it asks for something; doing it grants the wish.
enum WishId : uint8_t { WISH_NONE = 0, WISH_FOOD, WISH_PLAY, WISH_PARK, WISH_BATH, WISH_FISH, WISH_PET, WISH_KINDS };
#define WISHES_PER_DAY 3
#define WISH_LASTS_SECS (2UL * 3600)
#define WISH_GAP_SECS (3UL * 3600)       // at least this long between wishes
#define WISH_COINS 5
// A personality, from the species and the IVs: the same creature always has the same one.
enum Personality : uint8_t { PERS_PLAYFUL = 0, PERS_LAZY, PERS_HUNGRY, PERS_SHY, PERS_COUNT };
#define LONELY_SECS (3UL * 3600)          // awake and nobody cared for it this long
#define BIRTHDAY_DAYS 30                  // a party every month of its life
// Seasonal events by the date (besides the one-day holiday gifts).
enum EventId : uint8_t { EVENT_NONE = 0, EVENT_EASTER, EVENT_SUMMER, EVENT_HALLOWEEN, EVENT_WINTER };
// Story quests (phase 19, 12 stories since phase 31): one a week, four steps,
// each on a later day than the last.
enum QuestKind : uint8_t { QK_PARK = 0, QK_BEACH, QK_READ, QK_FISH, QK_COOK, QK_MEAL, QK_ADV, QK_CENTER, QK_BATH, QK_SONG, QK_GAME, QK_STUDY,
                           QK_GARDEN, QK_CHORE, QK_NAP, QK_TEETH, QK_QUIZ, QK_CAPSULE, QK_SHOP, QK_TICKETS, QK_STEPS, QK_FAVBERRY, QK_COUNT };
#define QUEST_COUNT 12
#define QUEST_STEPS 4
#define NEED_BEEP_AT 15         // a need this low starts the warning beep
#define NEED_BEEP_MAX 6
#define NEED_BEEP_MS 300000UL
#define WALK_PRIZES 8
#define WALK_RANKS (WALK_PRIZES + 1)
#define STEP_GOALS 3
#define QUEST_WALK_STEPS 3000    // the "long walk" step
#define GYM_STEPS_BASE 10000     // phase 36: steps to the first leader ...
#define GYM_STEPS_PER 2000       // ... and this much more for each one after
#define GYM_WEEK_DAYS 7          // at most one new badge a week
#define QUEST_COINS 30
#define EASTER_EGGS_PER_DAY 5
// Trophies: lifetime goals, each pays TROPHY_COINS once. Append at the END:
// a bit each in NVS. The tallies they count are a byte blob, also append-only.
enum TallyId : uint8_t {
  TALLY_MEALS = 0, TALLY_GAMES, TALLY_BATHS, TALLY_WALKS, TALLY_HARVEST,
  TALLY_STUDY, TALLY_TEETH, TALLY_PERFECT, TALLY_CAPSULE,
  TALLY_FISH, TALLY_COOK, TALLY_ROUTE, TALLY_CHORES,
  TALLY_COUNT
};
enum TrophyId : uint8_t {
  TRO_MEALS = 0, TRO_GAMES, TRO_BATHS, TRO_WALKS, TRO_HARVEST, TRO_COINS, TRO_HATS,
  TRO_DECOR, TRO_STREAK, TRO_LEVEL, TRO_STUDY, TRO_TEETH, TRO_PERFECT, TRO_CAPSULE,
  TRO_FISH, TRO_ROUTE,
  TRO_GOLD, TRO_PRIZES,                      // the arcade (phase 27)
  TRO_COUNT
};
static_assert(TRO_COUNT <= 32, "trophies needs a bit each");
#define TROPHY_COINS 10
// Brushing teeth (bath menu), homework time and the math quiz (the school).
#define TEETH_PER_DAY 2
#define STUDY_PER_DAY 3          // homework sessions that pay, a day
#define QUIZ_QUESTIONS 10
// Parent limits: a bedtime after which games are refused, and a games-a-day limit.
#define PARENT_BED_MIN 19
#define PARENT_BED_MAX 22
enum ParentBlock : uint8_t { PB_NONE = 0, PB_BEDTIME, PB_LIMIT };

// Step adventures: walking with the ball moves it along a route of stops.
// Each stop holds something; the last one finishes the route and opens the next.
#define ROUTE_COUNT 4
#define ROUTE_STOPS 5
enum AdvResult : uint8_t { ADV_NONE = 0, ADV_COINS, ADV_FOOD, ADV_SEEDS, ADV_FRIEND, ADV_DONE };
// Fishing at the lake: a few catches a day pay; after that it is just for fun.
#define FISH_PER_DAY 5
enum FishResult : uint8_t { FISH_KARP = 0, FISH_GOLDEEN, FISH_BOOT, FISH_SNACK, FISH_SHELL, FISH_PEARL };
// The kitchen: two foods from the bag make one better dish.
#define RECIPE_COUNT 3
struct Recipe { uint8_t a, b, out; };
// Family: chores the parents tick off, a piggy bank that pays a little interest,
// an alarm clock, and a diary of the last week.
#define CHORE_COUNT 6
#define CHORE_COINS 4
#define PIGGY_RATE_DIV 20        // 5% a week
#define PIGGY_INTEREST_MAX 20
#define DIARY_DAYS 7
enum DiaryFlag : uint8_t { DF_COOKED = 1, DF_ADVENTURE = 2, DF_TROPHY = 4, DF_RARE = 8, DF_ROUTE = 16 };
struct DiaryDay {
  uint16_t day;                 // days since 1970, 0 = empty
  uint8_t meals, games, baths, walks, fish, study, chores, flags;
  uint16_t coins;
};
struct FoodInfo {
  int8_t food, joy, energy, weight;   // what one serving does
  uint8_t price;
  bool sweet;                         // counts double towards a stomachache, like candy
};
static_assert(DEC_COUNT <= 16, "decorOwned needs a bit per item");
// Daily missions. Six a day, picked when the day starts (one from each group,
// then one more) from the ones she can do, and saved with their progress.
// Append new ones at the END: the saved pick indexes this list.
enum MissionId : uint8_t {
  MIS_FEED = 0,     // feed it N times
  MIS_FEED_FAV,     // feed its favourite berry
  MIS_PLAY,         // play N minigames
  MIS_STEPS,        // walk N steps
  MIS_BATH,         // give it a bath
  MIS_PET,          // pet it N times
  MIS_TRAIN_ATK,    // play a strength game
  MIS_TRAIN_SPE,    // play a speed game
  MIS_TRAIN_DEF,    // play a defence game
  MIS_SCORE,        // score N in any game
  // phase 31
  MIS_BUBBLE,       // a bubble bath
  MIS_TEETH,        // brush its teeth
  MIS_NAP,          // a nap or a lullaby
  MIS_HOMEMEAL,     // eat a dish from the kitchen
  MIS_SCORE_HI,     // score a lot in any game
  MIS_TICKETS,      // win N arcade tickets
  MIS_CAPSULE,      // spin the capsule machine
  MIS_QUIZ,         // play the type quiz
  MIS_CONTEST,      // enter the weekly contest
  MIS_STEPS_BIG,    // a long walk
  MIS_PARK,         // a walk in the park
  MIS_ADV,          // reach an adventure stop
  MIS_FISH,         // fish N times
  MIS_BEACH,        // a shell or a sand castle
  MIS_CENTER,       // visit the Pokemon Center
  MIS_COOK,         // cook a dish
  MIS_GARDEN,       // plant or harvest
  MIS_CHORES,       // N chores
  MIS_READ,         // read a book
  MIS_STUDY,        // study
  MIS_SONG,         // play a song
  MIS_MAIL,         // open a letter
  MIS_PIGGY,        // put coins in the piggy bank
  MIS_BUY_FOOD,     // buy food
  MIS_SHOP,         // buy anything
  MIS_DECOR,        // put out a decoration or furniture
  MIS_COUNT
};

enum PetMood : uint8_t { MOOD_HAPPY, MOOD_SAD, MOOD_EATING, MOOD_SLEEPING };

// the individual's medals (bitmask)
enum : uint16_t {
  MED_LV10 = 1 << 0, MED_LV25 = 1 << 1, MED_LV50 = 1 << 2,
  MED_BERRY = 1 << 3, MED_STREAK7 = 1 << 4, MED_BOND = 1 << 5,
  MED_FINAL = 1 << 6, MED_FIT = 1 << 7,
};
#define MED_COUNT 8

uint8_t moveUnlockLevel(int16_t dex, uint8_t idx);

// ---------------------------------------------------------------------------
// Which regions have their sprite pack on the microSD.
//
// One bit per region and THE DEFAULT IS ALL SET, which is doing two jobs: a
// board with no SD at all keeps today's behaviour rather than presenting an
// empty game, and pet.cpp stays free of any SD dependency. That second part is
// not tidiness -- pet.cpp is compiled into all 33 test binaries and none of
// them link sdmon.cpp, so a direct call would mean stubbing it 33 times. The
// firmware narrows the mask ONCE, after the card mounts. Same shape as
// link.cpp taking a transport pointer instead of calling ESP-NOW across a
// layer, and for the same reason: the tests can drive it.
//
// uint16_t, not uint8_t. REGION_COUNT is 5 today and this project has been
// bitten five separate times by a width chosen for the current size of the dex.
extern uint16_t gRegionArt;
static_assert(REGION_COUNT <= 16, "gRegionArt needs a bit per region");

// Is this region playable? REGION_ALL is available while ANY real region is --
// it is the mixed pool and must not vanish because one pack is missing.
bool regionAvailable(uint8_t r);

// Which region a dex number belongs to, or REGION_ALL if somehow none. The
// regions tile the dex exactly once (dexdata_test pins that), so this is a
// lookup rather than a judgement.
uint8_t regionOfDex(int16_t d);

// The next region the player can actually choose, skipping any whose sprite
// pack is missing. The egg pill cycles with this rather than (region + 1) %
// REGION_COUNT, which would land on a locked region and silently do nothing.
uint8_t nextAvailableRegion(uint8_t from);

// A Pet is copied (buildSquad() does `Pet tmp = pet;` to apply a level cap), and
// a plain Preferences copy SHARES the nvs_handle_t while ~Preferences() calls
// nvs_close() on it. The copy going out of scope therefore closed the LIVE pet's
// handle: every save after the first gym fight of a boot failed silently, and the
// next reboot rolled the game back -- badges won that session vanished. A copy
// gets an unopened store and a cleared `opened`, so it can read and never write.
struct PetPrefs : Preferences {
  PetPrefs() {}
  PetPrefs(const PetPrefs &) : Preferences() {}
  PetPrefs &operator=(const PetPrefs &) { return *this; }
};
struct PetOpened {
  bool v = false;
  PetOpened() {}
  PetOpened(const PetOpened &) {}
  PetOpened &operator=(const PetOpened &) { return *this; }
  PetOpened &operator=(bool b) { v = b; return *this; }
  operator bool() const { return v; }
};

class Pet {
public:
  // Estadisticas 0..100
  uint8_t fullness = 80;  // food
  uint8_t joy = 80;       // happiness
  uint8_t energy = 80;    // energy
  uint8_t hygiene = 100;  // cleanliness
  uint8_t poops = 0;      // poops on screen (max 3)
  uint8_t weight = 0;     // 0-100: treats fatten, the minigame burns
  // IV (individual values 0-31, as in the gen 3 games onward):
  // rolled at hatching and never change. They contribute IV x level / 100 to the
  // stat AND set the training ceiling (trMaxFor): a mediocre individual
  // not only starts worse, it cannot get as far.
  uint8_t ivAtk = 16, ivDef = 16, ivSpe = 16, ivHp = 16;
  uint8_t trAtk = 0, trDef = 0, trSpe = 0;
  bool berryKnown = false;  // has already discovered its favourite berry
  bool shiny = false;       // rare colour variant (rolled in the egg)
  uint32_t ageMinutes = 0;
  int16_t speciesId = -1;      // Pokedex number (1..DEX_COUNT), -1 = egg
  int16_t prevSpeciesId = -1;  // for the evolution animation
  uint8_t careMistakes = 0;   // mistakes: each one delays evolution by 1 level
  bool sleeping = false;
  uint32_t lastSeenEpoch = 0;   // last RTC time seen (for offline progression)
  uint8_t ceremony = CER_NONE;  // farewell/runaway/release in progress
  uint8_t lastEnd = CER_NONE;   // how the previous one ended (affects the egg)
  // A finished ceremony hands the creature over here before newEgg() wipes the
  // live state. The UI drains it (endedKind back to CER_NONE) once the pet has
  // either taken a party slot or been let go. Only farewell and release fill
  // it; a runaway leaves endedKind at CER_NONE and the pet is simply gone.
  PartyMon endedMon;
  uint8_t endedKind = CER_NONE;
  // Pokedex bitmaps, one bit per species. Widening these is SAFE on an existing
  // save: getBytes() copies only what was stored, and the array is zeroed by its
  // initialiser, so a 19-byte blob from the Kanto-only build lands in the front
  // and bits 1-151 keep exactly their old meaning.
  uint8_t dexReg[(DEX_COUNT + 7) / 8] = { 0 };       // raised
  uint8_t dexShinyReg[(DEX_COUNT + 7) / 8] = { 0 };  // raised as shiny
  // daily care streak (the player's: persists between raisings)
  uint16_t streak = 0, bestStreak = 0;
  uint32_t lastCareDay = 0;
  // bond (the creature's: rises slowly with care, resets when another is born)
  uint8_t bond = 0;
  char nick[12] = "";    // nickname (empty = species name)
  // medals: the individual's + running total across all raisings
  uint16_t medals = 0, totalMedals = 0;
  uint16_t newMedal = 0;   // just earned, to celebrate
  uint16_t lastMilestone = 0;  // streak milestone already celebrated
  uint16_t gameHi = 0;     // minigame record (the player's)
  uint16_t strHi = 0;      // bag hit record

  void begin();                 // loads state from NVS (or creates the first egg)
  void update(uint32_t nowMs);  // call on every loop()

  // Acciones (botones tactiles)
  void feed();              // red berry (compatibility)
  void feedBerry(uint8_t color);  // 0 red, 1 blue, 2 green
  void feedCandy();
  bool lovesBerry(uint8_t color) const {
    return !isEgg() && (speciesId % 3) == color;  // hidden taste per species
  }
  // The ball game: happiness AND defence training. Returns the DEF gained.
  uint8_t playResult(uint8_t score);
  uint8_t trainStrength(uint16_t hits);  // training bag (trains ATK)
  // Reaction test: its own trainer, so the ball game can go back to being purely
  // about joy instead of doubling as a stat grind.
  uint8_t trainSpeed(uint16_t hits);
  // What beating a gym leader is worth. Random WHICH stat, but only among the
  // ones with room left -- a random grant that landed on an already-capped stat
  // would silently evaporate, which reads as a bug rather than as luck. Writes
  // the stat into `which` (0 ATK, 1 DEF, 2 SPE) and returns what was actually
  // gained; 0 means every stat is at its ceiling. Costs nothing: the fight
  // already spent the energy, and that is what rate-limits rematching.
  uint8_t rewardTraining(uint8_t amount, uint8_t &which);
  uint16_t spdHi = 0;    // best reaction-test score

  // Fun Pack: the extra minigames' rewards and records
  uint8_t applyMiniResult(uint8_t game, uint8_t score);  // stat points, or joy for joy-only games
  static int8_t miniGameStat(uint8_t game);
  uint16_t miniHi[MINI_COUNT] = { 0 };
  // The arcade (phase 26): tickets from the Fun Pack games, spent at the prize
  // counter; this week's best per game; badges worked out from the records.
  uint16_t tickets = 0;
  uint32_t ticketDay = 0;
  uint8_t ticketsToday = 0;
  uint8_t lastTickets = 0, lastBadge = 0;     // what the last game gave, for its result screen
  uint8_t weekHi[MINI_COUNT] = { 0 };
  uint32_t weekId = 0;                         // (today()+3)/7 + 1: weeks start on Monday
  uint8_t earnTickets(uint8_t score, bool record);
  uint8_t winPrize(uint8_t p);                 // a PrizeResult
  bool prizeOwned(uint8_t p) const;
  static uint16_t prizePrice(uint8_t p);
  static uint8_t badgeTier(uint8_t game, uint16_t score);   // 0 none, 1 bronze, 2 silver, 3 gold
  uint8_t goldBadges() const;
  void weekRoll();
  bool beatThisWeek(uint8_t game) const { return weekHi[game] && weekHi[game] >= miniHi[game]; }
  // Fun Pack: statuses. All persisted under new keys, so an older save loads
  // healthy and with no records.
  uint8_t sickness = SICK_NONE;
  uint8_t sickDoses = 0;
  uint32_t lastDoseMinute = 0;
  uint16_t sickTicks = 0;
  uint8_t overfeed = 0;
  uint8_t dirtyTicks = 0;
  bool lightOn = false;        // dozed off by itself and the light is still on
  uint8_t lightOnTicks = 0;
  bool grumpy = false;
  uint16_t boredTicks = 0;
  bool isSick() const { return sickness != SICK_NONE; }
  bool isBored() const { return boredTicks >= BORED_TICKS; }
  bool canPlay() const { return !isSick(); }
  PillResult giveMedicine();
  void dozeOff();               // the sketch: left alone in the evening, screen on

  // Coins and daily missions. Player-wide like the streak: newEgg() leaves them.
  uint32_t coins = 0;
  uint16_t coinsToday = 0;      // activity coins earned today (against the cap)
  uint32_t coinDay = 0;         // the day coinsToday / missions / step coins belong to
  uint8_t stepCoinK = 0;        // thousands of steps already paid today
  uint8_t missionProg[MISSIONS_PER_DAY] = { 0 };
  uint8_t missionKind[MISSIONS_PER_DAY] = { 0 };   // the day's pick, rolled in rollCoinDay()
  uint32_t missionKindDay = 0;  // the day missionKind belongs to
  uint8_t missionDone = 0;      // bit per mission; bit 7 = the all-missions bonus paid
  uint32_t bedtimeDay = 0;      // the day the bedtime bonus was last paid
  uint16_t coinFlash = 0;       // the sketch shows "+N" while coinFlashUntil runs
  uint32_t coinFlashUntil = 0;
  // Adds coins; capped = counts against COIN_DAILY_CAP. Returns what was added.
  uint16_t earnCoins(uint16_t amount, bool capped);
  bool spendCoins(uint32_t amount);
  uint8_t missionAt(uint8_t slot) const;        // today's mission in slot 0..MISSIONS_PER_DAY-1
  static uint8_t missionGroup(uint8_t m);       // 0..MISSION_GROUPS-1
  bool missionOpen(uint8_t m, uint32_t day) const;   // can she do it today (the place is open)?
  void rollMissions(uint32_t day, uint8_t out[MISSIONS_PER_DAY]) const;
  void missionsToday() { rollCoinDay(); }       // the sketch: make sure today's pick is rolled
  static uint8_t missionGoal(uint8_t m);        // the N in "do it N times"
  uint8_t missionProgress(uint8_t slot) const;  // 0..goal
  bool missionComplete(uint8_t slot) const { return missionDone & (1 << slot); }
  bool allMissionsDone() const { const uint8_t a = (1 << MISSIONS_PER_DAY) - 1; return (missionDone & a) == a; }
  void noteMission(uint8_t m, uint8_t amount);  // progress any of today's slots for m
  uint16_t coinsCapLeft() const;

  // Hats. Player-wide: a new egg keeps what was bought (and what is worn).
  uint32_t hatsOwned = 0;       // bit per HatId
  int8_t hatOn = -1;            // the hat being worn, -1 = none
  // Where the player nudged the hat for THIS species: tops differ (BULBASAUR's
  // highest point is its bulb, not its head), so a nudge belongs to a species
  // and quietly resets when the creature evolves or a new one hatches.
  int8_t hatDx = 0, hatDy = 0;
  int16_t hatDex = 0;
  static uint16_t hatPrice(uint8_t h);
  bool ownsHat(uint8_t h) const { return h < HAT_COUNT && (hatsOwned & (1UL << h)); }
  bool buyHat(uint8_t h);       // false: unknown, already owned, or not enough coins
  void wearHat(int8_t h);       // -1 takes it off; an unowned hat is ignored
  void nudgeHat(int dx, int dy);
  int hatOffX() const { return hatDex == speciesId ? hatDx : 0; }
  int hatOffY() const { return hatDex == speciesId ? hatDy : 0; }

  // Toys and decorations. Player-wide, like the hats.
  uint16_t decorOwned = 0;      // bit per DecorId
  int8_t slotL = -1, slotR = -1;  // what stands left / right of the creature
  static uint16_t decorPrice(uint8_t d);
  static bool isToy(int8_t d) { return d >= 0 && d < DEC_TOY_COUNT; }
  bool ownsDecor(uint8_t d) const { return d < DEC_COUNT && (decorOwned & (1 << d)); }
  bool buyDecor(uint8_t d);     // false: unknown, already owned, or not enough coins
  // Tapping an item in the bag: puts it in the first free spot (left, then
  // right, else it replaces the left one), or picks it up if it is out.
  void placeDecor(uint8_t d);
  bool toyOut() const { return isToy(slotL) || isToy(slotR); }
  bool hasDecorOut(uint8_t d) const { return slotL == (int8_t)d || slotR == (int8_t)d; }

  // Foods in the bag. Player-wide, like the coins.
  uint8_t foodCount[FOOD_COUNT] = { 0 };
  static const FoodInfo &foodInfo(uint8_t f);
  bool buyFood(uint8_t f);      // one more serving; false without the coins or at the stack limit
  bool eatFood(uint8_t f);      // false when there is none, or it cannot eat now

  // Bath and sleep extras. Soap is used up, the brush is kept. Player-wide.
  uint8_t soapCount = 0;
  bool brushOwned = false;
  uint32_t napEnd = 0;          // ageMinutes the nap ends at, 0 = not napping
  uint32_t shineUntil = 0;      // millis: sparkles after brushing (not saved)
  bool buySoap();
  bool buyBrush();
  bool bubbleBath();            // uses a soap: the bath plus joy and bond
  bool brush();                 // needs the brush: joy, bond and a shine
  void scrubDone(uint8_t spots);  // the scrub game finished: a bath plus joy
  bool startNap();              // light off, wakes by itself after NAP_MINUTES
  bool lullaby();               // put to bed with a song: bedtime plus joy
  bool shining() const { return timeOn(millis(), shineUntil); }

  // Places. Player-wide.
  uint32_t parkDay = 0;         // the day parkFinds counts
  uint8_t parkFinds = 0;
  uint32_t centerDay = 0;       // the day of the last free heal
  uint8_t seedCount = 0;
  uint32_t plantEpoch = 0;      // when the garden was planted (RTC), 0 = empty
  uint32_t giftDay = 0;         // the day the last seasonal gift was handed out
  uint8_t lastFind = 0;         // coins the last PARK_COINS found (for the screen)
  ParkResult parkWalk();        // a walk in the park: tires a little, finds things
  CenterResult centerHeal();    // cures sickness and grumpiness, +30 ENE
  bool centerFreeToday() const { return centerDay != today(); }
  bool buySeed();
  bool plantSeed();             // needs a seed, an empty plot and the clock
  bool gardenReady() const;
  uint8_t gardenPercent() const;  // 0..100 grown
  int8_t harvest();             // the FoodId it gave, or -1 when not ready
  GiftKind holidayToday() const;  // which holiday the RTC date is, if any
  GiftKind claimHolidayGift();  // hands today's gift over once; the screen says what
  static void civilDate(uint32_t days, int &y, int &m, int &d);   // days since 1970 -> date

  // Capsules, scenery, trophies (phase 8). Player-wide.
  uint32_t capDay = 0;
  uint8_t capSpins = 0;         // capsules turned today
  uint16_t capWhat = 0;         // what the last capsule gave: the item id, or the coins
  CapsuleResult spinCapsule(bool paid = true);   // paid=false: a prize spin, no coins and no daily limit
  uint8_t capsulesLeft() const { return capDay == today() ? (capSpins < CAPSULES_PER_DAY ? CAPSULES_PER_DAY - capSpins : 0) : CAPSULES_PER_DAY; }
  uint8_t sceneryOwned = 0;     // bit per biome
  int8_t sceneryPick = -1;      // the backdrop chosen, -1 = the creature's own
  static uint16_t sceneryPrice(uint8_t b);
  bool ownsScenery(uint8_t b) const { return b == SCENERY_ROOM || (b < SCENERY_COUNT && (sceneryOwned & (1 << b))); }
  // the room (phase 13)
  uint16_t furnOwned = 0;       // bit per FurnId
  int8_t furnWallL = -1, furnWallR = -1, furnBackL = -1, furnBackR = -1;
  bool rugOn = false;
  int8_t wallpaper = -1;        // a FURN_PAPER_*, -1 = plain
  static uint16_t furnPrice(uint8_t f);
  bool ownsFurn(uint8_t f) const { return f < FURN_COUNT && (furnOwned & (1 << f)); }
  bool buyFurn(uint8_t f);
  bool furnPlaced(uint8_t f) const;
  void placeFurn(uint8_t f);    // into its spot (or out again); also switches to the room

  // The wardrobe (phase 16). Player-wide.
  uint16_t accOwned = 0;
  int8_t accOn[3] = { -1, -1, -1 };            // what is worn on the face, the neck, the back
  static uint16_t accPrice(uint8_t a);
  static uint8_t accSlot(uint8_t a) { return a / 4; }
  bool ownsAcc(uint8_t a) const { return a < ACC_COUNT && (accOwned & (1 << a)); }
  bool buyAcc(uint8_t a);
  void wearAcc(uint8_t a);                     // on, or off again if it is already on
  bool wearingAcc(uint8_t a) const { return a < ACC_COUNT && accOn[accSlot(a)] == (int8_t)a; }
  static const OutfitSet &outfitSet(uint8_t i);
  int8_t wearingSet() const;                   // the full set being worn, or -1

  // Town life (phase 17). Player-wide.
  uint32_t shellDay = 0, castleDay = 0, bookDay = 0, songDay = 0;
  uint8_t shellsToday = 0, booksToday = 0, songsToday = 0;
  bool findShell();                            // 1 coin, five a day
  bool buildCastle();                          // once a day: joy and bond
  bool readBook();                             // three a day: bond and joy
  bool playSong();                             // three a day: it dances
  uint32_t contestWeek = 0;
  uint8_t ribbons[3] = { 0, 0, 0 };            // bronze, silver, gold won
  uint8_t contestScore() const;
  bool contestDoneThisWeek() const { return contestWeek && contestWeek == today() / 7; }
  Ribbon enterContest(uint8_t score);
  uint8_t jobKind = JOB_NONE;
  uint32_t jobUntil = 0, jobDay = 0;
  bool jobDoneFlash = false;                   // the screen says it came back with its pay
  bool atWork() const { return jobKind != JOB_NONE && lastSeenEpoch < jobUntil; }
  bool startJob(uint8_t k);
  bool jobDoneToday() const { return jobDay == today(); }
  uint32_t mailDay = 0;
  uint8_t mailKind = 0;                        // the letter waiting, 1..LETTER_KINDS
  bool mailRead = true;
  bool hasMail() const { return mailKind && !mailRead; }
  uint8_t openMail();                          // reads it and takes the gift; the letter id

  // The Nursery (phase 21).
  NurseryState nursery = {};
  bool eggBred = false;                        // the waiting egg is the Nursery's
  static bool nurseryCanPair(const PartyMon &a, const PartyMon &b);
  int16_t lineBase(int16_t dex) const;         // the first form of its line an egg can hold, 0 = none
  uint8_t nurseryDrop(const PartyMon &a, const PartyMon &b);   // a NurseryResult
  bool nurseryBusy() const { return nursery.aDex > 0; }
  bool nurseryReady() const { return nurseryBusy() && lastSeenEpoch >= nursery.since + NURSERY_SECS; }
  uint32_t nurseryLeft() const { return nurseryReady() || !nurseryBusy() ? 0 : nursery.since + NURSERY_SECS - lastSeenEpoch; }
  bool nurseryCollect();                       // the egg, once ready and none already kept
  bool nurseryHasEgg() const { return nursery.eggDex > 0; }
  void nurseryGiveBack();                      // drop the kept egg: random eggs again
  bool nurseryWeekUsed() const { return nursery.week && nursery.week == today() / 7 + 1; }

  // A livelier pet (phase 14).
  uint8_t wishKind = WISH_NONE, wishArg = 0;   // the wish, and its food for WISH_FOOD
  uint32_t wishUntil = 0, wishNextAt = 0;      // epochs
  uint32_t wishDay = 0;
  uint8_t wishesToday = 0;
  bool wishGrantedFlash = false;               // the screen shows "wish granted" once
  void wishTick();                             // from tick(): maybe a new wish, or an old one fades
  bool grantWish(uint8_t kind, uint8_t arg = 0);   // a care action that may grant the wish
  bool hasWish() const { return wishKind != WISH_NONE && lastSeenEpoch < wishUntil; }
  Personality personality() const { return (Personality)((speciesId * 7 + ivAtk + ivSpe) % PERS_COUNT); }
  uint32_t lastCareEpoch = 0;                  // the last time anyone looked after it
  bool isLonely() const { return !isEgg() && !sleeping && lastCareEpoch && lastSeenEpoch > lastCareEpoch + LONELY_SECS; }
  bool isExcited() const;                      // every mission done today
  uint32_t bornDay = 0, bdayDay = 0;
  uint8_t birthdayMonths() const;              // 0 unless today is a birthday
  bool claimBirthday();                        // the party, once on the day
  EventId eventToday() const;
  static void easterDate(int year, int &m, int &d);
  uint32_t eggDay = 0, festDay = 0;
  uint8_t eggsFound = 0;
  bool findEasterEgg();                        // a walk in the park during Easter may find one
  uint8_t claimFestivalTreat();                // the day's event treat (summer, Halloween, winter): the EventId, or 0
  uint32_t questWeek = 0, questStepDay = 0;
  uint8_t questStep = 0;                       // steps done this week, 0..QUEST_STEPS
  bool questDoneFlash = false;
  uint8_t questToday() const { return (uint8_t)((today() / 7) % QUEST_COUNT); }
  static QuestKind questStepKind(uint8_t quest, uint8_t step);
  void questRoll();                            // a new week: a new quest
  bool questNote(uint8_t kind);                // something done that may be the next step
  bool questStepReady() const { return questStep < QUEST_STEPS && questStepDay != today(); }
  bool buyScenery(uint8_t b);
  void pickScenery(int8_t b);   // an unowned one is ignored; picking the one in use puts it back
  uint16_t tally[TALLY_COUNT] = { 0 };
  uint32_t coinsEarned = 0;     // every coin ever earned (a trophy)
  uint32_t trophies = 0;        // bit per TrophyId (low 16 in "trph", the rest in "trphX")
  void bumpTally(uint8_t t, uint16_t n = 1);
  void trophyProgress(uint8_t t, uint32_t &cur, uint32_t &goal) const;
  bool hasTrophy(uint8_t t) const { return t < TRO_COUNT && (trophies & (1UL << t)); }
  int8_t checkTrophies();       // awards the first newly reached one, -1 if none

  // Teeth, study, quiz, parent limits (phase 9).
  uint32_t teethDay = 0;
  uint8_t teethToday = 0;
  bool teethDone();             // a full brushing: false when already done twice today
  uint8_t teethLeftToday() const { return teethDay == today() ? (teethToday < TEETH_PER_DAY ? TEETH_PER_DAY - teethToday : 0) : TEETH_PER_DAY; }
  uint32_t studyDay = 0;
  uint8_t studyToday = 0;
  uint8_t studyDone(uint8_t minutes);   // coins paid (0 past STUDY_PER_DAY)
  uint8_t quizHi = 0;
  uint8_t quizDone(uint8_t correct);    // coins paid
  uint8_t parentBed = 0;        // 0 off, else the hour games stop (until 6:00)
  uint8_t parentGames = 0;      // 0 off, else games a day
  uint8_t gamesToday = 0;
  ParentBlock parentBlock() const;
  void saveParentLimits() { save(); }   // the settings page changed them

  // Step adventures, fishing, cooking (phase 10). Player-wide.
  uint8_t advRoute = 0, advStop = 0;   // the route, and how many of its stops are claimed
  uint32_t advSteps = 0;               // steps walked on this route
  uint16_t advWhat = 0;                // what the last stop gave (coins / food id / dex)
  static uint16_t advNeed(uint8_t stop);   // steps from the route's start to that stop
  bool advReady() const { return advStop < ROUTE_STOPS && advSteps >= advNeed(advStop); }
  AdvResult advClaim();
  uint32_t fishDay = 0;
  uint8_t fishToday = 0;
  uint8_t lastFishCoins = 0;
  uint8_t fishPaidLeft() const { return fishDay == today() ? (fishToday < FISH_PER_DAY ? FISH_PER_DAY - fishToday : 0) : FISH_PER_DAY; }
  FishResult fishCatch();              // a fish on the line, caught in time
  static const Recipe &recipe(uint8_t r);
  bool canCook(uint8_t r) const;
  bool cook(uint8_t r);

  // Alarm, chores, piggy bank, diary (phase 11).
  bool alarmOn = false;
  uint8_t alarmH = 7, alarmM = 0;
  uint32_t alarmDay = 0, bedRemDay = 0;
  bool alarmDue() const;               // within the hour after the alarm time, once a day
  void alarmRang() { alarmDay = today(); save(); }
  bool bedReminderDue() const;         // the parents' bedtime has come, once a day
  void bedReminded() { bedRemDay = today(); save(); }
  void saveAlarm() { save(); }
  uint32_t choreDay = 0;
  uint8_t choresDone = 0;              // bit per chore, today
  bool choreDoneToday(uint8_t c) const { return choreDay == today() && (choresDone & (1 << c)); }
  bool doChore(uint8_t c);
  uint32_t piggy = 0, piggyDay = 0;    // savings, and the day interest was last paid
  uint16_t piggyLastInterest = 0;
  bool piggyDeposit(uint32_t amount);
  uint32_t piggyWithdraw();
  uint8_t piggyDaysLeft() const;
  uint16_t piggyNextInterest() const;
  DiaryDay diary[DIARY_DAYS] = {};
  uint32_t diaryCoinBase = 0;
  void diaryRoll();
  void diaryFlag(uint8_t f) { diaryRoll(); diary[0].flags |= f; pendingSave = true; }
  uint16_t diaryCoins(uint8_t i) const { return i == 0 ? (uint16_t)(coinsEarned - diaryCoinBase) : diary[i].coins; }
  bool isDozeHour() const;

  // combat stats: real gen 1 base + level + IV + training
  uint16_t atkStat() const;
  uint16_t defStat() const;
  uint16_t speStat() const;
  uint16_t vitStat() const;  // vitality (bHp): not trained, only IV and level
  // The physical/special split lives on the species (bSpA/bSpD), not the
  // individual: special attack reuses ivAtk/trAtk and special defence reuses
  // ivDef/trDef, so no extra IVs and no save migration. See fetch_pokeapi.py.
  uint16_t spaStat() const;
  uint16_t spdStat() const;

  // The four known moves (indices into MOVE_TBL; 0 = empty slot). Player-chosen
  // once the learn/forget prompt exists -- until then, and for saves made before
  // moves were stored at all, relearnFromLevel() fills them in.
  uint8_t moves[MOVE_SLOTS] = { 0, 0, 0, 0 };
  uint8_t moveCount() const;
  bool knowsMove(uint8_t mv) const;
  // The newest MOVE_SLOTS moves this species has learned by its current level,
  // newest last. Used on hatch, on a fresh save, and to backfill empty slots.
  void relearnFromLevel();
  // The level at which `dex` may legally use learnset entry `i`. A level-up
  // move carries its own; a level-0 entry is a TM/tutor/egg move, which the
  // data gives no level at all, and those unlock together at TM_LEVEL.
  //
  // It is a free function, not a Pet method, because the move PICKER needs the
  // identical answer for a banked party member. Having its own gate is what let
  // a level 22 Charmeleon be offered FIRE BLAST.

  // Moves reachable at this level that are not already known, for the learn
  // prompt. Returns how many were written into out (at most max).
  uint8_t pendingLearnables(uint8_t *out, uint8_t max) const;

  // Player-wide, like the streak and the Pokedex: badges outlive the creature
  // that earned them, so newEgg() must never clear this.
  // A creature brought back out of the party or box. It is FROZEN: it does not
  // age, cannot evolve, and cannot be lost -- a companion rather than a
  // contender. The cost is that its level never rises again, so it stops
  // improving; ageMinutes is simply set to match its banked level, which keeps
  // level() working untouched rather than needing a second source of truth.
  bool frozen = false;
  void reviveFrom(const PartyMon &m);

  // The player's own name, alongside the badges and the streak: it belongs to
  // whoever is playing, not to the creature, so newEgg() must never clear it.
  char trainerName[12] = "";
  void renameTrainer(const char *n) {
    strncpy(trainerName, n, sizeof(trainerName) - 1);
    trainerName[sizeof(trainerName) - 1] = 0;
    save();
  }

  // Owner shown on the ABOUT page, set over serial with NAME. Device-level like
  // the trainer name, so newEgg() must never clear it. ASCII only (bitmap font).
  char ownerName[17] = "";
  void setOwnerName(const char *n) {
    size_t j = 0;
    for (size_t i = 0; n[i] && j < sizeof(ownerName) - 1; i++)
      if ((uint8_t)n[i] >= 0x20 && (uint8_t)n[i] < 0x7F) ownerName[j++] = n[i];
    ownerName[j] = 0;
    save();
  }

  // Which generation eggs come from. Player-wide, like the badges: it outlives
  // every creature, so newEgg() must never reset it.
  uint8_t region = REGION_ALL;
  // The species this egg would be in each region. Filled in as the player
  // looks, cleared by newEgg(). It exists so that switching region and back
  // shows the SAME creature rather than rolling a fresh one -- without it,
  // toggling would be a re-roll button.
  int16_t eggByRegion[REGION_COUNT] = { 0 };
  void setRegion(uint8_t r);
  const char *regionName() const { return REGIONS[region % REGION_COUNT].name; }
  // How much of one region's dex is filled in, for the Pokedex header.
  uint16_t registeredCountIn(uint16_t lo, uint16_t hi) const {
    uint16_t n = 0;
    for (uint16_t d = lo; d <= hi && d <= DEX_COUNT; d++) if (isRegistered(d)) n++;
    return n;
  }

  uint8_t avatar = 0;       // which player sprite, 0..3
  // Kanto's ladder, under the keys it has always used. Johto and Hoenn live in
  // a SEPARATE array under new keys rather than widening these -- purely
  // additive, so an existing save cannot be misread, exactly the reasoning that
  // put the box under its own key instead of growing the party blob.
  uint16_t badges = 0;      // bit n = trainer n beaten on easy
  uint16_t badgesHard = 0;  // ... and on hard
  uint16_t badgesX[GYM_REGIONS - 1] = { 0 };
  uint16_t badgesHardX[GYM_REGIONS - 1] = { 0 };

  uint16_t badgeMask(uint8_t rg, bool hard) const {
    if (rg == 0) return hard ? badgesHard : badges;
    if (rg >= GYM_REGIONS) return 0;
    return hard ? badgesHardX[rg - 1] : badgesX[rg - 1];
  }
  bool hasBadge(uint8_t rg, uint8_t i, bool hard) const {
    return (badgeMask(rg, hard) >> i) & 1;
  }
  // Phase 34: the road to the gym. Walking fills gymSteps; the next leader not
  // yet beaten on an easy ladder needs gymStepsNeed(its place) of them, and the
  // first win spends them (the rest carries on). Rematches and hard are free.
  // Phase 36: about a gym a week. More steps, a week between new badges, and the
  // road stops filling at the leader it leads to -- nothing piles up for the next.
  uint16_t gymSteps = 0;
  uint32_t gymLastWinDay = 0;                    // the day of the last NEW easy badge
  static uint16_t gymStepsNeed(uint8_t i) { return (uint16_t)(GYM_STEPS_BASE + GYM_STEPS_PER * i); }
  uint8_t gymWaitDays() const {                  // days until the week since the last new badge is up
    if (!gymLastWinDay) return 0;
    uint32_t d = today(), open = gymLastWinDay + GYM_WEEK_DAYS;
    if (d >= open) return 0;
    uint32_t wait = open - d;                 // a date before the last win: still only a week
    return wait > GYM_WEEK_DAYS ? GYM_WEEK_DAYS : (uint8_t)wait;
  }
  bool gymStepsOk(uint8_t rg, uint8_t i, bool hard) const {
    return hard || hasBadge(rg, i, false) || (gymSteps >= gymStepsNeed(i) && !gymWaitDays());
  }
  int8_t gymNextIn(uint8_t rg) const {           // the first leader not beaten on easy, or -1
    for (uint8_t i = 0; i < 13; i++) if (!hasBadge(rg, i, false)) return (int8_t)i;
    return -1;
  }
  uint8_t gymChallengeRegion() const {           // its own region first, else the first unfinished; 0xFF all done
    if (region < GYM_REGIONS && gymNextIn(region) >= 0) return region;
    for (uint8_t r = 0; r < GYM_REGIONS; r++) if (gymNextIn(r) >= 0) return r;
    return 0xFF;
  }
  int8_t gymChallengeIdx() const { uint8_t r = gymChallengeRegion(); return r < GYM_REGIONS ? gymNextIn(r) : -1; }
  bool gymWalked() const {                       // the steps are done (the week may still be running)
    int8_t i = gymChallengeIdx();
    return i >= 0 && gymSteps >= gymStepsNeed((uint8_t)i);
  }
  bool gymChallengeReady() const { return gymWalked() && !isEgg() && !gymWaitDays(); }
  void winBadge(uint8_t rg, uint8_t i, bool hard) {
    if (rg >= GYM_REGIONS) return;
    uint16_t bit = (uint16_t)1 << i;
    bool isNew = !hasBadge(rg, i, hard);
    if (isNew && !hard) {                          // a first easy win spends the walk, and starts the week
      gymSteps = 0;
      gymLastWinDay = today();
    }
    if (rg == 0) { if (hard) badgesHard |= bit; else badges |= bit; }
    else if (hard) badgesHardX[rg - 1] |= bit;
    else badgesX[rg - 1] |= bit;
    if (isNew) earnCoins(COIN_BADGE, false);   // a rematch pays nothing
    save();
  }
  uint8_t badgeCountIn(uint8_t rg, bool hard) const {
    uint16_t v = badgeMask(rg, hard);
    uint8_t n = 0;
    while (v) { n += v & 1; v >>= 1; }
    return n;
  }
  // Every region's badges together, for the player card's running total.
  uint8_t badgeCount(bool hard) const {
    uint8_t n = 0;
    for (uint8_t r = 0; r < GYM_REGIONS; r++) n += badgeCountIn(r, hard);
    return n;
  }

  // Level-up learning. lastLearnLevel is the highest level whose gates have
  // been handled, so a move declined once is not offered forever, and the
  // offline catch-up -- which can cross a dozen levels in one go -- queues its
  // offers instead of firing a dozen dialogs at boot.
  uint8_t lastLearnLevel = 0;
  uint8_t learnQueue[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  uint8_t learnQCount = 0;
  void checkLearnGates();
  bool hasLearnOffer() const { return learnQCount > 0; }
  uint8_t learnOffer() const { return learnQCount ? learnQueue[0] : 0; }
  void acceptLearn(uint8_t slot);   // put the pending move into slot 0..3
  void declineLearn();
  // training ceiling an IV allows: 77 (IV 8) .. 100 (IV 31)
  static uint8_t trMaxFor(uint8_t iv) { return 70 + (30 * (uint16_t)iv) / 31; }
  uint8_t trMaxAtk() const { return trMaxFor(ivAtk); }
  uint8_t trMaxDef() const { return trMaxFor(ivDef); }
  uint8_t trMaxSpe() const { return trMaxFor(ivSpe); }
  void play();
  void toggleLight();
  bool isNightHour() const;
  static bool isRestAt(uint32_t epoch);   // 20:00-08:00, a night's rest
  bool isRestHour() const { return lastSeenEpoch && isRestAt(lastSeenEpoch); }
  void setScreenOff(bool off);  // the sketch reports the PWR button
  bool screenIsOff = false;
  void dbgTick() { tick(); }   // tests drive minutes directly; tick() is private
  // syncClock() reads "seen" back out of NVS, so a test has to put it there
  void dbgSetSeen(uint32_t e) { lastSeenEpoch = e; prefs.putUInt("seen", e); }
  uint8_t sleepAuto = SLEEP_NONE;   // who decided the current sleep state  // sleep / wake
  void clean(uint8_t gain = 100);   // a wash: CLEAN up by gain; the poop stays for the broom
  bool sweep();                     // the broom: the poop goes; false when there is none
  void caress();  // pet the creature
  void eggTap();  // tap the egg: 3 taps and it hatches
  void newEgg();   // start over with a random starter
  void release();  // release (long press + confirm)
  void syncClock(uint32_t nowEpoch);  // applies the time elapsed while switched off
  // Sets the time without applying progression. trusted: this is the real date
  // (Wi-Fi), so a step day ahead of it was wrong and moves down, numbers kept.
  void setClock(uint32_t nowEpoch, bool trusted = false);
  void startFarewell();  // also usable from the serial console (BYE)
  // Retire on demand. Before the farewell is earned this is the SAME ceremony
  // -- the creature is banked exactly as it would be -- but it hands the next
  // creature EVO_PENALTY_LEVELS on every evolution threshold. Retiring one that
  // has already earned its farewell costs nothing: it is then just the button.
  void startRetire();
  bool canRetireNow() const;
  bool retireIsFree() const { return canFarewellNow(); }
  // Was the retire NOW IN PROGRESS an early one? THE single answer, asked by
  // snapshotForParty() and by the tests. Distinct from retireIsFree(), which asks
  // whether a retire started right now WOULD be free: this one remembers what the
  // retire that is already running actually was, and outlives the moment it began.
  bool retireIsEarly() const { return retirePending; }
  uint8_t evoPenalty() const { return evoPen; }
  void startRunaway();   // also usable from the serial console (RUN)

  bool isEgg() const { return speciesId < 0; }
  uint8_t eggCracks() const { return eggTaps; }
  void sweepOldTimers(uint32_t now) {
    for (uint32_t *t : { &eatUntil, &heartUntil, &evolveUntil, &medalUntil, &milestoneUntil,
                         &shineUntil, &coinFlashUntil, &ceremonyUntil }) sweepTimer(*t, now);
  }
  bool eating() const { return timeOn(millis(), eatUntil); }
  uint32_t eatLeftMs() const { uint32_t n = millis(); return timeOn(n, eatUntil) ? eatUntil - n : 0; }
  bool showHeart() const { return timeOn(millis(), heartUntil); }
  uint32_t heartLeftMs() const { uint32_t n = millis(); return timeOn(n, heartUntil) ? heartUntil - n : 0; }
  bool evolving() const { return timeOn(millis(), evolveUntil); }
  float evolveT() const {     // progress of the evolution animation 0..1
    uint32_t n = millis();
    uint32_t left = timeOn(n, evolveUntil) ? evolveUntil - n : 0;
    return 1.0f - (float)left / (float)EVOLVE_ANIM_MS;
  }
  bool canEvolveNow() const;  // evolution conditions met (ready)
  void evolve();              // triggers the transformation (called by a user tap)
  bool canFarewellNow() const;  // final form + 7 days: ready to say goodbye (button)
  // Total neglect RIGHT NOW. THE single answer: tick() counts against it and
  // canRunawayNow() re-checks it, because neglectTicks is frozen (neither
  // counted nor cleared) while asleep and so can outlive the state that
  // earned it -- see sleep_test.
  bool inTotalNeglect() const { return !fullness && !joy && !energy && !hygiene; }
  bool canRunawayNow() const;   // total neglect for 1h: ready to run away (sad button)
  // the user decides in a dialog; "keep/stay together" postpones and re-offers later
  bool wantEvolveButton() const { return canEvolveNow() && level() > evoDeclinedLv; }
  bool wantFarewellButton() const { return canFarewellNow() && ageMinutes >= farDeclinedAge; }
  void declineEvolve() { evoDeclinedLv = level(); }              // re-offers on level up
  void declineFarewell() { farDeclinedAge = ageMinutes + 1440; } // re-offers within 1 day
  // first game: the player picks a starter (Bulbasaur/Charmander/Squirtle)
  bool awaitingStarter() const { return starterPick; }
  void chooseStarter(int16_t dex) { eggTarget = dex; starterPick = false; save(); }
  void factoryReset() { prefs.clear(); }  // wipes the NVS (test: serial command WIPE)
  void dbgRunawayReady() { fullness = joy = energy = hygiene = 0; neglectTicks = RUNAWAY_TICKS; }  // test
  // test: force what the egg holds and hatch it now (serial command EGG).
  // The legendary/shiny IV guarantees only fire inside hatch(), so without
  // this there is no way to exercise them from outside the class.
  void dbgHatchAs(int16_t dex, bool wantShiny) {
    if (dex < 1 || dex > DEX_COUNT) return;
    eggTarget = dex;
    eggShiny = wantShiny;
    starterPick = false;
    speciesId = -1;
    eggTaps = 0;
    hatch();
  }
  // Capped at 100, the series' own ceiling. The cap is not cosmetic: ageMinutes
  // is never clamped, so without it 1 + ageMinutes/60 overflows this uint8_t
  // past ~10.6 days -- and the RTC catches up to 2 weeks offline, which lands on
  // level 337 and wraps to 81. Reached at 4d 3h; farewell is merely *offered* at
  // 3 days (level 73), so declining it to raise a stronger battler is a real
  // choice rather than an accident.
  uint8_t level() const {
    uint32_t l = 1 + ageMinutes / MINUTES_PER_LEVEL;
    return l > MAX_LEVEL ? MAX_LEVEL : (uint8_t)l;
  }
  bool isRegistered(int16_t dex) const {
    return dex >= 1 && dex <= DEX_COUNT && (dexReg[(dex - 1) >> 3] & (1 << ((dex - 1) & 7)));
  }
  bool isShinyRegistered(int16_t dex) const {
    return dex >= 1 && dex <= DEX_COUNT && (dexShinyReg[(dex - 1) >> 3] & (1 << ((dex - 1) & 7)));
  }
  uint16_t registeredCount() const;
  bool lineHasUnregistered(int16_t base) const;
  uint8_t eggRarity() const;       // rarity of the current egg (without revealing the species)
  int16_t pickEggSpecies();        // public so egg rolls can be simulated (EGGS)
  // What the waiting egg would hatch into. Hidden from the PLAYER, not from the
  // code: the serial console already simulates rolls, and the region tests have
  // to see which creature a switch landed on.
  int16_t eggPeek() const { return eggTarget; }
  int16_t rollInRegion(uint8_t r, uint8_t tier);
  // Which of EEVEE's branches this player can actually be handed right now.
  // THE single answer: evolve() picks from it and lineHasUnregistered() asks it
  // whether the line is finished, and those two having their own opinions is how
  // the branch stayed at 134-136 while five more Eeveelutions sat in the dex.
  //
  // Filtered by the sprite pack AND by whether the species has art at all --
  // evolving into something that can only draw as a dex number is the same fault
  // just removed from the gym rosters, and here it would be permanent. `out`
  // must hold EEVEE_EVO_COUNT; returns how many were written.
  uint8_t eeveeOptions(int16_t *out) const;
  uint8_t lowestStat() const { return min(min(fullness, joy), min(energy, hygiene)); }
  PetMood mood() const;
  // progress of the farewell/runaway ceremony, 0..1 (to animate it)
  float ceremonyT() const {
    if (ceremony == CER_NONE) return 0.0f;
    uint32_t n = millis();
    uint32_t left = timeOn(n, ceremonyUntil) ? ceremonyUntil - n : 0;
    return 1.0f - (float)left / (float)CEREMONY_MS;
  }

  // streak / bond / medals / name
  void rename(const char *name);
  bool hasMedal(uint16_t m) const { return medals & m; }
  bool showMedal() const { return timeOn(millis(), medalUntil); }
  bool showMilestone() const { return timeOn(millis(), milestoneUntil); }
  int careBonus() const;  // egg improvement from streak + bond

  // deferred periodic save: tick() marks it pending and the loop flushes it
  // when the screen is dimmed/off (the flash write freezes
  // ~1s on both cores: so it is neither seen nor cuts off the touch)
  bool savePending() const { return pendingSave; }
  uint32_t saveSerial() const { return savesDone; }   // moves on every finished save
  uint32_t savedEpoch() { return prefs.getUInt("seen", 0); }
  // The time to start from at boot. With no battery the clock chip forgets the
  // time when unplugged, so a blank or earlier reading gives way to the last time
  // the save recorded: the minutes unplugged are lost, but the date never goes back.
  static uint32_t bootEpoch(uint32_t rtc, uint32_t seen) {
    if (rtc && rtc >= seen) return rtc;
    const uint32_t floor = 1767225600UL;      // 1 Jan 2026, for a save that has no time at all
    return seen > floor ? seen : floor;
  }
  void flushSave();
  // Writes NOW, whatever pendingSave says. flushSave() is `if (pendingSave)
  // save()`, so a console command that changed something the game had not
  // already marked dirty wrote nothing at all and quietly depended on the next
  // autosave to notice. Defined out of line: save() is private and declared
  // further down.
  void saveNow();

  // Steps walked today. Player-wide like the streak: newEgg() leaves it alone.
  // Stored with the day it belongs to rather than cleared by a timer, so it also
  // resets correctly when the board was off across midnight. The day is today(),
  // which follows the RTC, so the turn happens within ~30 s of 00:00.
  uint32_t stepCount = 0;
  uint32_t stepDay = 0;
  // Phase 35: the last 7 days (stepHist[0] is stepHistDay, [1] the day before...)
  // and every step ever walked. stepCount is still "today" and resets at midnight.
  uint16_t stepHist[7] = { 0 };
  uint32_t stepHistDay = 0;
  uint32_t stepTotal = 0;
  void stepHistRoll(uint32_t d);              // move the history on to day d
  uint16_t stepsDaysAgo(uint8_t ago) const;   // 0 = today .. 6
  uint16_t stepsUnsaved = 0;
  // A date behind the step day is a wrong clock, not a new day: today stays today.
  uint32_t stepsToday() const { return stepDay >= today() ? stepCount : 0; }
  void addSteps(uint32_t n);
  void saveSteps();                           // just the step values, for use while walking
  // Prizes for all the steps ever walked (phase 40).
  uint8_t walkPrize = 0;                      // how many have been paid
  uint16_t walkPrizeGain = 0;                 // coins waiting to be announced
  static uint32_t walkPrizeAt(uint8_t i);     // 0 past the last one
  static uint16_t walkPrizeCoins(uint8_t i);
  // A rank for all those steps (phase 43): one more for each prize reached, so it
  // comes from the total and needs no saving of its own.
  static uint8_t walkRankOf(uint32_t total);
  uint8_t walkRank() const { return walkRankOf(stepTotal); }
  // Three goals a day that care for the Pokemon (phase 43). stepGoalsDone holds one
  // bit per goal paid and belongs to stepDay; stepGoalNew holds those not yet shown.
  uint8_t stepGoalsDone = 0;
  uint8_t stepGoalNew = 0;
  static uint32_t stepGoalAt(uint8_t i);      // 0 past the last one
  void claimStepGoals();                      // pays any reached, unless an egg or ceremony is waiting
  // Sounds it makes by itself stay silent from 21:00 to 07:00 (phase 40).
  static bool isQuietAt(uint32_t epoch);
  bool quietHours() const { return lastSeenEpoch && isQuietAt(lastSeenEpoch); }
  // A need very low: true when it is time for a warning beep. Every 5 minutes, at
  // most NEED_BEEP_MAX times, then silent until it is cared for.
  bool needBeepDue(uint32_t nowMs);
  void setStepsToday(uint32_t n);   // debug/console: sets today's total outright

private:
  PetPrefs prefs;
  // A Pet that was never begin()'d MUST NOT WRITE. Nothing enforced that, and
  // the battle code builds its opponent as a throwaway Pet:
  //
  //     Pet foe; foe.dbgHatchAs(dex, false);   // -> hatch() -> save()
  //
  // Every Pet shares the one "tamapoke" NVS namespace, so that save() landed on
  // the PLAYER'S stored creature -- species, IVs, level, the lot -- and
  // registerSpecies() put the opponent in their Pokedex on the way past. The
  // live `pet` object in RAM was untouched, so nothing looked wrong until the
  // board rebooted or was flashed and loaded the FOE as the player's creature.
  // That is the "flashed and my mon got replaced with an older one" report.
  //
  // The rule belongs here rather than at the call sites: every future throwaway
  // Pet is covered without anyone having to remember.
  PetOpened opened;
  uint32_t lastTick = 0;
  uint32_t eatUntil = 0;
  uint32_t heartUntil = 0;
  uint32_t evolveUntil = 0;
  int16_t eggTarget = 1;       // hidden dex that will come out of the egg
  bool eggShiny = false;       // surprise rolled when the egg is created
  uint8_t eggTaps = 0;
  uint8_t mistakeCooldown = 0;
  uint8_t ticksSinceSave = 0;
  bool pendingSave = false;     // periodic save pending flush
  uint32_t savesDone = 0;
  uint8_t needBeeps = 0;
  uint32_t needBeepMs = 0;
  uint8_t evoDeclinedLv = 0;    // "keep form": do not offer evolution until the next level up
  uint32_t farDeclinedAge = 0;  // "stay together": do not offer farewell until this age
  bool starterPick = false;     // first game: waiting for the player to pick a starter
  uint8_t evoPen = 0;           // levels added to this creature's evolution gate
  bool retirePending = false;   // an early retire is under way; newEgg() spends it
  uint8_t neglectTicks = 0;
  uint16_t goodTicks = 0;  // well-cared streak: forges DEF
  uint32_t ceremonyUntil = 0;
  uint8_t bondToday = 0;       // daily cap on bond gain
  uint32_t medalUntil = 0;     // medal celebration on screen
  uint32_t milestoneUntil = 0; // streak milestone celebration

  uint32_t today() const { return lastSeenEpoch ? lastSeenEpoch / 86400 : 0; }
  void registerCare();   // first care of the day: streak + bond
  void addBond(uint8_t amt);
  uint8_t rollIV(int bonus) const;  // a roll of 8-31 pushed up by care
  void rollIVs();                   // all 4, with the legendary/shiny guarantees
  uint8_t ivFromGene(uint8_t gene) const;  // save migration with genes
  void defTick(bool resting);       // calm forges defence (see pet.cpp)
  void snapshotForParty();          // copy into endedMon before newEgg() wipes it
  void checkMedals();
  void notePlayed();          // any minigame: cures boredom and grumpiness
  void rollCoinDay();         // a new day: reset the cap, the missions, the step coins
  void notePlayedGame(uint8_t cat, uint16_t score);   // coins + missions for a game (cat: MINI_STAT_* or MINI_NO_STAT)
  void noteBought();          // anything bought in a shop: the missions and the quest step
  void noteOverfeed(uint8_t amount);
  void wakeUp();              // waking applies the grumpiness from the light
  void statusTick();          // per awake minute: sickness, boredom, dozing off
  void tick();
  void applyAutoSleep();
  void hatch();
  void registerSpecies(int16_t dex);
  void save();
  void load();
  static uint8_t clamp100(int v) { return v < 0 ? 0 : (v > 100 ? 100 : v); }
};
