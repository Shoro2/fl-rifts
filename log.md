# Change log

- `fix(Rifts): harden element AIs from adversarial review` - Multi-agent
  verification pass over the new C++/SQL/logic (checked against the live
  AzerothCore headers/schema) confirmed two gameplay bugs, now fixed:
  (1) Galewind (airboss2) re-scheduled the already self-repeating Lightning Nova
  via `ScheduleEvent` on every Thundering Stomp — EventMap is a no-dedupe
  multimap, so Nova cycles stacked up over the fight; switched to
  `RescheduleEvent`. (2) `SummonRiftCreep` still hardcoded the pre-fix config
  defaults (`fire4` 80017, `air4` 80037) — an absent config key could summon
  shadowboss2 (80037) as air trash or a non-existent 80017; defaults aligned to
  80050 / 80030. No compile or dbimport issues were found.

- `feat(Rifts): Design + wire Fire/Water/Air elements` - Added the non-Shadow
  Rift elements. The spawner (`src/FLRifts.cpp` `SpawnRift`) now picks a random
  eligible element rift (90014-90017) instead of always Shadow, gated by new
  `FLRifts.Element.*` config toggles. Added boss AIs `boss_fl_fire`,
  `boss_fl_air`, `boss_fl_water` (each covers its two boss variants + one
  signature mechanic) plus helper NPCs `npc_fl_air_tornado` /
  `npc_fl_water_globule`, registered in `FLRifts_loader.cpp`. Fixed the config
  entry collision `air4=80037` (== `shadowboss2`) → `80030` and the outlier
  `fire4=80017` → `80050`. Full creature/spell design in
  `docs/element-design.md`; boss ScriptName bindings + a reconciliation scaffold
  in `sql/world/updates/2026_07_14_00_fl_rifts_elements.sql`. All spell IDs are
  stock 3.3.5a NPC abilities (no DBC patching), verified against the on-disk
  AzerothCore scripts. Build + in-game verification pending (see `todo.md`).

- `feat(Rifts): Add map marker and status UI` - T1: the Windows operator-box
  command `cmake --build C:\wowstuff\dcore_bin --config RelWithDebInfo`
  `--target worldserver --parallel 16` passed, both Lua files passed
  `lua52_compiler.exe -p`, and the matching binary/Lua files were deployed.
  T2: the operator confirmed the compact top status bar works in game. The
  complete Rift lifecycle remains pending.
