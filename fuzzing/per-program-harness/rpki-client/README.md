# rpki-client fuzzing harness

## 1. Build the `ptr_checker` library (sanity-check mode)

Place the `ptr_checker` directory next to this README and build it with the pointer-leak detector compiled out for the smoke test.

```sh
cd ptr_checker
make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=0
export BUFFER_CHECKER_ROOT=$PWD
export LD_LIBRARY_PATH="$BUFFER_CHECKER_ROOT"
cd ..
```

`BUFFER_CHECKER_ROOT` is read by the patched `src/Makefile.am` to locate `msg_generator.h`, `libmsg_generator.a`, and `libbuffer_check.so`. Rebuild with `ENABLE_PTR_CHECK=1` for the inspection pass in step 3b.

## 2. Get rpki-client-portable and apply the harness

```sh
git clone https://github.com/rpki-client/rpki-client-portable.git
cd rpki-client-portable
git checkout 71aed40a0e73d1fa0d7e4591a02bd36b58b7c350

patch -p1 < /PATH/TO/update.sh.patch

pkg install autoconf automake libtool libressl expat rsync

pw useradd _rpki-client -d /var/empty -s /usr/sbin/nologin \
    -c "RPKI client" || true

./autogen.sh

mkdir -p /usr/local/etc/rpki
cp ./*.tal ./*.constraints /usr/local/etc/rpki/

patch -p1 < /PATH/TO/fuzz-rpki-client.patch

autoreconf -fi

./configure CPPFLAGS=-I/usr/local/include LDFLAGS=-L/usr/local/lib
```

`update.sh.patch` pins the openbsd source commit before `update.sh` runs. `autogen.sh` clones `rpki-client-openbsd` at the pinned commit, copies sources into `./src`, and applies upstream portability patches; the harness patch is applied on top, then `autoreconf -fi` regenerates `Makefile.in`.

## 3. Fuzz with ASan

```sh
# Wipe stale per-target objects/bitcode so sanitizer state isn't reused.
find . \( -name '*.o' -o -name '*.lo' -o -name '*.a' -o -name '*.la' \
       -o -name '*.lto*' -o -name '*.bc' \) -delete
rm -rf compat/.libs src/.libs

export AFL_PATH=/path/to/AFLplusplus    # source dir, not install dir
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    AUTOMAKE=true AUTOCONF=true ACLOCAL=true AUTOHEADER=true \
    make CC=afl-clang-lto

# Sanity check: empty stdin should exit 0 after dispatching four EOMs.
mkdir -p /tmp/rpki-cache /tmp/rpki-out
printf '' | ./src/rpki-client -d /tmp/rpki-cache /tmp/rpki-out

mkdir -p seeds out_asan
dd if=/dev/urandom of=seeds/seed bs=512 count=8

afl-fuzz -i seeds -o out_asan -m none \
    -- ./src/rpki-client -d /tmp/rpki-cache /tmp/rpki-out
```

`AFL_PATH` must point at the AFL++ source/build directory (the one that contains `afl-compiler-rt.o`). `AUTOMAKE=true …=true` suppresses autotools regeneration mid-build.

## 3b. Detect cross-compartment pointer leaks (`ENABLE_PTR_CHECK=1`)

```sh
cd $BUFFER_CHECKER_ROOT
make clean
make ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
export AFL_PRELOAD="${BUFFER_CHECKER_ROOT}/libbuffer_check.so"

cd /PATH/TO/rpki-client-portable
./src/rpki-client -d /tmp/rpki-cache /tmp/rpki-out </dev/null
```

Aborts on the first imsg payload byte that lands inside a mapped code/data region. Inspection pass only.

## 4. Fuzz with MSan

```sh
cd $BUFFER_CHECKER_ROOT
make clean
make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'

cd /PATH/TO/rpki-client-portable
find . \( -name '*.o' -o -name '*.lo' -o -name '*.a' -o -name '*.la' \
       -o -name '*.lto*' -o -name '*.bc' \) -delete
rm -rf compat/.libs src/.libs
mkdir -p out_msan
AFL_USE_MSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    AUTOMAKE=true AUTOCONF=true ACLOCAL=true AUTOHEADER=true \
    make CC=afl-clang-lto

afl-fuzz -i seeds -o out_msan -m none \
    -- ./src/rpki-client -d /tmp/rpki-cache /tmp/rpki-out
```
