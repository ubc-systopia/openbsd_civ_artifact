## 1. Build the `ptr_checker` library

Place the `ptr_checker` directory next to this README, then build it.

```sh
cd ptr_checker
make ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
export BUFFER_CHECKER_ROOT=$PWD
export AFL_PRELOAD="${BUFFER_CHECKER_ROOT}/libbuffer_check.so"
cd ..
```

`BUFFER_CHECKER_ROOT` is read by the build; `AFL_PRELOAD` loads the runtime checker into each AFL fork.

## 2. Set up `ldapd`

```sh
pkg install libressl
git clone --recurse-submodules https://github.com/koue/ldapd.git
cd ldapd
# replace PATH/TO with the path to the respective file
patch -p1 < PATH/TO/fuzz-ldapd.patch

make
mkdir /usr/local/man && mkdir /usr/local/man/man3
make install
chmod 600 src/etc/examples/ldapd.conf
mkdir /etc/ldap
cp src/usr.sbin/ldapd/schema/* /etc/ldap
cd src/usr.sbin/ldapd/
mkdir /var/db/ldap

pw useradd ldap -d /var/empty -s /usr/sbin/nologin -c "ldap Daemon"

mkdir seeds
dd if=/dev/urandom of=seeds/random_seed bs=1 count=16
```

## 3. Fuzz with ASan and pointer detection

```sh
mkdir out_asan
make clean
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 make CC=afl-clang-lto
afl-fuzz -i seeds -o out_asan -- ./ldapd -d -f ../../etc/examples/ldapd.conf
```

## 4. Fuzz with MSan

```sh
cd $BUFFER_CHECKER_ROOT
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'
make clean
make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1

cd -
mkdir out_msan
make clean
AFL_USE_MSAN=1 make CC=afl-clang-lto
afl-fuzz -i seeds -o out_msan -- ./ldapd -d -f ../../etc/examples/ldapd.conf
```

The `.patch` file is located in the directory of this README. The `run_fuzzer.py` file dropped into `src/usr.sbin/ldapd` by the patch can be ignored.
