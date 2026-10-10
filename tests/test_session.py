"""Exercise managed game shutdown without touching real RX3 paths/processes."""
from pathlib import Path
import os, signal, subprocess, tempfile, time
# Validate the actual listener handoff as well as the child script lifecycle.
import re
launcher=Path('native/launcher.c').read_text()
call=re.search(r'execl\("/bin/sh","sh",("/tmp/rx3-launcher/start.sh".*?)\);',launcher)
assert call and call.group(1)=='"/tmp/rx3-launcher/start.sh",(char*)0', 'unexpected arguments passed into menu/game'
with tempfile.TemporaryDirectory() as tmp:
    root = Path(tmp)
    game = root / 'rx3-arcade'
    game.write_text('#!/bin/sh\n[ "${1:-}" != --probe ] || exit 0\necho $$ > "'+tmp+'/game.pid"\ntrap "exit 0" TERM INT\nwhile :; do sleep 0.05; done\n')
    game.chmod(0o755)
    script = Path('native/start.sh').read_text().replace('GAME=/tmp/rx3-arcade', 'GAME="'+tmp+'/running-game"').replace('LOCK=/tmp/rx3-arcade.lock','LOCK="'+tmp+'/game.lock"')
    # Never inspect or signal the host DJ application.
    script = script.replace('PIDS=$(pidof rbp 2>/dev/null || true)', 'PIDS=')
    start = root / 'start.sh'; start.write_text(script)
    p = subprocess.Popen(['/bin/sh',str(start)],env={**os.environ,'RX3_MANAGED':'1'},start_new_session=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    try:
        for _ in range(100):
            if (root/'game.pid').exists(): break
            time.sleep(.02)
        assert (root/'game.pid').exists(), 'game did not start'
        pid = int((root/'game.pid').read_text())
        os.killpg(p.pid,signal.SIGTERM)
        p.wait(timeout=3)
        assert not (root/'running-game').exists()
        assert not (root/'game.lock').exists()
        try: os.kill(pid,0)
        except ProcessLookupError: pass
        else: raise AssertionError('game remains alive')
    finally:
        if p.poll() is None: os.killpg(p.pid,signal.SIGKILL);p.wait()
print('Managed session TERM closes game, waits and removes its RAM files/lock')
