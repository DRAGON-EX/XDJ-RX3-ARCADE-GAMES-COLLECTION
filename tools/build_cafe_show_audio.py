"""Convert the café's vinyl song to the RX3's bounded streaming PCM format."""
from pathlib import Path
import argparse
import json
import os
import subprocess
import tempfile
import zipfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--source", type=Path, default=Path("assets/music/cafe-good.mp3"))
parser.add_argument("--output", type=Path, default=Path("work/cafe-show.pcm"))
args = parser.parse_args()
args.output.parent.mkdir(parents=True, exist_ok=True)
with tempfile.TemporaryDirectory(dir=args.output.parent) as folder:
    pcm = Path(folder) / "cafe-show.pcm"
    source = args.source
    if not source.exists() and source == Path("assets/music/cafe-good.mp3"):
        source = Path(folder) / "good.mp3"
        with zipfile.ZipFile("assets/music/good.zip") as archive:
            source.write_bytes(archive.read("good.mp3", pwd=b"good"))
    subprocess.run([
        "ffmpeg", "-v", "error", "-y", "-i", str(source),
        "-map", "0:a:0", "-ar", "22050", "-ac", "1", "-f", "s16le", str(pcm),
    ], check=True)
    size = pcm.stat().st_size
    if not size or size % 2 or size > 32 * 1024 * 1024:
        raise ValueError("Café show must decode to a nonempty, bounded S16 mono stream")
    os.replace(pcm, args.output)
metadata = {
    "source": str(source), "output": str(args.output), "encoding": "s16le",
    "channels": 1, "sample_rate": 22050, "bytes": size,
    "playback_rate": 44100, "duration_ms": round(size * 1000 / 44100),
    "stream_buffer_bytes": 1024,
}
args.output.with_suffix(".json").write_text(json.dumps(metadata, indent=2) + "\n")
print(f"Café show: {size:,} bytes, {size / 44100:.3f}s; 1 KB playback buffer")
