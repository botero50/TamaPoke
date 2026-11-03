// The photo album's bookkeeping (phase 33, album.cpp): it never holds more than
// ALBUM_MAX photos, overwrites the oldest non-favourite, keeps favourites, deletes,
// survives a reboot and carries over the album from before phase 33. Plus the
// BMP header every photo on the card starts with.
#include "Arduino.h"
#include "Preferences.h"
#include "pet.h"
#include "party.h"
#include "album.h"
#include <cstdio>
#include <cstring>
uint32_t g_seed=31; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
volatile int g_touchX=0,g_touchY=0; volatile bool g_touchDown=false; bool wasPressed=false;
static uint32_t g_ms=0;
uint32_t millis(){return g_ms;} void FakeESP::restart(){exit(0);}
int FakeSerial::available(){return 0;} String FakeSerial::readStringUntil(char){return String("");}
void sfxPlay(uint8_t){}
static int bad=0;
static void ck(bool ok,const char*w){printf("%s  %s\n",ok?"PASS":"FAIL",w); if(!ok)bad++;}

static uint32_t rd32(const uint8_t *p){ return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24); }
static uint16_t rd16(const uint8_t *p){ return p[0] | (p[1] << 8); }

// "take" a photo: pick, then mark, as albumSave does
static int8_t take(Album &a){ int8_t s = a.pickForSave(); if (s >= 0) a.markSaved((uint8_t)s); return s; }

int main(){
  // --- filling up, then the oldest goes
  {
    Album a; a.clear();
    ck(a.count() == 0 && a.slotFor(0) == -1, "an empty album");
    int8_t first = take(a);
    for (int i = 1; i < ALBUM_MAX; i++) take(a);
    ck(a.count() == ALBUM_MAX, "24 photos fill it");
    int8_t newest = a.slotFor(0);
    int8_t s = take(a);
    ck(s == first && a.count() == ALBUM_MAX, "the 25th takes the oldest's place: it never grows");
    ck(a.slotFor(0) == s && a.slotFor(1) == newest, "and it is now the newest, the old newest next");
    ck(a.slotFor(ALBUM_MAX) == -1, "nothing past the last");
  }

  // --- favourites are kept, up to ALBUM_FAV_MAX
  {
    Album a; a.clear();
    for (int i = 0; i < ALBUM_MAX; i++) take(a);
    int8_t oldest = a.slotFor(ALBUM_MAX - 1);
    ck(a.toggleFav((uint8_t)oldest) && a.isFav((uint8_t)oldest) && a.favCount() == 1, "the oldest is starred");
    int8_t s = take(a);
    ck(s != oldest && a.isFav((uint8_t)oldest), "a new photo passes over the favourite");
    ck(a.toggleFav((uint8_t)oldest) && !a.isFav((uint8_t)oldest), "and the star comes off again");
    Album b; b.clear();
    for (int i = 0; i < ALBUM_MAX; i++) take(b);
    int ok = 0;
    for (int i = 0; i < ALBUM_MAX; i++) ok += b.toggleFav((uint8_t)i) ? 1 : 0;
    ck(ok == ALBUM_FAV_MAX && b.favCount() == ALBUM_FAV_MAX, "no more than 20 favourites");
    bool kept = true;
    for (int k = 0; k < 40; k++) { int8_t t = take(b); kept = kept && t >= 0 && !b.isFav((uint8_t)t); }
    ck(kept && b.favCount() == ALBUM_FAV_MAX && b.count() == ALBUM_MAX, "40 more photos: always a slot, the 20 favourites untouched");
  }

  // --- deleting
  {
    Album a; a.clear();
    for (int i = 0; i < 6; i++) take(a);
    int8_t second = a.slotFor(1), third = a.slotFor(2);
    ck(a.remove((uint8_t)second) && a.count() == 5 && a.slotFor(1) == third, "deleting the 2nd: the 3rd moves up");
    ck(!a.remove((uint8_t)second), "an empty slot cannot be deleted twice");
    ck(take(a) == second, "the gap is filled first");
    a.toggleFav((uint8_t)a.slotFor(3));
    a.toggleFav((uint8_t)a.slotFor(5));
    uint8_t gone[ALBUM_MAX];
    uint8_t n = a.removeAllButFav(gone);
    ck(n == 4 && a.count() == 2 && a.favCount() == 2, "delete all: only the 2 favourites stay");
    ck(a.isFav((uint8_t)a.slotFor(0)) && a.isFav((uint8_t)a.slotFor(1)), "and they are all that is listed");
    ck(!a.toggleFav(gone[0]), "an empty slot cannot be starred");
  }

  // --- it survives a reboot
  {
    nvs().clear();
    Album a; a.clear();
    for (int i = 0; i < 9; i++) take(a);
    a.toggleFav((uint8_t)a.slotFor(4));
    a.remove((uint8_t)a.slotFor(0));
    a.save();
    Album b; b.load();
    bool same = b.count() == a.count() && b.favCount() == 1 && b.nextSeq == a.nextSeq;
    for (int i = 0; i < 8; i++) same = same && b.slotFor(i) == a.slotFor(i);
    ck(same, "the slots, their order and the star survive a reboot");
  }

  // --- the album from before phase 33: n photos taken, photo k in slot k % 24
  {
    struct { uint16_t n; uint8_t count; int8_t newest, oldest; } C[3] = { { 0, 0, -1, -1 }, { 5, 5, 4, 0 }, { 30, 24, 5, 6 } };
    for (auto &c : C) {
      nvs().clear();
      { Preferences p; p.begin("album", false); if (c.n) p.putUShort("n", c.n); p.end(); }
      Album a; a.load();
      char w[80];
      snprintf(w, sizeof(w), "old album of %u: %u photos, newest slot %d, oldest slot %d", c.n, c.count, c.newest, c.oldest);
      ck(a.count() == c.count && a.slotFor(0) == c.newest && (c.count == 0 || a.slotFor(c.count - 1) == c.oldest), w);
      if (c.n == 30) {
        int8_t s = take(a);
        ck(s == 6 && a.slotFor(0) == 6, "and the next photo overwrites the oldest, as before");
        Album b; b.load();
        ck(b.count() == 24 && b.slotFor(0) == 5, "the carried-over album was saved in the new form (before that photo)");
      }
    }
  }

  // --- the BMP header: a 233 x 233, 16-bit RGB565 picture, top-down
  {
    uint8_t h[ALBUM_BMP_HDR];
    albumBmpHeader(h);
    ck(h[0] == 'B' && h[1] == 'M', "it starts BM");
    ck(rd32(h + 2) == ALBUM_BMP_SIZE && ALBUM_BMP_SIZE == 66 + 468 * 233, "the file size: header plus 233 rows of 468 bytes");
    ck(rd32(h + 10) == 66 && rd32(h + 14) == 40, "pixels at 66, a 40-byte info header");
    ck((int32_t)rd32(h + 18) == 233 && (int32_t)rd32(h + 22) == -233, "233 wide, 233 tall top-down");
    ck(rd16(h + 26) == 1 && rd16(h + 28) == 16 && rd32(h + 30) == 3, "one plane, 16 bits, BI_BITFIELDS");
    ck(rd32(h + 54) == 0xF800 && rd32(h + 58) == 0x07E0 && rd32(h + 62) == 0x001F, "the RGB565 masks");
    ck(ALBUM_BMP_ROW % 4 == 0 && ALBUM_BMP_ROW >= ALBUM_W * 2, "rows padded to 4 bytes");
    ck(ALBUM_MAX * ALBUM_BMP_SIZE < 2700000, "the whole album stays under 2.7 MB");
  }

  if (bad) printf("FAILURES\n");
  return bad?1:0;
}
