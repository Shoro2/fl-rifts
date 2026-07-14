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

- (done) Entry map reconciled with the live FL DB — element content reuses the
  FL `[PH] <element> Rift monster/boss` placeholders; three new creatures
  (Water support 80175, globule 80176, tornado 80177). See
  `docs/element-design.md` "Entry map — reconciled". Real NPCs (Yorg Stormheart
  80050, Nil'un 80030, Arcane Magical Anomaly 80051) are untouched.
- (done, T1) Build worldserver (MSVC RelWithDebInfo, `dcore_bin`) — clean.
- (high) Apply `data/sql/db-world/base/fl_rifts_elements.sql` to the live
  `acore_world`, boot worldserver, and confirm `Errors.log` stays at the 7-line
  baseline (no "not assigned" / creature_template errors).
- (high) Test each element in game: spawner opens Fire/Air/Water rifts, trash
  casts its element spells, the two bosses per element run their rotation, and
  each signature fires (Fire flame patches, Air tornado 80177 / Thundering Stomp
  knockback, Water globule 80176 heal + interruptible Healing Wave).
- (medium) QA the known tuning points: support friendly-targeting (Frenzy 19451
  self-cast risk → Garr-Frenzy 19516 fallback; caster-class mobs may lack mana
  for cost spells), placeholder displays on 80175/80176/80177, Protective Bubble
  54306 flat reduction, and trash-spell / boss-rotation timers.
- (low) Optional: recolor the AIO status bar per element (add an element field to
  the `WAVE|`/`COUNTDOWN|` addon message in `src/FLRifts.cpp` + `lua/`).
