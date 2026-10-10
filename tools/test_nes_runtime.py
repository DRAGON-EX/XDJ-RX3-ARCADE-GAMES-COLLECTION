"""Run the local user ROM through the actual RX3 ARM build environment."""
import pathlib
import subprocess
rom = pathlib.Path('work/road-fighter.nes')
if not rom.exists():
    raise SystemExit('Provide your own local ROM at work/road-fighter.nes')
subprocess.run(['clang', '--target=arm-linux-gnueabi', '-march=armv7-a',
                '-mfpu=neon', '-mfloat-abi=softfp', '-O2', '-fno-stack-protector',
                '-fuse-ld=lld', '-nostdlib', '-no-pie', '-Wl,--hash-style=sysv',
                '-Wl,--dynamic-linker=/lib/ld-linux.so.3', 'native/start.S',
                'Games/EMU/NES/nes.c', 'tests/test_nes_runtime.c', '@work/nes-objects.rsp',
                'work/rx3-sysroot/lib/libc.so.6', 'work/rx3-sysroot/lib/libm.so.6',
                'work/rx3-sysroot/lib/libgcc_s.so.1', '-o', 'work/test-nes-runtime'], check=True)
subprocess.run(['qemu-arm', '-L', 'work/rx3-sysroot', 'work/test-nes-runtime', str(rom)], check=True)
