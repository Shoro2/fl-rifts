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

The C++ build, Lua syntax check, local Windows deployment, matching deployed
file hashes, listening world port, and empty new error log are verified. The
compact top status bar is operator-confirmed in game. The marker,
countdown-to-wave transition, live wave status, late-join synchronization,
drag persistence, and clear flow still require an in-game test. See `todo.md`
for the outstanding evidence steps.
