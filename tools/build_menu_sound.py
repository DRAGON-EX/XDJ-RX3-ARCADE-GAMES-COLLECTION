"""Prepare the authorized Debug Information sound for the RX3 output clock."""
from pathlib import Path
import subprocess

output = Path('work/menu-open.pcm')
output.parent.mkdir(exist_ok=True)
subprocess.run(['ffmpeg', '-v', 'error', '-y', '-i',
                'assets/sounds/zelda-menu-open.mp3', '-ar', '44100', '-ac', '2',
                '-f', 's16le', str(output)], check=True)
if output.stat().st_size > 384000:
    output.unlink()
    raise ValueError('Debug sound exceeds the two-second buffer limit')
print(f'Prepared debug menu sound: {output.stat().st_size} bytes')
