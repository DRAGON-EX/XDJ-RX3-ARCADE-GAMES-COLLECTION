"""Execute the actual ARM build with RX3 glibc; headless SDL is only a capture backend."""
from pathlib import Path
import json,os,re,shutil,subprocess
ROOT=Path(__file__).resolve().parents[1];BASE=ROOT/'work/warmux';RT=BASE/'runtime';META=ROOT/'Games/Native/WarMUX';STOCK=ROOT/'work/rx3-sysroot'
resource=subprocess.check_output(['clang','-print-resource-dir'],text=True).strip()
subprocess.run(['clang','--target=arm-linux-gnueabi','-march=armv7-a','-mfpu=neon','-mfloat-abi=softfp','-shared','-fPIC','-O2','-fno-stack-protector','-fuse-ld=lld','-nostdlib','-D_GNU_SOURCE','-nostdinc','-isystem',resource+'/include','-isystem',str(RT/'usr/include'),'-isystem',str(RT/'usr/include/arm-linux-gnueabi'),'-I'+str(RT/'usr/include/SDL'),str(META/'qemu_capture.c'),'-o',str(BASE/'capture.so')],check=True)
config=BASE/'probe-config/wormux';config.mkdir(parents=True,exist_ok=True)
shutil.copyfile(META/'probe-config.xml',config/'config.xml')
(BASE/'probe-data').mkdir(exist_ok=True)
# LD_PRELOAD splits on spaces: relative path is essential in this workspace.
env={'LC_ALL':'C','LANG':'C','LD_LIBRARY_PATH':':'.join(map(str,[RT/'lib/arm-linux-gnueabi',RT/'usr/lib/arm-linux-gnueabi',RT/'usr/lib/arm-linux-gnueabi/pulseaudio',RT/'usr/lib',STOCK/'lib'])),
 'LD_PRELOAD':'work/warmux/capture.so','RX3_WARMUX_SNAPSHOT':str(BASE/'game.ppm'),'RX3_WARMUX_FRAME':'360','WARMUX_DATADIR':str(RT/'usr/share/games/warmux')+'/',
 'WARMUX_FONT_PATH':os.environ.get('WARMUX_PROBE_FONT','/usr/share/fonts/TTF/DejaVuSans.ttf'),'XDG_CONFIG_HOME':str(BASE/'probe-config'),'XDG_DATA_HOME':str(BASE/'probe-data'),'SDL_VIDEODRIVER':'dummy','SDL_AUDIODRIVER':'dummy'}
if not Path(env['WARMUX_FONT_PATH']).is_file():raise RuntimeError('Set WARMUX_PROBE_FONT to a local DejaVuSans.ttf')
args=['qemu-arm','-L',str(STOCK)]
for k,v in env.items():args+=['-E',k+'='+v]
args+=[str(BASE/'warmux-rx3'),'--size','1280x800','--play','--unrandom']
run=subprocess.run(args,cwd=ROOT,capture_output=True,text=True,timeout=60)
log=run.stdout+run.stderr;(BASE/'probe.log').write_text(log)
if run.returncode or '[ Starting a new game ]' not in log or 'frame=360' not in log or 'Opened audio at 44100' not in log:raise RuntimeError('WarMUX ARM smoke test failed; see work/warmux/probe.log')
m=re.search(r'heap used=(\d+) free=(\d+) mmap=(\d+) frames=(\d+) elapsed_ms=(\d+)',log)
if not m:raise RuntimeError('Target allocation report missing')
values=list(map(int,m.groups()));report={'heap_used_bytes':values[0],'heap_free_bytes':values[1],'mmap_allocated_bytes':values[2],'frames':values[3],'qemu_elapsed_ms':values[4],'notes':'Target allocator values, not total RX3 RAM; SDL audio/video dummy. QEMU timing is not device FPS.'}
(BASE/'probe-result.json').write_text(json.dumps(report,indent=2)+'\n')
subprocess.run(['ffmpeg','-v','error','-y','-i',str(BASE/'game.ppm'),str(BASE/'warmux-game.png')],check=True)
print('WarMUX ARM: real classic match, 1280x800, 44.1 kHz mixer, 360 frames passed; target allocations',round((values[0]+values[2])/2**20,1),'MiB')
