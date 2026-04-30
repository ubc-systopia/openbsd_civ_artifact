This guide walks you through installing **AFL++ (v4.33c)** on **FreeBSD** (14.2), with support for **source-only fuzzing**. This includes components like `llvm_mode`, `libdislocator`, `libtokencap`, and `radamsa`.

---

## 📦 Step 1: Get AFL++ Source Code

### Clone the Development Version (Latest)
```sh
git clone https://github.com/AFLplusplus/AFLplusplus
cd AFLplusplus/
```

### Or Download the Release Version (v4.33c)
```sh
pkg install wget
wget https://github.com/AFLplusplus/AFLplusplus/archive/refs/tags/v4.33c.tar.gz
tar -xvzf v4.33c.tar.gz
rm v4.33c.tar.gz
cd AFLplusplus-4.33c
```

---

## 🛠️ Step 2: Patch for FreeBSD Compatibility

Edit the following header file:
```sh
vim include/forkserver.h
```

Add this near the top of the file:
```c
#include <sys/types.h>
```

---

## 📥 Step 3: Install Required Packages

```sh
pkg install gmake gcc llvm
```

---

## ⚠️ Notes on FreeBSD Support

- **QEMU mode** and **Frida mode** are **not supported** on FreeBSD.
- **Unicorn mode** requires additional unresolved dependencies.
- Use `make source-only` to build only components relevant to **source-code fuzzing**.

---

## 🧱 Step 4: Build and Install AFL++

```sh
make source-only LDFLAGS="-lexecinfo"
make install LDFLAGS="-lexecinfo"
```

---

## 🔗 Step 5: Create Missing Symlinks (if needed)

If you find that symlinks like `afl-clang-fast` or `afl-clang-lto` are missing from `/usr/local/bin`, add them manually:
```sh
cd /usr/local/bin
ln -sf afl-cc afl-clang-fast
ln -sf afl-cc afl-clang-fast++
ln -sf afl-cc afl-clang-lto
ln -sf afl-cc afl-clang-lto++
```

---

## 🧩 Step 6: Set `AFL_PATH` (if needed)

When running `afl-cc` if you see an error like:
```
[-] PROGRAM ABORT : Unable to find 'afl-compiler-rt.o'. Please set the AFL_PATH environment variable.
         Location : find_built_deps(), src/afl-cc.c:594
```

Then set the `AFL_PATH` environment variable:
```sh
echo 'export AFL_PATH=/root/AFLplusplus-4.33c' >> ~/.profile   
# The path to AFLplusplus-4.33c may be different depending on which directory it was git cloned to
. ~/.profile
```

---

## ✅ Done

You now have **AFL++ (source-only)** installed and ready for fuzzing on FreeBSD!
