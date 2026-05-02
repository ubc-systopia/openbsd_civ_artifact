# relayd fuzzing harness

## 1. Build the `ptr_checker` library (sanity-check mode)

The `ptr_checker` directory is bundled alongside this README. Build it with the pointer-leak detector compiled out for the productive ASan/MSan passes.

```sh
cd ptr_checker
make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=0
export BUFFER_CHECKER_ROOT=$PWD
export LD_LIBRARY_PATH="$BUFFER_CHECKER_ROOT"
cd ..
```

`BUFFER_CHECKER_ROOT` is read by the patched `usr.sbin/relayd/Makefile` to locate `msg_generator.h`, `libmsg_generator.a`, and `libbuffer_check.so`. Rebuild with `ENABLE_PTR_CHECK=1` for the inspection pass in step 8.

## 2. Install build dependencies (FreeBSD)

```sh
pkg install libpfctl libressl
pw useradd _relayd -d /var/empty -s /usr/sbin/nologin \
    -c "relayd user" || true
```

`libpfctl` provides `<libpfctl.h>` for `pfe.c`; `libressl` provides `<tls.h>` / `libtls.so`.

## 3. Get `freebsd-relayd` and apply the harness

```sh
git clone -b 7.4.2024.01.15 https://github.com/KlaraSystems/freebsd-relayd.git
cd freebsd-relayd
git checkout ec690b778f49ea9ce9955b02f4d4d1337d51e822

# replace PATH/TO with the path to the fuzz-relayd.patch file
patch -p1 < PATH/TO/fuzz-relayd.patch

./configure
```

## 4. Build the bundled libraries

```sh
cd lib/libevent && make
cd ../libutil   && make
cd ../../usr.sbin/relayd
```

These are linked into relayd as static `.a` archives; plain `make` is enough.

## 5. Create a minimal `relayd.conf`

```sh
cat > relayd.conf <<EOF
prefork 1
table <empty> { 127.0.0.1 }
relay default {
    listen on 127.0.0.1 port 8080
    forward to <empty> port 80
}
EOF
```

`prefork 1` keeps the relay/ca instance count to one so the harness only feeds one EOM per compartment.

## 6. Fuzz with ASan

```sh
export AFL_PATH=/path/to/AFLplusplus    # source dir, not install dir
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    make CC=afl-clang-lto

# Sanity check: empty stdin should exit 0 after one IMSG_END_OF_MSGS per compartment.
printf '' | ./relayd -d -f relayd.conf

mkdir -p seeds out_asan
dd if=/dev/urandom of=seeds/seed bs=512 count=4

afl-fuzz -i seeds -o out_asan -m none \
    -- ./relayd -d -f relayd.conf
```

`AFL_PATH` must point at the AFL++ source/build directory (the one that contains `afl-compiler-rt.o`).

## 7. Fuzz with MSan

```sh
cd $BUFFER_CHECKER_ROOT
make clean
make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'

cd /PATH/TO/freebsd-relayd/usr.sbin/relayd
rm -f *.o relayd
AFL_USE_MSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    make CC=afl-clang-lto

mkdir -p out_msan
afl-fuzz -i seeds -o out_msan -m none \
    -- ./relayd -d -f relayd.conf
```

`-fsanitize-recover=memory` is baked into the relayd Makefile so MSan continues past the first uninit hit.

## 8. Detect cross-compartment pointer leaks (`ENABLE_PTR_CHECK=1`)

```sh
cd $BUFFER_CHECKER_ROOT
make clean
make ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
export AFL_PRELOAD="${BUFFER_CHECKER_ROOT}/libbuffer_check.so"

cd /PATH/TO/freebsd-relayd/usr.sbin/relayd
./relayd -d -f relayd.conf </dev/null
```

Aborts on the first cross-compartment pointer; relayd's startup `IMSG_CTL_PROCFD` send trips it on every test case. Inspection pass only.
