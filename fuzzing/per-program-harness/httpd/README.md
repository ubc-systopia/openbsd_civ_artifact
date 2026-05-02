# httpd fuzzing harness

## 1. Build the `ptr_checker` library (sanity-check mode)

Place the `ptr_checker` directory next to this README and build it with the pointer-leak detector compiled out for the smoke test.

```sh
cd ptr_checker
make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=0
export BUFFER_CHECKER_ROOT=$PWD
export LD_LIBRARY_PATH="$BUFFER_CHECKER_ROOT"
cd ..
```

`BUFFER_CHECKER_ROOT` is read by the patched `src/usr.sbin/httpd/Makefile` to locate `msg_generator.h`, `libmsg_generator.a`, and `libbuffer_check.so`. The library is rebuilt with `ENABLE_PTR_CHECK=1` in step 5b.

## 2. Install build dependencies (FreeBSD)

```sh
pkg install libressl
pw useradd www -d /var/www -s /usr/sbin/nologin -c "WWW user" || true
mkdir -p /var/www/htdocs /var/www/logs
```

`libressl` provides `<tls.h>` / `libtls.so`.

## 3. Get `httpd` and apply the harness

```sh
git clone https://github.com/koue/httpd.git
cd httpd
git checkout 95821ec8ae1c2bacbfc9d628a28b5c06a1d6f60f
git submodule update --init

# replace PATH/TO with the path to the fuzz-httpd.patch file
patch -p1 < PATH/TO/fuzz-httpd.patch

make -C libimsg/src/lib/libutil
make -C libevent/src/lib/libevent

cd src/usr.sbin/httpd
```

Create `httpd.conf` in `src/usr.sbin/httpd/`:

```txt
prefork 1

server "default" {
	listen on * port 80
}
```

## 4. Sanity-check the harness wiring

```sh
export AFL_PATH=/path/to/AFLplusplus    # source dir, not install dir
make clean
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    make CC=afl-clang-lto

printf '' | ./httpd -d -f httpd.conf
echo $?
```

Expected: two `number of eoms received!, 2 expected` lines followed by `EXIT=0`. `AFL_PATH` must point at the AFL++ source/build directory containing `afl-compiler-rt.o`.

## 5. Fuzz with ASan

```sh
mkdir -p seeds out_asan
dd if=/dev/urandom of=seeds/seed bs=512 count=4

afl-fuzz -i seeds -o out_asan -m none -- ./httpd -d -f httpd.conf
```

## 5b. Detect cross-compartment pointer leaks (`ENABLE_PTR_CHECK=1`)

Rebuild `ptr_checker` with the detector enabled and re-run the same binary.

```sh
cd $BUFFER_CHECKER_ROOT
make clean
make ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
export AFL_PRELOAD="${BUFFER_CHECKER_ROOT}/libbuffer_check.so"

cd /PATH/TO/httpd/src/usr.sbin/httpd
./httpd -d -f httpd.conf </dev/null
```

The detector aborts on the first cross-compartment pointer it sees. This is an inspection pass rather than a productive AFL run.

## 6. Fuzz with MSan

```sh
cd $BUFFER_CHECKER_ROOT
make clean
make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=0
unset AFL_PRELOAD
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'

cd /PATH/TO/httpd/src/usr.sbin/httpd
make clean
AFL_USE_MSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    make CC=afl-clang-lto

mkdir -p out_msan
afl-fuzz -i in -o out_msan -m none -- ./httpd -d -f httpd.conf
```

`MSAN_OPTIONS` lists `exit_code=86` first (AFL refuses to run otherwise) and `exit_code=0` last so MSan parses `0` and the binary exits cleanly on uninit hits.
