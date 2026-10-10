"""Prepare a bounded RX3 playlist, metadata and embedded covers from local music."""
from pathlib import Path
import argparse, json, subprocess, tempfile, unicodedata

parser=argparse.ArgumentParser()
parser.add_argument('--debug-unlocked',action='store_true')
parser.add_argument('tracks', nargs='*', type=Path, help='User-supplied music files, in playback order')
args=parser.parse_args()
catalog=json.loads(Path('content/music-library.json').read_text())
tracks=args.tracks or [Path(t['source']) for t in catalog]
if not 1 <= len(tracks) <= 32: raise ValueError('Choose between 1 and 32 tracks')
known={Path(t['source']).stem:(t['title'],t['artist']) for t in catalog}
locked_sources={t['source'] for t in catalog if t.get('locked')}
output=Path('work/debug-music' if args.debug_unlocked else 'work');output.mkdir(exist_ok=True)
def clean(s,limit):
    return ''.join(c if 32<=ord(c)<=126 else ' ' for c in unicodedata.normalize('NFKD',s)).replace('\t',' ').strip()[:limit]
with tempfile.TemporaryDirectory() as folder:
    root=Path(folder);playlist=root/'menu-music.pcm';rows=[];manifest=[];position=0
    with playlist.open('wb') as out:
        for i,track in enumerate(tracks):
            probe=json.loads(subprocess.check_output(['ffprobe','-v','error','-show_format','-show_streams','-of','json',str(track)]))
            tags={k.lower():v for k,v in probe.get('format',{}).get('tags',{}).items()}
            fallback=known.get(track.stem,(track.stem,'Unknown artist'))
            title=clean(tags.get('title',fallback[0]),79);artist=clean(tags.get('artist',fallback[1]),63)
            metadata=next((t for t in catalog if t['source']==str(track)),{})
            # Scene songs stay packed while locked so fireworks and credits can
            # play them without adding another decoded copy to the RAM payload.
            secret=3 if metadata.get('unlock')=='dj-wars' else 2 if metadata.get('lock_message') else 0
            if str(track) in locked_sources and not secret and not args.debug_unlocked:
                rows.append(f'{position}\t{position}\t{title}\t{artist}\t1\n')
                manifest.append({'title':title,'artist':artist,'source':str(track),'locked':True})
                continue
            pcm=root/f'{i}.pcm'
            subprocess.run(['ffmpeg','-v','error','-y','-i',str(track),'-ar','22050','-ac','1','-f','s16le',str(pcm)],check=True)
            data=pcm.read_bytes();end=position+len(data)
            if end>80_000_000:raise ValueError('Playlist exceeds the current 80 MB RX3 RAM-package limit')
            out.write(data);rows.append(f'{position}\t{end}\t{title}\t{artist}'+(f'\t{secret}' if secret else '')+'\n')
            cover=None
            if any(s.get('codec_type')=='video' for s in probe.get('streams',[])):
                cover=root/f'music-cover-{i}.rgb'
                subprocess.run(['ffmpeg','-v','error','-y','-i',str(track),'-map','0:v:0','-frames:v','1','-vf','scale=180:180:force_original_aspect_ratio=increase,crop=180:180','-pix_fmt','rgb24','-f','rawvideo',str(cover)],check=True)
            manifest.append({'title':title,'artist':artist,'duration':len(data)/44100,'source':str(track),'cover':cover.name if cover else None,**({'locked':True,'unlock':metadata.get('unlock','credits'),'lock_message':metadata['lock_message']} if secret else {})});position=end
    # Publish outputs only after the whole import succeeds.
    (output/'menu-music.pcm').write_bytes(playlist.read_bytes())
    (output/'music-index.tsv').write_text(''.join(rows))
    (output/'music-library.json').write_text(json.dumps(manifest,indent=2))
    for old in output.glob('music-cover-*.rgb'):old.unlink()
    for cover in root.glob('music-cover-*.rgb'):(output/cover.name).write_bytes(cover.read_bytes())
print(f'Prepared {len(tracks)} tracks, metadata and covers: {position} bytes')
