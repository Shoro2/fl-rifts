# Forgotten Land Rifts

`fl-rifts` runs a repeating world Rift event. A randomly selected Rift spawn
location creates a visible Rift, waits two minutes, then starts two trash waves
and a boss wave. The next Rift becomes eligible 30 minutes after the event is
cleared.

## Player experience

- Players in the Rift's zone receive a small native map point of interest.
- Players who enter the zone while the Rift is present receive the same marker.
- The marker is cleared when the player leaves or the Rift is completed.
- The first wave cannot spawn until the two-minute pre-start phase expires.
- AIO renders a compact top-center status bar. Hold the right mouse button to
  move it; AIO saves the position.
- During the pre-start phase the bar shows the countdown. Once the event is
  active it shows the current trash/boss wave and remaining enemies.
- Native area-trigger announcements at 2:00, 1:00, 0:30, 0:10, and the final
  five seconds remain as a fallback.
- Air and Water Rifts render as their configured portal GameObjects while an
  invisible creature controller continues to own the event and its lifecycle.

## Runtime dependencies

- AzerothCore built with this module enabled.
- `mod-ale` plus the AIO server/client framework for the countdown bar.
- The Rift creature templates and spawn locations from the Forgotten Land
  content migration.

When `mod-ale` is present, `fl-rifts.cmake` copies `lua/` into
`lua_scripts/FLRifts` during the module build/install. After changing the AIO
client file, run `/aio reset` on the test client before verification.

The current Windows test server installs the directory directly below the
install prefix. The future Linux host follows the AzerothCore layout below
`<install-prefix>/bin/lua_scripts/FLRifts`.

## Configuration

Copy `conf/fl-rifts.conf.dist` to the runtime module config directory, enable
`FLRifts.Enable`, and adjust the configured wave/boss creature entries only if
the migrated content uses different IDs.

## Verification state

The prior element/UI revision was built and deployed on the Windows operator
box, and the compact top status bar is operator-confirmed in game. The current
element-combat and Rift-visual C++ remain T0 until they are built. The
Shadow-equivalent creature tuning is T1 database-verified in isolated live-data
and stock-style schemas, but `data/sql/db-world/base/fl_rifts_elements.sql` has
not been applied to the live DB. Rift lifecycle checks plus the Air/Water
GameObject appearance, tuned names, stats, scales, mana roles, hazards and boss
mechanics still require an in-game pass. See `todo.md` for the exact outstanding
evidence.
