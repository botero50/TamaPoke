#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>

// Region landscapes for the main screen. Our own art (tools/gen_scenery.py, made with a local
// ComfyUI), stored on the SD card as small palette images the firmware doubles to the screen.
// Pure logic with host tests (scenery_test.cpp).
//
// File "TPBG": 4 bytes magic, u16 width, u16 height, u8 colours, u8 0, then the palette (colours x
// RGB565, little endian) and width x height bytes, each an index into the palette. 233 x 156 is shown
// at twice the size: 466 x 312, the area above the care panel.

#define SCN_W 233
#define SCN_H 156
#define SCN_OUT_W (SCN_W * 2)
#define SCN_OUT_H (SCN_H * 2)
#define SCN_MAX_COLORS 64
#define SCN_REGIONS 7

enum : uint8_t { SCN_DAY = 0, SCN_GOLD, SCN_NIGHT, SCN_VARIANTS };

// The mood by the clock: night as the main screen already counts it (before 6 and from 20),
// golden hour for the first two hours of the day and the last two before night, otherwise day.
static inline uint8_t sceneryVariant(int hour, bool night) {
  if (night) return SCN_NIGHT;
  if (hour < 8 || hour >= 18) return SCN_GOLD;
  return SCN_DAY;
}

// "/mons/bg<region+1>_<d|t|n>.bin"
static inline void sceneryPath(char *out, size_t n, uint8_t region, uint8_t variant) {
  static const char V[SCN_VARIANTS] = { 'd', 't', 'n' };
  snprintf(out, n, "/mons/bg%u_%c.bin", (unsigned)(region + 1), V[variant % SCN_VARIANTS]);
}

struct SceneryInfo {
  uint8_t ncol;
  size_t palOff, pixOff;
};

// Checks a whole file image: magic, size, palette, and that it is not cut short.
static inline bool sceneryParse(const uint8_t *d, size_t n, SceneryInfo &info) {
  if (!d || n < 10) return false;
  if (d[0] != 'T' || d[1] != 'P' || d[2] != 'B' || d[3] != 'G') return false;
  uint16_t w = (uint16_t)(d[4] | (d[5] << 8)), h = (uint16_t)(d[6] | (d[7] << 8));
  if (w != SCN_W || h != SCN_H) return false;
  uint8_t ncol = d[8];
  if (ncol < 1 || ncol > SCN_MAX_COLORS) return false;
  info.ncol = ncol;
  info.palOff = 10;
  info.pixOff = 10 + (size_t)ncol * 2;
  return n >= info.pixOff + (size_t)SCN_W * SCN_H;
}

// Palette indices to a SCN_OUT_W x SCN_OUT_H RGB565 buffer, every pixel doubled. False (and the
// buffer untouched past the bad pixel) if the file is not valid or an index is outside its palette.
static inline bool sceneryExpand(const uint8_t *d, size_t n, uint16_t *out) {
  SceneryInfo in;
  if (!out || !sceneryParse(d, n, in)) return false;
  uint16_t pal[SCN_MAX_COLORS];
  for (uint8_t i = 0; i < in.ncol; i++) pal[i] = (uint16_t)(d[in.palOff + i * 2] | (d[in.palOff + i * 2 + 1] << 8));
  const uint8_t *px = d + in.pixOff;
  for (int y = 0; y < SCN_H; y++) {
    uint16_t *r0 = out + (size_t)(2 * y) * SCN_OUT_W, *r1 = r0 + SCN_OUT_W;
    for (int x = 0; x < SCN_W; x++) {
      uint8_t idx = px[(size_t)y * SCN_W + x];
      if (idx >= in.ncol) return false;
      uint16_t v = pal[idx];
      r0[2 * x] = r0[2 * x + 1] = r1[2 * x] = r1[2 * x + 1] = v;
    }
  }
  return true;
}

// ---------- the optional FX block: where the picture can sparkle ----------
// After the pixels: "FX", a count, then count x (x, y) in the 233 x 156 picture (tools/pack_scenery.py
// picks them from the picture itself: water, lit windows, stars). The sparkles land on those pixels.
#define SCN_MAX_SPOTS 32

static inline uint8_t scenerySpots(const uint8_t *d, size_t n, uint8_t *xy) {
  SceneryInfo in;
  if (!xy || !sceneryParse(d, n, in)) return 0;
  const size_t at = in.pixOff + (size_t)SCN_W * SCN_H;
  if (n < at + 3 || d[at] != 'F' || d[at + 1] != 'X') return 0;
  uint8_t c = d[at + 2];
  if (c > SCN_MAX_SPOTS) c = SCN_MAX_SPOTS;
  if (n < at + 3 + (size_t)c * 2) return 0;               // a cut block counts as none
  uint8_t k = 0;
  for (uint8_t i = 0; i < c; i++) {
    uint8_t x = d[at + 3 + i * 2], y = d[at + 4 + i * 2];
    if (x < SCN_W && y < SCN_H) { xy[k * 2] = x; xy[k * 2 + 1] = y; k++; }
  }
  return k;
}

// ---------- ambient life over the landscape ----------
// What drifts, by region and mood: all a pure function of the clock, so nothing is kept between frames.
enum : uint8_t { LFX_NONE = 0, LFX_PETALS, LFX_LEAVES, LFX_EMBERS, LFX_SNOW, LFX_POLLEN, LFX_FLOWERS, LFX_FIREFLIES, LFX_BIRDS };

static inline uint8_t fxKind(uint8_t region, uint8_t variant) {
  const bool night = variant == SCN_NIGHT;
  switch (region) {
    case 0: return night ? LFX_FIREFLIES : LFX_PETALS;     // Kanto: petals and seed fluff on the wind
    case 1: return night ? LFX_FIREFLIES : LFX_LEAVES;     // Johto: maple leaves
    case 2: return LFX_EMBERS;                            // Hoenn: ash by day, embers when it is darker
    case 3: return LFX_SNOW;                              // Sinnoh: snowfall
    case 4: return night ? LFX_NONE : LFX_BIRDS;           // Unova: a few birds across the sky
    case 5: return night ? LFX_FIREFLIES : LFX_POLLEN;     // Kalos: lavender pollen
    default: return LFX_FLOWERS;                          // Alola: hibiscus petals
  }
}

// In rain, snow or a storm the real weather overlay takes over: nothing else drifts.
// WeatherKind (weather.h): 0 none, 1 clear, 2 cloudy, 3 fog, then rain, snow, storm, New York snow.
static inline bool fxDriftAllowed(uint8_t weatherKind) { return weatherKind <= 3; }

static inline uint8_t fxCount(uint8_t kind, uint8_t variant) {
  switch (kind) {
    case LFX_PETALS: return 18;
    case LFX_LEAVES: return 16;
    case LFX_EMBERS: return variant == SCN_DAY ? 10 : 16;
    case LFX_SNOW: return 28;
    case LFX_POLLEN: return 20;
    case LFX_FLOWERS: return 14;
    case LFX_FIREFLIES: return 14;
    case LFX_BIRDS: return 3;
    default: return 0;
  }
}

// a steady pseudo-random number in [0, 1) for particle i and a salt
static inline float fxRand(uint32_t i, uint32_t salt) {
  uint32_t h = i * 2654435761u + salt * 40503u + 12345u;
  h ^= h >> 13; h *= 1274126177u; h ^= h >> 16;
  return (float)(h & 0xFFFFu) / 65536.0f;
}

// how far along its loop particle i is: 0 .. 1, repeating every periodMs, each one at its own offset
static inline float fxPhase(uint32_t now, uint32_t periodMs, uint32_t i) {
  uint32_t off = (uint32_t)(fxRand(i, 7) * (float)periodMs);
  return (float)((now + off) % periodMs) / (float)periodMs;
}

// a twinkle for spot i: 0 .. 1, mostly dark and then a flash
static inline float fxTwinkle(uint32_t now, uint32_t i) {
  uint32_t t = (now + i * 397u) % 1600u;
  float tri = t < 800u ? t / 800.0f : (1600u - t) / 800.0f;
  return tri * tri * tri;
}

struct FxP { int16_t x, y; uint8_t size, shade; };   // screen pixels (466 x 312), a size in px, which of the kind's colours

static inline FxP fxParticle(uint8_t kind, uint32_t now, uint16_t i) {
  FxP p = { -100, -100, 2, 0 };
  const float TAU = 6.2831853f;
  const float r1 = fxRand(i, 1), r3 = fxRand(i, 3), r4 = fxRand(i, 4), r6 = fxRand(i, 6);
  switch (kind) {
    case LFX_PETALS: {
      uint32_t period = 15000u + (uint32_t)(r1 * 9000.0f);
      float ph = fxPhase(now, period, i);
      p.x = (int16_t)(-24.0f + ph * 514.0f);
      p.y = (int16_t)(16.0f + r3 * 220.0f + ph * 70.0f + 18.0f * sinf(ph * 14.0f + r4 * TAU));
      p.size = r6 < 0.4f ? 5 : 4; p.shade = r6 < 0.6f ? 0 : (r6 < 0.85f ? 1 : 2);
      break;
    }
    case LFX_LEAVES: {
      uint32_t period = 9000u + (uint32_t)(r1 * 7000.0f);
      float ph = fxPhase(now, period, i);
      p.x = (int16_t)(r3 * 440.0f + 26.0f * sinf(ph * 9.0f + r4 * TAU) + ph * 40.0f);
      p.y = (int16_t)(-12.0f + ph * 336.0f);
      p.size = 6; p.shade = r6 < 0.45f ? 0 : (r6 < 0.8f ? 1 : 2);
      break;
    }
    case LFX_EMBERS: {
      uint32_t period = 6500u + (uint32_t)(r1 * 6500.0f);
      float ph = fxPhase(now, period, i);
      p.x = (int16_t)(200.0f + r3 * 150.0f + ph * 40.0f + 12.0f * sinf(ph * 10.0f + r4 * TAU));
      p.y = (int16_t)(170.0f - ph * 150.0f);
      p.size = 4; p.shade = r6 < 0.5f ? 0 : 1;
      break;
    }
    case LFX_SNOW: {
      const bool near = (i % 3) == 0;
      uint32_t period = near ? 8000u + (uint32_t)(r1 * 3000.0f) : 15000u + (uint32_t)(r1 * 6000.0f);
      float ph = fxPhase(now, period, i);
      p.x = (int16_t)(r3 * 466.0f + 14.0f * sinf(ph * 7.0f + r4 * TAU));
      p.y = (int16_t)(-6.0f + ph * 324.0f);
      p.size = near ? 5 : 3; p.shade = near ? 1 : 0;
      break;
    }
    case LFX_POLLEN: {
      uint32_t period = 16000u + (uint32_t)(r1 * 10000.0f);
      float ph = fxPhase(now, period, i);
      p.x = (int16_t)(r3 * 466.0f + 22.0f * sinf(ph * 8.0f + r4 * TAU));
      p.y = (int16_t)(300.0f - ph * 300.0f);
      p.size = 4; p.shade = r6 < 0.6f ? 0 : 1;
      break;
    }
    case LFX_FLOWERS: {
      uint32_t period = 11000u + (uint32_t)(r1 * 7000.0f);
      float ph = fxPhase(now, period, i);
      p.x = (int16_t)(r3 * 466.0f + 30.0f * sinf(ph * 7.0f + r4 * TAU));
      p.y = (int16_t)(-10.0f + ph * 332.0f);
      p.size = 6; p.shade = r6 < 0.4f ? 0 : (r6 < 0.75f ? 1 : 2);
      break;
    }
    case LFX_FIREFLIES: {
      const float r2 = fxRand(i, 2), r5 = fxRand(i, 5), t = (float)(now % 600000u);
      p.x = (int16_t)(40.0f + r3 * 386.0f + 26.0f * sinf(t / 1900.0f + r4 * TAU));
      p.y = (int16_t)(150.0f + r2 * 140.0f + 14.0f * cosf(t / 2300.0f + r5 * TAU));
      float tw = fxTwinkle(now, i * 7u);
      p.size = 4; p.shade = tw > 0.6f ? 2 : (tw > 0.2f ? 1 : 0);
      break;
    }
    case LFX_BIRDS: {
      uint32_t period = 24000u + (uint32_t)i * 5000u;
      float ph = fxPhase(now, period, i);
      p.x = (int16_t)(-30.0f + ph * 526.0f);
      p.y = (int16_t)(40.0f + i * 34.0f + 8.0f * sinf(ph * 18.0f));
      p.size = 12; p.shade = (uint8_t)((now / 260u + i) & 1u);
      break;
    }
    default: break;
  }
  return p;
}
