"""Run the real listener against a removable test directory, without RX3 devices."""
from pathlib import Path
import subprocess, tempfile, time
with tempfile.TemporaryDirectory() as tmp:
    root=Path(tmp); base=root/'rx3-launcher';base.mkdir()
    (base/'payload').mkdir();(base/'payload/farewell.pcm').write_bytes(b'bad')
    for name in ('start.sh','rx3-arcade','rx3-launcher'): (base/name).write_text('fixture')
    for name in ('rx3-launcher.pid','rx3-arcade','rx3-arcade.log','rx3-arcade-rbp.log'): (root/name).write_text('fixture')
    for name in ('rx3-launcher.lock','rx3-arcade.lock'): (root/name).mkdir()
    unrelated=root/'keep.txt';unrelated.write_text('keep')
    device=root/'usb-device';device.mkdir()
    source=root/'launcher.c'
    source.write_text(Path('native/launcher.c').read_text().replace('/tmp/rx3',tmp+'/rx3').replace('unlink(master_state_path);','master_state_path="'+tmp+'/rx3-launcher/master-level";unlink(master_state_path);'))
    binary=root/'listener'
    subprocess.run(['clang','-O2','-I','native',str(source),'-Wl,-l:libasound.so.2','-o',str(binary)],check=True)
    p=subprocess.Popen([str(binary),str(device),str(root),"1"],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    try:
        time.sleep(.3);assert p.poll() is None
        device.rmdir();assert p.wait(timeout=15)==0
        assert not base.exists()
        assert not list(root.glob('rx3*'))
        assert unrelated.read_text()=='keep'
    finally:
        if p.poll() is None:p.kill();p.wait()
print('USB disappearance exits real listener and removes only its owned files/locks')
