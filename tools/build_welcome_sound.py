"""Prepare the welcome cue for bounded RAM streaming."""
from pathlib import Path
import subprocess
output=Path('work/welcome.pcm');output.parent.mkdir(exist_ok=True)
subprocess.run(['ffmpeg','-v','error','-y','-i','assets/sounds/mixkit-winning-an-extra-bonus-2060.wav','-ar','44100','-ac','2','-f','s16le',str(output)],check=True)
if not 0 < output.stat().st_size <= 529200:
    output.unlink()
    raise ValueError('Welcome cue must be at most three seconds')
print(f'Prepared welcome sound: {output.stat().st_size} bytes')
