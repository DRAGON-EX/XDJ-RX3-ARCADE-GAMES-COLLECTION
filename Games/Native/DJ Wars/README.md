# DJ Wars — Nightclub Showdown

Original lightweight turn-based artillery game inspired by Worms.
A shared allocation-free C simulation and renderer run in the browser through
WebAssembly and compile for RX3 ARMv7 softfp. Browser preview:
[dj-wars.html](dj-wars.html).

- One 2640 × 1600 destructible nightclub map, with a DJ booth, disco ball, speakers and water. Terrain has separate upper and lower layers, tall ridges, valleys,
  uneven overhangs and an open passage between the layers. Club artwork is
  anchored in world coordinates and moves/scales with the camera.
- Five allies: DJ-DRAGONX the dragon (140 HP), Yuki, Peach, Cookie and Mochi (100 HP).
- Five AI opponents: four fictional mouse-helmet performers (100 HP) and Deck
  Boss (180 HP, 20% extra blast damage). The friendly crew uses the exact Purr House Café cat and dragon artwork.
- Alternating teams and fighters, 90-second action timer, changing wind,
  blast damage/knockback, friendly fire, drowning, a victory celebration or a guest-DJ rescue after defeat/draw, and restart.
- Unlimited bazooka, five grenades, two airstrikes, three homing rockets and two
  controllable Super Dragons per team. Grenades have a three-second fuse;
  Super Dragons detonate on impact, a second fire press, or after seven seconds.
- Mochi and Deadbeat 3 start on the lower level with two teleports and two
  ten-second jetpacks each. Teleport rejects obstructed/occupied destinations.
  A successful teleport ends the turn after a short effect; invalid destinations
  preserve the use and turn. Jetpacks keep the turn timer running. The enemy can
  use mobility to escape the lower layer.
- Red circular aiming reticle. Falls beyond 85 world pixels cause damage at
  0.22 HP per additional pixel; jetpack flight resets accumulated fall distance.
- Weapon crater radii: bazooka/homing 76 px, grenade 92 px, airstrike bomb
  58 px, Super Dragon 114 px. Damage extends another 32 px with distance falloff.
  Expanding fireballs and shock rings use eight fixed effect slots (no allocation).
- Each attack deals at most 50 HP per fighter, including all five airstrike bombs
  and the resulting landing damage. The boss obeys the same cap. Two or more
  attacks are needed to remove 100 HP. Independent falls and drowning retain
  their normal environmental damage rules.
- Forgiving enemy AI: coarse firing estimates, 60–240 px horizontal targeting
  error, imperfect wind estimation and additional angle/power variation.
  Strong specials have a combined 10% selection chance, only from turn 8 onward;
  guided enemy shots use an imprecise fixed point instead of tracking a fighter.
- Hits briefly pause combat and the turn clock for one second, focus the
  automatic camera on a victim and show individual damage numbers for 1.7 seconds.
  Each living fighter has a numeric HP badge; fallen fighters leave a small
  speaker grave that settles onto the terrain (or floats at the water surface).
- Existing bitmap font and red/black or neon palette. Shared native/WASM procedural soundtrack and sound effects.

## Controls

Arrow left/right: walk. Up/down: aim. Enter: jump. Space: hold for shot power,
release to fire. Airstrike and Super Dragon launch with a single press.
1–7 or E: select equipment; T: cycle enemy targets. Click the battlefield to
mark an airstrike/homing destination. A homing rocket follows a selected enemy,
or a manually chosen fixed point. All arrows steer the flying Super Dragon;
press Space again to detonate it. P pauses, R restarts, Esc returns to the launcher.
Drag the battlefield to pan; click the mini-map to reposition the camera.
M toggles the tactical overview, C restores automatic camera following.
Teleport: select 6, click clear space above the desired ground, then press Space.
Jetpack: select 7, press Space and steer with all arrows; Space stops the pack.
On-screen weapon and control buttons support touch and mouse.

## Build and checks

From the repository root:

```sh
make dj-wars test-dj-wars
python3 -m http.server 8766 --bind 127.0.0.1
```

`tests/test_dj_wars.c` checks terrain destruction, damage, team sizes, turn timer,
AI decisions, all five weapons, ammunition, homing/dragon steering, drowning and
end states. It also runs twelve randomized matches and can emit a PPM snapshot
when passed an output filename. Address/undefined-behaviour sanitizers were used.

The browser reserves a fixed **16 MiB** WebAssembly memory with growth disabled.
C simulation state occupies about 4 MiB (compiler alignment may vary);
framebuffer and cached static artwork together occupy 8,192,000 bytes. Browser
canvas and audio overhead are additional.

## Rendering performance

The static viewport cache moves with the camera. Only newly exposed horizontal
and vertical strips are redrawn. Crater, zoom and theme changes still rebuild it;
no additional framebuffer or map allocation is needed. The scrolling-cache test
compares each frame against a full repaint across pan directions, large jumps,
map edges, overview changes, palettes and terrain destruction.

The launcher skips unchanged frames. Device logs identify the active
scene and report game-step, drawing, display-copy and peak frame times separately.
Run `make test-dj-wars-render test-native` for cache and menu invalidation checks.

## RX3 controls

The RX3 launcher includes the game: COLOR FX moves/aims,
BEAT FX ON/OFF charges/fires, BROWSE changes equipment and its push jumps.
Touch selects weapons/targets, pauses and returns to MENU. Game state and the
render cache are released when returning to the launcher. The game uses the
RX3 1280 × 800 display. This project does not contain Worms code or original
game assets.

## Finale and sound

A victory revives fallen friendly fighters and celebrates on the current map.
The twelve-second party shows the defeated mouse boss, a close-up reunion of
the happy café cats and DJ-DRAGONX, and a fireworks finale. If the crew is wiped out (including a double knockout), David
Guetta makes a surprise appearance directly on the battlefield. His chibi
character scolds the player, revives the crew and clears the remaining real
opponents with precise shots. The terrain and fighters' positions carry through
the sequence; drowned fighters return to safe ground. The final result records
an assisted rescue separately from a player victory.

After the fireworks, credits start automatically. At the end of the second song
pass, the Purr House Café opens automatically. There is no intervening menu or
unlock card. Progress is session-only by default; the installer can enable persistent saves. The isolated browser previews
`?demo=guetta`, `?demo=victory` and `?demo=play` start paused and do not award
progress or leave the demo automatically.

DJ Wars uses its own original tactical soundtrack; the shared playlist pauses
at its current position and resumes on return to the menu. CH1 gain controls
sound effects, CH2 gain controls music, and MASTER controls both. The sound
uses bounded synthesis rather than adding another large decoded audio asset.
Native audio uses the measured 44.1 kHz RX3 output clock, despite the driver's
nominal 48 kHz setting. Browser synthesis follows its actual audio-context rate.

Fireworks replace the tactical music with **Game Level Pixel Quest Loop** by
Alex Morgan while retaining effects. The track appears as **SECRET SONG** in
the player until the DJ Wars finale has completed; its unlock is separate from
the credits song. Every café entry restarts and loops **Pixel Plains** by
Junipersona, then restores the previous playlist position and pause state on
exit. Native playback reads the existing packed music with bounded buffers.
