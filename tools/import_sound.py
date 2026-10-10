import zipfile,re,json,hashlib,sys
from pathlib import Path
rom=Path(sys.argv[1] if len(sys.argv)>1 else '/run/media/drakethedragon/Apps | Games | Data/Downloads/pacman.zip')
with zipfile.ZipFile(rom) as z:
 wave=z.read('82s126.1m')
assert len(wave)==256
s=Path('vendor/pacman-upstream.c').read_text()
a=s.index('static const uint8_t rom_wavetable[256] = {');b=s.index('};',a)
original=bytes(int(x,16) for x in re.findall(r'0x([0-9A-Fa-f]+)',s[a:b]))
print('Waveform bytes:',len(wave),'match upstream:',wave==original)
out='static const uint8_t rom_wavetable[256] = {'+','.join(str(x) for x in wave)+'};\n'
for name in ['snd_dump_prelude','snd_dump_dead']:
 a=s.index('static const uint32_t '+name+'[',s.index('static const uint8_t rom_wavetable[256] = {'))
 b=s.index('};',a)+2;out+=s[a:b]+'\n'
Path('Games/Native/Pacman/pacman_sound_data.h').write_text('/* Local ROM wavetable + upstream captured sound-register sequences. */\n'+out)
Path('assets/sound-provenance.json').write_text(json.dumps({'rom_file':rom.name,'wavetable_member':'82s126.1m','sha256':hashlib.sha256(wave).hexdigest(),'matches_upstream':wave==original,'sequences':'floooh/pacman.c captured start/death Namco WSG registers'},indent=2))
