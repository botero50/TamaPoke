#include <Preferences.h>
#include "sdmon.h"
#include "noart.h"
#include "pin_config.h"
#include "pet.h"   // gRegionArt, REGIONS -- the mask this narrows
#include <FS.h>
#include <SD_MMC.h>
#include <nvs.h>

bool sdReady = false;
bool sdDirty = false;
bool sdArtDirty = false;
SdThumbs thumbs;

bool PmdMon::load(int16_t dexNum, bool shiny) {
  // int16_t, NOT uint8_t. The dex reached 386 and this did not follow, so
  // everything from 256 up wrapped into Kanto: MARSHTOMP (258) opened
  // p002.bin and drew an IVYSAUR. Same trap that caught DexEntry::evolvesTo
  // and TrainerMon::dex when the expansion landed.
  if (dexNum < 1 || dexNum > 999) return false;
  unload();
  if (!sdReady) return false;

  char path[28];
  snprintf(path, sizeof(path), "/mons/p%s%03u.bin", shiny ? "s" : "", (unsigned)dexNum);
  File f = SD_MMC.open(path, FILE_READ);
  if (!f && shiny) {  // sin shiny PMD: usa el normal
    snprintf(path, sizeof(path), "/mons/p%03u.bin", (unsigned)dexNum);
    f = SD_MMC.open(path, FILE_READ);
  }
  if (!f) return false;

  uint32_t size = f.size();
  if (size < 7 || size > 3UL * 1024 * 1024) { f.close(); return false; }
  blob = (uint8_t *)ps_malloc(size);
  if (!blob || f.read(blob, size) != size || memcmp(blob, "TPK2", 4) != 0) {
    if (blob) { free(blob); blob = nullptr; }
    f.close();
    return false;
  }
  f.close();

  dex = dexNum;                  // what is actually in here, for the tests
  uint8_t nActs = blob[4];
  memcpy(&palCount, blob + 5, 2);
  if (palCount > 256 || (uint32_t)7 + palCount * 2 > size) { unload(); return false; }
  memcpy(pal, blob + 7, palCount * 2);

  const uint8_t *p = blob + 7 + palCount * 2;
  const uint8_t *end = blob + size;
  for (uint8_t i = 0; i < nActs && p + 4 <= end; i++) {
    uint8_t id = p[0], w = p[1], h = p[2], nf = p[3];
    p += 4;
    if (id >= PMD_NACTS || nf > 24) { unload(); return false; }
    // checks that ms[] and the frame data fit in the blob (truncated file)
    uint32_t bytes = (uint32_t)nf * 2 + (uint32_t)w * h * nf;
    if (w == 0 || h == 0 || nf == 0 || p + bytes > end) { unload(); return false; }
    PmdAct &a = acts[id];
    a.w = w;
    a.h = h;
    a.frames = nf;
    for (uint8_t k = 0; k < nf; k++) {
      a.ms[k] = pmdFrameMs(p[0] | (p[1] << 8));
      p += 2;
    }
    a.data = p;
    p += (uint32_t)w * h * nf;
    // lowest row with content in any frame: anchor by the feet
    uint8_t base = 1;
    for (uint8_t f = 0; f < nf; f++) {
      const uint8_t *fr = a.data + (uint32_t)f * w * h;
      for (int r = h - 1; r >= 0; r--) {
        bool any = false;
        for (int c = 0; c < w && !any; c++)
          if (fr[r * w + c] != 0xFF) any = true;
        if (any) { if (r + 1 > base) base = r + 1; break; }
      }
    }
    a.base = base;
  }
  loaded = true;
  Serial.printf("loaded %s (%u KB)\n", path, size / 1024);
  return true;
}

void PmdMon::unload() {
  if (blob) {
    free(blob);
    blob = nullptr;
  }
  for (auto &a : acts) {
    a.w = a.h = a.frames = a.base = 0;
    a.data = nullptr;
  }
  loaded = false;
}

bool SdThumbs::load() {
  if (!sdReady) return false;
  File f = SD_MMC.open("/mons/thumbs.bin", FILE_READ);
  if (!f) {
    Serial.println("no thumbs.bin (gallery without thumbnails)");
    return false;
  }
  uint32_t sz = f.size();
  if (sz < 10 || sz > 4UL * 1024 * 1024) {   // a real one is under 1 MB
    Serial.println("thumbs.bin invalido");
    f.close();
    return false;
  }
  data = (uint8_t *)ps_malloc(sz);
  if (!data || f.read(data, sz) != sz || memcmp(data, "TPTH", 4) != 0) {
    Serial.println("thumbs.bin invalido");
    if (data) { free(data); data = nullptr; }
    f.close();
    return false;
  }
  f.close();
  memcpy(&count, data + 4, 2);
  if (6UL + 4UL * count > sz) {              // the offset table must fit
    Serial.println("thumbs.bin invalido");
    free(data); data = nullptr; count = 0;
    return false;
  }
  size = sz;
  loaded = true;
  Serial.printf("thumbnails loaded: %u (%u KB)\n", count, sz / 1024);
  return true;
}

const uint8_t *SdThumbs::get(int16_t dex) const {
  return loaded ? thumbAt(data, size, count, dex) : nullptr;
}

// Which regions actually have their sprite pack on the card.
//
// Three probes per region, not one: the realistic failure is a half-finished
// copy of a 100 MB pack, and a single probe would call that region present.
// Start, middle and end, so a copy that stopped partway fails the check.
//
// This NARROWS gRegionArt, which starts as everything. A board with no SD is
// therefore untouched and keeps today's behaviour, which is the documented
// requirement -- an empty game would be a worse answer than a graceful one.
void sdScanRegionArt(bool verbose) {
  if (!sdReady) return;                 // no card: leave every region enabled
  uint16_t mask = 0;
  for (uint8_t r = 0; r < REGION_COUNT; r++) {
    if (r == REGION_ALL) continue;      // derived from the others, never probed
    const RegionInfo &rg = REGIONS[r];
    const int16_t probe[3] = { sdRegionProbe(r, 0), sdRegionProbe(r, 1),
                               sdRegionProbe(r, 2) };
    bool all = true;
    for (int i = 0; i < 3 && all; i++) {
      char path[28];
      snprintf(path, sizeof(path), "/mons/p%03u.bin", (unsigned)probe[i]);
      File f = SD_MMC.open(path, FILE_READ);
      if (!f) all = false; else f.close();
    }
    if (all) mask |= (uint16_t)(1u << r);
    if (verbose)
      Serial.printf("art: %-6s %s\n", rg.name, all ? "yes" : "NO (pack missing)");
  }
  gRegionArt = mask;
}

bool sdBegin() {
  SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_DATA);
  sdReady = SD_MMC.begin("/sdcard", true /* modo 1-bit */, true /* formatea si no monta */);
  if (sdReady) {
    Serial.printf("SD mounted: %llu MB\n", SD_MMC.cardSize() / (1024ULL * 1024ULL));
    SD_MMC.mkdir("/mons");
    sdScanRegionArt();
  } else {
    Serial.println("SD not detected (the game uses the flash sprites)");
  }
  return sdReady;
}

// ---------- the safety copies of the save (phase 39) ----------

static void copyPath(char *out, size_t n, uint8_t slot) {
  snprintf(out, n, "/save/safe%u.tps", (unsigned)(slot & 1));
}

bool sdCopyWrite(uint8_t slot, const uint8_t *data, size_t n) {
  if (!sdReady) return false;
  SD_MMC.mkdir("/save");
  char path[24];
  copyPath(path, sizeof(path), slot);
  File f = SD_MMC.open(path, FILE_WRITE);
  if (!f) return false;
  bool ok = f.write(data, n) == n;
  f.close();
  return ok;
}

size_t sdCopyRead(uint8_t slot, uint8_t *out, size_t cap) {
  if (!sdReady) return 0;
  char path[24];
  copyPath(path, sizeof(path), slot);
  File f = SD_MMC.open(path, FILE_READ);
  if (!f) return 0;
  size_t size = f.size(), got = 0;
  if (size && size <= cap) got = f.read(out, size);
  f.close();
  return got == size ? got : 0;
}

void sdCopyClear() {
  if (!sdReady) return;
  for (uint8_t k = 0; k < 2; k++) {
    char path[24];
    copyPath(path, sizeof(path), k);
    if (SD_MMC.exists(path)) SD_MMC.remove(path);
  }
}

bool nvsUsage(uint16_t *used, uint16_t *total) {
  nvs_stats_t st;
  if (nvs_get_stats(NULL, &st) != ESP_OK) return false;
  *used = (uint16_t)st.used_entries;
  *total = (uint16_t)st.total_entries;
  return true;
}

bool SdMon::load(int16_t dexNum, bool shiny) {
  if (dexNum < 1 || dexNum > 999) return false;
  unload();
  if (!sdReady) return false;

  char path[24];
  snprintf(path, sizeof(path), "/mons/%s%03u.bin", shiny ? "s" : "", (unsigned)dexNum);
  File f = SD_MMC.open(path, FILE_READ);
  if (!f && shiny) {  // no shiny variant: use the normal one
    snprintf(path, sizeof(path), "/mons/%03u.bin", (unsigned)dexNum);
    f = SD_MMC.open(path, FILE_READ);
  }
  if (!f) {
    Serial.printf("%s not found\n", path);
    return false;
  }

  char magic[4];
  uint16_t header[4];
  if (f.read((uint8_t *)magic, 4) != 4 || memcmp(magic, "TPK1", 4) != 0 ||
      f.read((uint8_t *)header, 8) != 8) {
    f.close();
    return false;
  }
  w = header[0];
  h = header[1];
  frames = header[2];
  frameMs = header[3];
  // bounds the dimensions: avoids an overflowed or absurd size with a corrupt file
  if (f.read((uint8_t *)&palCount, 2) != 2 || palCount > 256 ||
      w == 0 || w > 256 || h == 0 || h > 256 || frames == 0 || frames > 64) {
    f.close();
    return false;
  }
  if (f.read((uint8_t *)pal, palCount * 2) != palCount * 2) {
    f.close();
    return false;
  }

  uint32_t size = (uint32_t)w * h * frames;
  data = (uint8_t *)ps_malloc(size);
  if (!data) {
    Serial.println("no PSRAM for the sprite");
    f.close();
    return false;
  }
  uint32_t got = f.read(data, size);
  f.close();
  if (got != size) {
    Serial.printf("%s truncado (%u de %u)\n", path, got, size);
    unload();
    return false;
  }

  // integer zoom so the creature is ~200 px tall on screen
  scale = 200 / h;
  if (scale < 2) scale = 2;
  if (scale > 5) scale = 5;

  Serial.printf("loaded %s: %ux%u x%u frames @%ums, scale %u\n",
                path, w, h, frames, frameMs, scale);
  loaded = true;
  return true;
}

void SdMon::unload() {
  if (data) {
    free(data);
    data = nullptr;
  }
  loaded = false;
}

// ---------------------------------------------------------------------------
// USB upload protocol (to fill the SD without taking it out of the board):
//   PUT <path> <bytes>\n  + raw data   -> "OK" ... "DONE"
//   LS\n                                   -> listing of /mons
// Usar con tools/send_sd.py
// ---------------------------------------------------------------------------

bool sdSerialCommand(const String &line) {
  if (line.startsWith("PUT ")) {
    int sp = line.lastIndexOf(' ');
    String path = line.substring(4, sp);
    uint32_t size = line.substring(sp + 1).toInt();
    if (!sdReady || size == 0 || size > 4 * 1024 * 1024) {
      Serial.println("ERR");
      return true;
    }
    if (!path.startsWith("/")) path = "/" + path;
    // Sprites only: this path arrives straight from the serial line, and the save's
    // safety copies live on this same card.
    if (!path.startsWith("/mons/") || path.indexOf("..") >= 0) {
      Serial.println("ERR");
      return true;
    }
    SD_MMC.remove(path);    // a retry replaces a half-sent file, never adds to it
    File f = SD_MMC.open(path, FILE_WRITE);
    if (!f) {
      Serial.println("ERR");
      return true;
    }
    Serial.println("OK");
    static uint8_t buf[2048];
    uint32_t remaining = size;
    Serial.setTimeout(5000);
    while (remaining > 0) {
      size_t want = remaining > sizeof(buf) ? sizeof(buf) : remaining;
      size_t n = Serial.readBytes(buf, want);
      if (n == 0) break;  // timeout
      if (f.write(buf, n) != n) break;   // a full or failing card must not report DONE
      remaining -= n;
      Serial.println("#");  // ack: ready for the next block
    }
    f.close();
    if (remaining) SD_MMC.remove(path);  // never leave a cut file behind to load later
    Serial.setTimeout(1000);
    sdDirty = (remaining == 0);
    // A pack file just landed, so which regions are playable may have changed.
    // Flag it rather than rescanning here: this runs between the last data block
    // and the DONE the host is waiting on, and 15 file opens belong nowhere near
    // that. loop() picks it up.
    if (remaining == 0) sdArtDirty = true;
    Serial.println(remaining == 0 ? "DONE" : "ERR");
    return true;
  } else if (line == "LS") {
    File dir = SD_MMC.open("/mons");
    if (dir) {
      File e;
      while ((e = dir.openNextFile())) {
        Serial.printf("%s %u\n", e.name(), (uint32_t)e.size());
        e.close();
      }
      dir.close();
    }
    Serial.println("DONE");
    return true;
  }
  return false;
}

// ---------------------------------------------------------------------------
// The photo album. Which slots hold what lives in its own NVS namespace (album.h):
// it belongs to the card's files, not to the pet's save.
// ---------------------------------------------------------------------------

static Album gAlbum;
static bool gAlbumLoaded = false;
static Album &album() {
  if (!gAlbumLoaded) { gAlbum.load(); gAlbumLoaded = true; }
  return gAlbum;
}
static void albumPath(char *b, size_t n, uint8_t slot, bool bmp) {
  snprintf(b, n, "/album/%02u.%s", (unsigned)slot, bmp ? "bmp" : "raw");
}
static void albumRemoveFiles(uint8_t slot) {
  char p[24];
  albumPath(p, sizeof(p), slot, true);  if (SD_MMC.exists(p)) SD_MMC.remove(p);
  albumPath(p, sizeof(p), slot, false); if (SD_MMC.exists(p)) SD_MMC.remove(p);
}
// one photo as a BMP: the header, then each row with two bytes of padding
static bool albumWriteBmp(uint8_t slot, const uint16_t *px, int stride, int step) {
  char path[24];
  albumPath(path, sizeof(path), slot, true);
  File f = SD_MMC.open(path, FILE_WRITE);
  if (!f) return false;
  uint8_t h[ALBUM_BMP_HDR];
  albumBmpHeader(h);
  f.write(h, sizeof(h));
  static uint16_t row[ALBUM_BMP_ROW / 2];
  row[ALBUM_W] = 0;
  for (int y = 0; y < ALBUM_W; y++) {
    for (int x = 0; x < ALBUM_W; x++) row[x] = px[(y * step) * stride + x * step];
    f.write((const uint8_t *)row, ALBUM_BMP_ROW);
  }
  f.close();
  return true;
}

bool albumSave(const uint16_t *fb) {
  if (!sdReady || !fb) return false;
  SD_MMC.mkdir("/album");
  Album &a = album();
  int8_t slot = a.pickForSave();
  if (slot < 0) return false;
  albumRemoveFiles((uint8_t)slot);
  if (!albumWriteBmp((uint8_t)slot, fb, 466, 2)) return false;   // every other pixel: half size
  a.markSaved((uint8_t)slot);
  a.save();
  return true;
}

uint8_t albumCount() { return album().count(); }

bool albumLoad(uint8_t newest, uint16_t *out) {
  if (!sdReady || !out) return false;
  int8_t slot = album().slotFor(newest);
  if (slot < 0) return false;
  char path[24];
  albumPath(path, sizeof(path), (uint8_t)slot, true);
  File f = SD_MMC.open(path, FILE_READ);
  if (f) {
    bool ok = f.size() == ALBUM_BMP_SIZE && f.seek(ALBUM_BMP_HDR);
    static uint8_t row[ALBUM_BMP_ROW];
    for (int y = 0; ok && y < ALBUM_W; y++) {
      ok = f.read(row, ALBUM_BMP_ROW) == ALBUM_BMP_ROW;
      if (ok) memcpy(out + y * ALBUM_W, row, ALBUM_W * 2);
    }
    f.close();
    return ok;
  }
  albumPath(path, sizeof(path), (uint8_t)slot, false);   // not converted yet
  f = SD_MMC.open(path, FILE_READ);
  if (!f) return false;
  size_t want = ALBUM_W * ALBUM_W * 2;
  size_t got = f.read((uint8_t *)out, want);
  f.close();
  return got == want;
}

bool albumIsFav(uint8_t newest) {
  int8_t slot = album().slotFor(newest);
  return slot >= 0 && album().isFav((uint8_t)slot);
}

bool albumToggleFav(uint8_t newest) {
  int8_t slot = album().slotFor(newest);
  if (slot < 0 || !album().toggleFav((uint8_t)slot)) return false;
  album().save();
  return true;
}

bool albumDelete(uint8_t newest) {
  int8_t slot = album().slotFor(newest);
  if (slot < 0 || !album().remove((uint8_t)slot)) return false;
  if (sdReady) albumRemoveFiles((uint8_t)slot);
  album().save();
  return true;
}

uint8_t albumDeleteAllButFav() {
  uint8_t gone[ALBUM_MAX];
  uint8_t n = album().removeAllButFav(gone);
  for (uint8_t k = 0; sdReady && k < n; k++) albumRemoveFiles(gone[k]);
  if (n) album().save();
  return n;
}

// Once, at boot: the photos from before phase 33 (raw screen dumps) become BMPs,
// and a slot whose file is gone (another card) is let go.
void albumMigrate() {
  if (!sdReady) return;
  Album &a = album();
  bool changed = false;
  uint16_t *buf = albumBuffer();
  for (uint8_t k = 0; k < ALBUM_MAX; k++) {
    if (!(a.s[k].flags & AS_USED)) continue;
    char bmp[24], raw[24];
    albumPath(bmp, sizeof(bmp), k, true);
    albumPath(raw, sizeof(raw), k, false);
    if (SD_MMC.exists(bmp)) { if (SD_MMC.exists(raw)) SD_MMC.remove(raw); continue; }
    File f = SD_MMC.open(raw, FILE_READ);
    bool ok = f && buf && f.read((uint8_t *)buf, ALBUM_W * ALBUM_W * 2) == ALBUM_W * ALBUM_W * 2;
    if (f) f.close();
    if (ok && albumWriteBmp(k, buf, ALBUM_W, 1)) SD_MMC.remove(raw);
    else if (!SD_MMC.exists(raw)) { a.remove(k); changed = true; }
  }
  if (changed) a.save();
}

uint16_t *albumBuffer() {
  static uint16_t *b = nullptr;
  if (!b) b = (uint16_t *)ps_malloc(ALBUM_W * ALBUM_W * 2);
  return b;
}
