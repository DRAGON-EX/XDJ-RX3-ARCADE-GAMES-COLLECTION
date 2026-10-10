"""Prepare the local OmarCreative farewell cue; keep it outside the playlist."""
from pathlib import Path
import subprocess
output=Path('work/farewell.pcm');output.parent.mkdir(exist_ok=True)
subprocess.run(['ffmpeg','-v','error','-y','-i','assets/sounds/omarcreative-farewell.mp3',
                '-ar','44100','-ac','2','-f','s16le',str(output)],check=True)
if not 0 < output.stat().st_size <= 2116800:
    output.unlink()
    raise ValueError('Farewell sound must be at most 12 seconds')
print(f'Prepared farewell sound: {output.stat().st_size} bytes')
