import sys
import subprocess,re,pathlib
binary=sys.argv[1] if len(sys.argv)>1 else 'dist/RX3_ARCADE/rx3-arcade'
h=subprocess.check_output(['readelf','-h',binary],text=True)
assert 'ARM' in h and 'ELF32' in h and 'EXEC' in h
assert '/lib/ld-linux.so.3' in subprocess.check_output(['readelf','-l',binary],text=True)
s=subprocess.check_output(['readelf','--dyn-syms','--wide',binary],text=True)
libs=list(pathlib.Path('work/rx3-sysroot/lib').glob('*.so*'))+list(pathlib.Path('work/rx3-sysroot/usr/lib').glob('*.so*'))
exports=set()
for p in libs:
 if p.is_symlink() or not p.is_file():continue
 if not any(x in p.name for x in ['libc-','libasound','libgcc_s','librt-','libdl-','libm-','libpthread-']):continue
 try:out=subprocess.check_output(['readelf','--dyn-syms','--wide',str(p)],text=True,stderr=subprocess.DEVNULL)
 except subprocess.CalledProcessError:continue
 for line in out.splitlines():
  parts=line.split()
  if len(parts)>=8 and parts[6]!='UND':exports.add(parts[7].replace('@@','@'))
missing=[]
for line in s.splitlines():
 parts=line.split()
 if len(parts)>=8 and parts[6]=='UND' and parts[4]!='WEAK':
  sym=parts[7]
  if sym not in exports and (('@' in sym) or sym not in {x.split('@')[0] for x in exports}):missing.append(sym)
assert not missing,missing
print('ARMv7 ELF + interpreter verified; all imported symbols found in RX3 dump')
