# OpenNTPD-portable fuzzing harness

## 1. Install build dependencies and create the privsep user (FreeBSD)

```sh
pkg install autoconf automake libtool libressl
pw useradd _ntp -d /var/empty -s /usr/sbin/nologin -c 'OpenNTPD daemon'
```

`libressl` provides `<tls.h>` / `libtls.so.*`. `_ntp` must exist because `ntpd` calls `getpwnam("_ntp")` at startup.

Write `/usr/local/etc/ntpd.conf` with **exactly one** `constraint` line:

```sh
cat > /usr/local/etc/ntpd.conf <<'EOF'
servers pool.ntp.org
constraint from "9.9.9.9"
EOF
```

The harness hardcodes `constraint_cnt = 1`; multiple `constraint` lines either hang the run or trigger an ASan heap-buffer-overflow on the upstream-shipped config.

## 2. Clone upstream and apply the fuzz patch

```sh
git clone https://github.com/openntpd-portable/openntpd-portable.git
cd openntpd-portable
git checkout cf202d1
patch -p1 < /path/to/openntpd-fuzz.patch
```

## 3. Build the `ptr_checker` library

Place `ptr_checker` next to this tree, then build it.

```sh
cd ptr_checker
make ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
export BUFFER_CHECKER_ROOT=$PWD
cd ..
```

`BUFFER_CHECKER_ROOT` is read by the patched `src/Makefile.am` for `msg_generator.h` and `libmsg_generator.a`.

## 4. Bootstrap and configure

```sh
cd openntpd-portable
./autogen.sh
./configure CPPFLAGS=-I/usr/local/include LDFLAGS=-L/usr/local/lib
```

`autogen.sh` runs the patched `update.sh`, which clones `openntpd-openbsd`, checks out the pinned commit, copies in the source files, and applies every `patches/*.patch`.

## 5. Fuzz with ASan + pointer-leak detection

```sh
export AFL_PATH=/path/to/AFLplusplus     # source dir, not install dir
AFL_USE_ASAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    make CC=afl-clang-lto

# Sanity check: empty stdin should exit 0 within ~10ms.
printf '' | LD_PRELOAD=$BUFFER_CHECKER_ROOT/libbuffer_check.so ./src/ntpd -d

mkdir -p in out_asan
dd if=/dev/urandom of=in/seed bs=512 count=1

AFL_PRELOAD=$BUFFER_CHECKER_ROOT/libbuffer_check.so \
    afl-fuzz -i in -o out_asan -m none -- ./src/ntpd -d
```

`AFL_PATH` must point at the AFL++ source/build directory (the one that contains `afl-compiler-rt.o`). `AFL_PRELOAD` loads `libbuffer_check.so` into each forked child so every `sendmsg(2)` is scanned for pointer-shaped values.

## 6. Fuzz with MSan

```sh
cd $BUFFER_CHECKER_ROOT
make clean
make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'

cd /path/to/openntpd-portable
make clean
AFL_USE_MSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    make CC=afl-clang-lto

mkdir -p out_msan
afl-fuzz -i in -o out_msan -m none -- ./src/ntpd -d
```

`-fsanitize-recover=memory` is baked into `src/Makefile.am` so MSan continues past the first uninit hit.
