#!/bin/sh
# Owner-authorized USB-only diagnostic access; all runtime data lives in RAM.
set -u
BASE=/tmp/rx3-launcher
PAYLOAD=$BASE/payload/ssh
KEYDIR=/dev/rx3-games-ssh
ADDRESS=169.254.42.3
PORT=2222
SERVER=
IFACE=
OWNKEYS=0
OWNALIAS=0
REUSEDKEYS=0
cleanup() {
    trap - EXIT INT TERM
    if [ "$OWNALIAS" = 1 ]; then ifconfig "$IFACE:rx3dbg" down 2>/dev/null || :; fi
    if [ "$OWNKEYS" = 1 ]; then
        rm -f "$KEYDIR/authorized_keys"
        rmdir "$KEYDIR" 2>/dev/null || :
    fi
    if [ -n "$SERVER" ]; then
        /bin/sh "$BASE/ssh-stop.sh" "$SERVER"
        wait "$SERVER" 2>/dev/null || :
    fi
    rm -f "$BASE/ssh-server.pid"
}
trap cleanup EXIT
trap 'exit 0' INT TERM
# The public-key directory must have secure ancestors. /dev is RAM on RX3.
if ! awk '$2=="/dev" && ($3=="tmpfs" || $3=="devtmpfs" || $3=="ramfs"){ok=1}END{exit !ok}' /proc/mounts; then
    echo 'SSH skipped: /dev is not a confirmed RAM filesystem'; exit 1
fi
if ! mkdir "$KEYDIR" 2>/dev/null; then
    # Recover only our own exact public key; never delete an unknown directory.
    if [ -L "$KEYDIR" ] || [ ! -d "$KEYDIR" ] || [ ! -O "$KEYDIR" ] || [ -L "$KEYDIR/authorized_keys" ] || [ ! -O "$KEYDIR/authorized_keys" ] || ! cmp -s "$PAYLOAD/authorized_keys" "$KEYDIR/authorized_keys"; then
        echo 'SSH skipped: key directory belongs to another instance'; exit 1
    fi
    for proc in /proc/[0-9]*/exe; do
        exe=$(readlink "$proc" 2>/dev/null || :)
        case "$exe" in "$PAYLOAD/dropbearmulti"|"$PAYLOAD/dropbearmulti (deleted)")
            echo 'SSH skipped: previous server still running'; exit 1 ;;
        esac
    done
    REUSEDKEYS=1
    echo 'SSH recovering its own stale RAM key directory'
fi
OWNKEYS=1
chmod 700 "$KEYDIR"
cp "$PAYLOAD/authorized_keys" "$KEYDIR/authorized_keys" || exit 1
chmod 600 "$KEYDIR/authorized_keys" "$PAYLOAD/host_key"
# RX3 Link Export uses eth0. Accommodate a conventional usb0 gadget name too.
for candidate in eth0 usb0; do
    if [ -d "/sys/class/net/$candidate" ]; then IFACE=$candidate; break; fi
done
if [ -z "$IFACE" ]; then echo 'SSH skipped: RX3 USB network interface unavailable'; cat /proc/net/dev; exit 1; fi
# Add a secondary address only; preserve the stock address, routes and DNS.
if awk -v address="$ADDRESS" '$NF==address{found=1}END{exit !found}' /proc/net/fib_trie; then
    alias=$(ifconfig "$IFACE:rx3dbg" 2>/dev/null || :)
    name=$(printf '%s\n' "$alias" | awk 'NR==1{print $1}')
    if [ "$REUSEDKEYS" != 1 ] || [ "$name" != "$IFACE:rx3dbg" ] || ! printf '%s\n' "$alias" | grep -Fq "$ADDRESS"; then
        echo 'SSH skipped: debug address already in use'; exit 1
    fi
    echo 'SSH recovering its own stale USB alias'
fi
ifconfig "$IFACE:rx3dbg" "$ADDRESS" netmask 255.255.0.0 up || exit 1
OWNALIAS=1
set --
if [ -x "$PAYLOAD/ssh-preflight" ]; then
    "$PAYLOAD/ssh-preflight" > "$BASE/ssh-status"
    cat "$BASE/ssh-status"
    set -- -b "$BASE/ssh-status"
fi
# The stock DJ process may reconfigure eth0. Keep the listener pinned to the
# USB interface, accepting its addresses, and restore only our own alias.
"$PAYLOAD/dropbearmulti" dropbear -F -m -j -k -D "$KEYDIR" -r "$PAYLOAD/host_key" -p "0.0.0.0:$PORT" -l "$IFACE" -P "$BASE/ssh-server.pid" -I 900 "$@" &
SERVER=$!
echo "RX3 SSH starting: $ADDRESS:$PORT on $IFACE; public-key login only"
while kill -0 "$SERVER" 2>/dev/null; do
    sleep 2
    if ! awk -v address="$ADDRESS" '$NF==address{found=1}END{exit !found}' /proc/net/fib_trie; then
        ifconfig "$IFACE:rx3dbg" "$ADDRESS" netmask 255.255.0.0 up && echo 'RX3 SSH restored its USB address after network reconfiguration'
    fi
done
wait "$SERVER"
