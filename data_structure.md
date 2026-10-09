# Data structure

## Files

| Path | What |
|---|---|
| `src/FLRifts.cpp` | event core: spawner and Rift controller AIs, wave/boss logic, map marker, addon messages, World/Player scripts |
| `src/FLRifts_bosses.cpp` | `boss_shadow` (both Shadow bosses) |
| `src/FLRifts_boss_fire.cpp` | `boss_fl_fire` + Flame Strike trap `npc_fl_fire_flame_strike` |
| `src/FLRifts_boss_water.cpp` | `boss_fl_water` + Rising Tide globule `npc_fl_water_globule` |
| `src/FLRifts_boss_air.cpp` | `boss_fl_air` + tornado `npc_fl_air_tornado` |
| `src/FLRifts_loader.cpp` | `Addfl_riftsScripts()` registers the five script groups |
| `src/boss_template.cpp` | unused boss skeleton (`AddSC_BossTemplateScript` is never called) |
| `lua/FLRifts_Client.lua` | AIO client: status bar, countdown, wave text |
| `lua/FLRifts_Server.lua` | intentionally empty (C++ owns the state) |
| `fl-rifts.cmake` | with mod-ale: copies `lua/` to `bin/lua_scripts/FLRifts` on build and install |
| `conf/fl-rifts.conf.dist` | `FLRifts.*` keys (below) |
| `data/sql/db-world/base/fl_rifts_elements.sql` | element content: stubs, names, flags, classes, scales, the four new creatures, tuning, ScriptNames, trash SmartAI |
| `data/sql/db-world/updates/fl_rifts_fire_support_health.sql` | 80017 `HealthModifier` only (exp-aware) |
| `data/sql/db-world/updates/fl_rifts_rewards.sql` | element trash -> `lootid` 90001, element bosses + 80037 -> 90002 |
| `tests/rift_lifecycle.tbs` | bot scenario: one Rift from opening to the next (~40 min) |
| `tests/rift_fire_support_health.tbs` | bot scenario: 80017 HP against its peers |
| `docs/element-design.md` | element design reference |
| `README.md`, `todo.md`, `log.md` | description, open work, change log |
| `sql/` | legacy skeleton layout (placeholder files only); never applied by the updater |
| `README_ES.md`, `README_example.md`, `conf/conf.sh.dist`, `include.sh`, `.git_commit_template.txt`, `setup_git_commit_template.sh` | AzerothCore skeleton leftovers |
| `.github/workflows/core-build.yml` | CI: AzerothCore's reusable module build |

## Tables touched (acore_world)

| Table | Rows |
|---|---|
| `creature_template` | 80017, 80031-80034, 80036-80049, 80175-80178, 90014-90018: names, subnames, flags, classes, modifiers, ScriptName/AIName, `lootid` |
| `creature_template_model` | the same entries; invisible display 11686 for 90014/90015; scales for 90016/80042/80048 |
| `smart_scripts` | trash and support SmartAI of 80017, 80031-80033, 80039, 80041, 80043-80047, 80175 |

No characters or auth tables. The event state lives in memory only: a restart drops a running
Rift, and the next one opens once the spawner's grid is loaded again (no grid preload on the
workbench; see the header of `tests/rift_lifecycle.tbs`).

## Config keys (`conf/fl-rifts.conf.dist`)

| Key | Default | Meaning |
|---|---|---|
| `FLRifts.Enable` | 0 (dist), 1 on the workbench | master switch, read every update |
| `FLRifts.Element.Shadow/Fire/Air/Water` | 1 | element may be picked; none enabled -> Shadow |
| `FLRifts.<el>1..4` | see below | trash slots: 1 melee, 2 caster, 3 disruptor, 4 support |
| `FLRifts.<el>boss1/2` | see below | boss picked at random per Rift |

Defaults (conf and code agree): shadow 80027/80028/80029/80035, bosses 80036/80037; fire
80039/80041/80047/80017, bosses 80040/80042; air 80043/80044/80045/80046, bosses 80048/80049;
water 80031/80032/80033/80175, bosses 80034/80038.
