"""Prepare the no-cover animation; native playback keeps only one frame in memory."""
from pathlib import Path
import subprocess
p=Path('work/no-cover-cat.rgb');p.parent.mkdir(exist_ok=True)
subprocess.run(['ffmpeg','-v','error','-y','-i','assets/music/no-cover-cat.gif','-vf','scale=180:180:flags=neighbor','-frames:v','64','-pix_fmt','rgb24','-f','rawvideo',str(p)],check=True)
assert p.stat().st_size==64*180*180*3
print('No-cover animation: 64 frames at 60ms, one 97200-byte frame buffer')
