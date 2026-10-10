# Purr House Café record light show

Click the café record to play the supplied `good.mp3` with an authored café
and RX3 panel show. The shared cue frame uses the song's playback position,
its detected 128 BPM grid, and precomputed energy, bass, treble and transient
envelopes. No FFT or file analysis runs on the RX3 during playback.

The panel arrangement changes with the song: warm amber intro, cyan/pink
deck-to-deck groove, rising amber build, strong alternating pad banks for the
drop, low velvet colours for the break, then gold expanding fans for the
finale. Bass/transients accent the button groups; treble drives the mode
buttons. The right pad bank mirrors the left so chases travel across the
whole panel. Brightness stays below the firmware converter's output range.

## Playback and builds

The shelf record starts the set; tapping it again stops it. Leaving the café
also stops it. At the end, the usual Pixel Plains café soundtrack resumes.
The current music/master gains apply to the set; café meows stay quiet during
the song. The record interaction does not grant unlocks or save show state.

`assets/music/cafe-good.mp3` is the user-supplied recording, used only for this
record interaction. It is not added to the general track-player rotation.
`tools/build_cafe_show.py` extracts 50 ms energy/bass/treble/transient cues and
bakes the 128 BPM grid and phrase arrangement into `shared/cafe_show.h`.
`content/cafe-show.json` records the source hash, timing and section list.
There are 14,028 bytes of envelopes; no realtime audio analysis is required.

`tools/build_cafe_show_audio.py` produces a 7,728,576-byte, 22050 Hz S16 mono
payload. The audio worker reads a fixed 1 KB buffer and repeats samples for
the RX3's measured 44100 Hz output clock. Display and LEDs follow frames
actually heard, accounting for ALSA queue delay. The packaging/bootstrap and
USB cleanup paths include `cafe-show.pcm`; the existing 96 MiB RAM mount is
unchanged, and packaging rejects payloads over 95 MiB.

Run `make cat-scenes cafe-show test-cafe-show`, then `sh tools/build_native.sh`.
The standalone `cat-cafe-preview.html?scene=cafe&won=1` also shows the actual
programmed RX3 packet colours below the café. This panel is a desktop preview,
not a connection to physical hardware.

## Verified output coverage

The implementation in `native/cafe_show_leds.h` controls 59 named LED zones:

| Transfer | Byte offsets | Supported lights |
| --- | --- | --- |
| SUB | `deck * 48 + mode * 4`, modes 0–3 | Hot Cue, Beat Loop, Slip Loop and Beat Jump RGB mode buttons, both decks |
| SUB | `deck * 48 + 16 + pad * 4`, pads 0–7 | Eight RGB performance pads on each deck |
| SUB | 99 left / 98 right, bits 7, 6, 5, 4 | Loop In, Loop Out, Cue, Play/Pause |
| EUP | 1 left / 5 right, bits 7, 6, 5, 2 | Slip, Sync, Master, Quantize |
| EUP | 2 left / 6 right, bit 3 | Vinyl |
| EUP | 3 left / 7 right, bits 4, 3, 0 | Reloop, Master Tempo, Reverse |
| EUP | 19, bits 0–5 | Filter, Sweep, Dub Echo, Noise, Crush, Space |
| EUP | 19, bits 6, 7 | Beat FX On/Off, Tap |
| EUP | 10, bits 5, 4, 3 | Headphone Cue CH1, CH2, Master |

RGB records are `[enable, blue, green, red]`. This show caps channels at
60/28/26 respectively; protocol-derived converter bounds are 90/30/30.
EUP is the existing 100-byte driver transfer with 24 meaningful bytes and
CRC-X25 across five words; SUB is 104 bytes with CRC across 25 words. Bytes
are reversed within each word for CRC calculation, as in the existing
tutorial output. No device modes, key events or GPIO commands are sent.

The mapping was checked against the user's local firmware analysis files:

- `work/led-deck.txt`: `EupMiconTx::formatDeck`, start `0x2dd9d0`.
- `work/led-common.txt`: `EupMiconTx::formatCommon`, start `0x2dea50`.
- `work/led-sub-deck.txt`: `SubMiconTx::formatDeck` at `0x2dfc50`,
  colour byte order at `0x2dfec8`, enable bit at `0x2e0308`.
- `work/led-sub-common.txt`: `SubMiconTx::formatCommon`, `0x2de310`.
- `work/led-names.txt` / local `work/rbp`: `LedDefID2Text` at `0x373760`,
  name pointers at `0x4ddb98 + 0x4b8`, correlate the format's LedDef IDs with
  controls. ID 14–17 are modes, 18–25 pads, 41–46 COLOR FX, 47 Tap,
  48 Beat FX, 50 Headphone Cue and 51 Master Cue.

The source implementation is independent. Firmware dumps/disassembly are
local analysis evidence, not copied into the distributed asset package.
Unknown bits remain at the launcher's existing baseline. Jog LCD/ring
controllers, USB status indicators and level meters are not addressed by
this show.

## Native integration

Include the header and keep one zero-initialized `struct cafe_show_led_state`.
Every main-loop pass call:

```c
unsigned char eup[100],sub[104];
unsigned changed=cafe_show_led_update(&state,show_active,audible_ms,ns(),eup,sub);
if(changed&CAFE_SHOW_LED_EUP)led_send(panel_fd,eup,100);
if(changed&CAFE_SHOW_LED_SUB)led_send(cue_led_fd,sub,104);
```

Use the existing `/dev/subucom_spi1.0` (EUP) and `/dev/subucom_spi2.0` LED driver
paths / file descriptors, opening SUB once if necessary. This service must
run independently of successful panel reads; otherwise a missing input
packet would freeze output. The tutorial writer must yield while the show
owns the LEDs. On stop, end, café exit or shutdown call with `show_active=0`
before closing the devices. This immediately emits both normal launcher
packets exactly once, including the dim RGB baseline, all COLOR FX and
normal deck cue lights. It does not claim to recover the DJ application's
earlier colours, because that process is suspended and its prior LED state
is not readable through these controls.

During playback, each controller receives at most one changed packet per
50 ms, at most 20 Hz. Frozen audio generates no repeated writes. Stop and
entry are deliberate immediate exceptions. There are no allocations,
threads, random-state changes or blocking sleeps in the LED helper.

`tests/test_cafe_show_leds.c` exercises every 50 ms frame of the full song,
independently checks CRCs and reserved-byte masks, verifies that all mapped
groups animate, checks RGB bounds, six distinct section patterns, timing,
frozen audio, repeated stop, replay and exact restoration.
