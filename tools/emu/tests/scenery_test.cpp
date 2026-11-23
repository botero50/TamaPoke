// The landscape file format and the mood rule (scenery.h). The art itself is judged by eye in the
// previews; here the bytes: what is accepted, what is rejected, and exact pixels out.
#include "Arduino.h"
#include "Preferences.h"
#include "pet.h"
#include "scenery.h"
#include <cstdio>
#include <cstring>
#include <vector>
uint32_t g_seed = 33; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
volatile int g_touchX = 0, g_touchY = 0; volatile bool g_touchDown = false; bool wasPressed = false;
static uint32_t g_ms = 0; uint32_t millis() { return g_ms; }
void FakeESP::restart() { exit(0); }
int FakeSerial::available() { return 0; } String FakeSerial::readStringUntil(char) { return String(""); }
void sfxPlay(uint8_t) {}
static int bad = 0;
static void ck(bool ok, const char *w) { printf("%s  %s\n", ok ? "PASS" : "FAIL", w); if (!ok) bad++; }

// a valid file: 4 colours, pixel (x, y) = (x + 2 * y) % 4
static std::vector<uint8_t> makeFile(uint8_t ncol = 4) {
  std::vector<uint8_t> f = { 'T', 'P', 'B', 'G', SCN_W & 255, SCN_W >> 8, SCN_H & 255, SCN_H >> 8, ncol, 0 };
  const uint16_t pal[4] = { 0x0000, 0xF800, 0x07E0, 0x001F };
  for (uint8_t i = 0; i < ncol; i++) { uint16_t v = pal[i % 4]; f.push_back(v & 255); f.push_back(v >> 8); }
  for (int y = 0; y < SCN_H; y++) for (int x = 0; x < SCN_W; x++) f.push_back((uint8_t)((x + 2 * y) % ncol));
  return f;
}

int main() {
  // the mood by the clock, as the main screen counts night (before 6 and from 20)
  ck(sceneryVariant(12, false) == SCN_DAY && sceneryVariant(8, false) == SCN_DAY && sceneryVariant(17, false) == SCN_DAY, "8:00 to 17:59 is day");
  ck(sceneryVariant(6, false) == SCN_GOLD && sceneryVariant(7, false) == SCN_GOLD, "6:00 and 7:00 are golden hour");
  ck(sceneryVariant(18, false) == SCN_GOLD && sceneryVariant(19, false) == SCN_GOLD, "18:00 and 19:00 are golden hour");
  ck(sceneryVariant(20, true) == SCN_NIGHT && sceneryVariant(3, true) == SCN_NIGHT && sceneryVariant(5, true) == SCN_NIGHT, "night is night");
  ck(sceneryVariant(12, true) == SCN_NIGHT, "night wins over the hour");
  bool every = true;
  for (int h = 0; h < 24; h++) { bool night = h < 6 || h >= 20; uint8_t v = sceneryVariant(h, night); if (v >= SCN_VARIANTS) every = false; }
  ck(every, "every hour of the day has a mood");

  // file names
  char p[32];
  sceneryPath(p, sizeof(p), 0, SCN_DAY);   ck(!strcmp(p, "/mons/bg1_d.bin"), "Kanto by day: /mons/bg1_d.bin");
  sceneryPath(p, sizeof(p), 6, SCN_NIGHT); ck(!strcmp(p, "/mons/bg7_n.bin"), "Alola by night: /mons/bg7_n.bin");
  sceneryPath(p, sizeof(p), 3, SCN_GOLD);  ck(!strcmp(p, "/mons/bg4_t.bin"), "Sinnoh at golden hour: /mons/bg4_t.bin");

  // a valid file
  std::vector<uint8_t> f = makeFile();
  SceneryInfo in;
  ck(sceneryParse(f.data(), f.size(), in) && in.ncol == 4 && in.pixOff == 18, "a valid file parses");
  std::vector<uint16_t> out((size_t)SCN_OUT_W * SCN_OUT_H, 0xFFFF);
  ck(sceneryExpand(f.data(), f.size(), out.data()), "and expands");
  ck(out.size() == 466u * 312u, "to the full area above the panel (466 x 312)");
  const uint16_t pal[4] = { 0x0000, 0xF800, 0x07E0, 0x001F };
  bool exact = true;
  for (int y = 0; y < SCN_H && exact; y++)
    for (int x = 0; x < SCN_W; x++) {
      uint16_t want = pal[(x + 2 * y) % 4];
      if (out[(2 * y) * SCN_OUT_W + 2 * x] != want || out[(2 * y) * SCN_OUT_W + 2 * x + 1] != want ||
          out[(2 * y + 1) * SCN_OUT_W + 2 * x] != want || out[(2 * y + 1) * SCN_OUT_W + 2 * x + 1] != want) { exact = false; break; }
    }
  ck(exact, "every source pixel is doubled into a 2 x 2 block of its palette colour");
  ck(out[0] == pal[0] && out[(SCN_OUT_H - 1) * SCN_OUT_W + SCN_OUT_W - 1] == pal[((SCN_W - 1) + 2 * (SCN_H - 1)) % 4], "the corners are right");

  // bad files
  { std::vector<uint8_t> g = f; g[0] = 'X'; ck(!sceneryParse(g.data(), g.size(), in), "a wrong magic is rejected"); }
  { std::vector<uint8_t> g = f; g[4] = 100; ck(!sceneryParse(g.data(), g.size(), in), "a wrong width is rejected"); }
  { std::vector<uint8_t> g = f; g[6] = 7; ck(!sceneryParse(g.data(), g.size(), in), "a wrong height is rejected"); }
  { std::vector<uint8_t> g = f; g[8] = 0; ck(!sceneryParse(g.data(), g.size(), in), "no colours is rejected"); }
  { std::vector<uint8_t> g = f; g[8] = SCN_MAX_COLORS + 1; ck(!sceneryParse(g.data(), g.size(), in), "too many colours is rejected"); }
  { ck(!sceneryParse(f.data(), f.size() - 1, in), "a file cut short by one byte is rejected");
    ck(!sceneryParse(f.data(), 9, in) && !sceneryParse(nullptr, 100, in), "a tiny or missing file is rejected"); }
  { std::vector<uint8_t> g = f; g[in.pixOff + 500] = 9; std::vector<uint16_t> o2((size_t)SCN_OUT_W * SCN_OUT_H);
    SceneryInfo i2; sceneryParse(g.data(), g.size(), i2);
    ck(!sceneryExpand(g.data(), g.size(), o2.data()), "a pixel outside the palette is rejected"); }
  ck(!sceneryExpand(f.data(), f.size(), nullptr), "no output buffer is rejected");
  { std::vector<uint8_t> g = makeFile(SCN_MAX_COLORS); ck(sceneryParse(g.data(), g.size(), in) && in.ncol == SCN_MAX_COLORS, "a full 64-colour palette is fine"); }
  { ck(f.size() == 10 + 8 + (size_t)SCN_W * SCN_H, "a 4-colour file is 10 + 8 + 233 x 156 bytes"); }

  // ---- the FX block: where it can sparkle
  {
    std::vector<uint8_t> g = f;
    g.push_back('F'); g.push_back('X'); g.push_back(3);
    const uint8_t pts[6] = { 10, 20, 200, 100, 232, 155 };
    for (uint8_t b : pts) g.push_back(b);
    uint8_t xy[SCN_MAX_SPOTS * 2];
    ck(sceneryParse(g.data(), g.size(), in), "a file with an FX block still parses");
    ck(scenerySpots(g.data(), g.size(), xy) == 3 && xy[0] == 10 && xy[1] == 20 && xy[4] == 232 && xy[5] == 155, "its three spots are read");
    ck(scenerySpots(f.data(), f.size(), xy) == 0, "a file without the block has no spots");
    ck(scenerySpots(g.data(), g.size() - 1, xy) == 0, "a cut block counts as none");
    std::vector<uint8_t> o = f; o.push_back('F'); o.push_back('X'); o.push_back(2);
    o.push_back(233); o.push_back(5); o.push_back(7); o.push_back(8);
    ck(scenerySpots(o.data(), o.size(), xy) == 1 && xy[0] == 7 && xy[1] == 8, "a spot outside the picture is dropped");
    std::vector<uint8_t> big = f; big.push_back('F'); big.push_back('X'); big.push_back(40);
    for (int i = 0; i < 80; i++) big.push_back((uint8_t)(i % 100));
    ck(scenerySpots(big.data(), big.size(), xy) == SCN_MAX_SPOTS, "more than 32 declared is cut to 32");
    ck(scenerySpots(g.data(), g.size(), nullptr) == 0, "no output buffer is fine");
  }

  // ---- what drifts
  {
    bool valid = true;
    for (uint8_t r = 0; r < SCN_REGIONS; r++) for (uint8_t v = 0; v < SCN_VARIANTS; v++) if (fxKind(r, v) > LFX_BIRDS) valid = false;
    ck(valid, "every region and mood has a defined effect");
    ck(fxKind(0, SCN_DAY) == LFX_PETALS && fxKind(0, SCN_NIGHT) == LFX_FIREFLIES, "Kanto: petals by day, fireflies at night");
    ck(fxKind(1, SCN_GOLD) == LFX_LEAVES, "Johto: leaves");
    ck(fxKind(2, SCN_DAY) == LFX_EMBERS && fxKind(2, SCN_NIGHT) == LFX_EMBERS, "Hoenn: ash and embers");
    ck(fxKind(3, SCN_DAY) == LFX_SNOW && fxKind(3, SCN_NIGHT) == LFX_SNOW, "Sinnoh: snow in every mood");
    ck(fxKind(4, SCN_DAY) == LFX_BIRDS && fxKind(4, SCN_NIGHT) == LFX_NONE, "Unova: birds by day, none at night");
    ck(fxKind(5, SCN_DAY) == LFX_POLLEN && fxKind(5, SCN_NIGHT) == LFX_FIREFLIES, "Kalos: pollen by day, fireflies at night");
    ck(fxKind(6, SCN_GOLD) == LFX_FLOWERS, "Alola: falling hibiscus petals");
    ck(fxDriftAllowed(0) && fxDriftAllowed(1) && fxDriftAllowed(2) && fxDriftAllowed(3), "it drifts in clear, cloudy and foggy weather");
    ck(!fxDriftAllowed(4) && !fxDriftAllowed(5) && !fxDriftAllowed(6) && !fxDriftAllowed(7), "and stops in rain, snow, a storm and New York snow");
    ck(fxCount(LFX_NONE, SCN_DAY) == 0, "no effect has no particles");
    bool counts = true;
    for (uint8_t k = LFX_PETALS; k <= LFX_BIRDS; k++) for (uint8_t v = 0; v < SCN_VARIANTS; v++) { uint8_t c = fxCount(k, v); if (c < 1 || c > 40) counts = false; }
    ck(counts, "every effect has a sensible number of particles");

    // random numbers and phases
    float sum = 0, mn = 1, mx = 0; bool det = true;
    for (uint32_t i = 0; i < 4000; i++) { float r = fxRand(i, 3); sum += r; if (r < mn) mn = r; if (r > mx) mx = r; if (r != fxRand(i, 3)) det = false; }
    ck(mn >= 0.0f && mx < 1.0f && det, "the particle random numbers are in [0, 1) and steady");
    ck(sum / 4000 > 0.45f && sum / 4000 < 0.55f, "and spread evenly");
    bool ph = true;
    for (uint32_t t = 0; t < 100000; t += 777) for (uint32_t i = 0; i < 20; i++) { float v = fxPhase(t, 12000, i); if (v < 0.0f || v >= 1.0f) ph = false; }
    ck(ph, "a loop's phase is always in [0, 1)");
    ck(fabsf(fxPhase(5000, 12000, 4) - fxPhase(5000 + 12000, 12000, 4)) < 1e-4f, "and repeats every period");
    float tmax = 0, tmin = 1;
    for (uint32_t t = 0; t < 3200; t += 10) { float v = fxTwinkle(t, 5); if (v > tmax) tmax = v; if (v < tmin) tmin = v; }
    ck(tmax > 0.95f && tmin >= 0.0f && tmax <= 1.0f, "a twinkle goes from dark to a full flash");

    // where the particles are
    bool inside = true, same = true;
    for (uint8_t k = LFX_PETALS; k <= LFX_BIRDS; k++)
      for (uint8_t v = 0; v < SCN_VARIANTS; v++)
        for (uint16_t i = 0; i < fxCount(k, v); i++)
          for (uint32_t t = 0; t < 300000; t += 6173) {
            FxP a = fxParticle(k, t, i), b = fxParticle(k, t, i);
            if (a.x != b.x || a.y != b.y) same = false;
            if (a.x < -60 || a.x > 520 || a.y < -40 || a.y > 360 || a.size < 2 || a.size > 12) inside = false;
          }
    ck(inside, "every particle stays near the screen (466 x 312) at every time");
    ck(same, "and is a pure function of the clock");
    int down = 0, wrapped = 0;
    for (uint16_t i = 0; i < 28; i++) { FxP a = fxParticle(LFX_SNOW, 40000, i), b = fxParticle(LFX_SNOW, 40100, i); if (b.y > a.y) down++; else wrapped++; }
    ck(down >= 26, "snow falls");
    int up = 0;
    for (uint16_t i = 0; i < 20; i++) { FxP a = fxParticle(LFX_POLLEN, 40000, i), b = fxParticle(LFX_POLLEN, 40100, i); if (b.y < a.y) up++; }
    ck(up >= 18, "pollen rises");
    FxP e0 = fxParticle(LFX_EMBERS, 1000, 3), e1 = fxParticle(LFX_EMBERS, 1300, 3);
    ck(e1.y < e0.y || e1.y > e0.y + 100, "embers rise");
    FxP b0 = fxParticle(LFX_BIRDS, 1000, 1), b1 = fxParticle(LFX_BIRDS, 2000, 1);
    ck(b1.x > b0.x, "a bird flies left to right");
  }

  if (bad) { printf("FAILURES: %d\n", bad); return 1; }
  printf("scenery test passed\n");
  return 0;
}
