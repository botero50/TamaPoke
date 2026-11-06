# TamaPoke -- Fun Pack

[![Flash in browser](https://img.shields.io/badge/flash-in%20browser-FF6B00?logo=googlechrome&logoColor=white)](https://botero50.github.io/TamaPoke/web/)
[![MakerWorld](https://img.shields.io/badge/MakerWorld-3D%20case-00AE42?logo=bambulab&logoColor=white)](https://makerworld.com/es/models/2937822-tamapoke-a-pokemon-pokeball-tamagotchi)
![Board](https://img.shields.io/badge/board-ESP32--S3%20round%20AMOLED-E7352C?logo=espressif&logoColor=white)
![Firmware](https://img.shields.io/badge/firmware-v3.14--funpack.49-8A2BE2)
![Code](https://img.shields.io/badge/code-MIT-blue)
![Languages](https://img.shields.io/badge/languages-8-FFCB05)
[![Stars](https://img.shields.io/github/stars/botero50/TamaPoke?style=flat&logo=github&color=yellow)](https://github.com/botero50/TamaPoke/stargazers)

A Pokémon virtual pet in a 3D-printed Pokéball: raise it, feed it, play 28 minigames,
dress it up, visit places, watch the real weather outside, and **walk with it to reach
the next gym**. Free and open source,
in 8 languages (Spanish, English, French, German, Italian, Portuguese, Chinese, Korean),
and installed straight from your browser.

<table>
<tr>
<td align="center" width="50%"><img src="docs/funpack/tour-life.gif" width="300" alt="Life with your Pokémon: food, the bath, the broom, the shop, the room and the weather"></td>
<td align="center" width="50%"><img src="docs/funpack/tour-adventure.gif" width="300" alt="Adventures: the map, places, games, missions, steps and gym challenges"></td>
</tr>
<tr>
<td valign="top">

**Life with your Pokémon**
- Your Pokémon talks, gets hungry, sleepy and dirty
- Meals you can buy right from the food menu
- A quick shower, a sponge or a bubble bath, and a broom for the poop
- A shop with food, hats, outfits and furniture
- A room of its own
- The real weather outside, snow included

</td>
<td valign="top">

**Adventures**
- A map of places: Center, garden, lake, beach, school and more
- 28 minigames and an arcade with prizes
- 6 daily missions and a weekly story quest
- A steps panel with your week of walking
- Walk to the next gym: the leader challenges you
- A photo album with favourites

</td>
</tr>
</table>

## 🛠️ Build your own

### What you need
| Part | What for | Notes |
|---|---|---|
| **[Waveshare ESP32-S3-Touch-AMOLED-1.75](https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.75)** | The whole pet: round 466×466 screen, touch, motion sensor, clock, audio | Get the **Standard** or **-G** version. **Not "-B"** (its own case does not fit) and not the different "1.75**C**" board. **Alternative:** [this kit on Amazon](https://www.amazon.com/dp/B0F8867487), the same board sold with a speaker and a GPS antenna (the GPS is not used) |
| **microSD card** | The Pokémon sprites | Any size from 1 GB |
| **USB-C data cable** | Installing and charging | A charge-only cable will not be seen by the computer |
| **3D-printed Pokéball case** | The ball | **[Download it on MakerWorld](https://makerworld.com/es/models/2937822-tamapoke-a-pokemon-pokeball-tamagotchi)**, print profiles included (a remix of yoyothechicken's [Pokeball](https://makerworld.com/es/models/839922-pokeball)) |
| Small speaker *(optional)* | Sound and music | Plugs into the board's MX1.25 speaker connector |
| LiPo battery *(optional)* | Using it unplugged, and keeping the clock when unplugged | Check the MakerWorld page for a size that fits the case |
| 2.4 GHz Wi-Fi *(optional)* | The real time and the weather | 5 GHz-only networks will not work |

### Steps
1. **Print the case** from [MakerWorld](https://makerworld.com/es/models/2937822-tamapoke-a-pokemon-pokeball-tamagotchi).
2. **Install the firmware**: open the **[web installer](https://botero50.github.io/TamaPoke/web/)** in Chrome or Edge on a computer, plug the board in and press **Install TamaPoke**. No Arduino and no drivers.
3. **Load the sprites** onto the microSD from the same page (Kanto is enough to start), then restart.
4. **Optional: Wi-Fi.** On the same page, enter your 2.4 GHz network, so the clock sets itself and the sky shows the real weather.
5. **Assemble** the board into the ball, following the MakerWorld page.

## 🔄 Updating

On the [installer page](https://botero50.github.io/TamaPoke/web/), press **💾 Back up, then install**, then **Install now** with
**"Erase device" unticked**. Your pet, coins and everything it owns carry over. Release notes are on the
[Releases page](https://github.com/botero50/TamaPoke/releases).

## ✨ What the Fun Pack adds

Built on crenberries428's 3.14, all in 8 languages:

- **28 minigames** in six categories: Strength, Speed, Defence, Brain, Fun and Arcade.
- **Care with real moods**: food (buy a meal right from the food menu), three levels of clean (a quick shower, a sponge game, a bubble bath), **a broom for the poop**, brushing teeth, medicine, colds, stomachaches, boredom and wishes.
- **Restful nights**: asleep, its needs hold, so nobody wakes up to a starving pet.
- **6 daily missions** from 36 kinds, always ones you can do today, and **12 weekly story quests**. Coins for a **shop** with food, toys, 20 hats and outfits, and a **try-on mirror**.
- **A room of its own** with furniture and wallpapers, sceneries for the main screen, trophies, and a **photo album**: star your favourites, delete the rest, and copy the pictures from the SD card to any computer (at most 24 photos, about 2.6 MB).
- **Places to go**: park, store, Pokémon Center, garden, school, lake, kitchen, beach, library, music hall, contest and part-time jobs, each with its own animations (the Center heals with a machine and a jingle).
- **The road to the gym**: walk with the ball and a dial fills at the edge of the screen. When you've walked far enough, the next leader challenges you, about one gym a week, across 7 regions.
- **A steps panel**: today, your last 7 days, the road to the gym, the adventure route and every step you've ever walked.
- **Walking prizes**: coins at 5 000, 10 000 and on up to a million steps walked in total, once each.
- **A warning beep** when a need is very low, and **quiet nights**: from 21:00 to 07:00 the ball makes no sound by itself.
- **The Nursery**: two favourites from the party or box leave an egg for the next raising.
- **Wi-Fi time and the real weather**: fog, rain, storms, settling snow, and snow from New York for places where it never snows.
- **It talks**: about 150 things to say, picked by its needs, the weather, the time of day, events and things to do.
- **Family features**: a homework timer, a math quiz, chores, a piggy bank, an alarm, a diary, and parent limits for bedtime and games.
- **Events**: birthdays, Easter, summer, Halloween, winter, weekly story quests and daily mail.
- **Arcade night**: tickets from the games, a prize counter with exclusive prizes, weekly records and bronze/silver/gold badges.

## Screens

All shots are straight off the 466x466 round panel, rendered headlessly by the
emulator (`tools/emu/tamapoke-emu --shot <name>`), so they are exactly what the
hardware draws.

### The Fun Pack

| The road to the gym | The leader challenges you | Your steps |
|---|---|---|
| <img src="docs/funpack/screens/dial.png" width="240"> | <img src="docs/funpack/screens/card.png" width="240"> | <img src="docs/funpack/screens/steps.png" width="240"> |

| Everything in the bath | 6 missions a day | A weekly story |
|---|---|---|
| <img src="docs/funpack/screens/bath.png" width="240"> | <img src="docs/funpack/screens/missions.png" width="240"> | <img src="docs/funpack/screens/quest.png" width="240"> |

| 28 games in six categories | The Nursery | Try outfits on in the mirror |
|---|---|---|
| <img src="docs/funpack/screens/games.png" width="240"> | <img src="docs/funpack/screens/nursery.png" width="240"> | <img src="docs/funpack/screens/mirror.png" width="240"> |

| Real weather: rain | Snow from New York | Fog |
|---|---|---|
| <img src="docs/funpack/screens/rain.png" width="240"> | <img src="docs/funpack/screens/ny_snow.png" width="240"> | <img src="docs/funpack/screens/fog.png" width="240"> |

| A room of its own | The kitchen | The beach |
|---|---|---|
| <img src="docs/funpack/screens/room.png" width="240"> | <img src="docs/funpack/screens/kitchen.png" width="240"> | <img src="docs/funpack/screens/beach.png" width="240"> |

In the emulator the creature is drawn with the built-in sprite; on the ball it is the
animated one from the microSD.

### Starting out

| Pick a region | ...then its starter | Johto's three |
|---|---|---|
| <img src="docs/screens/region.png" width="240"> | <img src="docs/screens/starter.png" width="240"> | <img src="docs/screens/starterj.png" width="240"> |

A new game asks which region you are playing before it asks which creature you
want. The choice sets both: you pick from that region's three starters, and it
becomes where your eggs come from afterwards (changeable later on the egg's
region pill). Existing saves never see this screen -- it only appears when the
Pokedex is empty.

### Raising one

| Your creature | Its egg | Moves it knows |
|---|---|---|
| <img src="docs/screens/main.png" width="240"> | <img src="docs/screens/egg.png" width="240"> | <img src="docs/screens/moves.png" width="240"> |

The egg carries a **region pill** — pick whether it hatches from Kanto, Johto,
Hoenn or all three. Switching keeps the rarity it was granted and remembers each
region's answer, so it cannot be flipped to farm a legendary.

**Steps.** The board's QMI8658 accelerometer counts your steps (`steps.cpp`, in
any orientation, screen on or off, read about 60 times a second in the background)
and the home screen shows today's total on a plate curved along the left edge. Tap it
for the steps panel. Today's count resets at midnight; the 7-day history, the lifetime
total, the adventure route and the road to the gym carry on, and all of them are saved,
every 20 steps, so unplugging the ball loses at most 19. A counted walk of 100 steps gave 105, and it
counts nothing while the ball lies still.

### Day, dusk, night

| Day | Sunset | Night | Steps |
|---|---|---|---|
| <img src="docs/screens/sky_day.png" width="180"> | <img src="docs/screens/sky_sunset.png" width="180"> | <img src="docs/screens/sky_night.png" width="180"> | <img src="docs/screens/steps.png" width="180"> |

The sky follows the clock: a haloed sun and drifting clouds by day, a crescent
moon and twinkling stars at night. The last shot shows the step plate on the left
edge. The boot title card is `docs/screens/splash.png`.

### Battling

| The fight | Choosing a team | Winning |
|---|---|---|
| <img src="docs/screens/btlmenu.png" width="240"> | <img src="docs/screens/pick.png" width="240"> | <img src="docs/screens/win.png" width="240"> |

Turn- and move-based, with the real type chart, ailments and STAB. Real Game Boy
battle music plays throughout.

### Seven regions

| Pick a ladder | Johto's gyms | LAN battle |
|---|---|---|
| <img src="docs/screens/gympick.png" width="240"> | <img src="docs/screens/gymsj.png" width="240"> | <img src="docs/screens/lanready.png" width="240"> |

Kanto, Johto, Hoenn, Sinnoh, Unova, Kalos and Alola each have eight leaders, an Elite 4
and a champion, on easy and hard. A new leader takes real walking: 10 000 steps to the
first, 2 000 more for each one after, and a week between new badges. Rematches and the
hard ladder are always open. Kanto, Johto, Hoenn and Sinnoh are the games' own teams, checked
against the pokecrystal, pokeemerald and pokeplatinum disassemblies -- **0
differences across all 39 trainers**, re-checkable with
`tools/verify_rosters.py`.

**Unova, Kalos and Alola are the exceptions and say so.** pret has no disassembly for
them, so those ladders are written from knowledge rather than from the game's own tables
(Alola has no gyms, so its slots are trial captains and kahunas);
`verify_rosters.py` prints Unova as NOT VERIFIED rather than letting a clean run
imply otherwise. It follows Black 2 / White 2, which unlike Black/White has no
version- or starter-dependent leaders.

Sinnoh follows **Platinum**, where Fantina is the *third* gym rather than
Diamond/Pearl's fifth; the level ramp only runs 14/22/26/32/37/41/44/50 that
way. Same reasoning that makes Hoenn Emerald throughout.

### Collecting

| Pick a region | Kanto | Johto |
|---|---|---|
| <img src="docs/screens/dexpick.png" width="240"> | <img src="docs/screens/gallery.png" width="240"> | <img src="docs/screens/gallery2.png" width="240"> |

| Trainer card | Johto badges | The box |
|---|---|---|
| <img src="docs/screens/player.png" width="240"> | <img src="docs/screens/player2.png" width="240"> | <img src="docs/screens/box.png" width="240"> |

### Settings

Tap the name, then **Settings**. It is five swipeable pages (Time, Volume, Brightness, Language, About); swipe
left/right to move between them, and the dots show where you are. Only the clock
needs **OK** to apply; sound, brightness and language take effect the moment you
touch them.

| Time | Volume | Brightness |
|---|---|---|
| <img src="docs/screens/set_time.png" width="240"> | <img src="docs/screens/set_volume.png" width="240"> | <img src="docs/screens/set_bright.png" width="240"> |
| Set the hour and minute | Sound on/off, a draggable 0–100 volume slider, and a TEST beep at the chosen level | Screen brightness 1–10; sleep and idle dimming only ever go below it |

| Language | Chinese UI | |
|---|---|---|
| <img src="docs/screens/set_lang.png" width="240"> | <img src="docs/screens/set_lang_zh.png" width="240"> | The About page shows the firmware version |

## 📖 More

- **[Game manual](docs/GAME-MANUAL.md)**: every rule and number (stats, coins, eggs, evolution, battles).
- **[For developers](docs/DEVELOPERS.md)**: building from source, the emulator, sprites, serial console.
- **[Roadmap](ROADMAP.md)**: what is next.

> ### 🙏 Fork lineage
>
> TamaPoke was created by **Quique Tortosa** -- [**socquique/TamaPoke**](https://github.com/socquique/TamaPoke):
> the firmware, the sprite pipeline, the web installer, the whole thing. This repo builds on that work;
> if you like it, go **[star the original](https://github.com/socquique/TamaPoke)**.
> It also takes code from [**DylanPDao/TamaPoke**](https://github.com/DylanPDao/TamaPoke) by **Dylan Dao**.
> The 3.14 base is maintained by **Cheng Ren** -- [**crenberries428/TamaPoke**](https://github.com/crenberries428/TamaPoke).
> The Fun Pack is by **Salchi** ([botero50](https://github.com/botero50)). MIT, on the same terms as all of the above.


## Credits

Original project: [socquique/TamaPoke](https://github.com/socquique/TamaPoke) by
Quique Tortosa. Code also taken from the fork
[DylanPDao/TamaPoke](https://github.com/DylanPDao/TamaPoke) by Dylan Dao.
UI font: Fredoka (SIL OFL 1.1). Cries: pret/pokered and pret/pokecrystal.
All sprites: [PMD SpriteCollab](https://github.com/PMDCollab/SpriteCollab)
(community, CC BY-NC). Base stats: [PokéAPI](https://pokeapi.co). Pokémon is a ™ of
Nintendo / Game Freak / The Pokémon Company. Non-commercial, personal-use project.
Full list in [`CREDITS.md`](CREDITS.md).

## License

- **Source code** (firmware + tooling): **[MIT](LICENSE)** — © Cheng Ren, and
  © Quique Tortosa for the original TamaPoke.
- **Sprites & names**: © Nintendo / Game Freak / The Pokémon Company; pixel art
  from [PMD SpriteCollab](https://github.com/PMDCollab/SpriteCollab) (CC BY-NC 4.0).
  **Non-commercial use only.**
- **3D-printed case**: remix of *"Pokeball"* by **yoyothechicken**
  ([MakerWorld #839922](https://makerworld.com/es/models/839922-pokeball)),
  licensed **CC BY-NC-SA**, and shared here under the same terms.

This is an unofficial fan project, not affiliated with or endorsed by Nintendo.

