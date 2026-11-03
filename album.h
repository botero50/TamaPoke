#pragma once
#include <stdint.h>
#include <stddef.h>

// The photo album's bookkeeping (phase 33): which of the 24 slots on the SD card
// hold a photo, how new each one is, and which are favourites. No SD code here,
// so album_test can drive it; sdmon.cpp (the card) and the emulator's
// host_impl.cpp (memory) keep the pixels.
//
// The album never grows past ALBUM_MAX photos (about 2.6 MB): a new photo takes
// an empty slot, else the oldest that is not a favourite. At most ALBUM_FAV_MAX
// can be favourites, so there is always a slot for the next one.
#define ALBUM_MAX 24
#define ALBUM_W 233
#define ALBUM_FAV_MAX 20

// A photo on the card is a BMP a computer can open: 16-bit RGB565 with bit masks,
// stored top-down, each row padded to a multiple of 4 bytes.
#define ALBUM_BMP_HDR 66
#define ALBUM_BMP_ROW 468                      // 233 * 2 = 466, padded
#define ALBUM_BMP_SIZE (ALBUM_BMP_HDR + ALBUM_BMP_ROW * ALBUM_W)
void albumBmpHeader(uint8_t out[ALBUM_BMP_HDR]);

enum : uint8_t { AS_USED = 1, AS_FAV = 2 };
struct AlbumSlot { uint16_t seq; uint8_t flags; uint8_t pad; };

struct Album {
  AlbumSlot s[ALBUM_MAX];
  uint16_t nextSeq = 1;

  void clear();
  uint8_t count() const;
  uint8_t favCount() const;
  int8_t slotFor(uint8_t newest) const;        // the slot of the newest-th photo (0 = newest), or -1
  int8_t pickForSave() const;                  // an empty slot, else the oldest non-favourite
  void markSaved(uint8_t slot);                // a new photo was written there
  bool remove(uint8_t slot);
  bool isFav(uint8_t slot) const { return slot < ALBUM_MAX && (s[slot].flags & AS_FAV); }
  bool toggleFav(uint8_t slot);                // false when that would pass ALBUM_FAV_MAX
  uint8_t removeAllButFav(uint8_t out[ALBUM_MAX]);   // the slots emptied, and how many
  void fromOldCounter(uint16_t n);             // the ring used before phase 33: n photos ever taken
  void load();                                 // NVS "album": "slots" + "seq", or the old "n"
  void save() const;
};
