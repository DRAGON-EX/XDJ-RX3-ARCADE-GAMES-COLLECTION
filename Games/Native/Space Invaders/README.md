# Drop Defense

Original native alien-wave game using the collection's pixel font, neon sprites
and red/black or neon palette. The field is on the right, score, highscore,
wave and lives on the left. Mochi is rescued when wave 4 is reached, after clearing wave 3. Her rescue
celebration freezes gameplay briefly.

55 enemies in five rows of eleven march, reverse and descend. The formation
accelerates as enemies fall. Each invader takes one hit; scores are 10, 20 or 30
by row. A cleared formation leads to another wave with a lower starting position.
Four bunkers erode under fire from either side and approaching invaders. Only one
player shot can be on screen, with at most three enemy shots. The UFO gives
50/100/150/300 points. Three lives, one bonus life at 1,500 points, and defeat if
the formation reaches the player line. The UFO score is randomized in this
implementation; this is not a cycle-exact recreation of the original ROM.

The reference for formation, shield erosion, score values and initial lives is
the original [Taito service manual](https://manualzz.com/doc/10387354/taito-america-space-invaders-television-user-manual).
No original code, ROM, sprites or audio samples are included.

## Controls and sound

- NOISE / FILTER: move left / right.
- Crossfader: horizontal position, alongside the buttons and touch controls.
- BEAT FX ON/OFF (A): hold to fire as soon as the previous shot clears.
- MASTER LEVEL: procedural effects, four-step marching pulse and UFO tone.
- Touch: move/fire on the field, PAUSE, RESET and MENU on the left.
- Browser: Left/Right or A/D, Space/Enter, mouse/touch, P, R and Escape.
  SOUND enables browser audio.

The same bounded C simulation and renderer run natively and as WebAssembly.
The game state uses 2,088 bytes; it shares the existing native framebuffer.
60 Hz simulation, 30 FPS display target. Scores, highscores and rescue progress
are session-only by default, surviving game resets and menu navigation.
The installer can enable persistent saves across application restarts.

Build: `make space-invaders` and `sh tools/build_native.sh`.
Checks: `make test-space-invaders test-native`; core tests also run under address
and undefined-behavior sanitizers. Native ARM execution/rendering is checked in
QEMU.

Preview: `Games/Native/Space Invaders/space-invaders.html`.
Native flag: `--space-invaders`. Source files use the internal Space Invaders folder name.
