## Setting Up the `ptr_checker` Library
```sh
cp -r /path/to/ptr_checker ./ptr_checker
cd ptr_checker

make ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0

export BUFFER_CHECKER_ROOT=$PWD
export AFL_PRELOAD="${BUFFER_CHECKER_ROOT}/libbuffer_check.so"
cd ..
```
## Setting Up `ldapd`
```sh
pkg install libressl
git clone --recurse-submodules https://github.com/koue/ldapd.git
cd ldapd
# replace PATH/TO with the path to the respective file
patch -p1 < PATH/TO/fuzz-ldapd.patch

make
# cd src/regress/usr.sbin/ldapd/ && make
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
## Fuzzing With ASan and Pointer Detection
```sh
mkdir out_asan

make clean
AFL_USE_ASAN=1 make CC=afl-clang-lto
afl-fuzz -i seeds -o out_asan -- ./ldapd -d -f ../../etc/examples/ldapd.conf
```
## Fuzzing With MSan
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

The `.patch` file is located in the directory of this README.

**Note:**
The `run_fuzzer.py` file created by the `.patch` file in the `src/usr.sbin/ldapd` directory can be ignored.  It is not needed while fuzzing and was only used in our testing.
No CIVs were found while fuzzing `ldapd`, the crashes produced by the fuzzer are false positives found by the `ptr_checker` library.
 
