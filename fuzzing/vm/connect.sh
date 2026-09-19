#!/bin/sh
# Boot the CIV artifact VM under libvirt (KVM) and open an ssh login to it.
#
#   ./connect.sh            boot (if needed), wait for ssh, then ssh in: your
#                           ssh key is tried first; if the VM does not accept
#                           it, the credentials are printed and you type the
#                           password at the prompt.  You land in /root/civ.
#   ./connect.sh --stop     shut the VM down and exit
#   ./connect.sh --console  attach to the serial console instead of ssh
#
# Nothing is installed in the guest: a key works only if it is already in the
# image's /root/.ssh/authorized_keys; everyone else logs in by password.
# On exit from the shell the VM keeps running unless you answer yes to the
# shutdown prompt.  Uses KVM via libvirt (fast); no /dev/kvm access needed.
set -eu

DIR=$(cd "$(dirname "$0")" && pwd)
XML=$DIR/civ-artifact.libvirt.xml
# Everything is resolved from the script's own directory -- nothing is hard-coded
# to an absolute path, so the artifact works wherever the evaluator unpacks it.
# The XML ships with @@DISK_IMAGE@@/@@EMULATOR@@ placeholders that we fill below.
DISK_NAME=${DISK_NAME:-civ-artifact-freebsd-14.2.qcow2}
QCOW2=$DIR/$DISK_NAME
DOM=civ-artifact
URI=qemu:///system
VM_USER=root
VM_PASSWORD=artifact
# Size the guest to THIS host so connect.sh runs on a modest evaluator machine,
# not only a big one: the XML's 8192 MiB / 16 vCPU is a ceiling, clamped down to
# what is available.  Override explicitly with VM_MEM (MiB) / VM_CPUS.
VM_CPUS=${VM_CPUS:-$(nproc 2>/dev/null || echo 2)}
if [ -z "${VM_MEM:-}" ]; then
	_tot=$(awk '/MemTotal/{print int($2/1024)}' /proc/meminfo 2>/dev/null)
	[ -n "$_tot" ] || _tot=4096
	# leave the host ~25% headroom (>=256, <=1024 MiB); never ask for more than
	# it physically has, so a small machine still boots (just tighter).
	_head=$((_tot / 4)); [ "$_head" -lt 256 ] && _head=256; [ "$_head" -gt 1024 ] && _head=1024
	VM_MEM=$((_tot - _head))
	[ "$VM_MEM" -gt 8192 ] && VM_MEM=8192
	[ "$VM_MEM" -lt 1024 ] && VM_MEM=1024
fi
[ "$VM_MEM" -lt 3072 ] && echo "note: guest RAM is ${VM_MEM} MiB; fuzzing all 24 programs prefers >=4096 (override with VM_MEM=)." >&2
# Try the key first, then fall back to the password prompt.  The guest's host
# key is not pinned because the transient domain changes address between boots.
SSH_OPTS="-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o LogLevel=ERROR -o ConnectTimeout=4 -o PreferredAuthentications=publickey,keyboard-interactive,password"

command -v virsh >/dev/null 2>&1 || { echo "virsh not found (install libvirt-clients)"; exit 1; }
command -v ssh-keyscan >/dev/null 2>&1 || { echo "ssh-keyscan not found (install openssh-client)"; exit 1; }
running() { virsh -c "$URI" list --name 2>/dev/null | grep -qx "$DOM"; }
# sshd on $1 answers (hands out its host key) -- needs no credentials
sshd_up() { ssh-keyscan -T 4 "$1" 2>/dev/null | grep -q .; }
# this machine's ssh key gets us in without a password (BatchMode never prompts)
key_accepted() { ssh $SSH_OPTS -o BatchMode=yes "$VM_USER@$1" true 2>/dev/null; }

# locate a qemu binary without hard-coding a path (distros differ)
qemu_emulator() {
	command -v qemu-system-x86_64 2>/dev/null && return 0
	command -v qemu-kvm 2>/dev/null && return 0
	echo /usr/bin/qemu-system-x86_64
}

# The disk must exist and, because qemu:///system runs QEMU as its own
# unprivileged user, every directory on the way to it must be traversable by
# "other" -- otherwise libvirt fails with "Cannot access storage file ...".
preflight_disk() {
	[ -f "$QCOW2" ] || {
		echo "disk image not found: $QCOW2" >&2
		echo "(the qcow2 must sit next to this script; override the name with DISK_NAME=)" >&2
		exit 1
	}
	_warn='' _d=$(dirname "$QCOW2")
	while :; do
		_perm=$(stat -c '%A' "$_d" 2>/dev/null) || break
		[ "$(printf '%s' "$_perm" | cut -c10)" = x ] || _warn="$_warn $_d"
		[ "$_d" = / ] && break
		_d=$(dirname "$_d")
	done
	if [ -n "$_warn" ]; then
		echo "warning: these directories are not traversable by other users, so the system" >&2
		echo "QEMU (qemu:///system) may be unable to open the disk image:$_warn" >&2
		echo "  fix:  chmod o+x$_warn" >&2
		echo "  or:   boot with ./run-vm.sh instead (it runs QEMU as you -- no traversal issue)" >&2
	fi
	return 0   # never let a false [ -n ... ] test become the function's (set -e fatal) exit status
}

# Fail early with a clear, actionable message if the system libvirt is
# unreachable (daemon down, or the user is not in the 'libvirt' group) -- both
# otherwise surface only as a swallowed "Permission denied" and a silent abort.
require_libvirt() {
	virsh -c "$URI" version >/dev/null 2>&1 && return 0
	cat >&2 <<MSG
error: cannot reach the system libvirt daemon at $URI.
Usually libvirtd is not running, or you are not in the 'libvirt' group:
  start it:    sudo systemctl enable --now libvirtd
  permission:  sudo usermod -aG libvirt "\$(id -un)"   (then log out and back in)
  verify all:  ./check-requirements.sh
Or skip libvirt entirely and boot with plain QEMU:  ./run-vm.sh
MSG
	exit 1
}

# The VM's NIC and the DHCP-lease lookup both require the 'default' network.
require_default_net() {
	virsh -c "$URI" net-info default 2>/dev/null | grep -qi '^Active: *yes' && return 0
	cat >&2 <<MSG
error: the libvirt 'default' network is not active, but this VM needs it.
  start it:   sudo virsh net-start default
  on boot:    sudo virsh net-autostart default
Or boot with plain QEMU instead:  ./run-vm.sh
MSG
	exit 1
}

# Render a runtime copy of the domain XML with the placeholders filled from this
# host, printing its path.  Also overrides any stale <source>/<emulator> value,
# so an XML that lost its placeholders still boots against the local image.
make_runtime_xml() {
	_emu=$(qemu_emulator)
	_out=$(mktemp "${TMPDIR:-/tmp}/civ-artifact.XXXXXX") || return 1
	# escape sed replacement metacharacters (& | \) in the two paths
	_disk=$(printf '%s' "$QCOW2" | sed 's/[&|\\]/\\&/g')
	_emurep=$(printf '%s' "$_emu" | sed 's/[&|\\]/\\&/g')
	sed -e "s|@@DISK_IMAGE@@|$_disk|g" \
	    -e "s|@@EMULATOR@@|$_emurep|g" \
	    -e "s|<source file='[^']*'/>|<source file='$_disk'/>|g" \
	    -e "s|<emulator>[^<]*</emulator>|<emulator>$_emurep</emulator>|g" \
	    -e "s|<memory\([^>]*\)>[0-9]*</memory>|<memory\1>$VM_MEM</memory>|" \
	    -e "s|<vcpu\([^>]*\)>[0-9]*</vcpu>|<vcpu\1>$VM_CPUS</vcpu>|" \
	    "$XML" > "$_out" || { rm -f "$_out"; return 1; }
	printf '%s\n' "$_out"
}

case "${1:-}" in
--stop)
	require_libvirt
	if running; then echo "Shutting down $DOM ..."; virsh -c "$URI" shutdown "$DOM" >/dev/null; else echo "$DOM is not running."; fi
	exit 0 ;;
-h|--help)
	sed -n '2,14p' "$0"; exit 0 ;;
esac

# Preconditions the boot/console paths depend on (clear message beats a silent abort).
require_libvirt
require_default_net

# 1. boot if needed
if running; then
	echo "VM '$DOM' is already running."
else
	[ -f "$XML" ] || { echo "missing $XML"; exit 1; }
	preflight_disk
	RUNXML=$(make_runtime_xml) || { echo "could not prepare the domain XML" >&2; exit 1; }
	trap 'rm -f "$RUNXML"' EXIT INT TERM
	echo "Starting VM '$DOM' (${VM_CPUS} vCPU, ${VM_MEM} MiB; disk: $QCOW2) ..."
	virsh -c "$URI" create "$RUNXML" >/dev/null
	rm -f "$RUNXML"; trap - EXIT INT TERM
fi

# serial-console mode: hand off directly
if [ "${1:-}" = "--console" ]; then
	echo "Attaching serial console (login $VM_USER / $VM_PASSWORD; leave with Ctrl-])."
	exec virsh -c "$URI" console "$DOM"
fi

# 2. wait for a DHCP lease.  Match the domain's *live* MAC, not the hostname:
# the transient domain gets a new random MAC each boot, so old civ-artifact
# leases linger and a hostname match can pick a stale, dead IP.
MAC=$(virsh -c "$URI" domiflist "$DOM" 2>/dev/null | awk '$2=="network"||$3=="default"{print $5}' | grep -E '^([0-9a-f]{2}:){5}' | head -1)
[ -n "$MAC" ] || { echo "could not determine the VM's MAC"; exit 1; }
printf 'Waiting for the VM to get an IP (MAC %s)' "$MAC"
IP=""; i=0
while [ "$i" -lt 90 ]; do
	IP=$(virsh -c "$URI" net-dhcp-leases default 2>/dev/null | awk -v m="$MAC" 'index($0,m){print $5}' | cut -d/ -f1 | tail -1)
	[ -n "$IP" ] && break
	printf '.'; sleep 2; i=$((i + 1))
done
echo
[ -n "$IP" ] || { echo "No lease yet. Check: virsh -c $URI net-dhcp-leases default"; exit 1; }
echo "VM IP: $IP"

# 3. wait for sshd
printf 'Waiting for ssh'
i=0
while [ "$i" -lt 90 ]; do
	sshd_up "$IP" && break
	printf '.'; sleep 2; i=$((i + 1))
done
echo
sshd_up "$IP" || { echo "sshd did not answer on $IP. Try: $0 --console"; exit 1; }

# 4. hand over: key if the VM accepts it, otherwise print the credentials and
# let the user type the password
echo
if key_accepted "$IP"; then
	echo "The VM is up and accepts your ssh key -- logging in without a password."
	echo "(from another terminal: ssh $VM_USER@$IP; password login also works: $VM_USER / $VM_PASSWORD)"
else
	cat <<CREDS
The VM is up.  It does not accept an ssh key from this machine, so log in with:

    username: $VM_USER
    password: $VM_PASSWORD

(from another terminal: ssh $VM_USER@$IP)
CREDS
fi
echo "You will land in /root/civ -- run ./fuzz.sh to start fuzzing."
echo
ssh $SSH_OPTS -t "$VM_USER@$IP" 'cd /root/civ && exec sh' || true

# 5. offer to power off
echo
printf 'Shut the VM down now? [y/N]: '
read ans || ans=n
case "$ans" in
[yY]|[yY][eE][sS]) virsh -c "$URI" shutdown "$DOM" >/dev/null && echo "Shutting down." ;;
*) echo "VM left running. Stop it later with: $0 --stop" ;;
esac
