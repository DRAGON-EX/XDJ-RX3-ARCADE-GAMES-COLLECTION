"""Prepare bounded farewell RGBA data outside the small launcher binary."""
from pathlib import Path
import subprocess
p=Path('work/farewell-artwork.rgba');p.parent.mkdir(exist_ok=True)
subprocess.run(['ffmpeg','-v','error','-y','-i','assets/farewell-artwork-transparent.png','-vf','scale=400:400:flags=neighbor','-frames:v','1','-pix_fmt','rgba','-f','rawvideo',str(p)],check=True)
assert p.stat().st_size==640000
print('Farewell artwork: 640000 bytes, loaded only while drawing farewell')
