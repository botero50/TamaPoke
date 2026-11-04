#pragma once
#include <Arduino.h>

// Idiomas soportados. La fuente del firmware no tiene acentos: ambos textos van
// sin tildes ni enes (igual que ya iba el espanol).
enum Lang : uint8_t { LANG_ES = 0, LANG_EN, LANG_FR, LANG_DE, LANG_IT, LANG_PT, LANG_ZH, LANG_KO, LANG_COUNT };
#define LANG_DEFAULT LANG_EN  // idioma por defecto: ingles

extern Lang gLang;  // active language (defined in i18n.cpp)

// String IDs. The order must match the STRINGS table in i18n.cpp.
enum StrId : uint16_t {
  // creature status (statusMsg)
  S_EVOLVING, S_EATING, S_LIKES, S_HUNGRY, S_NEEDS_BATH,
  S_EXHAUSTED, S_SAD, S_CHUBBY, S_IS_SHINY, S_HAPPY,
  // farewell ceremonies
  S_FAREWELL, S_RUNAWAY, S_GOODBYE,
  // egg
  S_EGG_HDR, S_EGG_LEGEND, S_EGG_RARE, S_EGG_TOUCH, S_EGG_MOVES, S_EGG_ALMOST,
  // shared formats
  S_POKEDEX_FMT,   // "POKEDEX %u/151"
  S_NAME_FMT,      // "%s%s Nv.%u"
  // release dialog
  S_RELEASE_FMT, S_YES, S_NO,
  // minigame and bag
  S_HITS_FMT, S_STR_GAIN_FMT, S_NEW_RECORD, S_RECORD_FMT, S_HIT_FAST,
  S_SCORE_FMT, S_GREAT_JOY, S_PLUS_JOY,
  // clock / settings
  S_SET_TIME, S_HOUR, S_MIN, S_CLOCK_CANCEL, S_LANG_LABEL,
  // celebration
  S_MEDAL_BANNER, S_GREAT, S_STREAK_DAYS_FMT,
  // card: profile
  S_STREAK_FMT, S_VIN, S_BERRY_UNK, S_BERRY_RED, S_BERRY_BLUE, S_BERRY_GREEN,
  S_INFO_FMT, S_RENAME_HINT,
  // card: combat
  S_BATTLE, S_STAT_ATK, S_STAT_DEF, S_STAT_SPE, S_STAT_WGT, S_TRAIN_STR,
  // card: medals
  S_MEDALS_FMT, S_BACK,
  // keyboard and gallery
  S_NAME, S_DETAIL_BACK,
  // barras
  S_BAR_FOOD, S_BAR_JOY, S_BAR_ENE, S_BAR_HYG,
  // live minigame scoreboard
  S_REC_FMT,
  // card: progress page
  S_PROGRESS, S_LVL_FMT, S_NEXT_LVL_FMT, S_EVO_LABEL, S_FINAL_FORM,
  S_EVO_READY, S_EVO_BLOCKED, S_EVO_IN_FMT, S_MISTAKES_FMT,
  // sound switch (settings)
  S_SND_ON, S_SND_OFF,
  S_EVO_TAP,        // evolution button text
  S_FAREWELL_BTN,   // farewell button text (carries the name: "%s ...")
  S_RUNAWAY_BTN,    // runaway-from-neglect button text (sad ending)
  // decision dialogs (evolve/keep, say goodbye/stay together)
  S_EVO_Q, S_EVO_KEEP, S_FAR_Q, S_FAR_GO, S_FAR_STAY,
  S_CHOOSE_STARTER,  // starter choice title (first time)
  S_NO_SPRITES, S_LOAD_SPRITES,  // notice when the sprite is missing from the SD
  S_STAT_VIT,   // vitality (abbreviation of PS/HP in each language)
  S_IV_FMT,     // individual value next to each stat ("IV %u")
  // menu overlay (tap the name on the main screen) and the party
  S_MENU_TITLE, S_SETTINGS, S_CLOSE,
  S_PARTY_FMT,      // "PARTY %u/6"
  S_PARTY_EMPTY,    // empty slot marker
  S_PARTY_JOINED,   // "%s joined the party!"
  S_PARTY_FULL,     // prompt: pick someone to replace
  S_PARTY_LETGO,    // decline the swap
  // stats shortcut in the menu, and the training submenu behind the 5th icon
  S_STATS,          // menu row: jump straight to the stats card page
  S_TRAIN,          // training submenu title
  S_TR_ATK,         // row: train strength (the punching bag)
  S_TR_SPE,         // row: train speed (the ball game)
  S_TR_DEF,         // row: defence -- passive, shown for information only
  S_TR_DEF_HINT,    // why the defence row does nothing when tapped
  // moves card page + the move picker behind it
  S_MOVES,          // card page title
  S_MOVE_EMPTY,     // an unused move slot
  S_MOVE_PICK,      // picker title: choose what goes in this slot
  S_MOVE_TAP,       // hint on the moves page: tap a slot to change it
  S_MOVE_PWR,       // "PWR %u" next to an attacking move
  S_MOVE_STATUS,    // shown instead of power for a status move
  // level-up learn prompt
  S_LEARN_Q,        // "%s wants to learn" (name of the creature)
  S_LEARN_SKIP,     // decline the new move
  // battle
  S_BTL_USED,       // "%s used %s"
  S_BTL_SUPER, S_BTL_WEAK, S_BTL_IMMUNE,
  S_BTL_MISS, S_BTL_CRIT, S_BTL_FAINT,
  S_BTL_HURTSELF,   // confusion
  S_BTL_STATUS,     // "%s was %s" -- ailment name comes from S_AIL_*
  S_AIL_PARA, S_AIL_BURN, S_AIL_POISON, S_AIL_SLEEP, S_AIL_FREEZE, S_AIL_CONFUSE,
  S_BTL_WIN, S_BTL_LOSE,
  S_BTL_SENDS,      // "%s sends out %s"
  S_BTL_GO,         // "Go, %s!"
  S_GYMS,           // gym list title
  S_BADGES_FMT,     // "BADGES %u/8"
  S_TRAINER,        // player card title
  S_SPD_GAIN_FMT,   // "SPEED +%u" after the reaction test
  S_AVATAR_HINT,    // "tap: change avatar"
  S_MEDALS_TOTAL_FMT,  // "%u earned in all"
  S_EASY, S_HARD,   // gym ladder difficulty toggle
  S_PICK_FMT,       // "CHOSEN %u/%u"
  S_FIGHT,          // start the battle
  S_LOCKED,         // a gym not yet unlocked
  S_BTL_SWITCH,     // battle menu: bring on another creature
  S_BTL_BEAT,       // "%s defeated!" on the win screen
  S_BTL_NEWBADGE,   // a badge earned for the first time
  S_VOL_FMT,        // volume level, e.g. "VOL 7"
  S_BOX_FMT,        // "BOX %u/%u"
  S_BOX_SWAP,       // "swap with %s: pick a slot"
  S_BOX_BTN,        // the button on the party screen
  S_REVIVE,         // bring a banked creature back as a companion
  S_REVIVE_EGG,     // why it is unavailable right now
  // LAN battle
  S_LAN, S_LAN_HOST, S_LAN_JOIN, S_LAN_WAIT, S_LAN_READY, S_LAN_REFUSED,
  S_LAN_PICK, S_LAN_VS,
  S_LAN_GONE, S_LAN_WAITFOE, S_LAN_REMATCH,
  S_BTL_RUN,        // the third option in a fight: leave it
  S_EGG_REGION,     // hint under the egg's region pill
  S_WIN_TRAIN_FMT,  // "%s +%u" -- the training a gym win is worth
  S_WIN_MAXED,      // ... or that there is nowhere left to train
  S_CHOOSE_REGION,  // first boot: pick the region your starter comes from
  S_RETIRE,         // menu row: retire the creature on demand
  S_RETIRE_Q,       // ...and the confirmation
  S_RETIRE_COST,    // what retiring early costs the next creature
  S_EVO_SLOW,       // shown on the card while a creature carries that debt
  S_NEED_PACK,      // region chooser: this region's sprite pack is not on the SD
  // Letting a BANKED creature go for good, from the party or box detail sheet.
  // The confirmation itself reuses S_RELEASE_FMT / S_YES / S_NO, which the live
  // pet's own release dialog already owns -- the question is the same question.
  S_RELEASE_BTN,    // the button on the detail sheet
  S_RELEASE_GONE,   // ...and the warning under it: this one does not come back
  S_BOX_TAKE,       // box detail sheet: move this creature into the party
  S_RETIRE_GONE,    // retire confirm: an early retire is not banked either
  S_SET_VOLUME, S_SET_LANG, S_ABOUT,   // settings page titles
  S_SET_BRIGHT,     // settings page title
  S_TEST,           // volume page: play a beep at the chosen level
  // Fun Pack: the extra minigames and the statuses
  S_GAME_LR, S_GAME_HL, S_GAME_WHO, S_GAME_DROP, S_GAME_MOLE, S_GAME_ROPE, S_GAME_DANCE, S_GAME_HIDE, S_LR_HINT, S_HL_HINT, S_HIGHER, S_LOWER, S_WHO_HINT, S_DROP_HINT, S_MOLE_HINT, S_ROPE_HINT, S_DANCE_HINT, S_PERFECT, S_GOOD, S_MISS, S_HIDE_HINT, S_JOY_GAIN_FMT, S_DEF_GAIN_FMT, S_YES_GUESS, S_NO_GUESS, S_ST_STOMACH, S_ST_COLD, S_ST_GRUMPY, S_ST_BORED, S_ST_LIGHT, S_TOO_SICK, S_MED_CURED, S_MED_MORE, S_MED_WAIT, S_MED_BATH, S_MED_YUCK, S_MORE_GAMES, S_GAMES_TITLE,
  // Fun Pack: the extra minigames and the statuses
  S_GAME_BAG, S_GAME_REACT, S_GAME_BALL, S_CAT_FUN,
  // coins and daily missions
  S_COINS, S_MISSIONS_TITLE, S_MIS_FEED, S_MIS_FEED_FAV, S_MIS_PLAY, S_MIS_STEPS, S_MIS_BATH, S_MIS_PET, S_MIS_TRAIN_ATK, S_MIS_TRAIN_SPE, S_MIS_TRAIN_DEF, S_MIS_SCORE, S_COINS_TODAY_FMT, S_ALL_MISSIONS_FMT, S_EARN_TITLE, S_EARN_PLAY, S_EARN_STEPS_FMT, S_EARN_DAILY_FMT, S_EARN_MISSION_FMT, S_EARN_BADGE_FMT, S_EARN_BED_FMT, S_SHOP_SOON,
  // shop, bag and hats
  S_SHOP, S_BAG, S_HATS, S_HAT_CAP, S_HAT_BOW, S_HAT_FLOWER, S_HAT_BEANIE, S_HAT_PARTY, S_HAT_STRAW, S_HAT_WITCH, S_HAT_SANTA, S_HAT_TOPHAT, S_HAT_CROWN, S_BUY, S_CANCEL, S_OWNED, S_NOT_ENOUGH, S_BOUGHT, S_WEARING, S_TAKE_OFF, S_ADJUST, S_BAG_EMPTY, S_DONE,
  // toys and decorations
  S_TOYS, S_DECOR, S_ON_FLOOR, S_DEC_BALL, S_DEC_PLUSH, S_DEC_DUCK, S_DEC_BLOCKS, S_DEC_DRUM, S_DEC_KITE, S_DEC_PLANT, S_DEC_FLOWERS, S_DEC_LAMP, S_DEC_BED, S_DEC_GIFT, S_DEC_PUMPKIN,
  // foods
  S_FOOD_TITLE, S_MEALS, S_SWEETS, S_DRINKS, S_MEDICINE, S_FOOD_SANDWICH, S_FOOD_CURRY, S_FOOD_SOUP, S_FOOD_ONIGIRI, S_FOOD_CAKE, S_FOOD_COOKIE, S_FOOD_ICECREAM, S_FOOD_MILK, S_FOOD_JUICE, S_NONE_LEFT, S_COUNT_FMT,
  // bath and sleep menus
  S_BATH_TITLE, S_QUICK_WASH, S_BUBBLE_BATH, S_SCRUB, S_BRUSH, S_SCRUB_HINT, S_CLEAN_DONE, S_NEED_SOAP, S_NEED_BRUSH, S_SLEEP_TITLE, S_SLEEP_NOW, S_NAP, S_LULLABY, S_CARE, S_SOAP, S_BRUSH_ITEM,
  // places, garden, seasonal gifts
  S_MAP_TITLE, S_PLACE_PARK, S_PLACE_CENTER, S_PLACE_GARDEN, S_WALK, S_ENTER, S_LEAVE, S_HEAL, S_FREE_TODAY, S_HEALED, S_PARK_COINS_FMT, S_PARK_BERRY, S_PARK_FRIEND_FMT, S_PARK_NOTHING, S_TOO_TIRED, S_PLANT, S_HARVEST, S_NEED_SEED, S_GROWING_FMT, S_SEED, S_GOT_FOOD_FMT, S_GIFT_HALLOWEEN, S_GIFT_CHRISTMAS, S_GIFT_NEWYEAR, S_GIFT_VALENTINE,
  // capsules, scenery, trophies, teeth, school, quiz, parents, talk
  S_SCENERY, S_SCN_MEADOW, S_SCN_BEACH, S_SCN_FOREST, S_SCN_VOLCANO, S_SCN_MOUNTAIN, S_SCN_SNOW, S_SCN_GRAVEYARD, S_TROPHIES, S_TROPHY_NEW, S_TRO_MEALS, S_TRO_GAMES, S_TRO_BATHS, S_TRO_WALKS, S_TRO_HARVEST, S_TRO_COINS, S_TRO_HATS, S_TRO_DECOR, S_TRO_STREAK, S_TRO_LEVEL, S_TRO_STUDY, S_TRO_TEETH, S_TRO_PERFECT, S_TRO_CAPSULE, S_CAPSULE, S_CAP_LEFT_FMT, S_CAP_GOT_FMT, S_CAP_SOAP, S_CAP_SEED, S_CAP_COINS_FMT, S_CAP_LIMIT, S_TEETH, S_TEETH_DONE, S_TEETH_ENOUGH, S_TEETH_TAP, S_PLACE_SCHOOL, S_STUDY, S_QUIZ, S_STUDY_PICK, S_MIN_FMT, S_STUDYING, S_STUDY_DONE_FMT, S_STUDY_DONE, S_STOP, S_QUIZ_RESULT_FMT, S_SET_PARENT, S_PARENT_BED, S_PARENT_GAMES, S_OFF_LBL, S_PARENT_HINT, S_BLOCK_BED, S_BLOCK_LIMIT, S_TALK_HUNGRY, S_TALK_DIRTY, S_TALK_SLEEPY, S_TALK_SICK, S_TALK_PLAY, S_TALK_RAIN, S_TALK_SNOW, S_TALK_GARDEN, S_TALK_LOVE, S_TALK_HAPPY, S_TALK_STUDY,
  // adventures, lake, kitchen, chores, piggy bank, diary, alarm
  S_FOOD_PIE, S_FOOD_STEW, S_FOOD_PARFAIT, S_PLACE_LAKE, S_PLACE_ADVENTURE, S_PLACE_KITCHEN, S_FISH, S_FISH_WAIT, S_FISH_BITE, S_FISH_SOON, S_FISH_AWAY, S_FISH_GOT_FMT, S_ITEM_BOOT, S_ITEM_SHELL, S_ITEM_PEARL, S_FISH_LEFT_FMT, S_FISH_FUN, S_ROUTE_1, S_ROUTE_2, S_ROUTE_3, S_ROUTE_4, S_CLAIM, S_STEPS_TO_FMT, S_ADV_COINS_FMT, S_ADV_FOOD_FMT, S_ADV_DONE, S_ADV_HINT, S_COOK_MISSING, S_COOK_STIR, S_COOK_DONE_FMT, S_CHORES, S_PIGGY, S_DIARY, S_CHORE_BED, S_CHORE_HOMEWORK, S_CHORE_TIDY, S_CHORE_HELP, S_CHORE_READ, S_CHORE_TEETH, S_CHORE_THANKS, S_PIGGY_SAVED, S_TAKE_OUT, S_PIGGY_NEXT_FMT, S_PIGGY_HINT, S_DIARY_TODAY, S_DIARY_YESTERDAY, S_D_MEALS_FMT, S_D_GAMES_FMT, S_D_BATHS_FMT, S_D_WALKS_FMT, S_D_FISH_FMT, S_D_STUDY_FMT, S_D_CHORES_FMT, S_D_COINS_FMT, S_D_COOKED, S_D_ADVENTURE, S_D_TROPHY, S_D_RARE, S_D_EMPTY, S_SET_ALARM, S_ON_LBL, S_GOOD_MORNING, S_ALARM_STOP, S_BED_REMINDER, S_TRO_FISH, S_TRO_ROUTE,
  S_CENTER_SHORT,
  // phase 12 games
  S_GAME_MEMORY, S_GAME_SIMON, S_GAME_FLAPPY, S_GAME_MAZE, S_MEMORY_HINT, S_SIMON_HINT, S_SIMON_WATCH, S_FLAPPY_HINT, S_MAZE_HINT,
  // phase 13: hats and the room
  S_HAT_CHEF, S_HAT_PIRATE, S_HAT_CATEARS, S_HAT_BUNNY, S_HAT_HEADPHONES, S_HAT_GRAD, S_HAT_WREATH, S_HAT_HELMET, S_HAT_BERET, S_HAT_VIKING, S_FURNITURE, S_SCN_ROOM, S_FURN_WINDOW, S_FURN_CLOCK, S_FURN_PAINTING, S_FURN_SHELF, S_FURN_SOFA, S_FURN_TV, S_FURN_AQUARIUM, S_FURN_BOOKCASE, S_FURN_FIREPLACE, S_FURN_RUG, S_FURN_PAPER_BLUE, S_FURN_PAPER_PINK, S_FURN_PAPER_MINT,
  // phase 14: wishes, moods, birthdays, events, personality
  S_WISH_FOOD_FMT, S_WISH_PLAY, S_WISH_PARK, S_WISH_BATH, S_WISH_FISH, S_WISH_PET, S_WISH_DONE, S_ST_LONELY, S_ST_EXCITED, S_TALK_LONELY, S_HAPPY_BIRTHDAY, S_BIRTHDAY_FMT, S_EASTER_EGG, S_FESTIVAL_TREAT, S_PERSONALITY_FMT, S_PERS_PLAYFUL, S_PERS_LAZY, S_PERS_HUNGRY, S_PERS_SHY,
  // phase 15: the games menu and eight games
  S_GAMES, S_CAT_BRAIN, S_CAT_ARCADE, S_GAME_BALLOON, S_GAME_SHAKE, S_GAME_CATCH, S_GAME_BALANCE, S_GAME_QUIZ, S_GAME_SLIDE, S_GAME_BUBBLES, S_GAME_DRAW, S_BALLOON_HINT, S_SHAKE_HINT, S_CATCH_HINT, S_BALANCE_HINT, S_QUIZ_HINT, S_SLIDE_HINT, S_BUBBLES_HINT, S_DRAW_DONE,
  // phase 16: the wardrobe
  S_WARDROBE, S_ACC_SUNGLASSES, S_ACC_STARGLASSES, S_ACC_HEARTGLASSES, S_ACC_MONOCLE, S_ACC_SCARF, S_ACC_BOWTIE, S_ACC_MEDAL, S_ACC_BANDANA, S_ACC_CAPE, S_ACC_BACKPACK, S_ACC_WINGS, S_ACC_BATWINGS, S_ST_STYLISH,
  // phase 17: town life
  S_PLACE_BEACH, S_PLACE_LIBRARY, S_PLACE_MUSIC, S_PLACE_CONTEST, S_PLACE_ARCADE, S_PLACE_JOBS, S_CASTLE, S_SHELL_FOUND, S_SHELLS_DONE, S_CASTLE_DONE, S_CASTLE_AGAIN, S_READ, S_BOOKS_DONE, S_SONGS_DONE, S_SONG_1, S_SONG_2, S_SONG_3, S_ENTER_CONTEST, S_CONTEST_DONE, S_RIB_NONE, S_RIB_BRONZE, S_RIB_SILVER, S_RIB_GOLD, S_CONTEST_HINT, S_JOB_CAFE, S_JOB_FLOWERS, S_JOB_POST, S_JOB_INFO, S_AT_WORK, S_BACK_AT_FMT, S_JOB_DONE, S_JOB_TOMORROW, S_MAIL, S_NO_MAIL, S_LETTER_1, S_LETTER_2, S_LETTER_3, S_LETTER_4, S_LETTER_5, S_LETTER_6,
  // phase 19: quests and events
  S_QUEST, S_QUEST_1, S_QUEST_2, S_QUEST_3, S_QUEST_4, S_QK_PARK, S_QK_BEACH, S_QK_READ, S_QK_FISH, S_QK_COOK, S_QK_MEAL, S_QK_ADV, S_QK_CENTER, S_QK_BATH, S_QK_SONG, S_QK_GAME, S_QK_STUDY, S_QUEST_NEXT_DAY, S_QUEST_DONE, S_HALLOWEEN_TREAT, S_WINTER_TREAT, S_ANNIVERSARY,
  // the photo album
  S_ALBUM, S_NO_PHOTOS, S_PHOTO_SAVED, S_TAKE_PHOTO,
  // the Nursery
  S_PLACE_NURSERY, S_NUR_INFO, S_NUR_PICK, S_NUR_PICK_TITLE, S_NUR_PAY, S_NUR_CANT, S_NUR_DROPPED, S_NUR_WEEK, S_NUR_READY_IN, S_NUR_TAKE, S_NUR_GOT, S_NUR_NEXT, S_NUR_GIVE, S_NUR_SURE, S_NUR_HOME, S_EGG_BRED,
  // Wi-Fi
  S_WIFI_NONE, S_WIFI_BUSY, S_WIFI_OK, S_WIFI_FAIL,
  // snow from New York
  S_NY_SNOW, S_TALK_NY_SNOW, S_NY_SNOW_ON, S_NY_SNOW_OFF,
  // restful nights
  S_ST_RESTING,
  // the try-on mirror
  S_TRY_ON, S_MIRROR,
  // the arcade
  S_PRIZES, S_RECORDS, S_TICKETS, S_PRIZE_WON, S_NO_TICKETS, S_HAT_ARCADE, S_HAT_VISOR, S_DEC_CABINET, S_WEEK, S_BEST, S_BADGE_1, S_BADGE_2, S_BADGE_3,
  // arcade trophies
  S_TRO_GOLD, S_TRO_PRIZES,
  // what it says (phase 29)
  S_TK_HUNGRY_1, S_TK_HUNGRY_2, S_TK_HUNGRY_3, S_TK_HUNGRY_4, S_TK_HUNGRY_5, S_TK_HUNGRY_6,
  S_TK_DIRTY_1, S_TK_DIRTY_2, S_TK_DIRTY_3, S_TK_DIRTY_4, S_TK_DIRTY_5, S_TK_SLEEPY_1,
  S_TK_SLEEPY_2, S_TK_SLEEPY_3, S_TK_SLEEPY_4, S_TK_SLEEPY_5, S_TK_SICK_1, S_TK_SICK_2,
  S_TK_SICK_3, S_TK_SICK_4, S_TK_SICK_5, S_TK_LONELY_1, S_TK_LONELY_2, S_TK_LONELY_3,
  S_TK_LONELY_4, S_TK_LONELY_5, S_TK_BORED_1, S_TK_BORED_2, S_TK_BORED_3, S_TK_BORED_4,
  S_TK_BORED_5, S_TK_HAPPY_1, S_TK_HAPPY_2, S_TK_HAPPY_3, S_TK_HAPPY_4, S_TK_HAPPY_5,
  S_TK_HAPPY_6, S_TK_HAPPY_7, S_TK_HAPPY_8, S_TK_HAPPY_9, S_TK_HAPPY_10, S_TK_HAPPY_11,
  S_TK_HAPPY_12, S_TK_LOVE_1, S_TK_LOVE_2, S_TK_LOVE_3, S_TK_LOVE_4, S_TK_LOVE_5,
  S_TK_LOVE_6, S_TK_LOVE_7, S_TK_LOVE_8, S_TK_MORNING_1, S_TK_MORNING_2, S_TK_MORNING_3,
  S_TK_MORNING_4, S_TK_MORNING_5, S_TK_AFTERNOON_1, S_TK_AFTERNOON_2, S_TK_AFTERNOON_3, S_TK_AFTERNOON_4,
  S_TK_EVENING_1, S_TK_EVENING_2, S_TK_EVENING_3, S_TK_EVENING_4, S_TK_EVENING_5, S_TK_NIGHT_1,
  S_TK_NIGHT_2, S_TK_NIGHT_3, S_TK_SUN_1, S_TK_SUN_2, S_TK_SUN_3, S_TK_SUN_4,
  S_TK_CLOUD_1, S_TK_CLOUD_2, S_TK_CLOUD_3, S_TK_RAIN_1, S_TK_RAIN_2, S_TK_RAIN_3,
  S_TK_RAIN_4, S_TK_STORM_1, S_TK_STORM_2, S_TK_STORM_3, S_TK_FOG_1, S_TK_FOG_2,
  S_TK_FOG_3, S_TK_SNOW_1, S_TK_SNOW_2, S_TK_SNOW_3, S_TK_SNOW_4, S_TK_NYSNOW_1,
  S_TK_NYSNOW_2, S_TK_HOT_1, S_TK_HOT_2, S_TK_HOT_3, S_TK_COLD_1, S_TK_COLD_2,
  S_TK_COLD_3, S_TK_GARDEN_1, S_TK_GARDEN_2, S_TK_GARDEN_3, S_TK_STUDY_1, S_TK_STUDY_2,
  S_TK_STUDY_3, S_TK_ARCADE_1, S_TK_ARCADE_2, S_TK_ARCADE_3, S_TK_ARCADE_4, S_TK_NURSERY_1,
  S_TK_NURSERY_2, S_TK_MAIL_1, S_TK_MAIL_2, S_TK_QUEST_1, S_TK_QUEST_2, S_TK_JOB_1,
  S_TK_JOB_2, S_TK_BDAY_1, S_TK_BDAY_2, S_TK_BDAY_3, S_TK_HALLOWEEN_1, S_TK_HALLOWEEN_2,
  S_TK_HALLOWEEN_3, S_TK_WINTER_1, S_TK_WINTER_2, S_TK_WINTER_3, S_TK_EASTER_1, S_TK_EASTER_2,
  S_TK_SUMMER_1, S_TK_SUMMER_2, S_TK_BOND_1, S_TK_BOND_2, S_TK_BOND_3, S_TK_BOND_4,
  S_TK_EVOLVE_1, S_TK_EVOLVE_2,
  // phase 31: more missions and quests
  S_MIS_BUBBLE, S_MIS_TEETH, S_MIS_NAP, S_MIS_HOMEMEAL, S_MIS_TICKETS, S_MIS_CAPSULE, S_MIS_QUIZ, S_MIS_CONTEST, S_MIS_PARK, S_MIS_ADV, S_MIS_FISH, S_MIS_BEACH, S_MIS_CENTER, S_MIS_COOK, S_MIS_GARDEN, S_MIS_CHORES, S_MIS_READ, S_MIS_STUDY, S_MIS_SONG, S_MIS_MAIL, S_MIS_PIGGY, S_MIS_BUY_FOOD, S_MIS_SHOP, S_MIS_DECOR, S_QK_CHORE, S_QK_SHOP, S_QK_TICKETS, S_QK_STEPS, S_QUEST_5, S_QUEST_6, S_QUEST_7, S_QUEST_8, S_QUEST_9, S_QUEST_10, S_QUEST_11, S_QUEST_12,
  // phase 32: the broom
  S_SWEEP, S_NOTHING_SWEEP, S_SWEEP_TOO, S_CLEAN_GAIN_FMT, S_SWEPT,
  // phase 32b: what the broom cleans
  S_SWEPT_FMT,
  // phase 33: the album
  S_DEL_PHOTO_Q, S_DELETE, S_DELETE_ALL, S_DEL_ALL_Q, S_PHOTO_DELETED, S_FAV_FULL_FMT,
  // phase 34: the road to the gym
  S_GYM_CHALLENGE_FMT, S_GYM_BADGE_FMT, S_GYM_ELITE_FMT, S_GYM_CHAMPION, S_FIGHT_NOW, S_LATER,
  // phase 35: the steps panel
  S_STEPS_TITLE, S_STEPS_TODAY, S_STEPS_WEEK, S_STEPS_TO_GYM, S_ADV_ROUTE, S_STEPS_TOTAL_FMT, S_READY_FIGHT, S_DAY_LETTERS, S_ALL_GYMS_DONE,
  // phase 36: a gym a week
  S_GYM_WAIT_FMT, S_GYM_WAIT_1,
  // phase 36: a gym a week
  S_GYM_WAIT_SHORT_FMT, S_GYM_WAIT_SHORT_1,
  // phase 40: the walking prizes
  S_WALK_PRIZE_FMT, S_STEPS_PRIZE_FMT,
  // phase 43: daily step goals and lifetime ranks
  S_GOAL_SNACK, S_GOAL_WALK, S_GOAL_STRONG, S_RANK_0, S_RANK_1, S_RANK_2, S_RANK_3, S_RANK_4, S_RANK_5, S_RANK_6, S_RANK_7, S_RANK_8,
  // phase 44: wild encounters and catching
  S_PLACE_GRASS, S_BTL_BALLS, S_BALL_POKE, S_BALL_GREAT, S_BALL_ULTRA, S_WILD_APPEARS, S_WILD_CAUGHT, S_WILD_TO_BOX, S_WILD_ESCAPED, S_WILD_FULL, S_WILD_NO_BALLS, S_WILD_DROP, S_WILD_EARNED, S_WILD_WALK_FMT, S_WILD_ASLEEP, S_WILD_NONE, S_BALL_USE,
  // phase 45: the Pokemon trip
  S_PLACE_TRIP, S_ON_TRIP, S_TRIP_BACK, S_TRIP_SET_OFF, S_TRIP_NONE_LEFT, S_TRIP_LATE, S_TRIP_CALL, S_TRIP_CALL_Q, S_TALE_COINS_A, S_TALE_COINS_B, S_TALE_BALL_A, S_TALE_BALL_B, S_TALE_FRIEND_A, S_TALE_FRIEND_B, S_TALE_COOKIE_A, S_TALE_COOKIE_B, S_TALE_SEED_A, S_TALE_SEED_B, S_TALE_SOAP_A, S_TALE_SOAP_B, S_TALE_GRASS_A, S_TALE_GRASS_B, S_TALE_GREAT_A, S_TALE_GREAT_B,
  STR_COUNT
};

const char *T(StrId id);       // text in the active language
const char *medalName(int i);  // medal banner (MED_COUNT)
const char *medalLabel(int i); // short medal label
const char *medalDesc(int i);  // long medal description

void loadLang();             // reads the language from NVS (call in setup)
void setLang(Lang l);        // changes and persists the language
