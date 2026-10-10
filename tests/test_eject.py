"""Exercise orderly ejection with mocked unmount; never unmount host storage."""
from pathlib import Path
import subprocess,tempfile,time
with tempfile.TemporaryDirectory() as tmp:
    root=Path(tmp);base=root/'rx3-launcher';base.mkdir();device=root/'device';device.mkdir()
    payload=base/'payload';payload.mkdir();(payload/'farewell.pcm').write_bytes(b'bad');(payload/'rx3-arcade').write_text('game');(payload/'road-fighter.nes').write_text('fixture')
    (base/'stop-dj.sh').write_text('#!/bin/sh\nexit 0\n')
    for name in ('rx3-launcher','rx3-arcade','start.sh'):(base/name).write_text('fixture')
    source=root/'launcher.c';source.write_text(Path('native/launcher.c').read_text().replace('/tmp/rx3',tmp+'/rx3').replace('unlink(master_state_path);','master_state_path="'+tmp+'/rx3-launcher/master-level";unlink(master_state_path);'))
    wrapper=root/'wrapper.c'
    wrapper.write_text('#include <stdio.h>\n#include <errno.h>\nstatic int attempts;void __wrap_sync(void){}\nint __wrap_umount(const char*p){FILE*f=fopen("'+tmp+'/unmount-called","a");fprintf(f,"%s\\n",p);fclose(f);if(!attempts++){errno=EBUSY;return -1;}return 0;}\n')
    binary=root/'listener';subprocess.run(['clang','-O2','-I','native',str(source),str(wrapper),'-Wl,--wrap=umount','-Wl,--wrap=sync','-Wl,-l:libasound.so.2','-o',str(binary)],check=True)
    p=subprocess.Popen([str(binary),str(device),str(root),'1'],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    try:
        time.sleep(.3);(base/'eject-request').write_text('')
        assert p.wait(timeout=15)==0
        assert (root/'unmount-called').read_text().splitlines()==[str(root)]*2+[str(payload)]
        assert device.exists() # still physically connected
        assert not base.exists()
    finally:
        if p.poll() is None:p.kill();p.wait()
print('USB STOP performs normal unmount, retries busy storage, cleans launcher and exits')
