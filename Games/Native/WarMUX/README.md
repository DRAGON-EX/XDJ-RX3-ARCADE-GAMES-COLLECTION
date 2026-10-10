# WarMUX — experimental RX3 ARM build

The original WarMUX game builds for ARMv7 softfp and starts a real classic
match in QEMU using the RX3's glibc 2.13 and libstdc++ runtime. This is a port
experiment, not a replacement game or a browser reimplementation.

Current changes:

- Handle SDL's “all resolutions” sentinel without dereferencing it.
- Load effects on first use and bound retained decoded effects to 8 MiB.
- Avoid the X11 clipboard headers when X11 clipboard support is disabled.
- Disable update checks/curl integration in this build.
- Test at 1280 × 800, target 30 FPS, and a 48 kHz mixer.

The full original teams, weapons and maps remain available. The smoke test uses
Arcade Room and two AI-controlled teams. QEMU's SDL dummy backend captures the
real ARM game screen; it does not prove physical RX3 display/audio operation.

## Build and verify

From the repository root:

```sh
python3 tools/prepare_warmux.py
python3 tools/build_warmux.py
python3 tools/test_warmux.py
```

Requirements: Python 3.12+, clang/clang++, lld, patch, dpkg-deb, qemu-arm, ffmpeg,
and an existing local RX3 sysroot at `work/rx3-sysroot`. No sysroot or firmware is
included. The probe requires a local DejaVu Sans font; set `WARMUX_PROBE_FONT`
when the default `/usr/share/fonts/TTF/DejaVuSans.ttf` does not exist.

Downloads are pinned with SHA-256 in `upstream.json` and
`debian-armel-packages.json`; they are extracted locally, not installed on the
host or the RX3. Build outputs, third-party sources and data stay in ignored
`work/warmux/`. No ROM is needed.

Probe evidence is saved under `work/warmux/`: `probe.log`, `probe-result.json`
and `warmux-game.png`. Allocation totals exclude code, shared-library mappings
and other system memory. QEMU timings must not be advertised as RX3 FPS.

## Remaining device work

The current launcher/USB package is unchanged. RX3 framebuffer output,
controller/touch mapping, MASTER LEVEL volume, returning to the launcher and
USB-removal cleanup still need integration and tests on the device. The
capture helper is exclusively for QEMU tests and must not ship as the device
adapter. Do not enable a launcher entry until these are ready.

## Upstream and licensing

Source: [a-team/wormux](https://github.com/a-team/wormux), pinned commit
`e0dc503460055a14a0d7fd94e66d7484264ee62b`. Our modifications are in
`patches/rx3-low-memory.patch`. WarMUX is GPL-2.0-or-later; retain the original
notices, the applicable fixedpoint library notice and corresponding sources
when redistributing a built port. `COPYING` is retained here.

The free Debian Wheezy ARM packages supply development headers and SDL
libraries compatible with the old device runtime. Package-specific notices
are under the extracted `usr/share/doc/` directories. Check each game's data
and dependency notices before packaging or publishing them. We have not
uploaded these upstream binaries/data or any RX3 firmware/credentials.
