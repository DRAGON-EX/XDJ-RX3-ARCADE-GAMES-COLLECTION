"""Fetch pinned free upstream sources and ARM dependencies into ignored work/ only."""
from pathlib import Path
import concurrent.futures,hashlib,json,subprocess,tarfile,urllib.request
ROOT=Path(__file__).resolve().parents[1]
META=ROOT/'Games/Native/WarMUX'
BASE=ROOT/'work/warmux'
BASE.mkdir(parents=True,exist_ok=True)

def fetch(url,path,sha):
    if path.exists() and hashlib.sha256(path.read_bytes()).hexdigest()==sha:return path
    with urllib.request.urlopen(url,timeout=60) as response:payload=response.read()
    if hashlib.sha256(payload).hexdigest()!=sha:raise RuntimeError('Checksum mismatch: '+url)
    path.write_bytes(payload)
    return path

upstream=json.loads((META/'upstream.json').read_text())
archive=fetch(upstream['archive_url'],BASE/'upstream.tar.gz',upstream['sha256'])
source=BASE/'source'
marker=source/'.rx3-patched'
if not marker.exists():
    source.mkdir(exist_ok=True)
    with tarfile.open(archive) as bundle:
        for member in bundle.getmembers():
            pieces=member.name.split('/',1)
            if len(pieces)<2 or not pieces[1]:continue
            member.name=pieces[1]
            bundle.extract(member,source,filter='data')
    subprocess.run(['patch','-p1','--batch','-i',str(META/'patches/rx3-low-memory.patch')],cwd=source,check=True)
    marker.write_text(upstream['commit']+'\n')
packages=json.loads((META/'debian-armel-packages.json').read_text())
dest=BASE/'packages';dest.mkdir(exist_ok=True)
runtime=BASE/'runtime';runtime.mkdir(exist_ok=True)
def download(d):return fetch(packages['archive']+d['Filename'],dest/Path(d['Filename']).name,d['SHA256'])
with concurrent.futures.ThreadPoolExecutor(max_workers=6) as pool:files=list(pool.map(download,packages['packages']))
for path in files:subprocess.run(['dpkg-deb','-x',str(path),str(runtime)],check=True)
print('Prepared pinned WarMUX source and',len(files),'verified ARM packages in',BASE)
