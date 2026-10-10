# Bass Rocks — RX3 native edition

Playable native neon arcade game, with the playfield on the right and score,
best score, wave and lives on the left. Shares the RX3 palette and pixel font.

- Endless waves: 4, 6, 8, then 10 large asteroids; each splits into two medium,
  then two small rocks. Points: 20 / 50 / 100.
- Large and small UFOs (200 / 1000 points), hyperspace with re-entry risk,
  three lives and an extra life every 10,000 points.
- 60 Hz simulation, 30 FPS display target.
- Fixed object pools; game state is 7,052 bytes. No ROMs or original sound assets.
  Sound effects are procedural, with master volume and native 48 kHz output.

## Controls

| RX3 | Action |
| --- | --- |
| DUB ECHO | Thrust |
| NOISE / FILTER | Rotate left / right |
| Left jog wheel | Turn the ship with the wheel |
| BEAT FX ON/OFF | Fire |
| CRUSH / right headphone CUE | Hyperspace |
| Touch MENU | Return to launcher |
| MASTER LEVEL | Volume |

Browser: arrows / WASD, Space to fire, Down or B for hyperspace, P to pause,
R to restart, Escape for menu. Enable SOUND explicitly. Browser touch controls
are below the canvas.

Build with `make asteroids native`; verify with `make test-asteroids`.
Open `Games/Native/Asteroids/asteroids.html` through the local preview server.

## Sources and license

Adapted and extensively extended from the [raylib Asteroids example](https://github.com/raysan5/raylib-games/blob/master/classics/src/asteroids.c),
by Ian Eito, Albert Martos and Ramon Santamaria. The original reference source
is retained under `upstream/`; its zlib/libpng license is in `LICENSE.raylib-games`.
This is an altered implementation, not the unmodified original sample.

Wave and scoring rules follow the Atari Asteroids TM-143 operator manual.
This is a clone with modern visuals and original procedural audio, not a
cycle-accurate emulation of the Atari hardware.

Shown as Bass Rocks in the collection. Peach is rescued when wave 4 is reached
(after clearing wave 3). The celebration briefly freezes gameplay. Scores,
highscores and rescue progress are session-only by default; the installer can
enable persistent saves.
