"""Check optional RAM SSH bootstrap and owned-session cleanup without device writes."""
from pathlib import Path
import os, signal, subprocess, tempfile, time, json
SHELL=json.loads(os.environ.get("RX3_TEST_SHELL",'["/bin/sh"]'))

SOURCE=Path('native/ssh-start.sh').read_text()
with tempfile.TemporaryDirectory() as tmp:
    root=Path(tmp); base=root/'launcher'; payload=base/'payload/ssh'; payload.mkdir(parents=True)
    keys=root/'keys'; net=root/'net/eth0'; net.mkdir(parents=True)
    mounts=root/'mounts'; mounts.write_text('tmpfs /dev tmpfs rw 0 0\n')
    fib=root/'fib'; fib.write_text('')
    commands=root/'bin'; commands.mkdir()
    (commands/'ifconfig').write_text('#!/bin/sh\nif [ "$#" = 1 ]; then [ ! -f "'+tmp+'/alias" ] || cat "'+tmp+'/alias"; exit 0; fi\necho "$@" >> "'+tmp+'/network"\n')
    (commands/'ifconfig').chmod(0o755)
    (payload/'authorized_keys').write_text('test public key\n')
    (payload/'host_key').write_text('test host key\n')
    # The fake service and its child are real owned processes; the child leaves its group.
    (payload/'dropbearmulti').write_text('#!/bin/sh\necho $$ > "'+tmp+'/server"\nsetsid sh -c \'echo $$ > "'+tmp+'/child"; exec sleep 120\' &\nwait\n')
    (payload/'dropbearmulti').chmod(0o755)
    (base/'ssh-stop.sh').write_text(Path('native/ssh-stop.sh').read_text())
    start=root/'start.sh'
    start.write_text(SOURCE.replace('/tmp/rx3-launcher',str(base)).replace('/dev/rx3-games-ssh',str(keys)).replace('/proc/mounts',str(mounts)).replace('/proc/net/fib_trie',str(fib)).replace('/sys/class/net',str(root/'net')))
    env={**os.environ,'PATH':str(commands)+':'+os.environ['PATH']}
    def alive(pid):
        try:
            stat=Path(f'/proc/{pid}/stat').read_text().rsplit(') ',1)[1]
            return stat.split()[0]!='Z'
        except FileNotFoundError:return False
    unrelated=subprocess.Popen(['sleep','120'])
    service=None
    try:
        service=subprocess.Popen(SHELL+[str(start)],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
        for _ in range(200):
            if (root/'child').exists():break
            time.sleep(.01)
        server=int((root/'server').read_text()); child=int((root/'child').read_text())
        assert keys.stat().st_mode&0o777==0o700
        assert (keys/'authorized_keys').stat().st_mode&0o777==0o600
        service.terminate(); assert service.wait(timeout=4)==0
        assert not alive(server) and not alive(child)
        assert unrelated.poll() is None
        assert not keys.exists()
        assert (root/'network').read_text().splitlines()==['eth0:rx3dbg 169.254.42.3 netmask 255.255.0.0 up','eth0:rx3dbg down']
        # A pre-existing directory and address must not be deleted or changed.
        (root/'network').unlink(); keys.mkdir(); marker=keys/'authorized_keys'; marker.write_text('existing')
        assert subprocess.run(SHELL+[str(start)],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL).returncode!=0
        assert marker.read_text()=='existing' and not (root/'network').exists()
        marker.unlink();keys.rmdir();fib.write_text(' |-- 169.254.42.3\n')
        # fib_trie indents with a tree prefix: the address is its final field.
        assert subprocess.run(SHELL+[str(start)],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL).returncode!=0
        assert not keys.exists() and not (root/'network').exists()
        # Our exact key can be recovered, including its previously owned USB alias.
        for recover_alias in (False,True):
            keys.mkdir(); keys.chmod(0o700)
            (keys/'authorized_keys').write_bytes((payload/'authorized_keys').read_bytes())
            (keys/'authorized_keys').chmod(0o600)
            fib.write_text(' |-- 169.254.42.3\n' if recover_alias else '')
            if recover_alias:(root/'alias').write_text('eth0:rx3dbg Link encap:Ethernet\n inet addr:169.254.42.3 Mask:255.255.0.0\n')
            for name in ('child','server'):(root/name).unlink(missing_ok=True)
            service=subprocess.Popen(SHELL+[str(start)],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
            for _ in range(200):
                if (root/'child').exists():break
                time.sleep(.01)
            assert service.poll() is None
            server=int((root/'server').read_text());child=int((root/'child').read_text())
            service.terminate();assert service.wait(timeout=5)==0
            assert not alive(server) and not alive(child) and not keys.exists()
            assert unrelated.poll() is None
    finally:
        if service and service.poll() is None: service.kill();service.wait()
        unrelated.terminate();unrelated.wait()
print('SSH supervisor closes owned sessions, preserves unrelated processes and existing network/key state')
