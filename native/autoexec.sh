#!/bin/sh
USB=${1:-}
RX3_PERSISTENT_SAVE=0
[ ! -f /mnt/iso/persistent-save.enabled ] || RX3_PERSISTENT_SAVE=1
export RX3_PERSISTENT_SAVE
LOG=/tmp/rx3-arcade.log
if [ -n "$USB" ] && [ -d "$USB" ]; then LOG="$USB/rx3-arcade.log"; fi
: > "$LOG" 2>/dev/null || LOG=/tmp/rx3-arcade.log
echo "RX3 v0.4.7 RAM bootstrap started" >> "$LOG"
# Resolve the actual mounted USB block device, independent of cached files.
DEV=$(awk -v p="$USB" '$2==p || index(p,$2"/")==1 {if(length($2)>best){best=length($2);dev=$1}} END{print dev}' /proc/mounts)
case "$DEV" in /dev/sd*) DEVICE="/sys/class/block/${DEV##*/}" ;; *) echo "USB device not identified; launcher cancelled" >> "$LOG"; exit 1 ;; esac
[ -d "$DEVICE" ] || exit 1
case "$USB" in /media/usb2/*) PORT=2 ;; *) PORT=1 ;; esac
# A persistent listener runs from RAM after the encrypted ISO is unmounted.
if ! mkdir /tmp/rx3-launcher.lock 2>/dev/null; then
    PID=$(cat /tmp/rx3-launcher.pid 2>/dev/null)
    if [ -n "$PID" ] && kill -0 "$PID" 2>/dev/null; then
        echo "RX3 launcher already active" >> "$LOG"
        exit 0
    fi
    rmdir /tmp/rx3-launcher.lock 2>/dev/null || exit 1
    mkdir /tmp/rx3-launcher.lock || exit 1
fi
mkdir -p /tmp/rx3-launcher || { rmdir /tmp/rx3-launcher.lock; exit 1; }
# Keep only the small listener/scripts in the stock temporary filesystem.
# Emulator and local ROM need more space than the stock 512 KB RAM filesystem.
rm -f /tmp/rx3-launcher/rx3-arcade
mkdir -p /tmp/rx3-launcher/payload || exit 1
if ! mount -t tmpfs -o size=96m,mode=0700 rx3-games /tmp/rx3-launcher/payload; then
    echo "Dedicated games RAM mount failed" >> "$LOG"
    rmdir /tmp/rx3-launcher/payload /tmp/rx3-launcher.lock 2>/dev/null
    exit 1
fi
fail_bootstrap() {
    echo -1 > /tmp/rx3-launcher/load-progress
    if [ -n "${LOADING_PID:-}" ]; then wait "$LOADING_PID"; fi
    rm -rf /tmp/rx3-launcher/payload/ssh
    echo "RAM bootstrap copy failed" >> "$LOG"
    rm -f /tmp/rx3-launcher/payload/cat-*.pcm /tmp/rx3-launcher/payload/rx3-arcade /tmp/rx3-launcher/payload/road-fighter.nes /tmp/rx3-launcher/payload/menu-music.pcm /tmp/rx3-launcher/payload/cafe-show.pcm /tmp/rx3-launcher/payload/rescue-fanfare.pcm /tmp/rx3-launcher/payload/menu-open.pcm /tmp/rx3-launcher/payload/farewell.pcm /tmp/rx3-launcher/payload/welcome.pcm /tmp/rx3-launcher/payload/farewell-artwork.rgba /tmp/rx3-launcher/payload/no-cover-cat.rgb /tmp/rx3-launcher/payload/debug-artwork.rgba
    umount /tmp/rx3-launcher/payload
    rmdir /tmp/rx3-launcher/payload /tmp/rx3-launcher.lock 2>/dev/null
    exit 1
}
# Start the small overlay before copying the large game and soundtrack payload.
cp /mnt/iso/rx3-launcher /tmp/rx3-launcher/rx3-launcher || fail_bootstrap
chmod 755 /tmp/rx3-launcher/rx3-launcher
TOTAL=0
for file in /mnt/iso/rx3-arcade /mnt/iso/start.sh /mnt/iso/stop-dj.sh /mnt/iso/road-fighter.nes /mnt/iso/*.pcm /mnt/iso/*.rgba /mnt/iso/*.rgb /mnt/iso/music-index.tsv /mnt/iso/rx3-ssh/* ; do
    [ -f "$file" ] || continue
    size=$(wc -c < "$file"); TOTAL=$((TOTAL + size))
done
if [ -f /mnt/iso/rx3-ssh/dropbearmulti ]; then
    for file in /mnt/iso/ssh-start.sh /mnt/iso/ssh-stop.sh; do
        size=$(wc -c < "$file"); TOTAL=$((TOTAL + size))
    done
fi
DONE=0
echo 0 > /tmp/rx3-launcher/load-progress
(cd /tmp/rx3-launcher && exec ./rx3-launcher --loading "$DEVICE" </dev/null >> "$LOG" 2>&1) &
LOADING_PID=$!
copy_payload() {
    source=$1; dest=$2
    [ ! -d "$dest" ] || dest="$dest/${source##*/}"
    /tmp/rx3-launcher/rx3-launcher --copy "$source" "$dest" "$DONE" "$TOTAL" || return 1
    size=$(wc -c < "$source"); DONE=$((DONE + size))
}
copy_payload /mnt/iso/rx3-arcade /tmp/rx3-launcher/payload/rx3-arcade || fail_bootstrap
chmod 755 /tmp/rx3-launcher/payload/rx3-arcade
for name in start.sh stop-dj.sh; do
    copy_payload "/mnt/iso/$name" "/tmp/rx3-launcher/$name" || fail_bootstrap
done
if [ -f /mnt/iso/road-fighter.nes ]; then
    copy_payload /mnt/iso/road-fighter.nes /tmp/rx3-launcher/payload/road-fighter.nes || fail_bootstrap
fi
# Optional locally built SSH runtime; public client key and own host key only.
if [ -f /mnt/iso/rx3-ssh/dropbearmulti ]; then
    mkdir -p /tmp/rx3-launcher/payload/ssh || fail_bootstrap
    chmod 700 /tmp/rx3-launcher/payload/ssh
    for name in dropbearmulti authorized_keys host_key ssh-preflight; do
        copy_payload "/mnt/iso/rx3-ssh/$name" "/tmp/rx3-launcher/payload/ssh/$name" || fail_bootstrap
    done
    chmod 755 /tmp/rx3-launcher/payload/ssh/dropbearmulti /tmp/rx3-launcher/payload/ssh/ssh-preflight
    chmod 600 /tmp/rx3-launcher/payload/ssh/authorized_keys /tmp/rx3-launcher/payload/ssh/host_key
    for name in ssh-start.sh ssh-stop.sh; do
        copy_payload "/mnt/iso/$name" "/tmp/rx3-launcher/$name" || fail_bootstrap
    done
fi
chmod 755 /tmp/rx3-launcher/*
if [ -f /mnt/iso/menu-music.pcm ]; then
    copy_payload /mnt/iso/menu-music.pcm /tmp/rx3-launcher/payload/menu-music.pcm || fail_bootstrap
fi
for file in /mnt/iso/cat-*.pcm /mnt/iso/cafe-show.pcm /mnt/iso/rescue-fanfare.pcm /mnt/iso/story-artwork.rgba /mnt/iso/debug-artwork.rgba /mnt/iso/no-cover-cat.rgb /mnt/iso/farewell-artwork.rgba /mnt/iso/welcome.pcm /mnt/iso/farewell.pcm /mnt/iso/music-index.tsv /mnt/iso/music-cover-*.rgb; do
    [ -f "$file" ] || continue
    copy_payload "$file" /tmp/rx3-launcher/payload/ || fail_bootstrap
done
if [ -f /mnt/iso/menu-open.pcm ]; then
    copy_payload /mnt/iso/menu-open.pcm /tmp/rx3-launcher/payload/menu-open.pcm || fail_bootstrap
fi
[ "$DONE" -eq "$TOTAL" ] || fail_bootstrap
echo 100 > /tmp/rx3-launcher/load-progress
wait "$LOADING_PID"
echo "RX3 game payload copied to dedicated RAM filesystem" >> "$LOG"
# Ensure no background child retains a cwd or fd in /mnt/iso.
(cd /tmp/rx3-launcher && exec ./rx3-launcher "$DEVICE" "$USB" "$PORT" </dev/null >> "$LOG" 2>&1) &
echo $! > /tmp/rx3-launcher.pid
exit 0
