#include "save.h"
#include <Arduino.h>
#include <Preferences.h>
#include <string.h>

// Every key the firmware persists. Adding one here is the whole job of adding
// it to the backup; save_test fails if a key exists in NVS and not in this list.
const SaveField SAVE_FIELDS[] = {
  // the creature
  { "init", SK_BOOL },  { "full", SK_U8 },    { "joy", SK_U8 },
  { "ene", SK_U8 },     { "hyg", SK_U8 },     { "poop", SK_U8 },
  { "wgt", SK_U8 },     { "age", SK_U32 },    { "dexn", SK_I16 },
  { "eggT2", SK_I16 },  { "crack", SK_U8 },   { "mist", SK_U8 },
  { "sleep", SK_BOOL }, { "lend", SK_U8 },    { "seen", SK_U32 },
  { "bond", SK_U8 },    { "nick", SK_STR },   { "froz", SK_BOOL },
  // individual values and training
  { "ivat", SK_U8 },    { "ivdf", SK_U8 },    { "ivsp", SK_U8 },
  { "ivhp", SK_U8 },    { "tatk", SK_U8 },    { "tdef", SK_U8 },
  { "tspe", SK_U8 },
  // moves
  { "mvs", SK_BYTES },  { "mvlv", SK_U8 },
  // flags
  { "bk", SK_BOOL },    { "shy", SK_BOOL },   { "eshy", SK_BOOL },
  { "stpk", SK_BOOL },  { "evop", SK_U8 },    { "slpa", SK_U8 },    { "rtpn", SK_BOOL },
  // the player: outlives every creature, which is exactly why it must be here
  { "tnam", SK_STR },   { "onam", SK_STR },
     { "avtr", SK_U8 },    { "badg", SK_U16 },
  { "reg", SK_U8 },     { "eggR", SK_BYTES },
  { "badgX", SK_BYTES },{ "badhX", SK_BYTES },
  { "badh", SK_U16 },   { "dexreg", SK_BYTES }, { "dexsh", SK_BYTES },
  { "strk", SK_U16 },   { "bstrk", SK_U16 },  { "cday", SK_U32 },
  { "stps", SK_U32 },   { "sday", SK_U32 },
  { "medal", SK_U16 },  { "tmedal", SK_U16 }, { "mstone", SK_U16 },
  { "ghi", SK_U16 },    { "shi", SK_U16 },    { "qhi", SK_U16 },
  // Fun Pack: the extra minigames' records and the statuses
  { "tkt", SK_U16 },    { "tkd", SK_U32 },    { "tkn", SK_U8 },     { "wkhi", SK_BYTES },
  { "wkid", SK_U32 },
  { "mnhi", SK_BYTES }, { "sick", SK_U8 },    { "sdose", SK_U8 },
  { "sdmin", SK_U32 },  { "stick", SK_U16 },  { "ovf", SK_U8 },
  { "dirt", SK_U8 },    { "lon", SK_BOOL },   { "lont", SK_U8 },
  { "grmp", SK_BOOL },  { "bord", SK_U16 },
  // coins and daily missions
  { "coin", SK_U32 },  { "cntd", SK_U16 },  { "cnday", SK_U32 },
  { "stck", SK_U8 },   { "msprg", SK_BYTES }, { "msdon", SK_U8 }, { "mskd", SK_BYTES }, { "mskdy", SK_U32 },
  { "bedd", SK_U32 },
  // hats
  { "hats", SK_U16 },   { "hton", SK_I8 },    { "htdx", SK_I8 },
  { "htdy", SK_I8 },    { "htdex", SK_I16 },
  // toys and decorations
  { "deco", SK_U16 },   { "dslL", SK_I8 },    { "dslR", SK_I8 },
  { "food", SK_BYTES },
  { "soap", SK_U8 },    { "brsh", SK_BOOL },  { "napd", SK_U32 },
  // places, the garden, seasonal gifts
  { "pkday", SK_U32 },  { "pkfnd", SK_U8 },   { "ctday", SK_U32 },
  { "seed", SK_U8 },    { "plant", SK_U32 },  { "gift", SK_U32 },
  // capsules, scenery, trophies, teeth, study, quiz, parent limits
  { "capd", SK_U32 },   { "capn", SK_U8 },    { "scnO", SK_U8 },    { "scnP", SK_I8 },
  { "tly", SK_BYTES },  { "cnern", SK_U32 },  { "trph", SK_U16 },   { "trphX", SK_U16 },
  { "tthd", SK_U32 },   { "tthn", SK_U8 },    { "stdd", SK_U32 },   { "stdn", SK_U8 },
  { "qzhi", SK_U8 },    { "pbed", SK_U8 },    { "pgam", SK_U8 },    { "gmtd", SK_U8 },
  // adventures, fishing, alarm, chores, piggy bank, diary
  { "advR", SK_U8 },    { "advN", SK_U8 },    { "advS", SK_U32 },   { "gymS", SK_U16 },   { "fshd", SK_U32 },
  { "fshn", SK_U8 },    { "alOn", SK_BOOL },  { "alH", SK_U8 },     { "alM", SK_U8 },
  { "alD", SK_U32 },    { "brD", SK_U32 },    { "chrD", SK_U32 },   { "chrB", SK_U8 },
  { "pig", SK_U32 },    { "pigD", SK_U32 },   { "diary", SK_BYTES },{ "dcb", SK_U32 },
  // more hats, the room
  { "hatsX", SK_U16 },  { "furn", SK_U16 },   { "fwL", SK_I8 },     { "fwR", SK_I8 },
  { "fbL", SK_I8 },     { "fbR", SK_I8 },     { "frug", SK_BOOL },  { "fpap", SK_I8 },
  // wishes, moods, birthdays, events
  { "wsK", SK_U8 },     { "wsA", SK_U8 },     { "wsU", SK_U32 },    { "wsN", SK_U32 },
  { "wsD", SK_U32 },    { "wsC", SK_U8 },     { "lcare", SK_U32 },  { "born", SK_U32 },
  { "bday", SK_U32 },   { "egd", SK_U32 },    { "egn", SK_U8 },     { "fest", SK_U32 },
  // the wardrobe
  { "accO", SK_U16 },   { "acF", SK_I8 },     { "acN", SK_I8 },     { "acB", SK_I8 },
  // town life
  { "shd", SK_U32 },    { "shn", SK_U8 },     { "csd", SK_U32 },    { "lbd", SK_U32 },
  { "lbn", SK_U8 },     { "msd", SK_U32 },    { "msn", SK_U8 },     { "ctw", SK_U32 },
  { "rib", SK_BYTES },  { "jbk", SK_U8 },     { "jbu", SK_U32 },    { "jbd", SK_U32 },
  { "mld", SK_U32 },    { "mlk", SK_U8 },     { "mlr", SK_BOOL },
  // story quests
  { "qwk", SK_U32 },    { "qst", SK_U8 },     { "qsd", SK_U32 },   { "qver", SK_U8 },
  // the Nursery
  { "nurs", SK_BYTES }, { "eggB", SK_BOOL },
  // the banked creatures
  { "party", SK_BYTES }, { "box", SK_BYTES },
  // settings, so a restored device plays the way it did
  { "lang", SK_U8 },    { "snd", SK_BOOL },   { "vol", SK_U8 },
  { "vol2", SK_U8 },
  { "bri", SK_U8 },
};
const uint16_t SAVE_FIELD_COUNT = sizeof(SAVE_FIELDS) / sizeof(SAVE_FIELDS[0]);

#define MAX_VAL 768        // the box is the largest, at 18 records

static uint16_t crc16(const uint8_t *p, size_t n) {
  uint16_t c = 0xFFFF;
  for (size_t i = 0; i < n; i++) {
    c ^= (uint16_t)p[i] << 8;
    for (int b = 0; b < 8; b++) c = (c & 0x8000) ? (uint16_t)((c << 1) ^ 0x1021)
                                                : (uint16_t)(c << 1);
  }
  return c;
}

static uint8_t widthOf(uint8_t kind) {
  switch (kind) {
    case SK_U8: case SK_I8: case SK_BOOL: return 1;
    case SK_U16: case SK_I16: return 2;
    case SK_U32: return 4;
    default: return 0;               // variable
  }
}

// Reads one field out of NVS. Returns its length, or -1 if the key is absent --
// absent is normal (a fresh save has no party) and is simply left out.
static int readField(Preferences &p, const SaveField &f, uint8_t *val) {
  if (!p.isKey(f.key)) return -1;
  switch (f.kind) {
    case SK_U8:   val[0] = p.getUChar(f.key, 0); return 1;
    case SK_I8:   val[0] = (uint8_t)p.getChar(f.key, 0); return 1;
    case SK_BOOL: val[0] = p.getBool(f.key, false) ? 1 : 0; return 1;
    case SK_U16: { uint16_t v = p.getUShort(f.key, 0); memcpy(val, &v, 2); return 2; }
    case SK_I16: { int16_t v = p.getShort(f.key, 0); memcpy(val, &v, 2); return 2; }
    case SK_U32: { uint32_t v = p.getUInt(f.key, 0); memcpy(val, &v, 4); return 4; }
    case SK_BYTES: {
      size_t n = p.getBytesLength(f.key);
      if (n > MAX_VAL) return -1;
      p.getBytes(f.key, val, n);
      return (int)n;
    }
    case SK_STR: {
      char tmp[64] = {0};
      p.getString(f.key, tmp, sizeof(tmp));
      size_t n = strlen(tmp);
      memcpy(val, tmp, n);
      return (int)n;
    }
  }
  return -1;
}

static void writeField(Preferences &p, const SaveField &f,
                       const uint8_t *val, uint16_t n) {
  switch (f.kind) {
    case SK_U8:   if (n == 1) p.putUChar(f.key, val[0]); break;
    case SK_I8:   if (n == 1) p.putChar(f.key, (int8_t)val[0]); break;
    case SK_BOOL: if (n == 1) p.putBool(f.key, val[0] != 0); break;
    case SK_U16:  if (n == 2) { uint16_t v; memcpy(&v, val, 2); p.putUShort(f.key, v); } break;
    case SK_I16:  if (n == 2) { int16_t v; memcpy(&v, val, 2); p.putShort(f.key, v); } break;
    case SK_U32:  if (n == 4) { uint32_t v; memcpy(&v, val, 4); p.putUInt(f.key, v); } break;
    case SK_BYTES: if (n) p.putBytes(f.key, val, n); break;
    case SK_STR: {
      char tmp[64] = {0};
      if (n >= sizeof(tmp)) return;
      memcpy(tmp, val, n);
      tmp[n] = 0;
      p.putString(f.key, tmp);
      break;
    }
  }
}

size_t saveExportSize() {
  size_t n = SAVE_HDR + 2;
  for (uint16_t i = 0; i < SAVE_FIELD_COUNT; i++)
    n += 1 + strlen(SAVE_FIELDS[i].key) + 1 + 2 + MAX_VAL / 16;
  return n + 1024;                   // the two big blobs
}

size_t saveExport(uint8_t *out, size_t cap) {
  if (cap < SAVE_HDR + 2) return 0;
  Preferences p;
  p.begin("tamapoke", true);
  size_t at = SAVE_HDR;
  uint16_t count = 0;
  uint8_t val[MAX_VAL];
  for (uint16_t i = 0; i < SAVE_FIELD_COUNT; i++) {
    const SaveField &f = SAVE_FIELDS[i];
    int n = readField(p, f, val);
    if (n < 0) continue;             // absent: nothing to back up
    size_t klen = strlen(f.key);
    if (klen > 15) continue;         // NVS keys cannot be longer anyway
    if (at + 1 + klen + 1 + 2 + (size_t)n + 2 > cap) { p.end(); return 0; }
    out[at++] = (uint8_t)klen;
    memcpy(out + at, f.key, klen); at += klen;
    out[at++] = f.kind;
    out[at++] = (uint8_t)(n & 0xFF);
    out[at++] = (uint8_t)(n >> 8);
    memcpy(out + at, val, (size_t)n); at += (size_t)n;
    count++;
  }
  p.end();
  out[0] = SAVE_MAGIC0; out[1] = SAVE_MAGIC1;
  out[2] = SAVE_MAGIC2; out[3] = SAVE_MAGIC3;
  out[4] = SAVE_VERSION;
  out[5] = (uint8_t)(count & 0xFF);
  out[6] = (uint8_t)(count >> 8);
  out[7] = 0;
  uint16_t c = crc16(out, at);
  out[at++] = (uint8_t)(c & 0xFF);
  out[at++] = (uint8_t)(c >> 8);
  return at;
}

// Looks a key up in the table. An unknown key is skipped rather than trusted:
// the kind on the wire decides nothing, the table does.
static const SaveField *fieldFor(const char *key, uint8_t kind) {
  for (uint16_t i = 0; i < SAVE_FIELD_COUNT; i++)
    if (SAVE_FIELDS[i].kind == kind && !strcmp(SAVE_FIELDS[i].key, key))
      return &SAVE_FIELDS[i];
  return nullptr;
}

bool saveImport(const uint8_t *in, size_t n) {
  if (n < SAVE_HDR + 2) return false;
  if (in[0] != SAVE_MAGIC0 || in[1] != SAVE_MAGIC1 ||
      in[2] != SAVE_MAGIC2 || in[3] != SAVE_MAGIC3) return false;
  if (in[4] != SAVE_VERSION) return false;
  uint16_t count = (uint16_t)in[5] | ((uint16_t)in[6] << 8);
  uint16_t want = (uint16_t)in[n - 2] | ((uint16_t)in[n - 1] << 8);
  if (crc16(in, n - 2) != want) return false;

  // PASS ONE: walk the whole thing and prove it parses. Nothing is written
  // yet -- a restore that fell over halfway would leave a save that is neither
  // the old one nor the new one, which is the one outcome worse than no backup.
  size_t at = SAVE_HDR;
  uint16_t seen = 0;
  while (at + 4 <= n - 2) {
    uint8_t klen = in[at];
    if (!klen || klen > 15 || at + 1 + klen + 3 > n - 2) return false;
    size_t vat = at + 1 + klen;
    uint8_t kind = in[vat];
    uint16_t vlen = (uint16_t)in[vat + 1] | ((uint16_t)in[vat + 2] << 8);
    if (vlen > MAX_VAL) return false;
    if (vat + 3 + vlen > n - 2) return false;
    uint8_t w = widthOf(kind);
    if (w && vlen != w) return false;          // a scalar of the wrong width
    at = vat + 3 + vlen;
    seen++;
  }
  if (at != n - 2 || seen != count) return false;

  // PASS TWO: it parses, so commit. Cleared first, or keys the backup does not
  // contain would survive from whatever save happened to be on the device.
  Preferences p;
  p.begin("tamapoke", false);
  p.clear();
  at = SAVE_HDR;
  while (at + 4 <= n - 2) {
    uint8_t klen = in[at];
    char key[16] = {0};
    memcpy(key, in + at + 1, klen);
    size_t vat = at + 1 + klen;
    uint8_t kind = in[vat];
    uint16_t vlen = (uint16_t)in[vat + 1] | ((uint16_t)in[vat + 2] << 8);
    const SaveField *f = fieldFor(key, kind);
    if (f) writeField(p, *f, in + vat + 3, vlen);
    at = vat + 3 + vlen;
  }
  p.end();
  return true;
}
