# Local NES / Road Fighter build

Road Fighter is provided by the user. No ROM is distributed by this project.
The current integration supports the mapper-0 PAL NES edition. It uses FCEUmm
and the original game graphics and sound. Crossfader steering is translated to
NES left/right buttons, rather than modifying the ROM or setting the car's
absolute position. The middle is neutral; held FX direction buttons take priority.
The crossfader activates after movement to avoid immediate steering on entry.

Controls: DUB ECHO up, NOISE left, FILTER right, CRUSH down; BEAT FX ON/OFF A;
mixer headphone CH1 CUE Select, CH2 CUE Start; BROWSE press or touchscreen START sends NES Start. Touch MENU
to unload the game and return. MASTER LEVEL controls sound. Existing USB STOP
and removal cleanup still apply. Actual performance and analog direction require
an RX3 hardware test; ARM emulation on the development computer is not an FPS
benchmark for the RX3.

Build inputs (all local, under ignored `work/`):

- FCEUmm source archive from https://github.com/libretro/libretro-fceumm
  (`master` snapshot downloaded 2026-10-04), SHA-256
  `c992d909c375be1187b21666b983bb83eeeef30406821a79fbb5548b77cdb1d9`.
  Save as `work/fceumm-source.tar.gz`. Archive hashes are checked before build.
- ARM glibc 2.13 headers, extracted from Debian's
  `libc6-dev_2.13-38+deb7u10_armel.deb` into `work/arm-headers`.
  https://archive.debian.org/debian/pool/main/e/eglibc/
  Package SHA-256 `52b2b47e331577f1751e21413f4021224458747ba667500196ba35445066e090`.
  Linux UAPI headers are also needed (`linux/`, `asm-generic/`, and the ARM
  `asm/errno.h`/`asm/errno-base.h` definitions).
- Locally obtained RX3 libraries in `work/rx3-sysroot`, not published.

Run `python3 tools/build_nes.py`, then `make native`. The core is linked into the
application because loading the standalone shared core failed with the old RX3
runtime. HD texture packs and NTSC postprocessing are disabled. The PAL game
runs at its reported 50.007 Hz; audio is 48 kHz stereo. Rendering targets 30 Hz independently of emulation, using the changed-span renderer.
With active NES audio, ALSA provides pacing instead of an additional sleep.
Only displayed NES frames are converted, repeated scaled rows are copied, and
unchanged framebuffer rows are skipped. The ALSA latency target is 160 ms.

For a private USB package, provide `work/road-fighter.nes` locally. Packaging
copies it into the private encrypted image; the loader copies it to RAM and
removes that copy when the USB session ends. Published code contains no ROM.
Do not publish package binaries or screenshots/sound captures from the ROM.

FCEUmm is GPL-licensed: retain `vendor/LICENSE.fceumm` and provide the matching
source and applicable notices if distributing a build. The source archive above
contains upstream's files and notices. The upstream libretro interface header carries its own license.

Runtime payload storage: the stock temporary filesystem is configured as only
512 KB. The bootstrap mounts a dedicated 32 MiB tmpfs under
`/tmp/rx3-launcher/payload` for the game executable and local ROM. Only the small
listener/scripts stay in the stock area. The game runs directly from its payload
mount to avoid a second large copy. Session cleanup deletes the payload, unmounts
it, and removes its mountpoint after the game has stopped.

The libretro interface header is read from the locally extracted FCEUmm tree
under ignored `work/`; it is not shipped in this repository.
