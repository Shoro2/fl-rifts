# Change log

- `docs(Rifts): Record accepted Ichyron loot` - 2026-10-04: source bda3b1e
  is locally merged and incrementally built; native runs 327/328 confirm the
  same boss/corpse on actual floor, a normal bot kill and Rift closure without
  server interruption. Overall scripts remain FAIL 25/1 and 20/1 on GPS label
  parsing and optional Taunt; ordinary broker aborted before client hit/loot
  after actual combat displacement. Operator now explicitly confirms in-game
  loot (T2 user-reported) and authorizes merge. Close local acceptance without
  fabricating an automated melee PASS or repeating tests. No C++/SQL/client
  change, rebuild, restart or host action in this documentation update.
  Source publication and approved combined host rollout remain MIG-072 pending.

- `fix(Rifts): Normalize Fire support health` - Flamewaker Zealot 80017
  inherited exp=2, making the copied modifier 17 produce 214200 HP instead
  of the other supports' 72658. New narrow exp0/2-aware SQL changes only its
  HealthModifier, preserving exp, damage, abilities and the historical base
  SQL. Stock-style exp=0 remains at modifier 17; that path is source/arithmetic
  verified only. Root applied the guarded SQL on the Windows workbench and
  reloaded only template 80017. Run 322 of the new bounded
  `tests/rift_fire_support_health.tbs` passed 21/0 on 2026-10-04: fresh Fire
  support equalled all three support peers at 72658 HP, the other Fire roles
  remained 90814/63563/93475 HP, and all seven temporary copies disappeared.
  Complete selected-row comparison changed only 80017 HealthModifier; no
  active queue, online test bots, persistent spawns or test map-35 bindings
  remained. T1 server HP evidence, not full Rift combat or client proof.
  No build/restart/host action; production rollout is MIG-069 PENDING.

- `docs(Rifts): Record the lifecycle bot pass` - T1 bot-verified on the
  workbench (worldserver b4fde7ad = fl-rifts b71422b, `fl_rifts_rewards.sql`
  applied): run 315 of `tests/rift_lifecycle.tbs` PASSED 16/0 in 2240 s on an
  Air Rift - both waves verified standing and cleared, the Rift gone and
  "Closing completed Rift." logged in the tick of the boss kill, the boss
  corpse still there 3 s later, 100 Bloody Trophies and the Badge looted from
  it, and with the director 605 yd away for 2000 s the next Rift (Fire)
  opened 30 minutes after the close and its first wave stood on return. The
  rewards SQL now says that Fire support 80017 drops its inherited stock
  table 30847 (Raging Flame's world loot) for 90001 on purpose: 80017 has no
  world spawn on the workbench or the host, so only Fire Rift kills change.
  The comment changes the file's hash, so the updater re-applies it once on
  a box that already has it - harmless, it is idempotent.

- `test(Rifts): Make the lifecycle scenario deterministic` - The first
  version cast blindly three times; run 312 (2026-10-03, fixed build
  b4fde7ad) left a wave straggler or an unkilled boss behind and could not
  tell. The scenario is element-agnostic now: every Rift is named
  "Dimensional Rift" and the checks count creatures (`.mmap testarea`: the
  alive ones within 40 yd; `.npc near`: all, corpses included), each wave is
  verified standing (12) and cleared (2) before the next step, Xi'ri's Wrath
  is cast from the ground and from above (line of sight), the boss is killed
  from 15 yd above its spawn height and its corpse looted through a 12-yd
  sweep. Run 314 on the fixed build: waves 1 and 2 cleared; "Closing
  completed Rift." in the same tick as the boss kill; Rift gone and alive
  count 1 three seconds later; 100 Bloody Trophies and the Badge looted from
  the boss; with the director 605 yd away a new Rift opened 30 minutes after
  the close and its first wave stood on return. Its one FAIL was the corpse
  count: the boss died 10 yd from the ungrouped director, and mod-auto-loot
  calls `AllLootRemovedFromCorpse` on every corpse within 10 yd every 250 ms,
  each call halving the remaining corpse time, so the corpse was gone within
  about 3 s - hence the kill from above.

- `test(Rifts): Add a lifecycle bot scenario` - `tests/rift_lifecycle.tbs`
  (mod-fl-testbots) drives one Rift to its end with Xi'ri's Wrath (36944),
  checks that it closes as the boss dies and that the next Rift opens while
  the director waits 31 minutes ~600 yd away; README and `todo.md` describe the
  new lifecycle and the owed build, run and reward decision. Not run yet (needs
  the fixed build and the folder in `TestBots.ScenarioDirs`).

- `fix(Rifts): Pay element Rifts the Shadow loot` -
  Root cause of "Rift: no reward" (host, 2026-10-03; the testers cleared a
  Water and an Air Rift). The module hands out nothing itself; the reward is
  creature loot, and only the Shadow Rift has any: trash 80027-80029/80035
  `lootid` 90001 (Bloody Trophy x1-5) and boss 80036 `lootid` 90002 (100
  Bloody Trophies, The Blood Trails Badge, one item). Every Fire/Water/Air
  trash and boss and Shadow boss 2 (80037) carry `lootid` 0 (80017 a stock
  junk table) on the host and the workbench. New
  `data/sql/db-world/updates/fl_rifts_rewards.sql` gives them the Shadow
  tables (parity default - the operator may want element-specific loot),
  guarded by EXISTS so a stock DB stays clean. Trash loot was out of reach by
  hand as well: a `TEMPSUMMON_CORPSE_DESPAWN` corpse is gone with the
  creature's next update, so only mod-auto-loot's scan for an ungrouped player
  within 10 yd picked it up (it did for an ungrouped test bot, run 281); a
  grouped player never saw a lootable corpse. Trash now keeps its corpse for
  one minute (`RIFT_CREEP_CORPSE_MS`), which the kill-time wave count makes
  harmless. T1: the SQL applied to scratch copies of the workbench tables
  (idempotent; no-op without the loot tables) and passes `codestyle-sql.py`;
  the C++ passed an MSVC `/Zs` front-end check only (not built).

- `fix(Rifts): End the boss wave on the boss kill` -
  Root cause of "Rift hangs after killing boss" (host, 2026-10-03). Wave
  creatures only counted as gone in `SummonedCreatureDespawn`. Trash is a
  `TEMPSUMMON_CORPSE_DESPAWN` summon and vanishes on death, but the boss is a
  `TEMPSUMMON_MANUAL_DESPAWN` (= `DEAD_DESPAWN`) summon whose corpse only
  unsummons after its corpse decay: 300 s for the elite element bosses, halved
  to 150 s because they have no loot, 600 s for the world boss 80036 outside an
  instance. All that time the Rift sat on "Boss wave - 1 enemy" with no
  message, and players who walked off left it open for good (previous entry).
  The undefeated summons are now a GUID set: a summon leaves it on
  `SummonedCreatureDies` or when it despawns alive, and the later despawn of a
  counted corpse is ignored, so the Rift closes as the boss dies and its
  corpse stays lootable. T0: not built yet.

- `fix(Rifts): Keep the Rift spawner always updated` -
  Root cause of "Rifts don't respawn" (host, 2026-10-03). The spawner 90018
  runs `SpawnRift`, closes a finished Rift and carries the 30-minute respawn
  event on its own `m_Events`, but as an inactive DB creature it is only
  updated while a player is near it (`Creature::IsUpdateNeeded`: a marked
  cell, a visible player, combat ...; `EventProcessor` time advances only by
  the creature's own updates). The only spawner (guid 5300694, map 727) is
  about 580 yd from where players gather, so a finished Rift stayed open until
  someone came by and the respawn delay only ran while someone stood there.
  Host evidence: the 02:22 boot logged "Closing completed Rift." only right
  after a character login and opened no further Rift in the next six hours
  while players stayed at the hub; 310 workbench boots never opened more than
  one Rift. The spawner now calls `setActive(true)`. T0: not built yet - needs
  a rebuild and restart.

- `fix(Rifts): Use portal GameObjects for Air and Water` -
  Air Rift now renders stock GameObject 192819 (Defender's Portal) and Water
  Rift renders stock GameObject 195706 (Horde Gunship Portal Effects). Their
  existing creature bodies use invisible display 11686 while retaining all
  event logic. Each portal is non-selectable and owned by its Rift controller;
  resets replace it and AzerothCore's owned-GameObject cleanup removes it with
  the controller. Fire and Shadow visuals are unchanged. The changed C++ file's
  targeted codestyle/lifecycle checks and the isolated idempotent SQL check
  pass; build/live DB apply/in-game visual QA remain owed.

- `fix(Rifts): Balance elements against Shadow` -
  Read the live `acore_world.creature_template` values for all Shadow and
  elemental Rift creatures. Fire, Water and Air trash now copy the four Shadow
  archetype slots at health/damage modifiers 17/18, 17/15, 25/15 and 17/15;
  all six elemental bosses copy original Shadow boss 80036 at 47/19. The
  retrofitted Shadow placeholder 80037 is not used as a benchmark, and the
  globule, tornado and Flame Strike trap remain intentionally lightweight.
  The complete module SQL applied successfully to both a live-data clone and
  an empty stock-style schema; all expected values matched and both temporary
  schemas were removed. `git diff --check` passes. The official AzerothCore SQL
  checker passes every content/safety check and reports only the intentional,
  pre-existing module `base/` directory policy warning. T1 database-verified
  on the Windows operator box; live DB apply and in-game combat are still owed.

- `fix(Rifts): Polish element combat` -
  Replaced all configured Fire/Water/Air placeholder names plus Shadow boss 2,
  made every Rift non-selectable, and set exact scales for Fire Rift (3.0),
  Fire boss 2 (1.2) and Air boss 1 (1.05). Converted offensive casters,
  supports and caster bosses to mana-bearing classes; ranged trash now keeps
  18-yard distance. Trash kits are capped at one melee or at most two caster
  abilities, Flame Breath remains only on the Fire brute at a longer cooldown,
  and 80017's buffs explicitly target another friendly creature. Fire boss 1
  no longer casts Flame Breath and now creates damaging stock Scorched Ground
  `62548` at the selected player's real position, bypassing Ignis `62546`'s
  Ulduar-only hard-coded elevation. Fire boss 2 owns a new non-selectable trap
  `80178`: visual `44191` expires after five seconds, damage `44190` fires, and
  all traps clean up on boss death/reset. Air tornadoes now pulse fixed Nature
  damage plus knockback (`43121`) instead of weapon-percent spell `56855`.
  Water globules move at half speed and heal 7.5% (up from 5%); Water boss 2
  has mana and casts Protective Bubble at half the old frequency. Verified the
  SQL in an isolated clone schema without mutating `acore_world`; names,
  classes, flags, scales, scripts, ranged movement and 1/2-spell caps match.
  `git diff --check` passes. The official C++ checker reports only pre-existing
  issues in untouched `boss_template.cpp`, `FLRifts_bosses.cpp` and
  `FLRifts_loader.cpp`; SQL content checks pass, with the intended `base/`
  directory change requiring maintainer acknowledgement. Current evidence is
  T0: build, live SQL apply and in-game verification remain pending.

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
