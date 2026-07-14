# Change log

- `fix(Rifts): reconcile element entry map with the live FL DB` - The element
  SQL/config/design targeted 80050/80030/80051/80052 and treated 80031-80034 as
  Air / 80043-80049 as Water. Verified against the live `acore_world` (and the
  original FL `update_world`): those entries are **real FL NPCs** — 80050 *Yorg
  Stormheart* (a quest-giver for *The Exobeast*/*Chapter 1*), 80030 *Nil'un*,
  80051 *Arcane Magical Anomaly* — so applying the SQL would have deleted them
  and broken a quest chain; and the FL `[PH]` labels put 80031-80034/80038 on
  Water, 80043-80046/80048-80049 on Air (the reverse). Re-targeted all element
  content onto the FL `[PH] <element> Rift monster/boss` placeholders (which
  already carry hostile faction 16, level 80/82, rank and display models),
  corrected the Air/Water assignment, restored `fire4`=80017 (a valid FL Fire
  slot; the earlier 80017→80050 "fix" was itself wrong), overrode 80038's stray
  `boss_eloxin` binding (real Eloxin is 80067), and authored only three
  genuinely-new creatures: Water support 80175, globule 80176, tornado 80177.
  Updated `fl_rifts_elements.sql`, `fl-rifts.conf.dist`, the `FLRifts.cpp`
  GetOption defaults, the `boss_fl_air`/`boss_fl_water` entry constants and
  `docs/element-design.md`. Rebuilt worldserver clean (MSVC RelWithDebInfo,
  0 errors/warnings; T1). DB apply + in-game verification pending.

- `fix(Rifts): make element SQL CI-safe and auto-applied (data/sql/db-world)` -
  CI compiled the module but the worldserver dry-run failed because
  `Errors.log` listed "Script named 'X' is not assigned in the database" for all
  registered scripts (including the pre-existing `boss_shadow` /
  `FLRiftsCreatureRift` / `FLRiftsCreatureSpawner` — master was already red on
  this check). Cause: CI applies module SQL only from
  `modules/<name>/data/sql/db-world/` (`UpdateFetcher.cpp`), but the module
  shipped SQL under the legacy `sql/world/` which modern dbimport never applies.
  Consolidated all element DB content into
  `data/sql/db-world/base/fl_rifts_elements.sql` (applied on both stock CI and
  the FL server): `INSERT IGNORE` stubs (faction 14, class 1, display 11686) for
  the FL-resident entries so a stock DB boots clean, explicit defs for the new
  creatures (80050/80030/80051/80052), all ScriptName assignments, and the
  trash/support SmartAI. On the FL DB the stubs no-op and real data wins.
  Removed the old `sql/world/updates/2026_07_14_00_fl_rifts_elements.sql`.

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
