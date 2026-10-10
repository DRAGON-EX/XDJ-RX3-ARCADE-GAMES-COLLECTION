"""Open the ARM WarMUX build in a local SDL window through QEMU user emulation."""
from pathlib import Path
import os
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
BASE = ROOT / 'work/warmux'
RT = BASE / 'runtime'
STOCK = ROOT / 'work/rx3-sysroot'
config = BASE / 'window-config/wormux'
config.mkdir(parents=True, exist_ok=True)
if not (config / 'config.xml').exists():
    shutil.copyfile(BASE / 'config/wormux/config.xml', config / 'config.xml')
(BASE / 'window-data').mkdir(exist_ok=True)
env = {
    'LC_ALL': 'C', 'LANG': 'C',
    'LD_LIBRARY_PATH': ':'.join(map(str, [RT / 'lib/arm-linux-gnueabi',
        RT / 'usr/lib/arm-linux-gnueabi', RT / 'usr/lib/arm-linux-gnueabi/pulseaudio',
        RT / 'usr/lib', STOCK / 'lib'])),
    'WARMUX_DATADIR': str(RT / 'usr/share/games/warmux') + '/',
    'WARMUX_FONT_PATH': os.environ.get('WARMUX_PROBE_FONT', '/usr/share/fonts/TTF/DejaVuSans.ttf'),
    'XDG_CONFIG_HOME': str(BASE / 'window-config'),
    'XDG_DATA_HOME': str(BASE / 'window-data'),
    'SDL_VIDEODRIVER': 'x11', 'SDL_AUDIODRIVER': 'pulse',
    'PULSE_SERVER': os.environ.get('PULSE_SERVER',
        'unix:' + os.environ.get('XDG_RUNTIME_DIR', '/run/user/' + str(os.getuid())) + '/pulse/native'),
}
args = ['qemu-arm', '-L', str(STOCK), '-U', 'LD_PRELOAD']
for key, value in env.items():
    args += ['-E', key + '=' + value]
args += [str(BASE / 'warmux-rx3'), '--size', '1280x800', '--play', '--unrandom']
with (BASE / 'window.log').open('w') as log:
    process = subprocess.Popen(args, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
    (BASE / 'window.pid').write_text(str(process.pid) + '\n')
    print('WarMUX ARM QEMU PID', process.pid, flush=True)
    raise SystemExit(process.wait())
