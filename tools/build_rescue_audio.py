"""Prepare user-supplied rescue/cafe audio as mono 22.05 kHz streams."""
from pathlib import Path
import subprocess
for source in [Path('assets/sounds/rescue-fanfare.mp3'),*sorted(Path('assets/sounds/cats').glob('*.mp3'))]:
 target=Path('work')/(source.stem+'.pcm')
 subprocess.run(['ffmpeg','-v','error','-y','-i',str(source),*(['-t','8'] if source.stem.startswith('cat-purr') else []),'-ar','22050','-ac','1','-f','s16le',str(target)],check=True)
