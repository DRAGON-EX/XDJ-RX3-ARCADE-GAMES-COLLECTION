"""Build on the target OS; include source and license notices beside the executable."""
from pathlib import Path
import importlib.metadata as meta
import os,shutil,subprocess,sys
root=Path(__file__).resolve().parents[1];os.chdir(root)
out=root/'dist/installer';out.mkdir(parents=True,exist_ok=True)
name='DJ-DRAGON-X-Cat-Rescue-Saga-Installer'
# Standalone Python distributions may keep Tcl/Tk outside the system linker path.
extra=[]
if sys.platform.startswith('linux'):
 for lib in (Path(sys.base_prefix)/'lib').glob('libtcl9*.so'):
  extra.extend(['--add-binary',str(lib)+os.pathsep+'.'])
 for directory in (Path(sys.base_prefix)/'lib').glob('tcl[89]*'):
  if directory.is_dir(): extra.extend(['--add-data',str(directory)+os.pathsep+'_tcl_data'])
 for directory in (Path(sys.base_prefix)/'lib').glob('tk[89]*'):
  if directory.is_dir(): extra.extend(['--add-data',str(directory)+os.pathsep+'_tk_data'])

subprocess.run([sys.executable,'-m','PyInstaller','--noconfirm','--clean','--windowed','--onedir','--name',name,'--paths',str(root),'--distpath',str(out),'--workpath','work/pyinstaller','--specpath','work','--collect-all','certifi','--collect-all','inflate64','--add-data',str(root/'installer/source-parts.json')+os.pathsep+'installer','--add-data',str(root/'installer/vendor/rx3_toolkit/key_source.json')+os.pathsep+'installer/vendor/rx3_toolkit',*extra,'installer/run.py'],check=True)
bundle=out/name
shutil.copytree(root/'installer',bundle/'installer-source',dirs_exist_ok=True,ignore=shutil.ignore_patterns('__pycache__','*.pyc'))
licenses=bundle/'THIRD_PARTY_LICENSES';licenses.mkdir(exist_ok=True)
for package in ('cryptography','cffi','pycparser','pycdlib','inflate64','certifi','pyinstaller','setuptools','altgraph','packaging','pyinstaller-hooks-contrib'):
 d=meta.distribution(package)
 for file in d.files or []:
  if any(token in file.name.lower() for token in ('license','copying','notice')) and '.dist-info/' in str(file):
   src=Path(d.locate_file(file))
   if src.is_file():
    dst=licenses/package/file.name;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(src,dst)
# Python/Tcl/Tk licenses ship on every OS, independently of the build host layout.
shutil.copytree(root/'installer/licenses',licenses/'runtimes',dirs_exist_ok=True)
shutil.copy2(root/'installer/README.md',bundle/'README.md')
platform='Windows' if sys.platform=='win32' else 'Linux'
shutil.make_archive(str(out/(name+'-'+platform+'-0.4.7')),'zip',out,name)
print('Built',bundle)
