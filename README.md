# fl-rifts (Forgotten Land Rifts)

An AzerothCore module that runs a repeating world event: a Rift opens at a spawn point, counts down, sends waves of
elemental monsters and finally a boss. Part of the Forgotten Land server project.

## Features

- A random Rift spawn location is chosen, a Rift appears and, after a two-minute countdown, two trash waves of ten
  creatures and one boss wave attack. The Rift closes when the boss dies; the next Rift opens 30 minutes later,
  whether or not any player is near the spawner.
- Four elements (Shadow, Fire, Water, Air). Each Rift picks a random eligible element, with its own four trash types,
  two possible bosses and a boss mechanic (for example Scorched Ground and Flame Strike traps for Fire, tornadoes for
  Air, healing globules for Water). Air and Water Rifts show as portal gameobjects.
- Players in the Rift's zone get a native map marker ("Active Rift") that disappears when they leave or the Rift ends.
- A compact status bar at the top centre of the screen shows the countdown, then the current wave and the number of
  enemies left. Hold the right mouse button to move it; the position is saved. Area announcements at 2:00, 1:00, 0:30,
  0:10 and the last seconds act as a fallback.
- Wave creatures count as defeated the moment they die, so corpses stay lootable after the Rift closes.
- Fire support creature 80017 gets its health aligned with the other three supports
  (`data/sql/db-world/updates/fl_rifts_fire_support_health.sql`).

## Requirements

- AzerothCore (WotLK 3.3.5a), current master. No extra core patches are needed.
- For the status bar: [mod-ale](https://github.com/azerothcore/mod-ale) (the AzerothCore Lua Engine) with the AIO
  framework. Without them the Rifts still run; players only miss the status bar.
- Loot: the rewards script (`data/sql/db-world/updates/fl_rifts_rewards.sql`) points the Rift monsters at loot tables
  90001 (trash) and 90002 (bosses) from the Forgotten Land database. Those tables are not part of this repository and
  the script only takes effect when they exist, so create your own tables with these ids (or set `lootid` of the Rift
  creatures yourself) if you want the monsters to drop something.

## Installation

1. Clone into the `modules/` folder of your AzerothCore source tree:
   ```
   cd azerothcore-wotlk/modules
   git clone https://github.com/Shoro2/fl-rifts.git
   ```
2. Re-run CMake and rebuild the worldserver. When mod-ale is present, the build copies `lua/` to
   `lua_scripts/FLRifts` (in the build `bin` folder and, on install, below `<install-prefix>/bin/lua_scripts/FLRifts`).
   Make sure that folder ends up in the `lua_scripts` directory your worldserver loads.
3. Copy `conf/fl-rifts.conf.dist` to `fl-rifts.conf` in the folder of your `worldserver.conf` and set
   `FLRifts.Enable = 1` (the event is off by default).
4. Start the worldserver. The AzerothCore updater imports `data/sql/db-world/` (element creatures, scripts and AI;
   it also works on a stock database, where missing placeholder creatures are created as stubs). The `sql/` folders are
   empty legacy skeleton files and are not used.
5. Spawn at least one Rift spawn location (see Usage). After changing the AIO client file, run `/aio reset` in the
   client.

## Configuration

`conf/fl-rifts.conf.dist`:

| Key | Default | Meaning |
|---|---|---|
| `FLRifts.Enable` | 0 | enable the Rift event |
| `FLRifts.Element.Shadow / Fire / Air / Water` | 1 | whether the element may be picked for a new Rift (if none is enabled, Shadow is used) |
| `FLRifts.<element>1` - `<element>4` | see file | creature entries of the four trash types (melee bruiser, caster, disruptor, support) |
| `FLRifts.<element>boss1`, `<element>boss2` | see file | the two possible bosses; one is picked per event |

Changed entries apply to the next summon after a config reload.

## Usage

- Rift spawn locations are creatures with entry **90018** ("Rift Spawn Location"). Place one or more with a GM account,
  e.g. `.npc add 90018`; each new Rift opens at a random one of the loaded spawners on its map. The Rift controllers
  (entries 90014 Water, 90015 Air, 90016 Fire, 90017 Shadow) are summoned by the spawner and need no spawn.
- Players see the world text "A new Rift has formed. Its invasion begins in two minutes!", the map marker and the status
  bar. Kill all waves and the boss to close the Rift.
- `tests/` contains scenarios for the `mod-fl-testbots` bot runner; they are not needed to use the module.

## License

See the `LICENSE` file (MIT).
