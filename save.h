#pragma once
#include <stdint.h>
#include <stddef.h>

// A whole-save backup, over the serial console.
//
// A run is weeks of real time living in one NVS partition, and there is now a
// lot in it: the creature, the party, the box, both badge ladders, the Pokedex,
// the streak, the trainer name. Nothing could get it back out.
//
// The backup is KEY-DRIVEN, not a struct of fields. SAVE_FIELDS lists every key
// the firmware stores along with its type, and both directions walk that one
// table through the ordinary Preferences API -- which means the identical code
// runs on the board and in the emulator, so all of it is testable here. An
// explicit struct would have been a second description of the save that drifts
// the moment somebody adds a field; `save_test` compares the table against the
// keys actually present after a save, so forgetting to add one fails a test
// rather than silently dropping it from everyone's backup.

#define SAVE_MAGIC0 'T'
#define SAVE_MAGIC1 'K'
#define SAVE_MAGIC2 'P'
#define SAVE_MAGIC3 'S'
#define SAVE_VERSION 1
#define SAVE_HDR 8

enum SaveKind : uint8_t {
  SK_U8 = 1, SK_I8, SK_BOOL, SK_U16, SK_I16, SK_U32, SK_BYTES, SK_STR,
};

struct SaveField {
  const char *key;
  uint8_t kind;
};
extern const SaveField SAVE_FIELDS[];
extern const uint16_t SAVE_FIELD_COUNT;

// Serialises the live save. Returns the number of bytes written, or 0 if it
// would not fit in cap.
size_t saveExport(uint8_t *out, size_t cap);

// Restores one. VALIDATES THE WHOLE BLOB FIRST and only then touches NVS: a
// half-applied restore over a good save would be worse than no backup at all.
// Returns false and changes nothing if the blob is not ours, is the wrong
// version, or fails its checksum. The caller must reload afterwards.
bool saveImport(const uint8_t *in, size_t n);

// Roughly what saveExport needs, for sizing a buffer.
size_t saveExportSize();

// ---------- surviving a power cut (phase 39) ----------
//
// The ball has no battery, so unplugging it stops everything at once, and a game
// save is some 180 separate NVS writes. Pet::save() and saveImport() raise a flag
// before they start and lower it when they finish: a board that boots with the
// flag up was cut off in the middle, and its save may be part new and part old.
//
// The way back is a safety copy on the SD card: the same checksummed blob as a
// backup, in two files written in turn, so a cut while writing one leaves the
// other whole. A copy file is a 4-byte sequence number and then the blob.
#define SAVE_CUT_KEY "svW"         // the flag: a save or restore is under way
#define SAVE_RECOVERED_KEY "svR"   // how many times a copy has rescued this board
#define SAVE_COPY_HDR 4
#define SAVE_COPY_CAP (4096 + SAVE_COPY_HDR)
#define SAVE_COPY_EVERY_MS 120000UL
// A copy older than this is not a rescue: the card may have sat unwritten for
// days, and a slightly mixed save beats losing all of that.
#define SAVE_COPY_MAX_AGE 3600UL

bool saveWasCut();                 // is the flag up? Read it before anything saves
void saveCutClear();
uint16_t saveRecoveredCount();

// Builds a copy file. Returns its size, or 0 if it did not fit.
size_t saveCopyPack(uint8_t *out, size_t cap, uint32_t seq);
// Is this a complete, uncorrupted copy file? Reads nothing from NVS.
bool saveCopyValid(const uint8_t *in, size_t n, uint32_t *seq);
// Which of two copies to trust: 0, 1, or -1 if neither is valid.
int8_t saveCopyNewest(const uint8_t *a, size_t an, const uint8_t *b, size_t bn, uint32_t *seq);
// Restores the newest valid copy, unless it is too old. True if it did. The
// caller must not have loaded the pet yet, or must reload afterwards.
bool saveCopyRestore(const uint8_t *a, size_t an, const uint8_t *b, size_t bn);
