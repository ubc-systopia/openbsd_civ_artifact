# OpenIKED-portable fuzzing harness

## 1. Build the `ptr_checker` library (sanity-check mode)

Place the `ptr_checker` directory next to this README and build it with the pointer-leak detector compiled out for the smoke test.

```sh
cd ptr_checker
make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=0
export BUFFER_CHECKER_ROOT=$PWD
export LD_LIBRARY_PATH="$BUFFER_CHECKER_ROOT"
cd ..
```

`BUFFER_CHECKER_ROOT` is read by the patched `iked/CMakeLists.txt` to locate `msg_generator.h`, `libmsg_generator.a`, and `libbuffer_check.so`. Rebuild with `ENABLE_PTR_CHECK=1` for the inspection pass in step 6.

## 2. Install build dependencies (FreeBSD)

```sh
pkg install cmake libressl libevent bison
pw groupadd _iked
pw useradd _iked -g _iked -s /sbin/nologin -d /var/empty -c 'IKEv2 Daemon'
```

## 3. Clone upstream and apply the harness patch

```sh
git clone https://github.com/openiked/openiked-portable.git
cd openiked-portable
git checkout f0da8188ae8b1778dbdd215f7bc30bd8f166aa5e
patch -p1 < PATH/TO/fuzz-iked.patch
cp PATH/TO/iked.conf ./iked.conf
chmod 600 iked.conf
```

## 4. Create certificates

```sh
mkdir -p /usr/local/etc/iked/{ca,certs,private,crls,pubkeys/{ipv4,ipv6,fqdn,ufqdn}}
openssl genrsa -out /usr/local/etc/iked/private/local.key 4096
openssl genrsa -out /usr/local/etc/iked/private/myca.key 4096
openssl req -x509 -new -nodes -key /usr/local/etc/iked/private/myca.key \
    -sha256 -days 3650 -out /usr/local/etc/iked/ca/myca.crt \
    -subj "/C=CA/O=MyCA/CN=My IKEv2 CA"
openssl req -new -key /usr/local/etc/iked/private/local.key \
    -out /tmp/local.csr \
    -subj "/C=CA/ST=State/L=City/O=MyOrg/CN=$(hostname)"
openssl x509 -req -in /tmp/local.csr \
    -CA /usr/local/etc/iked/ca/myca.crt \
    -CAkey /usr/local/etc/iked/private/myca.key -CAcreateserial \
    -out /usr/local/etc/iked/certs/local.crt -days 365 -sha256
mv /usr/local/etc/iked/ca/myca.srl /usr/local/etc/iked/private/
```

## 5. Sanity-check and fuzz with ASan

```sh
mkdir -p build && cd build
export AFL_PATH=/path/to/AFLplusplus    # source dir, not install dir
CC=afl-clang-lto CXX=afl-clang-lto++ LDFLAGS=-lexecinfo \
    cmake -DCMAKE_BUILD_TYPE=Debug ..
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT make -j4

# Sanity check: empty stdin should exit 0 after one IMSG_END_OF_MSGS per compartment.
printf '' | ./iked/iked -d -f ../iked.conf
echo $?

mkdir -p seeds out
dd if=/dev/urandom of=seeds/seed bs=512 count=8

afl-fuzz -i seeds -o out -m none \
    -- ./iked/iked -d -f ../iked.conf
```

`AFL_PATH` must point at the AFL++ source/build directory (the one that contains `afl-compiler-rt.o`). `LDFLAGS=-lexecinfo` is needed on FreeBSD because AFL++'s `afl-compiler-rt.o` references `backtrace()` which lives in `libexecinfo.so`.

## 6. Detect cross-compartment pointer leaks (`ENABLE_PTR_CHECK=1`)

```sh
cd $BUFFER_CHECKER_ROOT
make clean
make ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
export AFL_PRELOAD="${BUFFER_CHECKER_ROOT}/libbuffer_check.so"

cd /PATH/TO/openiked-portable/build
./iked/iked -d -f ../iked.conf </dev/null
```

Aborts on the first cross-compartment pointer; suppress false positives via `ptr_check_skip()`.

## 7. Fuzz with MSan

```sh
cd $BUFFER_CHECKER_ROOT
make clean
make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'

cd /PATH/TO/openiked-portable/build
rm -rf *
CC=afl-clang-lto CXX=afl-clang-lto++ LDFLAGS=-lexecinfo \
    cmake -DCMAKE_BUILD_TYPE=Debug ..
AFL_USE_MSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT make -j4

mkdir -p out_msan
afl-fuzz -i seeds -o out_msan -m none \
    -- ./iked/iked -d -f ../iked.conf
```

`-fsanitize-recover=memory` is baked into `iked/CMakeLists.txt`'s `CFLAGS` so MSan continues past the first uninit hit.
