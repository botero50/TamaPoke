# Fun Pack roadmap

What is left, in the agreed order. Each phase gets a short written plan that is
approved before it is built; small fixes skip that. This file is updated at the
end of every phase.

_Last updated: 2 November 2026, with Phase 26._

## Done
| Phase | What | Release |
|---|---|---|
| 1-20 | Minigames, sickness and statuses, care, shop, hats, room, outfits, town life, quests, events, photo album, music | v1.0-v1.5 |
| 21 | The Nursery: two favourites leave an egg for the next raising | v1.6 |
| 22 | Wi-Fi time sync and the real weather (Open-Meteo) | v1.7 |
| 23 | Weather polish: fog, thunder, snow on the ground, puddles, snow from New York, 30-minute refresh, Wi-Fi diagnostics | v1.7 |
| 24 | Restful nights (asleep 20:00-08:00 the bars hold; bedtime from 21:00; an unplugged night counts as sleep) and the try-on mirror in the shop | v1.8 |
| 25 (prepared) | Show it to the world: posts in `Desktop\Tamapoke-backup\share`, social preview image, new tour GIF, repo description, a short README with "Build your own" and the screens | on GitHub |
| 26 | Arcade night: tickets from the Fun Pack games (30 a day), a prize counter (arcade cap, star visor, arcade cabinet, capsule, cake), the weekly records board, bronze/silver/gold badges per game; plus a Wi-Fi name typed in any letter case still connects | v1.9 (pending) |

## Next
| # | Item | Needs | Notes |
|---|---|---|---|
| 1 | **Checks on the real ball** | the family | Nursery sprites and the next-day egg; SD photo (DIARY -> ALBUM -> PHOTO); tilt direction in the maze, Catch and Balance; how hard the shake race is; New York snow on its first snowy day; a hat in the mirror sits as it does once bought; the arcade prizes. (Wi-Fi sync and weather: confirmed working on 2 Nov.) |
| 2 | **Phase 25: the posts** | the owner's accounts | MakerWorld thank-you and Make, r/esp32, r/tamagotchi, Hackster, r/arduino, Hackaday, in the order of `checklist.md`. No faces, names, city or Wi-Fi name |
| 3 | **Phase 27: friend visits and trading** | a second ball | Uses the existing ESP-NOW battle link; cannot be tested with one ball |
| 4 | **Battery saver** | a battery fitted | The ball reports no battery now (`bat=-1`) |
| 5 | **SD content packs** | only if space runs out | Firmware at ~75%; revisit near 90% |
| 6 | **Translations review** | native speakers | FR, DE, IT, PT, ZH and KO were machine-written |

## Decisions on record
- **Weather:** Open-Meteo (free, no key). The location comes from the browser, rounded
  to two decimals (~1 km). Refresh every 30 minutes with a place, every 6 hours without.
  Weather older than 3 hours is dropped.
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
- **Posting:** crenberries428's repo has issues and discussions off, so the thank-you note
  goes on their MakerWorld case page. Printables dropped: the case is only on MakerWorld.
