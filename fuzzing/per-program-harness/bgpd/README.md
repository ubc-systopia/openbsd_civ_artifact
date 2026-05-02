# OpenBGPD-portable fuzzing harness

## 1. Build the `ptr_checker` library (sanity-check mode)

Place the `ptr_checker` directory next to this README and build it with the pointer-leak detector compiled out for the smoke test.

```sh
cd ptr_checker
make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=0
export BUFFER_CHECKER_ROOT=$PWD
export LD_LIBRARY_PATH="$BUFFER_CHECKER_ROOT"
cd ..
```

`BUFFER_CHECKER_ROOT` is read by the patched `src/bgpd/Makefile.am` to locate `msg_generator.h`, `libmsg_generator.a`, and `libbuffer_check.so`. Rebuild with `ENABLE_PTR_CHECK=1` for the inspection pass in step 5b.

## 2. Install build dependencies (FreeBSD)

```sh
pkg install autoconf automake libtool libevent byacc
pw groupadd _bgpd
pw useradd _bgpd -g _bgpd -s /sbin/nologin -d /var/empty -c 'OpenBGPD daemon'
mkdir -p /usr/local/var/run    # control socket directory bgpd binds to
```

## 3. Clone upstream and apply the harness patch

```sh
git clone https://github.com/openbgpd-portable/openbgpd-portable.git
cd openbgpd-portable
git checkout 7518125c6930d0029901ca4c4e81f7073c0d8013
patch -p1 < PATH/TO/update.sh.patch
./autogen.sh
patch -p1 < PATH/TO/fuzz-bgpd.patch
# Re-bootstrap so Makefile.in regenerates from the patched Makefile.am.
autoreconf -i -f
./configure CPPFLAGS="-I/usr/local/include" LDFLAGS="-L/usr/local/lib"
cp PATH/TO/bgpd.conf /usr/local/etc/bgpd.conf
```

`autogen.sh` must run *before* applying `fuzz-bgpd.patch` because it calls `update.sh`, which copies `bgpd.c` and `bgpd.h` in from the upstream OpenBSD source.

## 4. Sanity-check the harness wiring

```sh
make -C compat

cd src/bgpd
make clean
export AFL_PATH=/path/to/AFLplusplus    # source dir, not install dir
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    make CC=afl-clang-lto

printf '' | ./bgpd -d -f /usr/local/etc/bgpd.conf
echo $?
```

Expected: `EXIT=0` after three `intercepting sendmsg!!!` lines and three `number of eoms received!, 3 expected` lines.

## 5. Fuzz with ASan

```sh
mkdir -p seeds out
dd if=/dev/urandom of=seeds/seed bs=512 count=8

export AFL_PATH=/path/to/AFLplusplus
afl-fuzz -i seeds -o out -m none \
    -- ./bgpd -d -f /usr/local/etc/bgpd.conf
```

`AFL_PATH` must point at the AFL++ source/build directory containing `afl-compiler-rt.o`.

## 5b. Detect cross-compartment pointer leaks (`ENABLE_PTR_CHECK=1`)

```sh
cd $BUFFER_CHECKER_ROOT
make clean
make ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
export AFL_PRELOAD="${BUFFER_CHECKER_ROOT}/libbuffer_check.so"

cd /PATH/TO/openbgpd-portable/src/bgpd
./bgpd -d -f /usr/local/etc/bgpd.conf </dev/null
```

Aborts on the first cross-compartment pointer; this is an inspection pass rather than a productive AFL run.

## 6. Fuzz with MSan

```sh
cd $BUFFER_CHECKER_ROOT
make clean
make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'

cd /PATH/TO/openbgpd-portable/src/bgpd
make clean
AFL_USE_MSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    make CC=afl-clang-lto

mkdir -p out_msan
afl-fuzz -i seeds -o out_msan -m none \
    -- ./bgpd -d -f /usr/local/etc/bgpd.conf
```

`-fsanitize-recover=memory` is baked into `src/bgpd/Makefile.am` so MSan continues past the first uninit hit.
