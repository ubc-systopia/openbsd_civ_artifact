Sometimes, even if a program can be successfully built individually, it may exhibit strange runtime issues (as I experienced with radiusd). In such scenarios, a complete OS build may be required to ensure the program functions properly. I suspect the reason for these issues is that the custom build instructions do not match the original build arguments, leading to unexpected behavior.

The benefit of building the entire OpenBSD system (kernel + userland) is that it is a one-time effort. Once you have built the entire system, you can simply call `make` to compile any userland programs as many times as needed. I highly recommend investing the time to build the entire system upfront to avoid future complications with tweaking build parameters.

The following instructions were written by ChatGPT.
However, if you are encountering errors refer to the official documentation:
https://man.openbsd.org/release

# 🛠️ OpenBSD: Compile Latest Kernel and Userland (No Install)

## ✅ Assumptions
- Running as **root**
- Using **OpenBSD 7.6**

---

## 📁 Step 1: Prepare Build Directories

```sh
mkdir -p /usr/src /usr/obj
export MAKEOBJDIRPREFIX=/usr/obj
export BUILDUSER=$(whoami)
```

---

## 🌐 Step 2: Clone Latest OpenBSD Source

```sh
cd /usr
git clone --depth=1 https://github.com/openbsd/src.git
```

---

## 🧱 Step 3: Build the Kernel

```sh
cd /usr/src/sys/arch/$(machine)/conf
config GENERIC.MP
cd ../compile/GENERIC.MP
make clean
make -j$(sysctl -n hw.ncpu)
make install
reboot
```

---

## 🏗️ Step 4: Build the Userland

```sh
export MAKEOBJDIRPREFIX=/usr/obj
export BUILDUSER=$(whoami)
cd /usr/src
make cleandir
make -j$(sysctl -n hw.ncpu) obj
make -j$(sysctl -n hw.ncpu) build
```

> This can take 30 minutes to several hours depending on your hardware.

---

## 🧼 Done — Kernel and Userland Are Compiled

- `bsd` kernel binary: `/usr/src/sys/arch/$(machine)/compile/GENERIC/bsd`
- Compiled userland lives under `/usr/obj`

---
