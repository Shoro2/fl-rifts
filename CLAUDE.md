# fl-rifts

Forgotten Land's repeating world event. A Rift opens at the Rift spawner on the hub map 727,
counts down two minutes, sends two trash waves of 10 and then one boss, closes on the boss kill
and opens again 30 minutes later. Each Rift is one of four elements (Shadow, Fire, Water, Air)
with its own trash, two possible bosses and a boss mechanic. Players get a map marker, a
top-centre AIO status bar (countdown, wave, enemies left) and the Shadow Rift loot.
Full description: [README.md](README.md); element design: [docs/element-design.md](docs/element-design.md).

## Ids and tables

| What | Id / name |
|---|---|
| Rift controllers | 90014 Water, 90015 Air, 90016 Fire, 90017 Shadow ("Dimensional Rift" in the FL DB), ScriptName `FLRiftsCreatureRift` |
| Spawner | 90018 "Rift Summoner", ScriptName `FLRiftsCreatureSpawner`; one spawn, guid 5300694 on map 727 |
| Shadow | trash 80027 80028 80029 80035; bosses 80036 "Trashlan", 80037 "Vorath, the Hollow King" (`boss_shadow`) |
| Fire | trash 80039 80041 80047 80017; bosses 80040 80042 (`boss_fl_fire`); Flame Strike trap 80178 (`npc_fl_fire_flame_strike`) |
| Water | trash 80031 80032 80033 80175; bosses 80034 Ichyron, 80038 Nethys (`boss_fl_water`); globule 80176 (`npc_fl_water_globule`) |
| Air | trash 80043 80044 80045 80046; bosses 80048 80049 (`boss_fl_air`); tornado 80177 (`npc_fl_air_tornado`) |
| Rift visuals | stock GameObjects 192819 (Air), 195706 (Water); stock helper creature 33123 (Fire boss 1's Scorched Ground) |
| Loot | trash `lootid` 90001, bosses 90002 (the Shadow Rift tables, from the FL DB) |
| Client | WorldState 1000 (wave counter); addon prefix `FLRIFTS`; Lua `lua/FLRifts_Client.lua` |

Tables: the module owns none. Its world SQL writes `creature_template`, `creature_template_model`
and `smart_scripts` rows for the ids above. Owned by this module: the four new creatures
80175-80178. Everything else (the `[PH]` placeholders, Shadow creatures, spawner spawn, loot
tables 90001/90002) is FL world-DB content from the July 2026 migration that the SQL renames,
tunes and binds. Spells are stock 3.3.5a ids; no custom DBC.

## Status and progress

- **Where it runs**: workbench (built from `azerothcore-wotlk/modules/fl-rifts`, deployed conf
  `FLRifts.Enable = 1`, all four elements eligible). Host: since the first deploy (2026-07-13);
  MIG-001 (catch-up 2026-07-25, `FLRifts.*` conf overrides), MIG-064 (HOST-4, 2026-10-03:
  lifecycle + loot), MIG-069 + MIG-072 (HOST-6, 2026-10-04: Fire support HP, Ichyron ground
  placement). The host runs `75028c2` = current `master`.
- **Evidence**: status bar T2 (operator, 2026-07-12); lifecycle T1, bot run 315 PASS 16/0
  (2026-10-03); Fire support HP T1, bot run 322 PASS 21/0 (2026-10-04); Ichyron loot T2
  user-reported (operator, 2026-10-04). The testers' re-check on the host and the in-game
  element pass are owed.
- **Done**:
  - Port to the current AzerothCore API (2026-07-09); config entries read as `uint32` since
    `79483fc` (2026-07-12, the W27 overflow fix).
  - Map marker + AIO status bar (2026-07-12).
  - Fire, Water and Air elements with own trash kits, bosses and helpers, tuned to the Shadow
    Rift (2026-07-14..18); entries reconciled with the FL DB, real NPCs 80030/80050/80051 untouched.
  - Lifecycle fixes (2026-10-03): spawner always active, a wave creature counts at death, the
    Rift closes on the boss kill, element Rifts pay the Shadow loot.
  - Fire support 80017 HP normalised; Ichyron spawns on the ground (2026-10-04).

## Next steps

1. Testers re-check a Rift on the host: close on the boss kill, loot, next Rift after 30 min
   with nobody near (queue row "fl-rifts — live on the host since HOST-4", MIG-064).
2. In-game element pass: Air/Water portal visuals, names, scales, boss mechanics, Shadow-level
   trash (todo "Element content").
3. Fix the queue row's "noticed, not fixed" list: 90014/90015 subnames swapped, every boss but
   Ichyron spawns 10 yd above the spawner, element trash and bosses have `detection_range` 1.
4. Tune Shadow boss 2 (80037 Vorath, an untuned placeholder) or point `FLRifts.shadowboss2` at a
   real boss (todo, queue row).
5. Bring README "Verification state" / "Ichyron ground placement" and the element-design status
   table up to date with HOST-6 (todo).

Open points in full: [todo.md](todo.md).

## Working here

- Branch `claude/<topic>-<sessionId>`, merge into `master` and push (project rule: no pull requests).
- Build worldserver from `C:\Users\Anwender\Documents\GitHub\azerothcore-wotlk` into
  `C:\wowstuff\dcore_bin`; restart the workbench only with
  `C:\wowstuff\ForgottenLand2.0\scripts\worldserver_restart.ps1`. The updater applies
  `data/sql/db-world/` at start in file-name order and re-applies a file whose bytes change, so
  keep every file idempotent and stock-DB safe (CI boots a stock DB). The legacy `sql/` folders
  are never applied.
- Lua: `lua/` also lives in fl-lua-scripts `FLRifts/` (the deployed `lua_scripts` on the
  workbench and the host); keep both byte-identical; test UI changes after `/aio reset`.
- Bot scenarios in `tests/` (mod-fl-testbots): this folder is not in the workbench's
  `TestBots.ScenarioDirs` (2026-10-09); add it or copy the scenario before queueing a run.
- Host-relevant changes need a MIG entry in share-public
  `docs/World of Warcraft/forgotten-land/15-host-migration-log.md`.
- Vault: `docs/World of Warcraft/forgotten-land/06-fl-modules.md`, `12-server-todo.md`,
  `forgotten-land/15-host-migration-log.md` (MIG-064, MIG-069, MIG-072), `testing/01-test-bots.md`.
- Doc set: [INDEX.md](INDEX.md), CLAUDE.md, [data_structure.md](data_structure.md),
  [functions.md](functions.md), [log.md](log.md) (newest first), [todo.md](todo.md).
