#include "album.h"
#include <Preferences.h>
#include <string.h>

static void put16(uint8_t *p, uint16_t v) { p[0] = v & 0xFF; p[1] = v >> 8; }
static void put32(uint8_t *p, uint32_t v) { for (int i = 0; i < 4; i++) p[i] = (v >> (8 * i)) & 0xFF; }

void albumBmpHeader(uint8_t h[ALBUM_BMP_HDR]) {
  memset(h, 0, ALBUM_BMP_HDR);
  h[0] = 'B'; h[1] = 'M';
  put32(h + 2, ALBUM_BMP_SIZE);                // the whole file
  put32(h + 10, ALBUM_BMP_HDR);                // where the pixels start
  put32(h + 14, 40);                           // BITMAPINFOHEADER
  put32(h + 18, ALBUM_W);
  put32(h + 22, (uint32_t)(-(int32_t)ALBUM_W));   // negative: rows top-down, as the screen
  put16(h + 26, 1);                            // planes
  put16(h + 28, 16);                           // bits per pixel
  put32(h + 30, 3);                            // BI_BITFIELDS
  put32(h + 34, ALBUM_BMP_ROW * ALBUM_W);      // the pixel data
  put32(h + 38, 2835); put32(h + 42, 2835);    // 72 dpi
  put32(h + 54, 0xF800); put32(h + 58, 0x07E0); put32(h + 62, 0x001F);   // RGB565
}

void Album::clear() { memset(s, 0, sizeof(s)); nextSeq = 1; }

uint8_t Album::count() const {
  uint8_t n = 0;
  for (int i = 0; i < ALBUM_MAX; i++) n += (s[i].flags & AS_USED) ? 1 : 0;
  return n;
}

uint8_t Album::favCount() const {
  uint8_t n = 0;
  for (int i = 0; i < ALBUM_MAX; i++) n += ((s[i].flags & (AS_USED | AS_FAV)) == (AS_USED | AS_FAV)) ? 1 : 0;
  return n;
}

// newest first: the newest-th largest sequence number among the used slots
int8_t Album::slotFor(uint8_t newest) const {
  bool taken[ALBUM_MAX] = { false };
  int8_t best = -1;
  for (uint8_t k = 0; k <= newest; k++) {
    best = -1;
    for (int i = 0; i < ALBUM_MAX; i++)
      if ((s[i].flags & AS_USED) && !taken[i] && (best < 0 || s[i].seq > s[best].seq)) best = (int8_t)i;
    if (best < 0) return -1;
    taken[best] = true;
  }
  return best;
}

int8_t Album::pickForSave() const {
  for (int i = 0; i < ALBUM_MAX; i++) if (!(s[i].flags & AS_USED)) return (int8_t)i;
  int8_t old = -1;
  for (int i = 0; i < ALBUM_MAX; i++)
    if (!(s[i].flags & AS_FAV) && (old < 0 || s[i].seq < s[old].seq)) old = (int8_t)i;
  return old;
}

void Album::markSaved(uint8_t slot) {
  if (slot >= ALBUM_MAX) return;
  s[slot].seq = nextSeq++;
  s[slot].flags = AS_USED;                     // a new photo is not a favourite yet
}

bool Album::remove(uint8_t slot) {
  if (slot >= ALBUM_MAX || !(s[slot].flags & AS_USED)) return false;
  s[slot].flags = 0; s[slot].seq = 0;
  return true;
}

bool Album::toggleFav(uint8_t slot) {
  if (slot >= ALBUM_MAX || !(s[slot].flags & AS_USED)) return false;
  if (s[slot].flags & AS_FAV) { s[slot].flags &= ~AS_FAV; return true; }
  if (favCount() >= ALBUM_FAV_MAX) return false;
  s[slot].flags |= AS_FAV;
  return true;
}

uint8_t Album::removeAllButFav(uint8_t out[ALBUM_MAX]) {
  uint8_t n = 0;
  for (int i = 0; i < ALBUM_MAX; i++)
    if ((s[i].flags & AS_USED) && !(s[i].flags & AS_FAV)) { remove((uint8_t)i); out[n++] = (uint8_t)i; }
  return n;
}

// Before phase 33 photo k of n went to slot k % 24, newest at (n - 1) % 24.
void Album::fromOldCounter(uint16_t n) {
  clear();
  uint8_t have = n > ALBUM_MAX ? ALBUM_MAX : (uint8_t)n;
  for (uint8_t i = 0; i < have; i++) {
    uint8_t slot = (uint8_t)((n - 1 - i) % ALBUM_MAX);
    s[slot].flags = AS_USED;
    s[slot].seq = (uint16_t)(have - i);        // the newest gets the largest number
  }
  nextSeq = (uint16_t)(have + 1);
}

void Album::load() {
  Preferences p;
  p.begin("album", true);
  if (p.getBytesLength("slots") == sizeof(s)) {
    p.getBytes("slots", s, sizeof(s));
    nextSeq = p.getUShort("seq", 1);
    p.end();
    return;
  }
  uint16_t n = p.getUShort("n", 0);            // an album from before phase 33
  p.end();
  fromOldCounter(n);
  if (n) save();
}

void Album::save() const {
  Preferences p;
  p.begin("album", false);
  p.putBytes("slots", s, sizeof(s));
  p.putUShort("seq", nextSeq);
  p.end();
}
