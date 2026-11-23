// Host versions of the three hardware modules. The sprite loader is the real
// TPK2/TPTH parser, just reading from a directory instead of the SD card.
#include "Arduino.h"
#include "sdmon.h"
#include "rtcbat.h"
#include "imu.h"
#include "audio.h"
#include "linknow.h"
#include <cstdio>
#include <string>

// Baked in by build.sh from the repo location; --sprites <dir> overrides it.
#ifndef SPRITE_DIR
#define SPRITE_DIR "../sdcard/mons"
#endif
static std::string g_spriteDir = SPRITE_DIR;
void emuSetSpriteDir(const char *d) { g_spriteDir = d; }

bool sdReady = true;
bool sdDirty = false;
// The firmware rescans region art when a pack lands over PUT. There is no card
// here and no PUT path, so the flag never sets and the scan is a no-op -- but both
// symbols are referenced from loop(), so they have to exist to link.
bool sdArtDirty = false;
void sdScanRegionArt(bool) {}
SdThumbs thumbs;

static uint8_t *slurp(const std::string &path, uint32_t *size) {
  FILE *f = fopen(path.c_str(), "rb");
  if (!f) return nullptr;
  fseek(f, 0, SEEK_END);
  long n = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (n <= 0) { fclose(f); return nullptr; }
  uint8_t *b = (uint8_t *)malloc(n);
  if (!b || fread(b, 1, n, f) != (size_t)n) { free(b); fclose(f); return nullptr; }
  fclose(f);
  *size = (uint32_t)n;
  return b;
}

bool megaArtOnCard(int16_t dex) {
  char p[64], q[40];
  if (!megaHas(dex)) return false;
  megaPath(q, sizeof(q), dex);                          // "/mons/mNNN.bin": the host folder is the mons folder
  snprintf(p, sizeof(p), "%s%s", g_spriteDir.c_str(), q + 5);
  FILE *f = fopen(p, "rb");
  if (!f) return false;
  fclose(f);
  return true;
}

bool PmdMon::load(int16_t dexNum, bool shiny, bool mega) {
  if (dexNum < 1 || dexNum > 999) return false;
  unload();
  char p[64];
  if (mega) { char q[40]; megaPath(q, sizeof(q), dexNum); snprintf(p, sizeof(p), "%s%s", g_spriteDir.c_str(), q + 5); }
  else snprintf(p, sizeof(p), "%s/p%s%03u.bin", g_spriteDir.c_str(), shiny ? "s" : "", (unsigned)dexNum);
  uint32_t size = 0;
  blob = slurp(p, &size);
  if (!blob && !mega) {
    snprintf(p, sizeof(p), "%s/p%03u.bin", g_spriteDir.c_str(), (unsigned)dexNum);
    blob = slurp(p, &size);
  }
  if (!blob) return false;
  if (size < 7 || memcmp(blob, "TPK2", 4) != 0) { unload(); return false; }

  uint8_t nActs = blob[4];
  memcpy(&palCount, blob + 5, 2);
  if (palCount > 256 || (uint32_t)7 + palCount * 2 > size) { unload(); return false; }
  memcpy(pal, blob + 7, palCount * 2);

  const uint8_t *q = blob + 7 + palCount * 2, *end = blob + size;
  for (uint8_t i = 0; i < nActs && q + 4 <= end; i++) {
    uint8_t id = q[0], w = q[1], h = q[2], nf = q[3];
    q += 4;
    if (id >= PMD_NACTS || nf > 24) { unload(); return false; }
    uint32_t bytes = (uint32_t)nf * 2 + (uint32_t)w * h * nf;
    if (w == 0 || h == 0 || nf == 0 || q + bytes > end) { unload(); return false; }
    PmdAct &a = acts[id];
    a.w = w; a.h = h; a.frames = nf;
    for (uint8_t k = 0; k < nf; k++) { a.ms[k] = pmdFrameMs(q[0] | (q[1] << 8)); q += 2; }
    a.data = q;
    q += (uint32_t)w * h * nf;
    uint8_t base = 1;
    for (uint8_t f = 0; f < nf; f++) {
      const uint8_t *fr = a.data + (uint32_t)f * w * h;
      for (int r = h - 1; r >= 0; r--) {
        bool any = false;
        for (int c = 0; c < w && !any; c++) if (fr[r * w + c] != 0xFF) any = true;
        if (any) { if (r + 1 > base) base = r + 1; break; }
      }
    }
    a.base = base;
  }
  dex = dexNum;
  loaded = true;
  return true;
}

void PmdMon::unload() {
  if (blob) { free(blob); blob = nullptr; }
  for (auto &a : acts) { a.w = a.h = a.frames = a.base = 0; a.data = nullptr; }
  loaded = false;
}

bool SdThumbs::load() {
  data = slurp(g_spriteDir + "/thumbs.bin", &size);
  if (!data) { Serial.println("emu: no thumbs.bin"); return false; }
  if (memcmp(data, "TPTH", 4) != 0) { free(data); data = nullptr; return false; }
  memcpy(&count, data + 4, 2);
  loaded = true;
  Serial.printf("emu: thumbnails loaded: %u\n", count);
  return true;
}

const uint8_t *SdThumbs::get(int16_t dex) const {
  return loaded ? thumbAt(data, size, count, dex) : nullptr;
}

bool SdMon::load(int16_t, bool) { return false; }   // legacy TPK1 path unused
void SdMon::unload() { if (data) { free(data); data = nullptr; } loaded = false; }
bool sdBegin() {
  Serial.printf("emu: sprites from %s\n", g_spriteDir.c_str());
  return true;
}
bool sdSerialCommand(const String &) { return false; }

// the save's safety copies: no card here, so they stay in memory
#include <vector>
std::vector<uint8_t> g_emuCopy[2];
bool sdCopyWrite(uint8_t slot, const uint8_t *data, size_t n) { g_emuCopy[slot & 1].assign(data, data + n); return true; }
size_t sdCopyRead(uint8_t slot, uint8_t *out, size_t cap) {
  const std::vector<uint8_t> &c = g_emuCopy[slot & 1];
  if (c.empty() || c.size() > cap) return 0;
  memcpy(out, c.data(), c.size());
  return c.size();
}
void sdCopyClear() { g_emuCopy[0].clear(); g_emuCopy[1].clear(); }
bool nvsUsage(uint16_t *, uint16_t *) { return false; }

// --- RTC / battery / PMU ---
static uint32_t g_epoch = 0;
bool rtcBegin() { return true; }
uint32_t rtcEpoch() { return g_epoch + millis() / 1000; }
void rtcSetEpoch(uint32_t e) { g_epoch = e - millis() / 1000; }
bool batBegin() { return true; }
void pmuEnablePanel() {}
int g_emuBatPct = 87;   // tests set it
int batPercent() { return g_emuBatPct; }
int batSmoothMv() { return 3950; }
bool batCharging() { return false; }
bool g_emuUsb = false;   // tests set it: the standby clock and self-care need USB power
bool usbPresent() { return g_emuUsb; }
bool batShouldShutdown() { return false; }
void batPowerOff() {}
void pwrSetup() {}
bool pwrShortPressed() { return false; }

// --- IMU: no sensor in the emulator; steps arrive through the STEPS console command ---
bool imuBegin() { return true; }
void i2cLock() {}
void i2cUnlock() {}
bool imuReadAccel(float &, float &, float &) { return false; }
uint8_t imuReadBatch(float *, uint32_t *, uint8_t) { return 0; }
bool imuFifoOn() { return false; }
static bool g_emuFifo = false;
bool imuFifoEnter() { g_emuFifo = true; return true; }
void imuFifoExit() { g_emuFifo = false; }
bool imuFifoActive() { return g_emuFifo; }
uint32_t imuFifoFailCount() { return 0; }
uint32_t imuFifoSampleCount() { return 0; }
uint32_t imuDroppedCount() { return 0; }

// --- audio (silent) ---
void audioBegin() {}
uint8_t g_emuLastSfx = 255; int g_emuSfxCount = 0;
void sfxPlay(uint8_t id) { g_emuLastSfx = id; g_emuSfxCount++; }
bool cryHas(int16_t d) { return d >= 1 && d <= 251; }   // Kanto and Johto only, like cries.h
void audioSetQuiet(bool) {}
// remembers the last request so tests can see which creature was asked to cry
int16_t g_emuLastCry = 0; int g_emuCryCount = 0;
void audioCry(int16_t dex) { g_emuLastCry = dex; g_emuCryCount++; }
// no radio here; the protocol itself is exercised by tests/link_test.cpp
struct Link;
// No radio here at all, which is why lossy_test drives Link directly instead
// of going through this.
bool linkNowBegin(Link *) { return false; }
// no Wi-Fi either: the weather is set by tests and the preview harness
#include "wifisync.h"
void wifiBegin() {}
void wifiTick(uint32_t, bool) {}
bool wifiBusy() { return false; }
bool wifiTakeTime(uint32_t *) { return false; }
void wifiSetNetwork(const char *, const char *) {}
void wifiForget() {}
void wifiSetTz(int16_t) {}
void wifiSetPlace(float, float) {}
void wifiSyncNow() {}
#include "weather.h"
void wifiSetNySnow(bool on) { weatherSetNySnow(on); }
uint8_t wifiState() { return WIFI_ST_NONE; }
uint32_t wifiLastSync() { return 0; }
const char *wifiNetwork() { return ""; }
void wifiDiag(char *out, size_t n) { if (n) out[0] = 0; }
void wifiScanPrint() {}
void linkNowEnd() {}
bool linkNowUp() { return false; }
void linkNowPoll() {}
static LinkNowStats gNoStats;
const LinkNowStats &linkNowStats() { return gNoStats; }
// audio is silent here, but the sketch calls these, so they have to exist
static uint8_t g_emuVol = 50, g_emuMusic = 0;
void audioMusic(uint8_t id) { g_emuMusic = id; }
uint8_t audioMusicNow() { return g_emuMusic; }
void audioSetVolume(uint8_t v) { g_emuVol = v > 100 ? 100 : v; }
void audioSaveVolume() {}
uint8_t audioVolume() { return g_emuVol; }
void audioSetEnabled(bool on) { (void)on; }
bool audioEnabled() { return true; }

// Why the last run ended. The tests link this file but not main_sdl.cpp, so the
// state lives here and the GUI sets it after reading a simulated crash.
static int g_resetReason = ESP_RST_POWERON;
esp_reset_reason_t emuResetReason() { return g_resetReason; }
void emuSetResetReason(int r) { g_resetReason = r; }

// the photo album: the real slot rules (album.cpp), the pixels kept in memory
#include <vector>
static Album g_albumSlots;
static bool g_albumLoaded = false;
static std::vector<uint16_t> g_albumPix[ALBUM_MAX];
static Album &hostAlbum() {
  if (!g_albumLoaded) { g_albumSlots.load(); g_albumLoaded = true; }
  return g_albumSlots;
}
bool albumSave(const uint16_t *fb) {
  if (!fb) return false;
  int8_t slot = hostAlbum().pickForSave();
  if (slot < 0) return false;
  std::vector<uint16_t> &ph = g_albumPix[slot];
  ph.assign(ALBUM_W * ALBUM_W, 0);
  for (int y = 0; y < ALBUM_W; y++) for (int x = 0; x < ALBUM_W; x++) ph[y * ALBUM_W + x] = fb[(y * 2) * 466 + x * 2];
  hostAlbum().markSaved((uint8_t)slot);
  hostAlbum().save();
  return true;
}
uint8_t albumCount() { return hostAlbum().count(); }
bool albumLoad(uint8_t newest, uint16_t *out) {
  int8_t slot = hostAlbum().slotFor(newest);
  if (slot < 0 || !out || g_albumPix[slot].size() != ALBUM_W * ALBUM_W) return false;
  for (size_t i = 0; i < g_albumPix[slot].size(); i++) out[i] = g_albumPix[slot][i];
  return true;
}
bool albumIsFav(uint8_t newest) { int8_t s = hostAlbum().slotFor(newest); return s >= 0 && hostAlbum().isFav((uint8_t)s); }
bool albumToggleFav(uint8_t newest) {
  int8_t s = hostAlbum().slotFor(newest);
  if (s < 0 || !hostAlbum().toggleFav((uint8_t)s)) return false;
  hostAlbum().save();
  return true;
}
bool albumDelete(uint8_t newest) {
  int8_t s = hostAlbum().slotFor(newest);
  if (s < 0 || !hostAlbum().remove((uint8_t)s)) return false;
  g_albumPix[s].clear();
  hostAlbum().save();
  return true;
}
uint8_t albumDeleteAllButFav() {
  uint8_t gone[ALBUM_MAX];
  uint8_t n = hostAlbum().removeAllButFav(gone);
  for (uint8_t k = 0; k < n; k++) g_albumPix[gone[k]].clear();
  if (n) hostAlbum().save();
  return n;
}
void albumMigrate() {}
uint16_t *albumBuffer() { static std::vector<uint16_t> b(ALBUM_W * ALBUM_W); return b.data(); }

// ---------- the region landscape (the real one is in sdmon.cpp, reading the SD card) ----------
SceneryBg gScenery;
const uint16_t *SceneryBg::get(uint8_t r, uint8_t v) {
  if (r >= SCN_REGIONS || v >= SCN_VARIANTS) return nullptr;
  if (region == (int8_t)r && variant == (int8_t)v && px) return px;
  const uint32_t bit = 1UL << (r * SCN_VARIANTS + v);
  if (failMask & bit) return nullptr;
  char name[24];
  sceneryPath(name, sizeof(name), r, v);                              // "/mons/bgN_x.bin": the host folder is the mons folder
  std::string path = g_spriteDir + (name + 5);                        // skip "/mons"
  uint32_t size = 0;
  uint8_t *buf = slurp(path.c_str(), &size);
  if (!buf) { failMask |= bit; return nullptr; }
  if (!px) px = (uint16_t *)malloc((size_t)SCN_OUT_W * SCN_OUT_H * 2);
  bool ok = px && sceneryExpand(buf, size, px);
  nspots = ok ? scenerySpots(buf, size, spots) : 0;
  free(buf);
  if (!ok) { region = -1; failMask |= bit; return nullptr; }
  region = (int8_t)r; variant = (int8_t)v;
  return px;
}
