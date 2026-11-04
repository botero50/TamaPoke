// Unplugging mid-save. The ball has no battery, so this is a normal way for a
// session to end: the flag must show it, and the safety copy must put it right
// without ever making things worse. The last part boots the real setup().
#include "Arduino.h"
#include "Arduino_GFX_Library.h"
#include "Preferences.h"
#include "pet.h"
#include "party.h"
#include "save.h"
#include "sdmon.h"
#include <cstdio>
#include <vector>

uint32_t g_seed = 11; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
volatile int g_touchX = 0, g_touchY = 0; volatile bool g_touchDown = false;
void FakeESP::restart() { exit(0); }
int FakeSerial::available() { return 0; }
String FakeSerial::readStringUntil(char) { return String(""); }
void setup(); void loop();
extern Pet pet;
extern uint32_t copySeq, copySerial, copyDoneAt; extern uint8_t copySlot; extern bool screenOff;
void copyTick(uint32_t now);
void copiesAtBoot(bool cut);

static int bad = 0;
static void ck(bool ok, const char *w) { printf("%s  %s\n", ok ? "PASS" : "FAIL", w); if (!ok) bad++; }

typedef std::vector<uint8_t> Bytes;
static Bytes pack(uint32_t seq) {
  Bytes b(SAVE_COPY_CAP);
  size_t n = saveCopyPack(b.data(), b.size(), seq);
  b.resize(n);
  return b;
}
// The whole save, minus this board's own two keys, to compare before and after.
static NvsStore petKeys() {
  NvsStore s = nvs();
  s.erase(SAVE_CUT_KEY);
  s.erase(SAVE_RECOVERED_KEY);
  return s;
}

int main() {
  const uint32_t T0 = 1793570433UL;
  {
    Pet p;
    p.begin();
    p.dbgHatchAs(25, false);
    p.lastSeenEpoch = T0;
    p.coins = 100;
    p.saveNow();

    // --- the flag
    ck(nvs().count(SAVE_CUT_KEY) == 1 && !saveWasCut(), "after a finished save the flag is down");
    uint32_t s0 = p.saveSerial();
    p.saveNow();
    ck(p.saveSerial() == s0 + 1, "every save moves the serial on");
    saveCutClear();

    // --- a good copy, then a save that is cut off
    Bytes a = pack(5);
    ck(a.size() > SAVE_COPY_HDR, "a copy packs");
    uint32_t seq = 0;
    ck(saveCopyValid(a.data(), a.size(), &seq) && seq == 5, "and reads back valid, with its number");
    Preferences raw; raw.begin("tamapoke", false);
    raw.putBool(SAVE_CUT_KEY, true);     // what a cut leaves behind: flag up, values half new
    raw.putUInt("coin", 999);
    ck(saveWasCut(), "a cut leaves the flag up");

    // --- which copy to trust
    Bytes older = a; memcpy(older.data(), "\x04\0\0\0", 4);
    Bytes torn(a.begin(), a.begin() + a.size() / 2);
    Bytes flip = a; flip[a.size() / 2] ^= 0x40;
    seq = 0;
    ck(saveCopyNewest(a.data(), a.size(), older.data(), older.size(), &seq) == 0 && seq == 5, "the newer copy wins, in slot 0");
    ck(saveCopyNewest(older.data(), older.size(), a.data(), a.size(), &seq) == 1 && seq == 5, "and in slot 1");
    ck(saveCopyNewest(torn.data(), torn.size(), older.data(), older.size(), &seq) == 1 && seq == 4, "a torn copy loses to an older whole one");
    ck(saveCopyNewest(flip.data(), flip.size(), older.data(), older.size(), &seq) == 1, "so does one with a flipped byte");
    ck(saveCopyNewest(torn.data(), torn.size(), flip.data(), flip.size(), &seq) == -1, "two bad ones: nothing to trust");
    ck(saveCopyNewest(nullptr, 0, nullptr, 0, &seq) == -1, "no card, no copies: the same");

    // --- a bad copy changes nothing
    ck(!saveCopyRestore(torn.data(), torn.size(), flip.data(), flip.size()), "damaged copies are refused");
    ck(raw.getUInt("coin", 0) == 999 && saveWasCut(), "and the save is exactly as it was");

    // --- a copy too old is no rescue
    raw.putUInt("seen", T0 + SAVE_COPY_MAX_AGE + 60);
    ck(!saveCopyRestore(a.data(), a.size(), older.data(), older.size()), "a copy over an hour behind the save is refused");
    ck(raw.getUInt("coin", 0) == 999, "and leaves it alone");
    raw.putUInt("seen", T0 + 60);

    // --- the rescue
    uint16_t before = saveRecoveredCount();
    ck(saveCopyRestore(a.data(), a.size(), older.data(), older.size()), "a recent copy restores");
    ck(raw.getUInt("coin", 0) == 100, "every value is back");
    ck(!saveWasCut(), "the flag comes down");
    ck(saveRecoveredCount() == before + 1, "and the rescue is counted");

    // --- a key the copy lacks does not survive, and the bookkeeping is not carried
    raw.putBool(SAVE_CUT_KEY, true);
    raw.putBytes("nurs", "xx", 2);
    Bytes noNurs;
    {
      nvs().erase("nurs");
      noNurs = pack(6);
      raw.putBytes("nurs", "xx", 2);
    }
    uint16_t now = saveRecoveredCount();
    ck(saveImport(noNurs.data() + SAVE_COPY_HDR, noNurs.size() - SAVE_COPY_HDR), "a restore over a save with an extra key");
    ck(!raw.isKey("nurs"), "removes what the copy did not have");
    ck(saveRecoveredCount() == now, "and leaves this board's count alone");

    raw.end();
  }

  // --- unplugged at EVERY point of a save: the flag always tells
  nvs().clear();
  {
    Pet q; q.begin(); q.dbgHatchAs(25, false); q.lastSeenEpoch = T0; q.coins = 100; q.saveNow();
    Bytes good = pack(7);
    NvsStore wanted = petKeys();

    // how many writes a whole save takes; one fewer is the latest possible cut
    const long BIG = 1000000;
    Preferences::writeBudget() = BIG;
    q.saveNow();
    const long saveWrites = BIG - Preferences::writeBudget();
    Preferences::writeBudget() = -1;
    int points = 0, missed = 0;
    for (long at = 1; at < saveWrites; at++) {
      q.coins = (uint32_t)(200 + at);                // something new for this save to write
      Preferences::writeBudget() = at;
      q.saveNow();
      Preferences::writeBudget() = -1;
      points++;
      if (!saveWasCut()) missed++;
      saveCutClear();
    }
    printf("      (a save cut at %d different points)\n", points);
    ck(points > 150, "a save really is that many separate writes");
    ck(missed == 0, "and a cut at any of them leaves the flag up");

    // --- unplugged at EVERY point of a restore: boot restores again, and it ends whole
    Preferences::writeBudget() = BIG;
    saveImport(good.data() + SAVE_COPY_HDR, good.size() - SAVE_COPY_HDR);
    const long restoreWrites = BIG - Preferences::writeBudget();
    Preferences::writeBudget() = -1;
    ck(!saveWasCut(), "a restore left to finish ends with the flag down");
    int restores = 0, lost = 0, wrong = 0;
    for (long at = 0; at < restoreWrites; at++) {
      Preferences w; w.begin("tamapoke", false);
      w.putUInt("coin", 1);                          // the mixed-up save being rescued
      w.putBool(SAVE_CUT_KEY, true);
      w.end();
      Preferences::writeBudget() = at;
      saveImport(good.data() + SAVE_COPY_HDR, good.size() - SAVE_COPY_HDR);
      Preferences::writeBudget() = -1;
      restores++;
      if (!saveWasCut()) lost++;                     // the next boot would not know
      saveImport(good.data() + SAVE_COPY_HDR, good.size() - SAVE_COPY_HDR);
      if (petKeys() != wanted) wrong++;
    }
    printf("      (a restore cut at %d different points)\n", restores);
    ck(restores > 150, "a restore is that many writes too");
    ck(lost == 0, "the flag stays up through a cut anywhere in a restore");
    ck(wrong == 0, "and restoring again always ends with the save exactly as copied");
  }

  // --- the real boot: flag up, a copy on the card, setup() puts it right
  nvs().clear();
  {
    Pet p; p.begin(); p.dbgHatchAs(25, false); p.lastSeenEpoch = T0; p.coins = 123; p.saveNow();
    Bytes c = pack(9);
    sdCopyWrite(1, c.data(), c.size());
    Preferences raw; raw.begin("tamapoke", false);
    raw.putUInt("coin", 777);
    raw.putBool(SAVE_CUT_KEY, true);
    raw.end();
  }
  setup();
  ck(pet.coins == 123, "boot after a cut: the pet comes back from the copy");
  ck(!saveWasCut(), "the flag is down");
  ck(saveRecoveredCount() == 1, "the rescue is counted once");
  ck(copySeq == 9 && copySlot == 0, "the next copy goes to the other file, numbered on");
  ck(pet.lastSeenEpoch >= T0, "and the date never went back past the save");

  // --- the copies themselves
  screenOff = true;                       // no screen check to pass
  copyDoneAt = 0; copySerial = pet.saveSerial() - 1;
  copyTick(1000);
  ck(copySeq == 10 && copySlot == 1, "a change makes a copy, and the files take turns");
  copyTick(1000 + 5000);
  ck(copySeq == 10, "nothing new, nothing written");
  pet.saveNow();
  copyTick(1000 + 60000);
  ck(copySeq == 10, "a new save within 2 minutes waits");
  copyTick(1000 + SAVE_COPY_EVERY_MS);
  ck(copySeq == 11 && copySlot == 0, "and is copied once they have passed");
  screenOff = false;
  extern bool cardOpen;
  cardOpen = true;
  pet.saveNow();
  copyTick(1000 + 2 * SAVE_COPY_EVERY_MS);
  ck(copySeq == 11, "not while a menu is open");
  cardOpen = false;

  // --- WIPE takes the copies with it
  sdCopyClear();
  uint8_t tmp[16];
  ck(sdCopyRead(0, tmp, sizeof(tmp)) == 0 && sdCopyRead(1, tmp, sizeof(tmp)) == 0, "a wipe leaves no copy to bring an old pet back");

  // --- the clock: never backwards, whatever the chip says
  ck(Pet::bootEpoch(0, 0) == 1767225600UL, "new board, blank clock: the fixed date");
  ck(Pet::bootEpoch(0, T0) == T0, "blank clock: the last saved time, not January 2026");
  ck(Pet::bootEpoch(T0 + 500, T0) == T0 + 500, "a clock that kept running is believed");
  ck(Pet::bootEpoch(T0 - 86400, T0) == T0, "a clock behind the save gives way to it");
  ck(Pet::bootEpoch(T0 - 86400, 0) == T0 - 86400, "with no save to compare, the clock stands");

  if (bad) printf("FAILURES\n");
  return bad ? 1 : 0;
}
