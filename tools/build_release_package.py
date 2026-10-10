"""Package only explicitly selected game files; never keys, firmware or SSH state."""
from pathlib import Path
import hashlib,json,shutil,subprocess,tempfile,zipfile
ROOT=Path(__file__).resolve().parents[1]
version=(ROOT/'VERSION').read_text().strip()
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
with tempfile.TemporaryDirectory(dir=ROOT/'work',prefix='release-') as temp:
 stage=Path(temp);files={}
 for name in ('rx3-arcade','rx3-launcher','start.sh','stop-dj.sh'):
  files[name]=ROOT/'dist/RX3_ARCADE'/name
 files['autoexec.sh']=ROOT/'native/autoexec.sh'
 for name in ('menu-music.pcm','rescue-fanfare.pcm','menu-open.pcm','farewell.pcm','welcome.pcm','farewell-artwork.rgba','no-cover-cat.rgb','debug-artwork.rgba','story-artwork.rgba','music-index.tsv'):
  files[name]=ROOT/'work'/name
 for pattern in ('cat-*.pcm','music-cover-*.rgb'):
  for p in (ROOT/'work').glob(pattern):files[p.name]=p
 for name,p in files.items():
  if not p.is_file():raise ValueError('Missing release input: '+str(p))
  shutil.copy2(p,stage/name)
 # Use the updated PCM and the user's MP3 in a standard password ZIP.
 protected=ROOT/'assets/music/good.zip'
 with tempfile.TemporaryDirectory(dir=ROOT/'work') as audio:
  audio=Path(audio);shutil.copy2(ROOT/'assets/music/cafe-good.mp3',audio/'good.mp3');shutil.copy2(ROOT/'work/cafe-show.pcm',audio/'cafe-show.pcm')
  valid=False
  if protected.exists():
   try:
    with zipfile.ZipFile(protected) as z:
     valid=set(z.namelist())=={'good.mp3','cafe-show.pcm'} and all(hashlib.sha256(z.read(p.name,pwd=b'good')).hexdigest()==sha(p) for p in audio.iterdir())
   except (OSError,ValueError,RuntimeError,zipfile.BadZipFile): pass
  if not valid:
   protected.unlink(missing_ok=True)
   subprocess.run(['zip','-q','-j','-P','good',str(protected),str(audio/'good.mp3'),str(audio/'cafe-show.pcm')],check=True)
  music={p.name:sha(p) for p in audio.iterdir()}
 shutil.copy2(protected,stage/'good.zip')
 metadata={'version':version,'files':{p.name:sha(p) for p in stage.iterdir()},'music':music}
 (stage/'payload.json').write_text(json.dumps(metadata,indent=2)+'\n')
 payload_size=sum(p.stat().st_size for p in stage.iterdir() if p.name not in ('good.zip','payload.json'))+(ROOT/'work/cafe-show.pcm').stat().st_size
 if payload_size>95*1024*1024:raise ValueError('RAM budget exceeded')
 archive=ROOT/'work/runtime-0.4.7.zip'
 with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
  for p in sorted(stage.iterdir()):z.write(p,p.name)
release=ROOT/'release';chunks=release/'chunks';chunks.mkdir(parents=True,exist_ok=True)
for p in chunks.glob('*.part'):p.unlink()
parts=[]
with archive.open('rb') as f:
 while data:=f.read(750*1024):
  p=chunks/f'{len(parts):03d}.part';p.write_bytes(data)
  parts.append({'path':p.relative_to(ROOT).as_posix(),'size':len(data),'sha256':sha(p)})
(release/'package.json').write_text(json.dumps({'format':1,'version':version,'model':'XDJ-RX3','sha256':sha(archive),'payload_bytes':payload_size,'chunks':parts},indent=2)+'\n')
print(f'Runtime {archive.stat().st_size:,} bytes; {len(parts)} chunks; RAM payload {payload_size:,} bytes')
