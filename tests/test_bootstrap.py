"""Verify RAM placement and early errors without mounting host filesystems."""
from pathlib import Path
import os,subprocess,tempfile,time
for fail,ssh_enabled in ((False,False),(False,True),(True,False)):
 with tempfile.TemporaryDirectory() as tmp:
  root=Path(tmp);iso=root/'iso';iso.mkdir();usb=root/'usb';usb.mkdir();(root/'devices/sda1').mkdir(parents=True)
  bin=root/'bin';bin.mkdir()
  (bin/'mount').write_text('#!/bin/sh\necho "$@" >> "'+tmp+'/mounts"\nexit 0\n')
  (bin/'umount').write_text('#!/bin/sh\necho "$@" >> "'+tmp+'/unmounts"\nexit 0\n')
  (bin/'cp').write_text('#!/bin/sh\ncase "$2" in *payload/*) ;; *) [ "$(wc -c < "$1")" -le 524288 ] || exit 90 ;; esac\nexec /bin/cp "$@"\n')
  for p in bin.iterdir():p.chmod(0o755)
  if not fail:(iso/'rx3-arcade').write_bytes(b'X'*2700000)
  (iso/'road-fighter.nes').write_bytes(b'fixture')
  (iso/'farewell.pcm').write_bytes(b'farewell fixture')
  (iso/'welcome.pcm').write_bytes(b'welcome fixture')
  (iso/'farewell-artwork.rgba').write_bytes(b'X'*640000)
  (iso/'rx3-launcher').write_text('#!/bin/sh\ncase "$1" in --copy) exec /bin/cp "$2" "$3" ;; --loading) exit 0 ;; esac\necho launched >> "'+tmp+'/daemon"\n')
  if ssh_enabled:
   (iso/'rx3-ssh').mkdir()
   for name in ('dropbearmulti','authorized_keys','host_key','ssh-preflight'):(iso/'rx3-ssh'/name).write_bytes(b'ssh fixture')
  for name in ('start.sh','stop-dj.sh','ssh-start.sh','ssh-stop.sh'):(iso/name).write_text('#!/bin/sh\nexit 0\n')
  s=Path('native/autoexec.sh').read_text().replace('/mnt/iso',str(iso)).replace('/tmp/rx3-launcher',tmp+'/rx3-launcher').replace('/sys/class/block',tmp+'/devices')
  s='\n'.join('DEV=/dev/sda1' if line.startswith('DEV=$(awk') else line for line in s.splitlines())
  script=root/'autoexec.sh';script.write_text(s)
  r=subprocess.run(['/bin/sh',str(script),str(usb)],env={**os.environ,'PATH':str(bin)+':'+os.environ['PATH']},stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
  log=(usb/'rx3-arcade.log').read_text();assert 'bootstrap started' in log
  assert '-t tmpfs -o size=96m,mode=0700' in (root/'mounts').read_text()
  if fail:assert r.returncode!=0 and 'copy failed' in log and (root/'unmounts').exists()
  else:
   assert r.returncode==0 and (root/'rx3-launcher/payload/rx3-arcade').stat().st_size==2700000
   assert not (root/'rx3-launcher/rx3-arcade').exists()
   assert (root/'rx3-launcher/payload/farewell.pcm').read_bytes()==b'farewell fixture'
   assert (root/'rx3-launcher/payload/farewell-artwork.rgba').stat().st_size==640000
   assert (root/'rx3-launcher/payload/welcome.pcm').read_bytes()==b'welcome fixture'
   if ssh_enabled:
    for name in ('dropbearmulti','authorized_keys','host_key','ssh-preflight'):assert (root/'rx3-launcher/payload/ssh'/name).read_bytes()==b'ssh fixture'
    assert (root/'rx3-launcher/payload/ssh/host_key').stat().st_mode&0o777==0o600
    assert (root/'rx3-launcher/ssh-start.sh').exists()
   for _ in range(50):
    if (root/'daemon').exists():break
    time.sleep(.01)
   assert (root/'daemon').exists()
print('Large game bypasses stock 512 KB area; bootstrap failures are logged and RAM mount cleaned')
