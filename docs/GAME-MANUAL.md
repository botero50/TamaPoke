# TamaPoke Fun Pack: game manual

Every rule and number, for players who want the details. Back to the [README](../README.md).

## Screens

All shots are straight off the 466x466 round panel, rendered headlessly by the
emulator (`tools/emu/tamapoke-emu --shot <name>`), so they are exactly what the
hardware draws.

### Starting out

| Pick a region | ...then its starter | Johto's three |
|---|---|---|
| <img src="screens/region.png" width="240"> | <img src="screens/starter.png" width="240"> | <img src="screens/starterj.png" width="240"> |

A new game asks which region you are playing before it asks which creature you
want. The choice sets both: you pick from that region's three starters, and it
becomes where your eggs come from afterwards (changeable later on the egg's
region pill). Existing saves never see this screen -- it only appears when the
Pokedex is empty.

### Raising one

| Your creature | Its egg | Moves it knows |
|---|---|---|
| <img src="screens/main.png" width="240"> | <img src="screens/egg.png" width="240"> | <img src="screens/moves.png" width="240"> |

The egg carries a **region pill** — pick whether it hatches from Kanto, Johto,
Hoenn or all three. Switching keeps the rarity it was granted and remembers each
region's answer, so it cannot be flipped to farm a legendary.

**Steps.** The board's QMI8658 accelerometer counts your steps (`steps.cpp`, in
any orientation, screen on or off) and the home screen shows today's total on a
plate curved along the left edge, digits stacked down it. The arc on the rim fills
toward 10,000 and turns gold when you get there. The count resets at midnight by
the RTC. It is a counter only: it changes nothing in the game. The threshold is
tuned against synthetic gait in `steps_test`, not yet against a person walking.

### Day, dusk, night

| Day | Sunset | Night | Steps |
|---|---|---|---|
| <img src="screens/sky_day.png" width="180"> | <img src="screens/sky_sunset.png" width="180"> | <img src="screens/sky_night.png" width="180"> | <img src="screens/steps.png" width="180"> |

The sky follows the clock: a haloed sun and drifting clouds by day, a crescent
moon and twinkling stars at night. The last shot shows the step plate on the left
edge. The boot title card is `docs/screens/splash.png`.

### Battling

| The fight | Choosing a team | Winning |
|---|---|---|
| <img src="screens/btlmenu.png" width="240"> | <img src="screens/pick.png" width="240"> | <img src="screens/win.png" width="240"> |

Turn- and move-based, with the real type chart, ailments and STAB. Real Game Boy
battle music plays throughout.

### Five regions

| Pick a ladder | Johto's gyms | LAN battle |
|---|---|---|
| <img src="screens/gympick.png" width="240"> | <img src="screens/gymsj.png" width="240"> | <img src="screens/lanready.png" width="240"> |

Kanto, Johto, Hoenn, Sinnoh and Unova each have eight leaders, an Elite 4 and a
champion, on easy and hard. Four of the five are the games' own teams, checked
against the pokecrystal, pokeemerald and pokeplatinum disassemblies -- **0
differences across all 39 trainers**, re-checkable with
`tools/verify_rosters.py`.

**Unova is the exception and says so.** pret has no Gen 5 disassembly, so that
ladder is written from knowledge rather than from the game's own tables;
`verify_rosters.py` prints it as NOT VERIFIED rather than letting a clean run
imply otherwise. It follows Black 2 / White 2, which unlike Black/White has no
version- or starter-dependent leaders.

Sinnoh follows **Platinum**, where Fantina is the *third* gym rather than
Diamond/Pearl's fifth; the level ramp only runs 14/22/26/32/37/41/44/50 that
way. Same reasoning that makes Hoenn Emerald throughout.

### Collecting

| Pick a region | Kanto | Johto |
|---|---|---|
| <img src="screens/dexpick.png" width="240"> | <img src="screens/gallery.png" width="240"> | <img src="screens/gallery2.png" width="240"> |

| Trainer card | Johto badges | The box |
|---|---|---|
| <img src="screens/player.png" width="240"> | <img src="screens/player2.png" width="240"> | <img src="screens/box.png" width="240"> |

### Settings

Tap the name, then **Settings**. It is five swipeable pages (Time, Volume, Brightness, Language, About); swipe
left/right to move between them, and the dots show where you are. Only the clock
needs **OK** to apply; sound, brightness and language take effect the moment you
touch them.

| Time | Volume | Brightness |
|---|---|---|
| <img src="screens/set_time.png" width="240"> | <img src="screens/set_volume.png" width="240"> | <img src="screens/set_bright.png" width="240"> |
| Set the hour and minute | Sound on/off, a draggable 0–100 volume slider, and a TEST beep at the chosen level | Screen brightness 1–10; sleep and idle dimming only ever go below it |

| Language | Chinese UI | |
|---|---|---|
| <img src="screens/set_lang.png" width="240"> | <img src="screens/set_lang_zh.png" width="240"> | The About page shows the firmware version |

## Game manual (the actual numbers)

A quick reference to how the game really works (values straight from the code).

### Time & leveling
- **1 real minute = 1 in-game minute.** Your Pokémon gains **+1 level every hour**
  of real time. Leveling is purely time-based — caring well doesn't speed it up,
  but neglect *delays evolution*.
- **Level caps at 100**, reached at 4 days 3 hours. Farewell is only *offered* at
  3 days (level 73) — **not a deadline**: your Pokémon is still growing, and
  declining it to reach 100 is a real choice. Declining re-offers a day later.
- It keeps **aging while powered off** (the RTC runs), catching up to **2 weeks** max.

### The four stats (0–100)
Needs: **FOOD**, **JOY**, **ENE** (energy), **HYG** (hygiene). Start 80 / 80 / 80 / 100.
While **awake**, per minute:

| Stat | Drain/min | Notes |
|---|---|---|
| FOOD | −2 | |
| ENE | −1 | −1 extra if overweight (weight > 50 → sluggish) |
| HYG | −1 | **−4 more per poop** on screen (max 3 poops) |
| JOY | −1 | **−2 extra** if FOOD < 30, **−2 extra** if HYG < 30 |

- ~**15 %/min** chance to poop (only if FOOD > 40). Poops tank hygiene fast.
- **Care slip-up** = letting any stat hit **≤ 10** (60-min cooldown so it counts once).
  Each slip-up **delays evolution by 1 level** and cools the bond by 1.

### Actions
- 🍎 **Berry** (3 flavors): +25 FOOD. Each species has a **hidden favorite flavor**
  → +35 FOOD, +10 JOY, ♥, bond, and it gets revealed.
- 🍬 **Candy:** +10 FOOD, +12 JOY, but **+12 weight** (fattening).
- ⚽ **Play / minigame:** **+5 JOY, plus 2 per rally** (max +35), −ENE, burns
  weight. Pure happiness — it trains nothing, so playing is never a stat grind.
  Leaving early keeps what you earned.
- 🎯 **Reaction test:** a target appears, tap it before it shrinks away. Trains
  **SPEED**; the window tightens as you go.
- 🥊 **Training bag:** trains **STRENGTH** (~4 hits = 1 pt, cap +18/session), tires it.
- 🫧 **Bath:** clears poops, HYG → 100.
- 👆 **Pet it:** +5 JOY + bond.
- 🌙 **Sleep:** rest — ENE **+6/min**, needs drain ~**4× slower** with floors
  (FOOD 30 / JOY 35 / HYG 45). No poops, no slip-ups, can't run away while asleep.
  **Screen off + between midnight and 06:00 puts it to sleep**, and it stays
  asleep until you turn the screen back on -- it wakes when *you* do, not at a
  fixed hour. Re-checked every minute, so a device put down at 23:00 nods off
  when midnight comes. The
  evening is deliberately outside the window -- a six-hour night is the only part
  of the day you are not expected to be around. The light
  button beats both: a creature you sent to bed stays there.

### Fun Pack: more minigames
TRAIN shows four categories -- **STRENGTH**, **SPEED**, **DEFENCE** and **FUN** --
and each opens a list of its games, the original trainer first: STRENGTH = bag,
Mole Pop; SPEED = reaction test, Berry Drop, Jump Rope; DEFENCE = ball game,
Who is it?; FUN = Dance, Hide & Seek, Left / Right, Higher / Lower. Each of the
eight new games keeps its own record.
Tapping the top strip quits and keeps what was earned (a swipe works too in
the four guessing games). Every game cures **bored** and **grumpy**.

| Game | How it plays | Reward |
|---|---|---|
| Left / Right | guess which way it turns; 3 lives | JOY 4 + 2/pt (max +20), bond 2 |
| Higher / Lower | cards 1–9: is the next one higher? 3 lives | JOY 4 + 2/pt (max +20), bond 2 |
| Who is it? | name the silhouette before the timer (5 s → 2.6 s) | DEF score/3 (max +8), JOY |
| Berry Drop | 5 lanes, 40 s; rocks cost a life | SPE score/4 (max +10), FOOD score/3 (max +15) |
| Mole Pop | 25 s; Diglett yes, Voltorb no | ATK score/3 (max +12) |
| Jump Rope | tap as the rope reaches the ground; speeds up | SPE score/3 (max +12), burns 2 weight/pt |
| Dance | 48 notes; ±90 ms = 2 pts, ±180 ms = 1 pt; 5 lives | JOY 6 + 2/pt (max +30), bond 3 |
| Hide & Seek | follow the bush: 3 → 11 swaps, faster each round | JOY 4 + 2/pt (max +20), bond 2 |

Stat gains still stop at the IV ceiling (`trMaxFor`), like the trainers.

### Fun Pack: sickness and statuses
- 🤢 **Stomachache:** feeding at FOOD ≥ 90 adds 1 to an overeating count (candy
  adds 2); at **3** it is sick. The count drops by 1 every 30 min.
- 🤧 **Cold:** 3 poops or HYG < 15 for **45 min in a row**.
- **While sick:** −1 FOOD and −1 JOY extra per minute, sad, a germ beside it,
  and TRAIN, the minigames and gym fights are refused. Every **6 h** left sick
  is one care mistake. Never fatal on its own.
- 💊 **Medicine** (MEDICINE row of the food menu): a stomachache needs **1 dose**;
  a cold needs **2 doses at least 10 min apart, then a bath**. Healthy, it just
  tastes bad: −10 JOY.
- 💡 **Lights left on:** from **21:00 to 06:00**, awake, on the main screen and
  untouched for **20 min** with the screen on, it dozes off **with the light on**
  (ENE only +3/min). The light button turns the light off and it sleeps on; the
  next press wakes it. The power button turning the screen off also counts as
  switching the light off.
- 😠 **Grumpy:** woken after **30+ min** asleep with the light on: −15 JOY, ENE
  capped at 70, and minigame JOY is halved until it plays once.
- 😐 **Bored:** **8 h** awake without any minigame: −1 JOY extra per minute.

### Coins and daily missions
Two round buttons at the top of the main screen: **missions** (left, with a red
count of the ones still open) and the **purse** (right, with your coins), which opens SHOP / BAG / HOW TO EARN. Coins
are the player's, so a new egg keeps them. They will buy things in the shop.

| Source | Coins |
|---|---|
| Any game | 1 + score/10, at most 5 a game |
| Feed, pet, bath | 1 each |
| *Games + care together* | **at most 50 a day** |
| Every 1 000 steps | 2 (up to 10 000 steps a day) |
| First care of the day | 5, plus 15 on every 7th day of the streak |
| Each daily mission | 8, plus 10 for finishing all three |
| A badge won for the first time | 15 |
| Sent to bed with the light off, 21:00-06:00 | 3, once a night |

A good day is about 60-80 coins (at most ~110): a cap or a bow in a day or two,
the crown in about a week.

Three missions a day, the same all day (picked from the date): feed it 3 times,
its favourite berry, play 2 games, walk 2 000 steps, a bath, pet it 3 times, a
strength / speed / defence game, or score 10 in one game.

### Shop, bag and hats
The **shop** and the **bag** open on their categories -- HATS, TOYS, DECORATION -- like TRAIN does. The shop sells ten hats; a hat you buy goes straight on. The **bag** lists the
hats you own (tap one to wear it or take it off), plus **MOVE HAT**: four arrows
on the main screen nudge it 4 px at a time, up to 60 px each way.

| Hat | Coins |
|---|---|
| Cap, Bow | 80 |
| Flower | 100 |
| Beanie | 120 |
| Party hat, Straw hat | 150 |
| Witch hat, Santa hat | 200 |
| Top hat | 300 |
| Crown | 500 |

The hat sits on the first row of the sprite that is nearly as solid as its widest
row, so ear tips and antennae do not lift it into the air; it follows every
animation frame. A nudge belongs to the species: after an evolution or a new
egg the hat starts centred again. Hats are the player's -- a new egg keeps them.

### Toys and decorations
Bought in the shop, they stand on the main screen's floor, one each side of the
creature. In the bag, tapping one puts it in the first free spot (left, then
right; with both full it replaces the left one) or picks it up again.

| Toys | Coins | Decorations | Coins |
|---|---|---|---|
| Beach ball | 40 | Plant | 50 |
| Plush | 90 | Flowers | 60 |
| Rubber duck | 60 | Lamp | 90 |
| Blocks | 70 | Bed | 150 |
| Drum | 110 | Gift | 80 |
| Kite | 120 | Pumpkin | 70 |

- **A toy out** keeps it company: boredom builds **half as fast**, and now and
  then it walks over to play (the toy bounces while it is next to it).
- **The bed** helps it sleep: ENE **+7/min** instead of +6 with the light off.
- Everything else is decoration. Like the hats, they are the player's.

### Food menu and foods
The food button opens the **food menu**: the free berries and the candy on the
first row (one tap feeds, as before), then **MEALS**, **SWEETS** and **DRINKS**,
which list the servings in the bag, and **MEDICINE**. Servings are bought in the
shop (FOOD category) or eaten from the bag. A food with none left shows its price.

| Food | Coins | FOOD | JOY | ENE | Weight |
|---|---|---|---|---|---|
| Sandwich | 12 | +30 | +5 | | +2 |
| Curry | 20 | +45 | +10 | | +4 |
| Soup | 15 | +25 | +5 | +10 | |
| Rice ball | 8 | +20 | | | +1 |
| Cake (sweet) | 25 | +15 | +25 | | +15 |
| Cookie (sweet) | 8 | +5 | +12 | | +5 |
| Ice cream (sweet) | 12 | +8 | +18 | | +8 |
| Milk | 12 | +10 | | +20 | |
| Juice | 8 | +5 | +10 | +5 | +2 |

Eating at FOOD 90+ counts towards a stomachache like the berries: sweets count
double, like the candy. Each serving counts for the "feed it" mission and pays
1 coin (inside the daily cap). Up to 99 of each.

### Bath and sleep menus
**The bath button** opens: **QUICK WASH** (free, as before), **BUBBLE BATH** (uses a
soap: the wash plus +10 JOY, bond and a heart; a rubber duck you own joins in),
**SCRUB** (a little game: rub the mud spots off with your finger -- each takes 3
rubs; all clean = the wash plus up to +12 JOY) and **BRUSH** (needs the brush:
+5 JOY, +3 bond and a shine for a minute).

**The light button**, while it is awake, opens: **BEDTIME** (lights off, as before),
**30 MIN NAP** (lights off; it wakes up by itself, rested and never grumpy) and
**LULLABY** (Brahms' lullaby plays, it goes to bed with +5 JOY; counts as bedtime).
While it sleeps the button works at once: it turns a light that was left on off,
or wakes it.

With the light off the scene goes dark and blue, and now and then it dreams.

The shop's **CARE** category sells **soap** (10 coins a bar, used up) and the
**brush** (60 coins, once).

### Places, the garden, gifts and weather
**The map button** (under the purse) opens **GO OUT**:

- **Park** -- WALK: a stroll costs 5 ENE and gives +3 JOY, and finds something:
  coins (2-4, at most 3 finds a day), a wild berry (+10 FOOD, +5 JOY), or a wild
  Pokemon that says hello (+3 JOY, bond). Too tired below 15 ENE.
- **Store** -- GO IN opens the shop over the store; leaving it returns there.
- **Pokemon Center** -- HEAL cures any sickness and grumpiness and gives +30 ENE.
  Free once a day, then 20 coins.
- **Garden** -- PLANT a seed (5 coins, CARE in the shop); it grows for **8 hours**
  of real time, then HARVEST gives one serving of food (mostly juice and sweets,
  sometimes a meal) into the bag.

**Seasonal gifts**, by the RTC date, once on the day: Halloween (31 Oct) the
pumpkin, Christmas (24-26 Dec) the Santa hat, New Year (1 Jan) the party hat,
Valentine's (14 Feb) the flowers -- or 20 coins if it is already owned.
**Weather**: one a day, from the date -- mostly fair, about 1 day in 5 rain (snow
in December-February). Only the sky changes.

### Capsules, sceneries, trophies (phase 8)
- **Capsule machine** (in the store, tap it): `30` coins, three a day. About one in
  ten gives a hat or decoration not owned yet; the rest is food, soap, seeds or
  10-40 coins back. On average a capsule is worth about its price (`extras_test`).
- **Scenery** (SHOP -> SCENERY): the seven biome backdrops, 100-200 coins each.
  Pick one from the bag; pick it again to go back to the creature's own.
- **Trophies** (purse -> TROPHIES): 14 lifetime goals -- 100 meals, 50 games,
  30 baths, 25 walks, 5 harvests, 1 000 coins earned, 5 hats, 6 decorations, a
  7-day streak, level 20, 10 study times, 14 brushings, a perfect quiz and 10
  capsules. Each pays 10 coins once.

### The school, teeth, talking, parents (phase 9)
- **School** (map -> SCHOOL): **STUDY** runs a 10, 20 or 30 minute homework timer
  (the screen may switch off; it chimes at the end) and pays 2/4/6 coins, three
  sessions a day, outside the daily game cap. **MATH** is a 10-question quiz:
  sums to 10, then to 20 with take-aways, then bigger numbers, then times tables.
  It counts as a game.
- **Brush teeth** (bath menu): a two-minute brush, twice a day; tapping scrubs faster.
- **Speech bubbles** now and then on the main screen: it says when it is hungry,
  dirty, sleepy or sick, when the garden is ready, when it rains -- or just that
  it loves you.
- **Parents** (settings, swipe down, the fifth page): a bedtime (19:00-22:00)
  after which games, training and battles are refused until 6:00, and a limit of
  10/20/30 games a day. Tap a row to change it. It is not locked: it is a
  reminder for the family, not a security feature.

### Out and about (phase 10)
The map is a grid of eight places now.
- **Adventure**: steps walked with the ball move it along a route of five stops
  (300, 800, 1 500, 2 500 and 4 000 steps). Each stop is a chest of 2-5 coins, a
  serving of food, two seeds or a wild Pokemon; the last one pays 10 coins and
  opens the next route (forest path, sunny beach, mountain trail, volcano cave).
- **Lake**: FISH casts; when the float dips, tap within 0.8 s. A Magikarp,
  a Goldeen, an old boot, a rice ball, a shell (a seed) or now and then a pearl.
  The first five catches a day pay (about 7 coins); after that it is just for fun.
- **Kitchen**: cookie + juice = berry pie, soup + rice ball = stew, ice cream +
  milk = parfait. Tap the pot to stir. Cooked dishes are better than shop food,
  strengthen the bond, and are never sold.

### Family (phase 11)
- **Chores** (purse -> CHORES): make the bed, homework, tidy the room, help at
  home, read, brush teeth. The parents tick what was done; each pays 4 coins once a day.
- **Piggy bank** (purse -> PIGGY BANK): save 10 or 50 at a time; a week after the
  first coin it pays 5% interest (at least 1, at most 20 a week). Take it out any time.
- **Diary** (purse -> DIARY): a page a day for the last week -- meals, games,
  baths, walks, fishing, study, chores, cooking, adventures, trophies, coins.
- **Alarm** (settings, the fifth page): a time and ON/OFF. It wakes the screen,
  plays a tune and says good morning; a tap stops it. With a parents' bedtime set,
  it also says when it is time for bed.
- **Shower bubbles**: in a quick wash, tap a bubble to pop it -- the first pop
  turns the wash into a 10-second bubble game.

### More games (phase 12)
In TRAIN: **Memory** (find the Pokemon pairs, 60 s) and **Simon says** (repeat a
growing tune, three lives) train DEFENCE; **Flappy flight** (tap to fly between
the trunks) trains SPEED; the **Tilt maze** (roll the berry to the flag -- tilt the
ball, or hold a finger where it should go) trains STRENGTH. Records are kept like
the other games, and old saves keep the old records.

### Home and dress-up (phase 13)
- **My room** is a free scenery: a wall and a wooden floor instead of the landscape.
- **FURNITURE** in the shop: window, clock, painting and shelf hang on the wall
  (two spots); sofa, TV, aquarium, bookcase and fireplace stand at the back (two
  spots); a rug lies under the creature; three wallpapers recolour the wall.
  Tap a piece in the bag to put it out (it opens the room) or away.
- **Ten more hats**: chef, pirate, cat ears, bunny ears, headphones, graduation
  cap, flower crown, hard hat, beret and viking helmet.

### A livelier pet (phase 14)
- **Wishes**: up to three a day it thinks of something -- a particular food, a
  game, the park, a bath, fishing or a cuddle -- shown in a thought cloud. Doing it
  within two hours grants the wish: joy, bond and 5 coins.
- **Personality** (the diary's first page says which): playful (more joy from
  games), lazy, a big eater (more from meals) or shy (more bond from cuddles). It
  comes from the species and IVs, so a creature always has the same one.
- **Moods**: *lonely* after three hours awake with nobody caring for it (a cuddle
  fixes it), *excited* when every mission of the day is done.
- **Birthday** every 30 days of its life: confetti all day, a cake and 20 coins.
- **Events**: the Easter week (eggs in the grass, and walks in the park find up to
  five eggs a day) and the July summer festival (lanterns, fireworks at night, a
  free ice cream a day).

### Games menu and arcade (phase 15)
TRAIN opens a grid of six categories with icons -- **Strength**, **Speed** and
**Defence** show how far each stat is trained -- and each opens a list of its games.
New: **Balloon pop** (Strength), **Shake race** (Speed: shake the ball or tap fast),
**Catch** and **Balance** (Defence: tilt the ball or use a finger), the **Type quiz**
and the **Slide puzzle** (Brain), **Bubbles** and **Drawing** (Arcade).

### Outfits (phase 16)
OUTFITS in the shop now holds the 20 hats and 12 accessories: sunglasses, star,
heart glasses, monocle; scarf, bow tie, medal, bandana; cape, backpack, angel and
bat wings. One of each kind can be worn with a hat. Full sets (pirate, superhero,
angel, spooky, chef) keep it cheerful and impress the contest judges.

### Town life (phase 17)
The map's second page: the **beach** (collect shells, build a sandcastle), the
**library** (read about a Pokemon), the **music hall** (play the game's tunes and
watch it dance), the **contest** (a weekly style show -- outfit, bond and joy win
bronze, silver or gold ribbons), the **arcade** (the games menu) and **jobs**: the
creature goes to the cafe, the flower shop or the post office for three hours and
comes home with 15 coins (nothing drains while it is away). **Mail** in the purse
brings a letter a day, each with a small gift.

### Quests and seasons (phase 19)
The missions panel has a **QUEST** button: a little story each week (Pikachu's
lost hat, Meowth's lunch, Chansey's lost egg, a song for Jigglypuff) in three
steps, each on a later day. Finishing it gives a hat not owned yet. **Halloween
week** (24-31 October) brings pumpkins, a ghost and a daily treat; the **winter
holidays** (20 December - 6 January) a snowman and hot cocoa. A whole year
together is a bigger birthday party.

### The photo album and the places' music
Big moments -- a trophy, a birthday, a finished quest, an evolution -- are photographed
and kept on the SD card (half size, the last 24, in `/album`). DIARY -> ALBUM shows
them full screen; PHOTO there takes one of the main screen right away. Each place
hums its own public-domain tune while you are there: the park *Twinkle, Twinkle*,
the school *Frere Jacques*, the garden and the lake *Ode to Joy*, the beach *La
Cucaracha*.

### The Nursery
Map page two, NURSERY. LEAVE TWO picks two creatures from the party or the box
(40 coins, once a week). They spend a day together; then TAKE EGG. The egg waits
and becomes the **next** egg, when the current creature finishes -- the live pet
is never replaced. GIVE BACK drops it again, and random eggs return.

- **Species:** the first form of one parent's line, 50/50 (Venusaur gives a
  Bulbasaur, every Eeveelution an Eevee).
- **IVs:** three of the four come from the better parent, the fourth is rolled.
- **Shiny:** 1 in 12 with a shiny parent, otherwise the usual 1 in 48.
- Legendaries cannot go, nor branch forms no egg can hold (Bellossom, Slowking...).
- The parents are copied at drop-off: releasing or moving one afterwards changes
  nothing. A bred egg ignores the region chooser.

### Restful nights
Asleep between 20:00 and 08:00, joy and hygiene do not fall and food only creeps down
to 60, so nobody wakes up to a hungry, sad, dirty Pokemon after a night it could not
be cared for. With the screen off it goes to bed by itself from 21:00 (the old window
started at midnight, and the evening before that drained it empty). A ball switched off
overnight counts the night as sleep. Daytime naps keep the slower-than-awake drain, and
the moon button still wakes it at any hour.

### The try-on mirror
In the shop, an outfit she does not own offers BUY, TRY ON and NO. TRY ON shows the
Pokemon in a mirror wearing it together with what it already wears, with BUY and BACK.
Trying changes nothing: not the outfit, not the coins, not the save.

### Wi-Fi: the real time and the weather
The board's RTC has no backup cell, so when the power runs out it wakes on the
1 January seed. Set up Wi-Fi once from the web installer (the 📶 section) and the
board fixes its own clock: about 15 s after boot and then every 30 minutes (every 6 h
without a place), it joins the network on a background task, asks NTP for the time,
and turns the radio off again. A big jump is applied **without** catch-up, like the
serial `TIME` command, so a month-long gap does not replay on the creature.

With a place set (the installer asks the browser, rounded to two decimals, ~1 km),
it also fetches the current weather from [Open-Meteo](https://open-meteo.com/):
grey skies when cloudy, rain or a storm behind the creature, snow, and the
temperature left of the clock. Weather older than 3 h is dropped. Without Wi-Fi the
old made-up weather (about 1 day in 5) still plays.

- **Fog** drifts across in pale bands; **rain** fills puddles that splash; a **storm**
  flashes about every 20 s with a thunder rumble (silent at night and while it sleeps);
  **snow** whitens the hill and leaves drifts that build over ~20 min and melt over ~1 h.
- **Snow from New York:** the same request also asks about New York. On days that are
  only clear or cloudy at home, New York's snow falls on the ball (the temperature stays
  yours), with a toast once a day. Anything real at home wins. Switch: Settings -> About,
  the installer, or serial `NYSNOW 0/1`.
- **When it will not connect:** `WIFISCAN` lists the 2.4 GHz networks in reach, and
  STATS prints a `wifidiag` line with the step that stopped and the access point's reason.
  Network names are case-sensitive.

- The SSID and password live in their own NVS namespace, `wifi`, so they are never
  in an EXPORT backup. Serial: `WIFI <ssid><TAB><password>`, `TZ <minutes east>`,
  `GEO <lat> <lon>`, `WIFISYNC`, `WIFIOFF`; STATS prints a `wifi=` line.
- 2.4 GHz only. A sync and the ESP-NOW battle link never hold the radio together.
- Settings -> About shows whether the clock came from Wi-Fi.

### The economy, checked
`economy_test` plays thirty days through the real rules (missions, caps, steps):

| Player | Coins a day |
|---|---|
| Light (3 feeds, 2 games, 1 000 steps) | ~33 |
| Normal (6 feeds, 5 games, a bath, 3 000 steps, bedtime) | ~76 |
| Busy (10 feeds, 15 games, 8 000 steps) | ~110 |

Food in the shop is 8-25 a serving, and the berries, candy and medicine stay free,
so a pet can always be fed and cured. A normal player who also spends ~20 a day
on food saves ~56: a cap in under 2 days, the crown in about 9, everything in the
shop once (2 930 coins) in about 52 days.

### Eggs & who you get (spawn odds)
- **First ever pet:** you pick a starter — **Bulbasaur / Charmander / Squirtle**.
- Hatch the egg: tap it **3×** (or wait — it hatches on its own).
- Every later egg rolls a **rarity tier** (over the ~79 base forms that come from eggs):

| Tier | Base chance | After a proper goodbye | # species |
|---|---|---|---|
| ✨ Legendary | ~3 %\* | ~10 % | 5 |
| 🔵 Rare | ~27 % | ~45 % | 27 |
| ⚪ Common | the rest | the rest | 47 |

  \* Legendaries only start appearing once you've **registered ≥ 25** Pokémon.
- A daily **streak** and high **bond** push rare/legendary odds higher.
- A clean **goodbye blesses** the next egg; a **run-away curses** it (forces Common).
- Within a tier it favors species whose **evolution line you haven't finished** (so
  all 386 are completable).
- **Shiny:** base **1 / 48** (→ **1 / 24** right after a goodbye), improved by
  streak/bond down to a best of **1 / 8**. Tracked separately in the dex.
- Every hatch rolls unique **IVs** (see below) — no two are identical.

### Evolution

**Cross-generation evolutions are linked.** When the dex reached 386 the targets
arrived but nothing connected them, so six Kanto species could not evolve into
creatures that were sitting right there in the Pokedex: GOLBAT -> CROBAT,
ONIX -> STEELIX, CHANSEY -> BLISSEY, SEADRA -> KINGDRA, SCYTHER -> SCIZOR and
PORYGON -> PORYGON2. Those six now evolve (friendship pairs at 25, trade pairs
at 40), and the six targets stopped hatching straight from eggs -- rarity is
derived from being somebody's evolution, so they became evolution-only the way
every other evolved form already was.

`gen_dex_data.py --link` is the rule rather than a one-off edit: it fills in any
evolution whose target has since joined the table, and only ever touches rows
whose value is 0, so it cannot retune an evolution anybody already has. Sinnoh
brings ELECTIVIRE, MAGMORTAR and RHYPERIOR waiting on exactly the same thing.

- Triggers when **level ≥ its evolution level** (16 for most base forms; ~30 for
  stone-style, ~40 for trade-style) **and every stat ≥ 40** at that moment.
- **Never automatic** — a button appears and **you tap to witness it** (with a
  flicker between the old and new form). Each **slip-up delays it by 1 level**.
- You can **decline** ("keep form"); it re-offers at the next level.
- *Eevee* branches toward whichever of its **eight** evolutions you're still
  missing — Vaporeon, Jolteon, Flareon, Espeon, Umbreon, Leafeon, Glaceon and
  Sylveon. None of the eight hatches from an egg any more: the only way to a
  Sylveon is to raise an Eevee. A branch is offered only if that region's
  sprite pack is on the card, so evolving can never hand you a creature the
  device cannot draw.

### Your party
- A **farewell** or a **release** doesn't end the relationship any more — the creature
  **joins your party** (6 slots), keeping its species, nickname, shiny status, IVs,
  training and the level it reached. Frozen there: it no longer ages or trains.
- A **runaway does not join.** It's the one ending with a cost, and letting a
  neglected pet come back on the team would remove it. **Neither does an early
  retire** — see "Retiring a creature early" below.
- **Letting one go for good.** Tap a party or box slot to open its sheet;
  **RELEASE** removes that creature permanently. It asks first, every time, and
  the creature does not fall through into the box — this is the one way to free
  a slot without something taking its place. A box slot now opens the same sheet
  rather than jumping straight into the party, and **TO PARTY** does that.
- With a full party you're taken straight to the party screen to pick who the
  newcomer replaces — or to let it go. Nothing is ever overwritten silently.
- *(Gym battles, which is what the party is for: on the roadmap.)*

### The three endings (you choose & witness each — none auto-fire)
- 💛 **Farewell** — when it's a **final form** that has lived **3 days**. A button
  appears; triggering it **blesses your next egg**. You can **postpone** ("stay
  together", re-offered in a day). The good ending.
- 💔 **Run-away** — if you let **all four stats sit at 0 for a full hour**. A single
  act of care cancels it. It **curses the next egg** (forces Common). The sad ending.
- 👋 **Release** — long-press the creature to let it go on your terms (neutral).

After any ending, a **new egg** appears.

### Bonds, streaks, medals, Pokédex
- **Streak** (player-wide, survives across pets): first care each real day; milestones
  at **3 / 7 / 30 / 100** days; skipping a day breaks it.
- **Bond** (per pet, resets on hatch): grows with affection (**cap +20/day**), cools on
  neglect. Both streak & bond improve egg/shiny odds — **and the IVs of your next pet**.
- **8 medals** (Lv10/25/50, favorite berry found, 7-day streak, max bond, final form,
  "fit" = weight 0 & no slip-ups), per-pet + a global counter.
- **Pokédex:** raising a species registers it; **809 + shinies** to complete.
  Browsed **one region at a time** — swipe vertically between regions,
  horizontally to page within it, so nothing is more than ten pages from the front.
- **Region:** the pill under a waiting egg picks which generation it comes from —
  **Kanto / Johto / Hoenn / Sinnoh / Unova / Kalos / Alola / All**. A first egg gives that
  region's starter. A handful of species have no sprite art anywhere (13 of
  Unova's 156, and Pyroar); they keep their dex numbers but never hatch, since the egg would
  give a creature that could only ever draw as a number.
  A region whose **sprite pack is not on the card** shows as locked and is kept
  out of the egg pool, so a partial sprite install is a supported state. A pack
  sent from the web installer is picked up **as it lands** — the region unlocks
  without a reboot.

### Battle stats & IVs
Every pet rolls four **IVs** (individual values, 0–31) at hatch — ATK / DEF / SPD /
VIT — that never change and make each one genuinely unique:

```
stat = base + level + (IV × level)/100 + training
```

The `(IV × level)/100` term is the **real formula from Gen III onward** — a perfect
IV is worth +31 at level 100. But IVs do a second job here that they don't do in
the games: **they cap training.**

| | Effect |
|---|---|
| Innate bonus | up to **+22** at level 73 (end of a normal life) |
| Training ceiling | `70 + (30 × IV)/31` → **77** at IV 8, **100** at IV 31 |
| Total spread | ~**40 points**, about **15 %** between a great and a poor individual |

- **Rolls are 8–31, never 0.** In the real games a 0 IV is survivable because you can
  breed hundreds of eggs; here a pet lives 3 days, so a dud would just be a punishment.
- **Streak + bond bias the roll upward** (up to +7) — using the *previous* pet's care
  score. Raising one well genuinely improves the next.
- **Legendaries hatch with 3 of 4 IVs perfect**, exactly as they're guaranteed 3
  perfect IVs in the games.
- **Shinies floor every IV at 20** — a nod to Gen 2, where shininess *was* a DV
  pattern and a shiny was never mediocre.
- IVs are shown on the Battle page of the stat card; a perfect 31 is highlighted.

Training: **STRENGTH** ← the bag, **SPEED** ← the reaction test, **DEFENSE** ← the
ball game (and still 1 h of wellbeing passively). **VIT** can't be trained. All three
live in the training menu now; the ball moved off the home row when it became
defence's trainer.

**TMs unlock at level 40**, all of them, and nothing before. A TM carries no level
requirement in the data — true of the games, wrong here, because a young creature
has few level-up moves and the spare slots were filled with the strongest TMs in the
table. A **level 1 Squirtle opened with SURF and BLIZZARD and could beat Brock.**

One number rather than a curve: the first five leaders sit at **14–43**, so you
fight the early ladder on what your species actually learns, and TMs arrive as you
enter the back half. A creature retires at 73 and caps at 100.

That only works because the move table now carries the **cheap early attacks** —
SCRATCH, PECK, POISON STING, BUBBLE, ABSORB, SPARK, FURY ATTACK and the rest. Before
them, ~15 % of species reached level 15 with no attacking move at all and were
quietly leaning on TMs to fill the gap.

**Gym wins train too**, which is what makes the ladder worth replaying rather than
a checklist you tick once:

### Retiring a creature early

The farewell is only *offered* at final form and three days. **RETIRE** on the
menu ends a creature whenever you like -- but what that costs depends entirely
on whether the farewell had been earned yet.

| | |
|---|---|
| Retiring one that has **earned** its farewell | free -- it is simply the farewell reached by another button. It **joins your party** and **blesses** the next egg |
| Retiring one that has **not** | it is **gone for good** -- not banked, not in the box -- the next egg is **neutral** instead of blessed, and the **next** creature evolves a day later |

An early retire used to bank the creature and bless the next egg exactly as a
farewell does, which made it the good ending with a small tax on it: you could
retire, check the egg, and retire again, farming blessed rolls at no real cost.
Giving the creature up is the price now, and the confirm dialog spells out both
halves before you accept.

The penalty is `EVO_PENALTY_LEVELS` (24 at `MINUTES_PER_LEVEL 60`) added to
every evolution threshold, the same sum `careMistakes` moves. It lands on the
creature that hatches next, is spent by hatching it, and does **not** compound:
three early retires in a row still cost one day. The creature's card says
"evolves a day later" while it carries the debt, and the confirm dialog says the
price before you accept it.

### Choosing your egg's region

The species is decided when the egg **appears**, not when it cracks, so changing
region moves the egg you are holding. Two rules stop that being a re-roll button:

| | Rule |
|---|---|
| Rarity | the tier the egg was granted is **kept** — only which species of that tier changes, so flipping can never fish for a legendary |
| Memory | each region's answer is **remembered** for the current egg, so switching back shows the same creature |

The region is first chosen at the **very start of a new game**, on the screen
before the starter -- so the creature you begin with and the eggs that follow
come from the same place. Everything below is about changing it afterwards.

A region is decided by the **base** species, and evolutions follow wherever they
lead — a Kanto run still reaches Crobat and Blissey.

| | Training a win is worth |
|---|---|
| Easy | **3–5** points, **+1 per 3 leaders** deeper into the ladder |
| Hard | **6–10** points, same ladder bonus |
| Which stat | **random**, but only among stats **not already at their ceiling** |
| Who gets it | the **live pet**, and only if it was in the squad |

A random stat that landed on a maxed one would silently evaporate, so it never
picks one; and the IV-bound ceiling above still applies, so a win can never push a
stat past what its IV allows. Banked members are frozen at what they were banked
with, and battling already costs the live pet energy — that, not a cooldown, is
what rate-limits rematching. A fully trained creature is told so.

## How to play

On first run you **choose a starter** (Bulbasaur / Charmander / Squirtle). After
that you start with an **egg**. Tap it 3 times or wait and it hatches. From then
on, care for your companion:

**Four stats** that decay: **FOOD**, **JOY**, **ENE** (energy), **HYG** (hygiene).
If one bottoms out it counts as a *slip-up*.

**Buttons (bottom arc, icons):**
- 🍎 **Feed** → food menu: 3 berries (each species has a hidden favourite that
  gives a bonus) and a candy (+happiness but it fattens; weight makes it sluggish).
- ⚽ **Play** → the pokeball minigame (joy only).
- 🌙 **Light** → sleep/wake (recovers energy, dims the screen). While asleep,
  needs decay much slower (rest).
- 🫧 **Bath** → a foam scene that cleans up the poops.

**Touch gestures:**
- **Tap the name** at the top = the **menu** (Pokédex / Party / Settings). Close it
  with the CLOSE row, by tapping anywhere outside the panel, or with any swipe.
- Tap the creature = pet it (+happiness, bond).
- Horizontal swipe = open the **Pokédex / gallery**.
- Vertical swipe up = open the **stat card** (4 pages: Profile / Battle / Medals /
  Progress; swipe between them; tap the name on Profile to rename; on Battle the
  "Train strength" button opens the bag).
- **Settings** (menu row): clock, sound + volume, brightness, language, about (swipe
  left/right between the pages). Swipe down = the **player card**.
- Long press (3 s) on the creature = **release** dialog.

**Physical PWR button:** short = screen on/off · long (4 s) = full power-off
(the RTC stays alive, so time passes even while it's off).

## Decisions: you choose, and you watch

The three life-cycle endings and evolution **don't happen on their own** — when
the conditions are met a button appears and you tap it (so you're present to
witness it), each opening a two-option dialog:

- **Evolution** (red button): *Evolve* (epic animation: halo, rays, sparkles and
  a **flicker between the old and new form**) or *Keep form* (re-offered next level).
- **Farewell** (gold button, final form + 3 days): *Say goodbye* (warm farewell,
  rising hearts → new egg) or *Stay together* (keep your companion; re-offered in
  a day). Tension: a maxed-out friend vs. completing the Pokédex.
- **Runaway** (dark button, total neglect for 1 h): a somber "feels abandoned"
  ending in the rain — caring for the creature cancels it.

  **It does not ask, and that is the point** -- a creature you have to authorise
  to leave is not really at stake. What it must never be is the price of going
  to bed, so **the screen being off between midnight and 06:00 puts the creature to
  sleep**, and sleep floors the stats at FOOD 30 / JOY 35 / HYG 45 with the
  neglect check skipped entirely. A night costs you a hungry, grubby creature at
  breakfast instead of an empty one.

  **Both halves are needed.** The screen alone would pause the game every time
  you pocketed the device, and the creature is meant to get hungry during the
  day. The hour alone would send it to bed while you were still playing. Set the
  clock in **SETTINGS** (or `RTCSET`); a board whose clock was never set simply
  never auto-sleeps, which fails safe -- it keeps draining and the light button
  still works by hand.

  This was a real loss: a player left the board running overnight and came back
  to a Dratini that had gone. The live tick was the only drain path with no
  floor -- offline floors at 15, sleep at 30/35/45 -- so a board left *running*
  was punished where a board switched *off* was not. `night_test` runs ten
  simulated hours and fails if that ever comes back.

## Types

Every species carries its real **typing** (one or two of the 18 types) and the game
ships the full **18×18 effectiveness chart** — `dex.h` holds both, generated from
`tools/dex_types.py`.

The chart is the **current (Gen 6+) one, not Gen 1's**, which is a deliberate call:

- Gen 1's chart shipped real bugs — Ghost moves did literally nothing to Psychic —
  and Psychic was resisted only by Psychic, so it ran away with every fight.
- The base stats here were **already** pulled from PokéAPI at current values, so
  modern stats with an ancient chart was the inconsistent pairing.
- **Fairy earns its keep:** Dragonite has the highest Attack in the dex, and before
  Gen 6 Dragon was resisted only by Steel. Dragon → Fairy is **0×**, so Clefable
  hard-walls the strongest thing you can hatch.

Seven of the original 151 differ from their Gen 1 typing: Magnemite and Magneton gained Steel
(Gen 2), and Clefairy, Clefable, Jigglypuff, Wigglytuff and Mr. Mime gained Fairy
(Gen 6) — the first two losing Normal entirely.

Typing is shown on the Battle page of the stat card. *(Battles: on the roadmap.)*

## Battle stats and training

Each creature has ATK/DEF/SPD/VIT = **base stat** + level + **IV** (0–31, rolled
at hatch, `IV × level/100` exactly as in the real games) + **training**:
- SPEED ← the **reaction test** (~2 reactions = 1 pt, cap +18 per session)
- DEFENSE ← accumulated wellbeing (1 h resting or well-cared = +1)
- STRENGTH ← the training bag (~4 hits = 1 pt, cap +18 per session)
- VIT (vitality, from the base HP stat) — not trainable

### Moves

Each creature knows up to **4 moves**, from a pool of 77. Two kinds:

- **Level-up moves** are gated: Charizard learns FLAMETHROWER at 34, WING ATTACK
  at 36, DRAGON RAGE at 54. A hatchling starts with **only** what its species
  knows at level 1 — a Charmander opens with GROWL alone, and the other three
  slots stay empty. Crossing a gate fills an empty slot silently; with all four
  full you get a **prompt** asking which to forget (or to skip it). Offers queue,
  so coming back to a pet that aged two weeks offline asks one at a time.
  Evolving keeps the moves it already has, and the new form's gates take over —
  moves it would have learned *below* your current level are not backfilled,
  same as the real games.
- **TMs** have no level gate and are chosen on demand, from the **MOVES** page of
  the stats card (swipe across, then tap a slot).

Levels come from FireRed/LeafGreen, the Kanto games that still gate properly.
A move that is *also* a TM keeps its level gate — otherwise every gated move
would come free, since most of them were sold as TMs at some point.

Moves **freeze when a pet is banked** into the party, alongside its level and
training: the set you chose while it was alive is what it fights with forever.

**Special attack and defence** come off the species' own `bSpA`/`bSpD` base stats
(Alakazam is 50 Attack but 135 Special Attack), reusing the physical IV and
training rather than rolling their own: special attack runs off the ATK IV and
training, special defence off the DEF IV and training. So the physical/special
split lives on the species, not the individual — no extra IVs to roll.

The IV also sets **how far each stat can be trained at all** (77–100), so a
well-rolled individual has a genuinely higher ceiling, not just a head start.
See [Battle stats & IVs](#battle-stats--ivs) for the numbers.

Shown on the Battle page of the stat card. The (hidden) weight goes up with candy
and burns off with training.

## Retention: streak, bond, medals, name

- **Streak** (the player's, persists across creatures): the first care of each
  real day advances the streak; 3/7/30/100 milestones are celebrated; skipping a
  day breaks it. Flame badge on the main screen.
- **Bond** (the creature's): rises slowly with care and petting, drops with slip-ups.
- **Medals** for the individual (level, berry, streak, bond, final form, fit) +
  a global counter. Medals page of the stat card.
- **Name**: touch keyboard; the nickname rules the header and the card.

High streak and bond **improve the egg roll** (rarity and shiny): caring well
always pays off.

## Life cycle, eggs by rarity, languages

The life cycle lasts **3 days** of play. Three endings (all leave a new egg):
**farewell** (final form + 3 days), **release** (long press), **runaway** (all 4
bars at zero for 1 h). Each bred species is recorded in the **bred Pokédex**
(normal and shiny separately).

The egg rolls rarity over the ~79 base forms (47 common / 27 rare / 5 legendary),
**biased towards the lines you're missing** (all 386 are completable), blessed by
a farewell and punished by a runaway. Legendaries only with 25+ registered.
**Shiny** 1/48 (better with streak/bond/farewell).

**Languages:** the UI ships in 8 languages — English (default), Chinese (simplified),
Spanish, Korean, French, German, Italian, Portuguese — pick one from the grid on the settings Language page (menu > Settings, then swipe to Language).

Chinese and Korean text is drawn from `cjkfont.h`, a 12x12 subset (only the
characters the UI uses, ~520) generated by `tools/gen_cjk.py` from Fusion Pixel
Font. Re-run the script after editing any zh/ko string; `i18n_test` fails on a
character with no glyph. Species and move names stay in English in every language.

## Backgrounds: biome + real time

The idle screen paints the sky from the **RTC's real time** (dawn / day / dusk /
night) and the scenery from the creature's **biome**, chosen by its primary type
(`TYPE_BIOME` in `tools/gen_dex.py`, stored in `DexEntry::biome`; eggs are always
meadow). The sky follows the clock only: a creature asleep at noon sleeps in daylight.

By day a haloed sun with slowly turning rays sits over three puffy, shaded
clouds that drift at different speeds. At night a crescent moon with craters and a
soft halo rises over 40 stars. Their positions are rolled at random on every boot and each twinkles on its own
rhythm; the brightest ones sparkle with a small cross. The same stars are
behind the minigames.

Each biome is hand-drawn in code (`drawScene()` and one `draw<Biome>()` per
biome in `TamaPoke.ino`): layered tones lit from the sun side, animated, and
laid out around the UI. Scenery is kept off the lower left (where poops land),
off the step counter at the far left and below the sun and moon.

| Biome | Types | Scenery | Day / night residents |
|---|---|---|---|
| Meadow | normal, electric, fighting, poison, psychic, fairy, flying | windy tall grass that ripples in travelling waves, wildflowers, layered hills, drifting seed fluff | Butterfree overhead + Rattata in the grass / fireflies + Venomoth |
| Beach | water, dragon, fossils | banded sea with drifting crests and a glitter path, island, sailboat, a tide washing foam over wet sand | Wingull + Exeggutor on the shore / the shore alone |
| Forest | grass, bug, dark | two tree lines with mist, a swaying oak with Oran berries, a pine, bush, ferns, mushrooms, falling leaves | Beedrill + Caterpie / fireflies + Zubat |
| Volcano | fire | craggy cone with lava streams, bubbling crater, periodic eruption bursts, lava bombs, smoke, glowing ground cracks, faceted boulders, a charred tree | Slugma (any hour) |
| Mountain | rock, ground, steel | three hazy snow-capped ranges, drifting fog, a cliff with an animated waterfall and misty pool, a pine, boulders, edelweiss | Fearow + Geodude / Golbat + Cubone |
| Snow | ice | polar ice: faceted icebergs, an open lead, ridges and cracks, heavy slanted snowfall; at night a swaying aurora of green, cyan and violet rays | Piplup (Seel without the Sinnoh pack) |
| Graveyard | ghost | haunted house with flickering windows, gnarled trees, iron fence, seven gravestones, scattered bones, drifting fog, will-o'-wisps, bats at night | Gastly / Haunter |

Day on the top row, night underneath (regenerate with `bash tools/make_screens.sh`):

| Meadow | Beach | Forest | Volcano |
|---|---|---|---|
| <img src="screens/biome_meadow_day.png" width="180"> | <img src="screens/biome_beach_day.png" width="180"> | <img src="screens/biome_forest_day.png" width="180"> | <img src="screens/biome_volcano_day.png" width="180"> |
| <img src="screens/biome_meadow_night.png" width="180"> | <img src="screens/biome_beach_night.png" width="180"> | <img src="screens/biome_forest_night.png" width="180"> | <img src="screens/biome_volcano_night.png" width="180"> |

| Mountain | Snow | Graveyard |
|---|---|---|
| <img src="screens/biome_mountain_day.png" width="180"> | <img src="screens/biome_snow_day.png" width="180"> | <img src="screens/biome_graveyard_day.png" width="180"> |
| <img src="screens/biome_mountain_night.png" width="180"> | <img src="screens/biome_snow_night.png" width="180"> | <img src="screens/biome_graveyard_night.png" width="180"> |

The residents are real PMD sprites, kept small and behind the pet. Each is loaded
only while its biome is on screen (about 110-180 KB of PSRAM) and freed when the
biome changes; a species whose pack is not on the card simply does not appear.
Flyers use the species' Walk animation, which for Butterfree, Beedrill, Venomoth,
Golbat, Zubat, Wingull and Fearow is a flapping flight (Pidgeotto's is a ground
walk, so it is not used). Battles fight on the foe's biome art; ghosts borrow the
forest's, since there is no graveyard battle scene.

To look at one without hunting for a species, use the serial `BIOME <0-6>`
command (0 meadow, 1 beach, 2 forest, 3 volcano, 4 mountain, 5 snow, 6 graveyard).
It is not saved; a reboot goes back to the species' own biome.

