// The Mine (mine.h and the Pet's dig / shard / fossil state).
#include "Arduino.h"
#include "Preferences.h"
#include "pet.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
uint32_t g_seed = 31; FakeSerial Serial; FakeESP ESP; FakeWire Wire;
volatile int g_touchX = 0, g_touchY = 0; volatile bool g_touchDown = false; bool wasPressed = false;
static uint32_t g_ms = 0; uint32_t millis() { return g_ms; }
void FakeESP::restart() { exit(0); }
int FakeSerial::available() { return 0; } String FakeSerial::readStringUntil(char) { return String(""); }
void sfxPlay(uint8_t) {}
static int bad = 0;
static void ck(bool ok, const char *w) { printf("%s  %s\n", ok ? "PASS" : "FAIL", w); if (!ok) bad++; }
static const uint32_t DAY = 86400, D0 = 20000 * DAY + 12 * 3600;

int main() {
  // taps: strong types and a strong arm break the rock sooner
  ck(mineTapsFor(12, 255, 0) == 8 && mineTapsFor(16, 255, 0) == 8 && mineTapsFor(6, 255, 0) == 8 && mineTapsFor(1, 255, 0) == 8, "rock, steel, fighting, fire: 8 taps");
  ck(mineTapsFor(8, 255, 0) == 10 && mineTapsFor(5, 255, 0) == 10, "ground and ice: 10");
  ck(mineTapsFor(0, 255, 0) == 14 && mineTapsFor(2, 9, 0) == 14, "the rest: 14");
  ck(mineTapsFor(12, 8, 0) == 8 && mineTapsFor(0, 12, 0) == 8, "the better of the two types counts");
  ck(mineTapsFor(0, 255, 120) == 11 && mineTapsFor(12, 255, 400) == 4 && mineTapsFor(0, 255, 4000) == 4, "attack saves taps, never under 4");

  // the loot table by roll
  int cnt[5] = { 0 };
  for (int r = 0; r < 100; r++) cnt[mineLootKind((uint8_t)r)]++;
  ck(cnt[ML_FOSSIL] == 3 && cnt[ML_SHARD] == 25 && cnt[ML_COINS] == 45 && cnt[ML_SEEDS] == 15 && cnt[ML_SNACK] == 12, "loot odds: 3 % fossil, 25 % shard, 45 % coins, 15 % seeds, 12 % snack");
  bool coinsOk = true; for (int r = 0; r < 256; r++) { int c = mineCoinsFor((uint8_t)r); if (c < 2 || c > 5) coinsOk = false; }
  ck(coinsOk, "a coin find is 2 to 5 coins");

  // fossils
  ck(FOSSIL_COUNT == 11 && isFossilDex(140) && isFossilDex(698) && !isFossilDex(25) && !isFossilDex(0), "eleven fossil species");
  bool inDex = true, sorted = true;
  for (int i = 0; i < FOSSIL_COUNT; i++) { if (FOSSIL_DEX[i] < 1 || FOSSIL_DEX[i] > DEX_COUNT) inDex = false; if (i && FOSSIL_DEX[i] <= FOSSIL_DEX[i - 1]) sorted = false; }
  ck(inDex && sorted, "all inside the dex");
  bool all = true; for (int r = 0; r < 256; r++) if (!isFossilDex(fossilPick((uint8_t)r))) all = false;
  ck(all, "a pick is always a fossil species");

  // shards
  ShardResult s = { 0, 0, false };
  for (int i = 0; i < 9; i++) { s = mineAddShard(s.shards, s.stones); ck(!s.stone, "no stone before the tenth shard"); if (bad) break; }
  s = mineAddShard(s.shards, s.stones);
  ck(s.stone && s.shards == 0 && s.stones == 1, "ten shards make exactly one stone");
  ShardResult f = mineAddShard(9, MEGA_STONE_MAX);
  ck(!f.stone && f.shards == 9 && f.stones == MEGA_STONE_MAX, "stones full: the shards wait at nine");
  ShardResult g = mineAddShard(9, MEGA_STONE_MAX - 1);
  ck(g.stone && g.stones == MEGA_STONE_MAX, "the last free slot still fills");

  // digs a day
  ck(mineDigsLeft(5, 0, 5) == 5 && mineDigsLeft(5, 3, 5) == 2 && mineDigsLeft(5, 5, 5) == 0 && mineDigsLeft(5, 9, 5) == 0, "digs left today");
  ck(mineDigsLeft(4, 5, 5) == 5, "a new day starts again");

  // a companion
  {
    Pet q; q.begin(); q.dbgHatchAs(74, false); q.setClock(D0);   // Geodude: a rock type
    ck(q.mineTaps() <= 8, "a Geodude needs 8 taps at most");
    ck(q.canMine() && q.mineDigsLeft() == MINE_DIGS_PER_DAY, "fresh: five digs");
    q.energy = 50;
    Pet::MineFind a = q.mineDig();
    ck(q.energy == 50 - MINE_ENERGY_COST && q.mineDigsLeft() == MINE_DIGS_PER_DAY - 1, "a dig costs 10 energy and one of the five");
    ck((int)a.kind >= 0 && (int)a.kind <= ML_FOSSIL, "and finds something");
    q.energy = 5;
    ck(!q.canMine(), "too tired: no dig");
    uint8_t before = q.mineDigsLeft(); Pet::MineFind none = q.mineDig();
    ck(q.mineDigsLeft() == before && q.energy == 5 && none.coins == 0, "a refused dig costs nothing");
    q.energy = 100;
    int digs = 1;
    while (q.canMine()) { q.mineDig(); q.energy = 100; digs++; }
    ck(digs == MINE_DIGS_PER_DAY && q.mineDigsLeft() == 0, "five digs, then it stops");
    q.setClock(D0 + DAY);
    ck(q.mineDigsLeft() == MINE_DIGS_PER_DAY && q.canMine(), "next day: five again");
    q.sleeping = true; ck(!q.canMine(), "not while asleep"); q.sleeping = false;
  }

  // loot over many days: the rates come out near the table, stones never pass the cap, a fossil is kept once
  {
    Pet q; q.begin(); q.dbgHatchAs(74, false); q.setClock(D0);
    int kinds[5] = { 0 }, n = 0, stones = 0; bool capOk = true, shardOk = true;
    for (int d = 0; d < 400; d++) {
      q.setClock(D0 + (d + 1) * DAY);
      for (int i = 0; i < MINE_DIGS_PER_DAY; i++) {
        q.energy = 100;
        Pet::MineFind r = q.mineDig(); kinds[r.kind]++; n++; if (r.stone) stones++;
        if (q.megaStones > MEGA_STONE_MAX) capOk = false;
        if (q.mineShards >= SHARDS_PER_STONE) shardOk = false;
        if (r.kind == ML_COINS) ck(r.coins <= 5, "coins within range");
      }
      if (bad) break;
    }
    ck(n == 2000, "2000 digs");
    ck(kinds[ML_SHARD] > 2000 * 20 / 100 && kinds[ML_SHARD] < 2000 * 30 / 100, "about a quarter are shards");
    ck(kinds[ML_COINS] > 2000 * 38 / 100 && kinds[ML_COINS] < 2000 * 52 / 100, "about 45 % coins");
    ck(capOk && shardOk, "stones never over 3, shards under 10");
    ck(q.megaStones == MEGA_STONE_MAX && q.mineShards == SHARDS_PER_STONE - 1, "full stones, shards waiting at nine");
    ck(stones == MEGA_STONE_MAX, "only three stones were ever made");
    ck(kinds[ML_FOSSIL] >= 1 && kinds[ML_FOSSIL] <= 2, "a fossil is found, and only one while it waits");
    ck(isFossilDex((int16_t)q.fossilDex), "the waiting fossil is a fossil species");
  }

  // the fossil egg
  {
    Pet q; q.begin(); q.dbgHatchAs(74, false); q.setClock(D0);
    q.fossilDex = 140;
    q.newEgg();
    ck(q.isEgg() && q.eggPeek() == 140 && q.eggFossil, "the next egg is the fossil");
    q.saveNow();
    Pet r; r.begin();
    ck(r.isEgg() && r.eggPeek() == 140 && r.eggFossil && r.fossilDex == 140, "it survives a save");
    q.setRegion(2);
    ck(q.eggPeek() == 140, "a region change does not reroll it");
    q.dbgHatchAs(q.eggPeek(), false);
    ck(!q.isEgg() && q.speciesId == 140 && q.fossilDex == 0 && !q.eggFossil, "it hatches a Kabuto and the fossil is used up");
    q.newEgg();
    ck(q.eggPeek() != 140 || !q.eggFossil, "the next egg is random again");
    // a first-game starter pick never takes the fossil
    Pet t; t.begin(); t.fossilDex = 138; t.newEgg();
    ck(!t.eggFossil || !t.awaitingStarter(), "no fossil while the player picks a starter");
  }

  // save round trip
  {
    Pet q; q.begin(); q.dbgHatchAs(74, false); q.setClock(D0);
    q.energy = 100; q.mineDig(); q.mineShards = 7; q.fossilDex = 564;
    q.saveNow();
    Pet r; r.begin();
    ck(r.mineDay == q.mineDay && r.mineToday == 1 && r.mineShards == 7 && r.fossilDex == 564, "digs, shards and fossil survive a save");
    r.setClock(D0);
    ck(r.mineDigsLeft() == MINE_DIGS_PER_DAY - 1, "and the day's count with them");
  }

  if (bad) { printf("FAILURES: %d\n", bad); return 1; }
  printf("mine test passed\n");
  return 0;
}
