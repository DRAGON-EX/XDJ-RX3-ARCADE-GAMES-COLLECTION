# NES

- `nes.c` / `nes.h`: our libretro frontend for the locally built FCEUmm core,
  including video conversion, audio buffers and timing.
- [RoadFighter](RoadFighter/README.md): RX3 crossfader/button mapping and local
  build instructions for Road Fighter. The frontend currently validates this
  game's mapper-0 ROM layout and selects PAL timing; other NES games need their
  own compatibility checks before being added.

The emulator source archive and ARM objects remain under ignored `work/`.
Run `python3 tools/build_nes.py` and `make native` from the repository root.
FCEUmm's notice is preserved in [LICENSE.fceumm](../../../vendor/LICENSE.fceumm).
No NES ROM is included.
