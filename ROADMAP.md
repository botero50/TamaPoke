# Fun Pack roadmap

What is left, in the agreed order. Each phase gets a short written plan that is
approved before it is built; small fixes skip that. This file is updated at the
end of every phase.

_Last updated: 2 November 2026, with v1.7._

## Done
| Phase | What | Release |
|---|---|---|
| 1-20 | Minigames, sickness and statuses, care, shop, hats, room, outfits, town life, quests, events, photo album, music | v1.0-v1.5 |
| 21 | The Nursery: two favourites leave an egg for the next raising | v1.6 |
| 22 | Wi-Fi time sync and the real weather (Open-Meteo) | v1.7 |
| 23 | Weather polish: fog, thunder, snow on the ground, puddles, snow from New York, 30-minute refresh, Wi-Fi diagnostics | v1.7 |

## Next
| # | Item | Needs | Notes |
|---|---|---|---|
| 1 | **Checks on the real ball** | the family | Nursery sprites and the next-day egg; SD photo (DIARY -> ALBUM -> PHOTO); tilt direction in the maze, Catch and Balance; how hard the shake race is; New York snow on its first snowy day. (Wi-Fi sync and weather: confirmed working on 2 Nov.) |
| 1b | **Small fix: Wi-Fi name in any case** | nothing | Accept "alejandro" for "Alejandro" by matching the scan; the first setup failed on exactly that |
| 2 | **Phase 24: outfit try-on mirror** | nothing | Small: see a hat or accessory on the pet before buying |
| 3 | **Phase 25: arcade extras** | nothing | Game tickets, a weekly high-score board, badges for beating records |
| 4 | **Phase 26: friend visits and trading** | a second ball | Uses the existing ESP-NOW battle link; cannot be tested with one ball |
| 5 | **Battery saver** | a battery fitted | The ball reports no battery now (`bat=-1`) |
| 6 | **SD content packs** | only if space runs out | Firmware at ~74%; revisit near 90% |
| 7 | **Translations review** | native speakers | FR, DE, IT, PT, ZH and KO were machine-written |
| 8 | **Share drafts** | the owner | Update from v1.0 to the current version before posting |

## Decisions on record
- **Weather:** Open-Meteo (free, no key). The location comes from the browser, rounded
  to two decimals (~1 km). Refresh every 30 minutes with a place, every 6 hours without.
  Weather older than 3 hours is dropped.
- **Clock jumps** from a Wi-Fi sync apply without replaying the missing time on the pet.
- **The Wi-Fi password** lives in its own NVS namespace and never goes into a backup.
- **Snow from New York:** only on days that are clear or cloudy at home; anything real
  at home wins. The temperature stays local. Switch on Settings -> About and the installer.
- **The Nursery** copies both parents at drop-off, so releasing one later changes nothing.
