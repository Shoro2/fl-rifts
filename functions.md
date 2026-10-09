# Functions

## Event lifecycle (`src/FLRifts.cpp`)

Timing constants: countdown `RIFT_START_DELAY_MS` 2 min, respawn `RIFT_RESPAWN_DELAY_MS` 30 min,
retry `RIFT_RETRY_DELAY_MS` 5 s, pause between waves `RIFT_WAVE_DELAY_MS` 10 s, wave size 10,
trash corpse `RIFT_CREEP_CORPSE_MS` 60 s. State: `RiftPhase` Inactive -> Countdown -> Active,
`waveNumber` 0..6, `riftCreeps` (GUID set of undefeated wave creatures and the boss), all
file-scope globals behind one `std::recursive_mutex`.

1. **Spawner** `FLRiftsCreatureSpawner` (90018) calls `setActive(true)` in its AI constructor so it
   updates with nobody near. `UpdateAI`: when `FLRifts.Enable`, Inactive and unlocked ->
   `SpawnRift()`: picks a random loaded, alive 90018 on its map as the location and a random
   eligible element (`FLRifts.Element.*`), summons the Rift there at Z+5
   (`TEMPSUMMON_MANUAL_DESPAWN`), sends the world text "A new Rift has formed. Its invasion
   begins in two minutes!" and starts the countdown. The workbench DB has one 90018 spawn, so
   every Rift opens at the same place.
2. **Countdown**: `FLRiftsWorldScript::OnUpdate` -> `ActivateRiftOnSchedule()` switches to Active
   after 2 min and sends "The Rift invasion has begun!".
3. **Waves** (Rift controller `FLRiftsCreatureRift`, `UpdateAI` while Active):
   - `waveNumber` 0 and 2: when `riftCreeps` is empty, summon 10 creatures, each a random one of
     the element's four trash entries, within +-20 yd, moved to the floor,
     `TEMPSUMMON_CORPSE_TIMED_DESPAWN` 60 s.
   - 1 and 3: when cleared, wait 10 s (`DelayedWaveSpawn`).
   - 4: summon one of the two bosses within +-20 yd at the Rift's Z+5 (Z+10 above the spawner),
     `TEMPSUMMON_MANUAL_DESPAWN`. Ichyron (80034) instead gets the ground height from
     `GetMapHeight`; no valid ground -> retry.
   - 5: boss dead -> "The Rift was closed, next one approaching in 30 minutes."
   - 6: the spawner closes the event (`ClearRiftEvent`: hide markers and bar, despawn the Rift,
     reset the state) and unlocks the next spawn after 30 min.
   - A failed wave or boss summon retries after 5 s.
4. **Counting**: a summon leaves `riftCreeps` on `SummonedCreatureDies` or on
   `SummonedCreatureDespawn` while alive; a counted corpse's later despawn is ignored. The Rift
   therefore closes in the tick of the boss kill and the corpses stay lootable.
5. **Early loss**: the Rift despawning before the end (spawner `SummonedCreatureDespawn` for the
   Rift GUID) resets the event and retries the spawn after 5 s. A Rift removed without
   `UnSummon` (e.g. `.npc delete`) never reaches that hook (todo).
6. **Visuals**: Air (90015) and Water (90014) controllers summon their portal GameObject
   (192819 / 195706, non-selectable, owned by the controller) in `Reset()`; Fire and Shadow use
   their creature models.

## Player side

| Piece | What |
|---|---|
| Map marker | `SMSG_GOSSIP_POI` "Active Rift" (icon 7, flags 99) at the Rift for players in its map and zone; an empty POI clears it on leave or close |
| Addon messages | whisper to self, `LANG_ADDON`, prefix `FLRIFTS`: `COUNTDOWN\|<seconds>`, `WAVE\|<1-3>\|<enemies>\|<boss 0/1>`, `HIDE` |
| Re-sync | the client sends `REQUEST` on load; the server answers with the current state |
| WorldState 1000 | `waveNumber`, sent to the zone on every wave change |
| `FLRiftsPlayerScript` | `OnPlayerLogin`, `OnPlayerUpdateZone`, `OnPlayerMapChanged` (sync marker + bar), `OnPlayerBeforeSendChatMessage` (the `REQUEST`) |
| `lua/FLRifts_Client.lua` | `FLRiftsStatusFrame` 260x30 at the top centre; right-button drag, position saved by `AIO.SavePosition`; countdown "Rift begins in mm:ss", then "Wave n/3 - x enemies" or "Boss wave - x enemies" |

## Boss and helper scripts

| ScriptName | Entries | Kit (stock spell ids) |
|---|---|---|
| `boss_shadow` | 80036, 80037 | Charge 74399 + 26478 (20 s), Silence 64189, Strike 62130 on top threat, Cleave 70670; `Talk` 0 engage, 1 death, 3 evade |
| `boss_fl_fire` | 80040 Emberlord | Charge 74399 + 26478, Cleave 56909, Fireball 19391, Scorched Ground: stock helper 33123 casts 62548 at a player for 15 s |
| | 80042 Conflagrator | Living Bomb 20475, Inferno 19695, Pyroblast 36819, Fireball, Flame Strike trap 80178 every 14-16 s |
| `npc_fl_fire_flame_strike` | 80178 | telegraph 44191 for 5 s, then 44190 damage, despawn; gone with the boss (`SummonList` on death/reset) |
| `boss_fl_air` | 80048 Stormcaller | Charge 32323 + Stormhammer 62042, Arc Lightning 52921, Lightning Nova 52960, Chain Lightning 62131, 2 tornadoes every 20-25 s |
| | 80049 Galewind | Charge 32323, Arc Lightning, Nova, Thundering Stomp 60925 (Nova rescheduled to 1.5 s after), Lightning Bolt 53044 |
| `npc_fl_air_tornado` | 80177 | passive, wanders 12 yd, pulses 43121 (Nature damage + knockback) every 1.5 s, 15 s life |
| `boss_fl_water` | 80034 Ichyron | Water Blast 54237, Frost Shock 12548, Water Bolt Volley 54241, Geyser 37478, Frostbolt 15497; below 50 % three globules every 23-27 s |
| | 80038 Nethys | Water Blast, Frost Shock, Frostbolt Volley 70759, Healing Wave 12491 (cast time, interruptible), Protective Bubble 54306 every 36-44 s |
| `npc_fl_water_globule` | 80176 | half speed, follows the boss, heals it 7.5 % of max HP within 4 yd, 30 s life |

Trash and supports run SmartAI from `fl_rifts_elements.sql` section 4 (spell list in
`docs/element-design.md`); supports buff or heal an ally (events 14/16), ranged slots keep 18 yd
(action 79). The Shadow trash 80027-80029/80035 runs its FL-DB SmartAI; its legacy ScriptName
`FLRiftsCreatureTrash` has had no code since `6b68a43` (2023-08-17).

## Config reads

| Key | Read |
|---|---|
| `FLRifts.Enable` (bool, default false) | every update of both AIs |
| `FLRifts.Element.*` (bool, default true) | at each `SpawnRift()` |
| `FLRifts.<el>1..4`, `FLRifts.<el>boss1/2` (`uint32`) | at each summon, so a reloaded conf applies to the next wave |

## SQL

The module pass applies the files in name order: `fl_rifts_elements.sql` ->
`fl_rifts_fire_support_health.sql` -> `fl_rifts_rewards.sql` (MIG-069). Placeholder stubs use
`INSERT IGNORE` (a stock DB boots clean, the FL rows win); the four new creatures use
`INSERT ... ON DUPLICATE KEY UPDATE`; the rewards updates only run when loot tables 90001/90002
exist; the HP update only touches 80017 at level 80, class 2, exp 0 or 2.

## Bot scenarios (`tests/`, mod-fl-testbots)

- `rift_lifecycle.tbs`: god mode at the spawner, clears each wave with Xi'ri's Wrath 36944, counts
  creatures with `.mmap testarea` / `.npc near`, kills the boss from 15 yd above (mod-auto-loot
  shortens a corpse next to an ungrouped looter), loots it, then waits 31 min ~600 yd away for the
  next Rift. Run 315 PASS 16/0 (2026-10-03).
- `rift_fire_support_health.tbs`: temporary copies on map 35; 80017's fresh max HP equals the
  three other supports, the other Fire roles unchanged. Run 322 PASS 21/0 (2026-10-04).

## Logging

Logger `module.fl-rifts`: spawn, countdown end, summon failures, early Rift loss;
"Closing completed Rift." at debug level.
