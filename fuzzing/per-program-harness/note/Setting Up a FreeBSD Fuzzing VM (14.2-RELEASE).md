This guide walks you through setting up a FreeBSD 14.2 VM for fuzzing purposes using QEMU.

---

## 📥 Download and Prepare the VM Image

```sh
wget https://download.freebsd.org/releases/VM-IMAGES/14.2-RELEASE/amd64/Latest/FreeBSD-14.2-RELEASE-amd64.qcow2.xz
xz -d FreeBSD-14.2-RELEASE-amd64.qcow2.xz
```

---

## ⚙️ Create a Run Script

> **Note:** Change `2222` to another unused port if necessary.

```sh
echo '#!/bin/bash
qemu-system-x86_64 \
  -enable-kvm \
  -cpu host \
  -smp 2 \
  -m 4096 \
  -drive file=FreeBSD-14.2-RELEASE-amd64.qcow2,format=qcow2,if=virtio \
  -netdev user,id=net0,hostfwd=tcp::2222-:22 \
  -device virtio-net-pci,netdev=net0 \
  -nographic \
  -serial mon:stdio' > run-freebsd.sh && chmod +x run-freebsd.sh
```

---

## 💾 Resize the VM Disk

Additional disk space will be needed for installing AFL++

```sh
qemu-img resize FreeBSD-14.2-RELEASE-amd64.qcow2 +32G
```

---

## 🖥️ Launch the VM in tmux

```sh
tmux new -s freebsd-aponi-2
./run-freebsd.sh
```

---

## 🧰 Configure Serial Console (Inside the VM)

When you see:
```
Autoboot in 7 seconds. [Space] to pause
```
- Press `3` to enter the loader prompt
- Run:

```sh
set console="comconsole"
boot
```

Log in with:
```
login: root
password: (just press Enter)
```

### Make Serial Console Persistent

```sh
echo 'console="comconsole"' >> /boot/loader.conf
```

---

## 📈 Expand Root Filesystem

```sh
gpart recover vtbd0
gpart resize -i 4 vtbd0
growfs /dev/gpt/rootfs
```

---

## 🔐 Enable SSH Root Login

```sh
echo "PermitRootLogin yes" >> /etc/ssh/sshd_config
echo 'sshd_enable="YES"' >> /etc/rc.conf
service sshd start
```

---

## 🔑 Set Up SSH Authorized Keys

Replace the example public keys below with your own keys.

```sh
mkdir -p ~/.ssh
chmod 700 ~/.ssh

cat << EOF > ~/.ssh/authorized_keys
ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAEXAMPLEKEY000000000000000000000000000000 replace-with-your-key-comment-1
ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAEXAMPLEKEY111111111111111111111111111111 replace-with-your-key-comment-2
EOF

chmod 600 ~/.ssh/authorized_keys
```

---

## 🔚 Detach the tmux Session

Press: `Ctrl + B`, then `D`

---

## 🖧 SSH Config (On Local Machine)

Append to `~/.ssh/config`:

```sshconfig
Host jump-host
    HostName jump.example.net
    User remoteuser
    ProxyJump relayuser@relay.example.net

Host freebsd-vm          # Change the hostname to anything you like
    HostName localhost
    Port 2222            # Change the port to the one used in run-freebsd.sh
    User root
    ProxyJump jump-host
```

---

## 🔁 SSH Back Into the VM

```sh
ssh freebsd-vm
```

---

## 🛠️ Additional Setup (Inside the VM)

Install helpful tools:

```sh
pkg install tmux htop vim git
```

Configure Vim to use syntax highlighting, smaller tab spaces, and line numbers:

```sh
cat << EOF > ~/.vimrc
syntax on
filetype plugin indent on
set tabstop=4
set shiftwidth=4
set number
EOF
```

---

✅ **Your FreeBSD fuzzing VM is now ready!**
See [[Installing AFL++ on FreeBSD (Source-Only)]]
