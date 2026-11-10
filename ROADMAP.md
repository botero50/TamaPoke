# Fun Pack roadmap

What is left, in the agreed order. Each phase gets a short written plan that is
approved before it is built; small fixes skip that. This file is updated at the
end of every phase.

_Last updated: 6 November 2026, with the battle fixes._

## Done
| Phase | What | Release |
|---|---|---|
| 1-20 | Minigames, sickness and statuses, care, shop, hats, room, outfits, town life, quests, events, photo album, music | v1.0-v1.5 |
| 21 | The Nursery: two favourites leave an egg for the next raising | v1.6 |
| 22 | Wi-Fi time sync and the real weather (Open-Meteo) | v1.7 |
| 23 | Weather polish: fog, thunder, snow on the ground, puddles, snow from New York, 30-minute refresh, Wi-Fi diagnostics | v1.7 |
| 24 | Restful nights (asleep 20:00-08:00 the bars hold; bedtime from 21:00; an unplugged night counts as sleep) and the try-on mirror in the shop | v1.8 |
| 25 | Show it to the world: the posts (published by the owner), social preview image, new tour GIF, repo description, a short README with "Build your own" and the screens | done |
| 26 | Arcade night: tickets from the Fun Pack games (30 a day), a prize counter (arcade cap, star visor, arcade cabinet, capsule, cake), the weekly records board, bronze/silver/gold badges per game; plus a Wi-Fi name typed in any letter case still connects | v1.9 |
| 27 | Trophies past 16 (now 18): "10 gold badges" and "All 3 arcade prizes" | v1.10 |
| 28 | The places come alive (the Center's healing machine and an original jingle, plus an animation in the garden, beach, library, contest, jobs, adventure; idle life in the park, store, school, music hall), the map as a fifth bottom button, a slimmer steps gauge | v1.11 |
| 29 | A talkative Pokemon (147 phrases in 35 groups, 8 languages, never one of the last six) and honest weather (rain needs 0.3 mm really falling, drizzle is never rain, grey skies need 60% cloud, no pretend rain with Wi-Fi, a stale sky clears after 75 min) | v1.12 |
| 30 | The quick wash is a real shower (water, no bubbles); the bubble bath keeps the bubble game; food with none in the bag is bought and eaten in one tap from the food menu | v1.13 |
| 31 | Six daily missions from 36 kinds (care, play, outdoors, home, shop: one of each, plus one), only ones she can do today; two tabs (MISIONES DE HOY / AVENTURA) on the missions panel; 12 weekly stories of 4 steps; the Center step of Chansey's story now counts | v1.14 |
| 32 | Levels of clean (quick shower +35, sponge +65, bubble bath to 100) with a toast of what it did; washes leave the poop, and a broom sweeps it away (its own row, a 2 s animation); the bath menu gets six compact rows | v1.15 |
| 32b | The shop and bag categories come in 2 pages of 4 (arrows) instead of 7 squeezed rows; seeds get their own aisle, HUERTO; the garden sells a seed on its button when there is none | v1.16 |
| 32c | Sweeping adds +10 CLEAN per pile, and the toast says how much | v1.17 |
| 33 | The photo album: a star keeps a favourite (up to 20), the bin deletes one or all but the favourites, photos are saved as BMPs a computer opens (old ones converted once); still at most 24 photos, about 2.6 MB on the SD card | v1.18 |
| 34 | The road to the gym: walking fills a step ring at the right edge; at 1000 + 500 x the leader's place a gold badge button appears, opening a challenge card (FIGHT -> team picker); the ladder shows the steps still to walk. Rematches and hard stay free | v1.19 |
| 35 | Honest steps: no free first gym challenge (gymV resets the road to 0 once); a gold gym dial at the right edge; the step plate and the dial open a PASOS panel (today, last 7 days, road to the gym with FIGHT when ready, adventure route, lifetime total); STATS prints steps= | v1.20 |
| 36 | A gym a week: 10 000 + 2 000 steps per place, at most one new badge every 7 days, and the gym road stops at its leader (a win empties it); a clock on the dial and "challenge in N days" while the week runs. Steps from a background reader every 10 ms instead of one sample a frame: a counted walk went from 73 to 105 of 100, and 0 at rest | v1.20 |
| 36b | One I2C lock for the step reader, touch, RTC and PMU (the task garbled the clock and USB reads, so "today" jumped and reset the step count); RTC reads that jump are double-checked; a clock glitch never resets today or wipes the 7 days; STATS shows clockjumps= and dropped= | v1.21 |
| 37 | Menus that fit the round screen: chores and bath in pages with arrows, the adventure note and missions footer off the edge, DIARY / ALBUM tabs instead of a button over the title; the PASOS panel titles its gym section "Steps to reach the gym" | v1.22 |
| 38 | Bigger page arrows (a near miss turns the page instead of closing the menu), the tasks bubble moved onto the glass, a Pokéball team button at the left edge, the bath as six tiles on one page; a README with two GIFs and the new features | v1.23 |
| 39 | A sturdier ball, from the fork review: the date never goes back after a power cut, safety copies of the save on the SD card to recover a save cut off midway, nothing drawn with the screen off, a sound task that cannot spin | v1.24 |
| 40 | Steps that cannot be lost: a clock change never erases them, and they are saved every 20 steps. A warning beep when a need is very low, quiet nights from 21:00 to 07:00, and coins for lifetime steps | v1.25 |
| 46b | Fixes from a player report: the win tune no longer plays on after a fight (tapping out stops it), and frames keep 55 ms clear after a slow one so scrubbing no longer flickers | v1.33 |
| 46 | Four new games drawn in code: **Merge** (slide numbers that count up), **Brick Bash** (paddle by finger or tilt, trains defence), **Marble Roll** (tilt her Pokemon around a tray, trains strength) and a hidden **Eevee Run** (tap the version on the About page five times). Swing the ball to throw a Pokemon ball (tap still works), and shake it on the main screen to play, five times a day | v1.32 |
| 45b | Battle fixes from the first real fights: the whole turn is shown (messages page 4 lines at a time, and a foe's reply can no longer be hidden), a wild foe always has an attack, her HP bar starts at her real health, and **damage stays until the Pokemon Center**: each Pokemon keeps its health, a fainted one sits out, the Center heals everyone for free, and the team screen shows a health bar | v1.31 |
| 45 | The Pokemon trip: twice a day it can go off for 30 to 60 minutes with its needs paused, and comes home with one of 8 stories and a small reward. A Call back button brings it home early, with no story | v1.30 |
| 44 | Wild encounters and catching: a Tall Grass tile on the map, one free encounter a day plus one per 500 steps, three kinds of ball in the shop and bag, and caught Pokemon join the team or box and fill the Pokedex | v1.29 |
| 43 | Walking rewards: 2 500 steps a day give a snack, 5 000 joy, energy and bond, 10 000 a training reward, each once a day. A rank for lifetime steps, from Beginner to Legend, on the steps panel and in the prize toast | v1.28 |
| 42 | Back up, then install: the installer checks each backup is complete before it shows Install. Sprite uploads resume after a cut. Fixes from the fork review: timers safe when `millis()` wraps after 49.7 days, damaged sprite and thumbnail files can't freeze or crash the game, a harder `PUT`, taps outside a grid, and a LAN battle no longer drops a slow player (both balls need this version) | v1.27 |
| 41 | Gym arenas from tubiana's fork: gym, Elite 4 and Champion fights in the leader's own arena (8 scenes). Each pile of poop takes 2 CLEAN per tick, not 4. A marked X at the top centre is the only way out of a game: taps elsewhere on the strip and swipes no longer quit | v1.26 |

## Next
The order agreed from the fork review. Each phase still gets its own short plan.
| Phase | What | Notes |
|---|---|---|
| 44b | Potions, training items and choosing which caught Pokemon to raise | Changes how saved Pokemon are stored, so its own phase and tests |
| 47 | New training games | Replaces bag, speed and defence training, so compare first |
| 48 | Jukebox | Prefer public-domain tunes; decide then |
| 49 | Three save slots on the SD card | E1d0k4 |
| 50 | Kids' manual to print | Last, so it describes the finished game |

Also open:
| # | Item | Needs | Notes |
|---|---|---|---|
| 1 | **Checks on the real ball** | the family | Nursery sprites and the next-day egg; SD photo (DIARY -> ALBUM -> PHOTO); tilt direction in the maze, Catch and Balance; how hard the shake race is; New York snow on its first snowy day; a hat in the mirror sits as it does once bought; the arcade prizes and hats on a Venusaur. (Wi-Fi sync and weather: confirmed working on 2 Nov.) |
| 2 | **Anything that comes up while playing** | the family | Fixed as small fixes; bigger ideas get a plan first |

## Later
| Item | Needs | Notes |
|---|---|---|
| **Friend visits and trading** | a second ball | "Not for now" (2 Nov). Would use the existing ESP-NOW battle link |
| **Battery saver** | a battery fitted | The ball reports no battery now (`bat=-1`) |
| **SD content packs** | only if space runs out | Firmware at 84%; revisit near 90% |
| **Translations review** | native speakers | FR, DE, IT, PT, ZH and KO were machine-written |

## Decisions on record
- **Weather:** Open-Meteo (free, no key). The location comes from the browser, rounded
  to two decimals (~1 km). Refresh every 30 minutes with a place, every 6 hours without.
  Weather older than 75 minutes is dropped (phase 29; was 3 hours). Rain, snow and storms
  need at least 0.3 mm falling, drizzle is never drawn as rain, grey skies need 60% cloud:
  forecast models flagged drizzle over a sunny house. With Wi-Fi set up, the old made-up
  weather never shows; without Wi-Fi it rains in 1-2 hour spells, not all day.
- **Steps:** a clock change never erases them; only Wi-Fi may move the step day back, numbers kept. Saved every 20 steps, not every step, to spare the flash: an unplug loses at most 19.
- **Quiet nights:** decided by the clock and the last touch (a minute), not per sound. Thunder is silent at night too. The alarm uses music, so it always rings.
- **Clock jumps** from a Wi-Fi sync apply without replaying the missing time on the pet.
- **The Wi-Fi password** lives in its own NVS namespace and never goes into a backup.
- **Snow from New York:** only on days that are clear or cloudy at home; anything real
  at home wins. The temperature stays local. Switch on Settings -> About and the installer.
- **The Nursery** copies both parents at drop-off, so releasing one later changes nothing.
- **Restful nights:** asleep between 20:00 and 08:00, joy and hygiene hold and food creeps
  to a floor of 60; daytime naps keep the slow drain. With the screen off it goes to bed
  by itself from 21:00. A ball switched off overnight counts the night as sleep.
- **The mirror** is RAM only: trying on never changes what the pet wears or the save.
- **The arcade:** tickets only from the 20 Fun Pack games (1 + score/20, at most 5, +3 for a
  record, 30 a day). The prizes are never sold, nor handed out by capsules or quests. Badges
  are worked out from the records (1x, 2x, 3x each game's "good score"), so they need no save.
  The cabinet is a decoration beside the pet, not room furniture, and there is no trophy for
  gold badges: the trophy list is full at 16.
- **The thunder sound stays** as it is (the owner likes the sound effects), with no setting.
  It is already silent at night and while the pet sleeps.
- **The cabinet in the room:** decorations are drawn over any background, so with the room
  scenery picked the cabinet stands beside the pet in the room. No furniture slot needed.
- **Trophies** are 32 bits since phase 27: the low 16 in `trph`, the rest in `trphX`.
- **Posting:** crenberries428's repo has issues and discussions off, so the thank-you note
  goes on their MakerWorld case page. Printables dropped: the case is only on MakerWorld.
- **Missions** are rolled once when the day starts and saved (`mskd`, `mskdy`), so one that
  opens at noon (a letter, coins) waits for tomorrow. 6 coins each, 12 for all six: 48 on a
  perfect day, outside the 50-coin activity cap. An old save rolls today's six fresh.
- **Stories** never use the contest (once a week, it could leave a story stuck). A story
  finished under the old 3 steps stays finished (`qver`).
- **Washing and sweeping are separate** since phase 32: a wash never removes poop, and a
  wash with poop on the floor says to sweep it too (poop keeps draining CLEAN).
- **Photos** live on the SD card only (`/album/NN.bmp`), at most 24 (about 2.6 MB); the
  ball keeps a 96-byte slot table in NVS `album`. A new photo takes an empty slot, else the
  oldest non-favourite; favourites are capped at 20 so a slot is always free.
- **Gym steps** (`gymS`) count every step, cap at 60 000, and are spent only by a first easy
  win; the challenge is the pet's own region first, then the first unfinished ladder. Old
  saves start with their next leader ready.
- **Steps:** today (`stps`) resets at midnight; the 7-day history (`sth7`/`sthd`), the
  lifetime total (`stpT`), the adventure (`advS`) and the gym road (`gymS`) carry on.
- **The step detector** gets every accelerometer sample from a small task that polls the
  QMI8658 data registers every 10 ms and queues them with their time; one sample per render
  frame missed about a quarter of real steps. The chip's FIFO was tried and rejected: on
  this board it returned garbage (0..2.5 g at rest, phantom steps) and each read took ~1 s.
  Serial IMULOG shows 4 s of samples, their spread and the steps they make.
- **The I2C bus** is shared by the step-reader task and the loop (touch, RTC, PMU): every
  user holds `i2cLock()` (imu.cpp). Without it the RTC read 0, January 2026 and the right
  time within seconds.
- **List menus** end by y 376 (`PANEL_Y + 300`): the panel's corners and the round screen cut
  anything lower. `static_assert`s beside the layouts keep it that way; longer lists page.
