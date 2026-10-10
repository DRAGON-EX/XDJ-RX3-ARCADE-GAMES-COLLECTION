"""Build the open FCEUmm core using locally prepared ARM glibc 2.13 headers.

No commercial ROM, firmware file or key is downloaded or embedded here.
Inputs under ignored work/: fceumm-source.tar.gz, arm-headers, rx3-sysroot.
"""
import hashlib
import pathlib
import subprocess
import tarfile

root = pathlib.Path(__file__).resolve().parents[1]
work = root / 'work'
archive = work / 'fceumm-source.tar.gz'
expected = 'c992d909c375be1187b21666b983bb83eeeef30406821a79fbb5548b77cdb1d9'
if not archive.exists() or hashlib.sha256(archive.read_bytes()).hexdigest() != expected:
    raise SystemExit('Provide the pinned FCEUmm source archive described in Games/EMU/NES/RoadFighter/README.md')
source = work / 'libretro-fceumm-master'
if not source.exists():
    with tarfile.open(archive) as t:
        t.extractall(work, filter='data')
headers = work / 'arm-headers/usr/include'
if not (headers / 'stdio.h').exists():
    raise SystemExit('Prepare ARM glibc 2.13 development headers; see Games/EMU/NES/RoadFighter/README.md')
resource = subprocess.check_output(['clang', '-print-resource-dir'], text=True).strip()
cc = ('clang --target=arm-linux-gnueabi -march=armv7-a -mfpu=neon '
      '-mfloat-abi=softfp -fno-stack-protector -nostdinc '
      f'-isystem {resource}/include -isystem ../arm-headers/usr/include '
      '-isystem ../arm-headers/usr/include/arm-linux-gnueabi')
subprocess.run(['make', '-j8', 'platform=linux-portable', 'STATIC_LINKING=1',
                'HAVE_HDPACK=0', 'HAVE_NTSC=0', f'CC={cc}', 'AR=llvm-ar',
                'CFLAGS=-O2 -fPIC -D__LIBRETRO__ -DPATH_MAX=1024 '
                '-DFCEU_VERSION_NUMERIC=9900 -DFRONTEND_SUPPORTS_RGB565 -D_GNU_SOURCE'],
               cwd=source, check=True)
objects = sorted(source.joinpath('src').rglob('*.o'))
if not objects:
    raise SystemExit('No emulator objects built')
(work / 'nes-objects.rsp').write_text('\n'.join('"'+str(p)+'"' for p in objects))
print(f'FCEUmm ARMv7 softfp: {len(objects)} object files ready')
