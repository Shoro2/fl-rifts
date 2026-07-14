# Open work

- (high) Verify in game that the native POI appears for players already in the
  zone, appears for late entrants, and disappears on leave/clear.
- (high) Run `/aio reset`, then verify the compact two-minute countdown, its
  saved right-button drag position, and the first trash spawn at expiry.
- (medium) Complete one full Rift through both trash waves and the boss, then
  confirm live wave/enemy counts and marker/UI clear for every player in the
  zone.
- (medium) Validate the Linux `bin/lua_scripts/FLRifts` install path when the
  future server host receives its first deployment.

## Element content (Fire / Water / Air)

- (high) Author the DB content in `sql/world/updates/2026_07_14_00_fl_rifts_elements.sql`
  scaffold against the live FL world DB: create the reassigned Support mobs
  (fire4 80050, air4 80030) and the boss helper NPCs (tornado 80051, globule
  80052) with real display IDs / faction / stats, and add the trash spell
  loadouts + Support SmartAI (see `docs/element-design.md`).
- (high) Build the module and test each element in game: confirm the spawner
  opens Fire/Air/Water rifts, trash casts its element spells, the two bosses per
  element run their rotation, and each signature fires (Fire flame patches, Air
  tornado/stomp knockback, Water globule heal + interruptible Healing Wave).
- (medium) Confirm the entry-ID fixes in game: air4 (80030) and shadowboss2
  (80037) are distinct creatures; fire4 (80050) resolves to the Flamewaker
  Zealot, not a stray 80017 creature.
- (low) Optional: recolor the AIO status bar per element (add an element field to
  the `WAVE|`/`COUNTDOWN|` addon message in `src/FLRifts.cpp` + `lua/`).
