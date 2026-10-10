#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
python3 tools/build_rescue_audio.py
python3 tools/build_cafe_show_audio.py
python3 tools/build_cafe_show.py
test -f work/rx3-sysroot/lib/libc.so.6 || python3 tools/prepare_sysroot.py
test -f work/nes-objects.rsp || python3 tools/build_nes.py
mkdir -p dist/RX3_ARCADE
clang --target=arm-linux-gnueabi -march=armv7-a -mfpu=neon -mfloat-abi=softfp -marm -O2 -Wall -Wextra -ffreestanding -fno-builtin -fno-stack-protector -fuse-ld=lld -nostdlib -no-pie -Wl,--hash-style=sysv -Wl,--dynamic-linker=/lib/ld-linux.so.3 native/start.S native/arcade.c Games/EMU/NES/nes.c Games/Native/Pacman/pacman_core.c @work/nes-objects.rsp work/rx3-sysroot/lib/libc.so.6 work/rx3-sysroot/usr/lib/libasound.so.2 work/rx3-sysroot/lib/libgcc_s.so.1 work/rx3-sysroot/lib/librt.so.1 work/rx3-sysroot/lib/libm.so.6 work/rx3-sysroot/lib/libpthread.so.0 -o dist/RX3_ARCADE/rx3-arcade
python3 tools/check_native.py
cp native/start.sh dist/RX3_ARCADE/start.sh
cp native/ssh-start.sh native/ssh-stop.sh dist/RX3_ARCADE/
cp native/stop-dj.sh dist/RX3_ARCADE/stop-dj.sh
cp vendor/LICENSE.fceumm dist/RX3_ARCADE/LICENSE.fceumm
cp vendor/LICENSE.pacman dist/RX3_ARCADE/LICENSE.pacman
clang --target=arm-linux-gnueabi -march=armv7-a -mfpu=neon -mfloat-abi=softfp -marm -O2 -Wall -Wextra -ffreestanding -fno-builtin -fno-stack-protector -fuse-ld=lld -nostdlib -no-pie -Wl,--hash-style=sysv -Wl,--dynamic-linker=/lib/ld-linux.so.3 native/start.S native/launcher.c work/rx3-sysroot/lib/libc.so.6 work/rx3-sysroot/usr/lib/libasound.so.2 work/rx3-sysroot/lib/libgcc_s.so.1 work/rx3-sysroot/lib/librt.so.1 -o dist/RX3_ARCADE/rx3-launcher
python3 tools/check_native.py dist/RX3_ARCADE/rx3-launcher

python3 - <<'PY'
from pathlib import Path
size=Path("dist/RX3_ARCADE/rx3-launcher").stat().st_size
assert size < 192 * 1024, "Launcher exceeds the stock startup memory budget; move large assets to payload"
print(f"Launcher startup budget verified: {size} bytes")
PY
