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

- (done) Entry map reconciled with the live FL DB — element content reuses and
  renames the FL `[PH] <element> Rift monster/boss` placeholders; four new
  creatures (Water support 80175, globule 80176, tornado 80177, Flame Strike
  trap 80178). See
  `docs/element-design.md` "Entry map — reconciled". Real NPCs (Yorg Stormheart
  80050, Nil'un 80030, Arcane Magical Anomaly 80051) are untouched.
- (done, prior revision T1) Build worldserver (MSVC RelWithDebInfo,
  `dcore_bin`) — clean before the current combat-polish changes.
- (high) Build the current `claude/balance-rift-creatures-019f74ad` branch and
  run the module/Core codestyle checks.
- (high) Apply `data/sql/db-world/base/fl_rifts_elements.sql` to the live
  `acore_world`, boot worldserver, and confirm `Errors.log` stays at the 7-line
  baseline (no "not assigned" / creature_template errors).
- (high) Test each element in game: spawner opens Fire/Air/Water rifts, trash
  casts its element spells, the two bosses per element run their rotation, and
  each signature fires (Fire boss 1 Scorched Ground 62548 visibly damages and
  cleans up; Fire trap 80178 damages at telegraph expiry and cleans up on boss
  death; Air tornado 80177 damages/knocks back; Water globule 80176 arrives
  slowly and heals 7.5%; Water boss 2 casts mana abilities and bubbles at half
  the prior frequency).
- (medium) Confirm all four Rifts are non-selectable; Fire Rift is 3x its old
  scale, Fire boss 2 is 2x, and Air boss 1 is 1.5x.
- (medium) Confirm no `[PH]` Rift mob/boss names remain, Fire boss 1 no longer
  uses Flame Breath, 80017 buffs other mobs, and trash follows the one-melee /
  two-caster ability cap.
- (medium) Confirm Fire/Water/Air trash now survives and hits at the accepted
  Shadow-Rift level: slots 1-4 use health/damage modifiers 17/18, 17/15,
  25/15 and 17/15; all six elemental bosses use 47/19.
- (medium) QA the remaining placeholder displays on 80175/80176/80177 and the
  Protective Bubble `54306` flat-reduction behavior.
- (low) Optional: recolor the AIO status bar per element (add an element field to
  the `WAVE|`/`COUNTDOWN|` addon message in `src/FLRifts.cpp` + `lua/`).
