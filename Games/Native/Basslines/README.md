# Basslines

An original light-cycle arcade game for the RX3. Steer your neon bike, leave a
permanent trail, and outlast PULSE, VOLT and ECHO. The last surviving rider scores
one point. Win all eight arenas to rescue Cookie. An opponent reaching eight
wins first ends the match. Simultaneous crashes can draw.
The separate arcade score awards 10 points per survived second, 250 per round win
and 1,000 for a match win. Scores and highscores are session-only by default;
the installer can enable persistent saves across application restarts.

Rounds are capped at 90 seconds. Each player win advances to the next arena;
a loss or draw repeats the current arena.

## Controls

- DUB ECHO: up. NOISE: left. FILTER: right. CRUSH: down.
- BEAT FX ON/OFF (A): hold for boost; press after a result for the next round.
- Boost doubles movement speed for up to two seconds. It recharges while unused;
  after exhaustion release A before boosting again.
- Touch: direction pads, BOOST, PAUSE, RESET, MENU.
- Browser: arrow keys / WASD, Space or Enter for boost / next round.
  P pauses, R resets the match, Escape returns to the launcher. SOUND enables audio.
- MASTER LEVEL controls procedural game sound on the RX3.

No U-turns. Walls and all trails remain lethal, even after a rider crashes.
Destinations resolve simultaneously, so head-on collisions have no update-order
advantage. Three computer riders evaluate nearby free space with bounded work.

## Implementation and checks

One shared C simulation/renderer serves native ARMv7 and WebAssembly. Uses the
existing pixel font, red/black or neon theme, right-hand playfield and left-hand
scoreboard. No ROM, external engine or licensed game assets are required.
The game state occupies 15,432 bytes; AI scratch space stays below 8 KiB and is
reused on the stack. Framebuffer memory is shared with the native launcher.
60 Hz simulation, 30 FPS rendering target. Actual RX3 performance/control testing
remains necessary; QEMU is a compatibility check, not a hardware benchmark.

Build: `make basslines` and `sh tools/build_native.sh`.
Tests: `make test-basslines test-native` (collisions, simultaneous ties, turns,
boost exhaustion/recovery, match progression, eight layouts and long simulations).
Browser: `Games/Native/Basslines/basslines.html`.
Native: `rx3-arcade --basslines`.

Rescue mission: Cookie. Eight arena victories trigger her five-second rescue
celebration. Rescue progress follows the selected session-only or persistent save mode.
