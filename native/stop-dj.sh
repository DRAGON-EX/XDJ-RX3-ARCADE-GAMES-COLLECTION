#!/bin/sh
# Stop the DJ process before a normal USB unmount; remember to restart it.
PIDS=$(pidof rbp 2>/dev/null || true)
[ -n "$PIDS" ] || exit 0
: > /tmp/rx3-launcher/restart-dj
kill -TERM $PIDS || exit 1
i=0
while pidof rbp >/dev/null 2>&1; do
    i=$((i+1))
    [ "$i" -lt 10 ] || exit 1
    sleep 1
done
