# Open work

- (medium) Testers' re-check of a Rift on the host: it closes on the boss kill,
  pays loot, and the next Rift opens 30 minutes later with nobody near. Live
  there since HOST-4 (MIG-064); owed per the queue row "fl-rifts — live on the
  host since HOST-4" (share-public `docs/World of Warcraft/12-server-todo.md`).
- (medium) Shadow boss 2 (80037, "Vorath, the Hollow King") is an untuned
  placeholder (rank 0, health/damage modifiers 1.5/2 against 47/19 for
  80036) and wins half of all Shadow Rifts; tune it or point
  `FLRifts.shadowboss2` at a real boss.
- (medium) Element trash (80031-80033, 80039, 80041, 80043-80047) and every
  boss have `detection_range` 1, so they aggro only within the 5-yd floor of
  `Creature::GetAggroRange` (Shadow trash: 15).
- (medium) The FL DB swaps the Rift subnames: 90014 (the Water Rift in
  `FLRifts.cpp`) shows "Air", 90015 (Air) shows "Water" (queue row above;
  workbench DB checked 2026-10-09). Set them in the module SQL.
- (medium) Every boss except Ichyron is summoned at the Rift's Z+5, i.e. 10 yd
  above the spawner (queue row above); Ichyron got ground placement in
  `bda3b1e`. Decide whether the other bosses get the same.
- (low) A Rift that leaves the world without `UnSummon` (e.g. `.npc delete`)
  never reaches the spawner's `SummonedCreatureDespawn`, leaving the event
  Active without a Rift until a restart; the spawner could re-check
  `riftState.RiftGuid` in `UpdateAI`.
- (high) Verify in game that the native POI appears for players already in the
  zone, appears for late entrants, and disappears on leave/clear.
- (high) Run `/aio reset`, then verify the compact two-minute countdown and its
  saved right-button drag position (the first trash spawn at expiry is
  bot-verified, `tests/rift_lifecycle.tbs` run 315).
- (medium) With a real client, follow one Rift through both trash waves and
  the boss and confirm the live wave/enemy counts and that marker and UI
  clear for every player in the zone (the server side - waves, close on the
  boss kill, loot, next Rift - is bot-verified by `tests/rift_lifecycle.tbs`).
- (medium) Confirm on the host that `bin/lua_scripts/FLRifts` matches `lua/`.
  The host's `lua_scripts` is the fl-lua-scripts checkout, which carries its own
  copy in `FLRifts/`; no record of this check in the vault.
- (low) README "Verification state" and "Ichyron ground placement" and the
  status table in `docs/element-design.md` still call MIG-069/MIG-072, the
  current build and the element SQL apply pending; all are done (workbench
  updater rows; host HOST-6, 2026-10-04). The README also promises native
  area-trigger countdown announcements (2:00 ... last 5 s) that no code in
  `src/` sends; drop the line or add the announcements.
- (low) Before any `MapUpdate.Threads` > 1 experiment, re-check the file-scope
  Rift globals (one recursive mutex), as share-public
  `forgotten-land/10-mass-pull-performance-plan.md` §5.3 asks.
- (low) (suggestion) Clear the legacy ScriptName `FLRiftsCreatureTrash` that
  the FL DB still carries on Shadow trash 80027-80029/80035; no code registers
  it since `6b68a43`, their SmartAI runs. (The non-Rift NPCs 80010 and 80030
  carry it too; they are FL world content, not this module's.)

## Element content (Fire / Water / Air)

- (high) Test each element in game: spawner opens Fire/Air/Water rifts, trash
  casts its element spells, the two bosses per element run their rotation, and
  each signature fires (Fire boss 1 Scorched Ground 62548 visibly damages and
  cleans up; Fire trap 80178 damages at telegraph expiry and cleans up on boss
  death; Air tornado 80177 damages/knocks back; Water globule 80176 arrives
  slowly and heals 7.5%; Water boss 2 casts mana abilities and bubbles at half
  the prior frequency).
- (medium) Confirm all four Rifts are non-selectable; Fire Rift is 3x its old
  scale, Fire boss 2 is 2x, and Air boss 1 is 1.5x.
- (medium) Confirm Air Rift renders GameObject 192819 and Water Rift renders
  GameObject 195706 at the controller position, with no visible underlying
  creature, no interaction cursor, no duplicate after reset, and no orphaned
  portal after Rift closure. Fire and Shadow must remain visually unchanged.
- (medium) Confirm no `[PH]` Rift mob/boss names remain, Fire boss 1 no longer
  uses Flame Breath, 80017 buffs other mobs, and trash follows the one-melee /
  two-caster ability cap.
- (medium) Confirm Fire/Water/Air trash now survives and hits at the accepted
  Shadow-Rift level: slots 1-4 use health/damage modifiers 17/18, 17/15,
  25/15 and 17/15, with the exp-aware Fire support HP correction described in
  `docs/element-design.md`; all six elemental bosses use 47/19. Fresh support
  HP parity and the other three Fire-role HP values passed run 322; full
  in-Rift combat remains owed.
- (medium) QA the remaining placeholder displays on 80175/80176/80177 and the
  Protective Bubble `54306` flat-reduction behavior.
- (low) Optional: recolor the AIO status bar per element (add an element field to
  the `WAVE|`/`COUNTDOWN|` addon message in `src/FLRifts.cpp` + `lua/`).
