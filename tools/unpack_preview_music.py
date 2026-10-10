"""Restore the ignored browser-preview MP3 from the distributed protected archive."""
from pathlib import Path
import zipfile
root=Path(__file__).resolve().parents[1]
with zipfile.ZipFile(root/'assets/music/good.zip') as archive:
    audio=archive.read('good.mp3',pwd=b'good')
(root/'assets/music/cafe-good.mp3').write_bytes(audio)
print('Browser preview music restored (ignored by Git).')
