# Equalizarr

Native Breakout-style game for the RX3, with a shared C/WebAssembly browser preview.
The 50-level campaign has 50 individually authored silhouettes on a
16 × 14 grid: towers, staircases, tunnels, gates, islands and layered fortresses.
Armored bricks, chain-reaction explosive bricks and indestructible steel barriers
change the routes through each stage. A score/combo system and a final victory
screen round out the campaign. All breakable blocks count toward completion;
steel does not. Ball speed rises through the campaign.

Power-ups: three-hit shield, twin lasers, up to six balls, wide paddle, sticky
paddle, slow motion, fireball and extra life. Bonuses drop every five destroyed
bricks. Timed powers expire; losing the last ball costs one life and clears powers.
Scores and highscores are session-only by default. Restarting a game preserves
the highscore; the installer can enable persistent saves across application restarts.
A session starts with four lives. Level completion preserves remaining lives and score.

## Controls

- RX3 NOISE / FILTER: move left / right.
- Crossfader: horizontal position, alongside the buttons and touch controls.
- RX3 BEAT FX ON/OFF (A): launch the ball; hold to fire collected lasers.
- Touch the playfield to move the paddle and launch; PAUSE, RESET and MENU on the left.
- MASTER LEVEL controls procedural game sounds.
- Browser: mouse/touch or Left/Right / A/D, Space or Enter to launch/fire.
  P pauses, R restarts, Escape returns to the menu. SOUND enables browser audio.
- Browser PRACTICE selects any of the 50 levels; normal play advances sequentially.

The field is on the right, score/lives/powers on the left. Uses the launcher's
pixel font and red/black or neon theme. No ROM or external graphics/audio assets.
The bounded game state is 3,108 bytes; framebuffer memory is shared with the launcher.
60 Hz simulation and a 30 FPS rendering target; RX3 performance still needs a device test.

Build with `make equalizarr` and `sh tools/build_native.sh`.
Test with `make test-equalizarr test-native`. The level audit checks all 50
occupancy silhouettes are distinct and every breakable block becomes reachable
by a ball after exposed layers clear, without removing steel.
Browser: `Games/Native/Equalizarr/equalizarr.html`.
Native direct start: `rx3-arcade --equalizarr`.

## Origin

The [raylib Arkanoid sample](https://github.com/raysan5/raylib-games/blob/master/classics/src/arkanoid.c)
by Marc Palau and Ramon Santamaria was used as the small C starting reference.
This substantially altered implementation replaces the window/render/input layer
and adds the campaign, powers, bounded pools, collision substeps and original sounds.
The original source is retained in `upstream/arkanoid.c`; the zlib/libpng license
is retained in `LICENSE.raylib-games`. Raylib itself is not a runtime dependency.

Rescue mission: Yuki. Completing ten levels in one run triggers her rescue celebration and unlocks her music-player fallback artwork.
