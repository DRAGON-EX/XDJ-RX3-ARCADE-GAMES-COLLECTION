"""Prepare debug artwork for on-demand loading."""
from pathlib import Path
import subprocess
p=Path('work/debug-artwork.rgba');p.parent.mkdir(exist_ok=True)
subprocess.run(['ffmpeg','-v','error','-y','-i','assets/debug-artwork.png','-vf','scale=440:440:flags=neighbor','-frames:v','1','-pix_fmt','rgba','-f','rawvideo',str(p)],check=True)
assert p.stat().st_size==440*440*4
