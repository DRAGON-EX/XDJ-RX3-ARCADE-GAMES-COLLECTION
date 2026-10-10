# Neon Maze

- `pacman_core.c`: gameplay and sound synthesis adapted from floooh/pacman.c.
- `game.js` and `style.css`: browser game presentation, opened through the root `index.html`.
- RX3 rendering and launcher integration currently remain in `../../../native/arcade.c`.

The upstream MIT notice is retained in [LICENSE.pacman](../../../vendor/LICENSE.pacman).
Locally generated `pacman_sound_data.h` and `pacman.wasm` are excluded from Git.
No ROM or original sound data is distributed in this directory.

Run `make`, `make native`, or `make test` from the repository root after preparing
local dependencies as described in the main README. `tools/extract_core.py` and
`tools/import_sound.py` write their generated inputs into this directory.
