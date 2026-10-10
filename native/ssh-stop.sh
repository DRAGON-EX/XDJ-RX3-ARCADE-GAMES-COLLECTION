#!/bin/sh
# Stop only the owned server and its descendants, guarding against PID reuse.
set -u
ROOT=${1:-}
case "$ROOT" in ''|*[!0-9]*) exit 1 ;; esac
[ "$ROOT" -gt 1 ] || exit 1
[ -r "/proc/$ROOT/stat" ] || exit 0
kill -STOP "$ROOT" 2>/dev/null || exit 0
SNAPSHOT=$(awk -v root="$ROOT" '
 {pid=FILENAME;sub(/^\/proc\//,"",pid);sub(/\/stat$/,"",pid);
  sub(/^.*\) /,"");parent[pid]=$2;born[pid]=$20;ids[++n]=pid}
 END{owned[root]=1;for(pass=0;pass<n;pass++)for(i=1;i<=n;i++)if(owned[parent[ids[i]]])owned[ids[i]]=1;
 for(i=1;i<=n;i++)if(owned[ids[i]])print ids[i] ":" born[ids[i]]}' /proc/[0-9]*/stat 2>/dev/null || :)
signal_owned() {
    for entry in $SNAPSHOT; do
        pid=${entry%:*};born=${entry#*:}
        now=$(awk '{sub(/^.*\) /,"");print $20}' "/proc/$pid/stat" 2>/dev/null || :)
        if [ "$now" = "$born" ]; then kill "-$1" "$pid" 2>/dev/null || :; fi
    done
}
signal_owned STOP
signal_owned TERM
signal_owned CONT
if command -v usleep >/dev/null 2>&1; then usleep 300000; else sleep 1; fi
signal_owned KILL
