#!/bin/sh
# Run only while the RX3 is idle: this temporarily closes its DJ application.
set -u
BASE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
GAME=/tmp/rx3-arcade
LOCK=/tmp/rx3-arcade.lock
mkdir "$LOCK" 2>/dev/null || exit 1
RAM_GAME=0
if [ -f "$BASE/payload/rx3-arcade" ]; then
    GAME="$BASE/payload/rx3-arcade"
    RAM_GAME=1
else
    cp "$BASE/rx3-arcade" "$GAME" || { rmdir "$LOCK"; exit 1; }
fi
chmod 755 "$GAME"
"$GAME" --probe || { [ "$RAM_GAME" = 1 ] || rm -f "$GAME"; rmdir "$LOCK"; exit 1; }
RESTART=0
GAME_PID=
cleanup() {
    trap - EXIT INT TERM
    if [ -n "$GAME_PID" ]; then kill -TERM "$GAME_PID" 2>/dev/null; wait "$GAME_PID" 2>/dev/null; fi
    [ "$RAM_GAME" = 1 ] || rm -f "$GAME"
    rmdir "$LOCK" 2>/dev/null
    if [ "$RESTART" = 1 ] && [ "${RX3_MANAGED:-0}" != 1 ]; then
        (cd /root/pdj && ./rbp > /tmp/rx3-arcade-rbp.log 2>&1) &
    fi
}
trap cleanup EXIT
trap 'exit 130' INT TERM
PIDS=$(pidof rbp 2>/dev/null || true)
if [ -n "$PIDS" ]; then
    [ "${RX3_MANAGED:-0}" != 1 ] || : > "$BASE/restart-dj"
    kill -TERM $PIDS || exit 1
    RESTART=1
    i=0
    while pidof rbp >/dev/null 2>&1; do
        i=$((i+1))
        if [ "$i" -ge 10 ]; then RESTART=0; rm -f "$BASE/restart-dj"; echo "DJ app still running; launch cancelled"; exit 1; fi
        sleep 1
    done
fi
"$GAME" "$@" &
GAME_PID=$!
wait "$GAME_PID"
STATUS=$?
GAME_PID=
exit "$STATUS"
