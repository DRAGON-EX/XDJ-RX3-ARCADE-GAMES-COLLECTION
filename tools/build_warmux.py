from pathlib import Path
import subprocess,re,concurrent.futures,time
root=Path(__file__).resolve().parents[1];base=root/'work/warmux';src=base/'source';rt=base/'runtime';out=base/'objects';out.mkdir(exist_ok=True)
config=src/'lib/warmux/include/config_autotools.h';s=(src/'lib/warmux/include/config_autotools.h.in').read_text()
yes={'HAVE_DIRENT_H','HAVE_INTTYPES_H','HAVE_LIBSDL_GFX','HAVE_LIBSDL_IMAGE','HAVE_LIBSDL_MIXER','HAVE_LIBSDL_NET','HAVE_LIBSDL_TTF','HAVE_MEMORY_H','HAVE_STDINT_H','HAVE_STDLIB_H','HAVE_STRINGS_H','HAVE_STRING_H','HAVE_SYS_STAT_H','HAVE_SYS_TYPES_H','HAVE_UNISTD_H','STDC_HEADERS'}
for name in yes:s=re.sub(r'(?m)^#undef '+name+r'$','#define '+name+' 1',s)
for name,val in {'PACKAGE':'warmux','PACKAGE_NAME':'WarMUX','PACKAGE_VERSION':'11.04','PACKAGE_STRING':'WarMUX 11.04 RX3','VERSION':'11.04'}.items():s=re.sub(r'(?m)^#undef '+name+r'$','#define '+name+' "'+val+'"',s)
config.write_text(s)
flags=['clang++','--target=arm-linux-gnueabi','-march=armv7-a','-mfpu=neon','-mfloat-abi=softfp','-marm','-O2','-std=gnu++98','-fno-stack-protector','-fno-pie','-DHAVE_CONFIG_H','-D_GNU_SOURCE','-D_FILE_OFFSET_BITS=64','-DINSTALL_DATADIR="/tmp/RX3_ARCADE/warmux/data/"','-DINSTALL_LOCALEDIR="/tmp/RX3_ARCADE/warmux/locale/"','-DFONT_FILE="DejaVuSans.ttf"','-DPACKAGE_VERSION="11.04"','-nostdinc','-isystem',subprocess.check_output(['clang','-print-resource-dir'],text=True).strip()+'/include']
for path in [rt/'usr/include/c++/4.6',rt/'usr/include/c++/4.6/arm-linux-gnueabi',rt/'usr/include/c++/4.6/backward',rt/'usr/include',rt/'usr/include/arm-linux-gnueabi']:
 flags+=['-isystem',str(path)]
for path in [rt/'usr/include/SDL',rt/'usr/include/libxml2',src/'src',src/'lib/warmux/include',src/'lib/fixedpoint']:
 flags+=['-I',str(path)]
files=[]
for folder in ['src','lib/warmux','lib/fixedpoint']:
 am=(src/folder/'Makefile.am').read_text().split('AM_CXXFLAGS')[0].split('if WIN32')[0]
 files.extend(src/folder/p for p in re.findall(r'(?m)^\s*([\w./-]+\.cpp)\s*(?:\\)?\s*$',am))
def compile(p):
 obj=out/(str(p.relative_to(src)).replace('/','_')+'.o')
 if obj.exists() and obj.stat().st_mtime>p.stat().st_mtime:return obj
 run=subprocess.run(flags+['-c',str(p),'-o',str(obj)],capture_output=True,text=True)
 if run.returncode:raise RuntimeError(str(p.relative_to(src))+'\n'+run.stderr[:6500])
 return obj
start=time.monotonic();print('Compiling',len(files),'ARM C++ units',flush=True)
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:objects=list(pool.map(compile,files))
print('Compiled in',round(time.monotonic()-start,1),'seconds',flush=True)
libs=[rt/'usr/lib/arm-linux-gnueabi'/n for n in ['libSDL.so','libSDL_gfx.so','libSDL_image.so','libSDL_mixer.so','libSDL_net.so','libSDL_ttf.so','libxml2.so']]
libs.append(rt/'lib/arm-linux-gnueabi/libpng12.so.0')
stock=root/'work/rx3-sysroot/lib';crt=rt/'usr/lib/arm-linux-gnueabi'
args=['clang++','--target=arm-linux-gnueabi','-mfloat-abi=softfp','-march=armv7-a','-fuse-ld=lld','-nostdlib','-no-pie','-Wl,--hash-style=sysv','-Wl,--dynamic-linker=/lib/ld-linux.so.3',str(crt/'crt1.o'),str(crt/'crti.o')]+list(map(str,objects))+list(map(str,libs))+[str(stock/n) for n in ['libc.so.6','libstdc++.so.6','libm.so.6','libgcc_s.so.1','libpthread.so.0','libdl.so.2','librt.so.1']]+[str(crt/'libc_nonshared.a'),str(crt/'crtn.o'),'-o',str(base/'warmux-rx3')]
linked=subprocess.run(args,capture_output=True,text=True)
if linked.returncode:raise RuntimeError(linked.stderr)
print('Built',base/'warmux-rx3')
