## 1. Build the `ptr_checker` library

Place the `ptr_checker` directory next to this README, then build it.

```sh
cd ptr_checker
make ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
export BUFFER_CHECKER_ROOT=$PWD
export AFL_PRELOAD="${BUFFER_CHECKER_ROOT}/libbuffer_check.so"
cd ..
```

`BUFFER_CHECKER_ROOT` is read by the build to locate `msg_generator.h` and `libmsg_generator.a`; `AFL_PRELOAD` is read by AFL to load the runtime checker into each forked child.

## 2. Set up `openssh`

```sh
pkg install autoconf automake libressl
git clone https://github.com/openssh/openssh-portable.git
cd openssh-portable
git checkout ae44cd74f3a4ac711152f50b2712803ccf785593

# replace PATH/TO with the path to the respective file
patch -p1 < PATH/TO/fuzz-sshd.patch
tar -xvf PATH/TO/in.tar.gz
mv PATH/TO/sshd-rexec-dump .

# Create a local user used by sshd; defaults are fine for all other fields.
adduser
# Username : fuzzuser
# Password : fuzzuser

autoreconf
CC=afl-clang-lto ./configure --with-kerberos5 --with-pam
```

## 3. Fuzz with ASan and pointer detection

```sh
mkdir out_asan
AFL_USE_ASAN=1 make
afl-fuzz -i in -o out_asan -g 4000 -m none -- $(pwd)/sshd-session -R
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
AFL_USE_MSAN=1 make
afl-fuzz -i in -o out_msan -g 4000 -m none -- $(pwd)/sshd-session -R
```

`MSAN_OPTIONS` lists `exit_code=86` first (AFL refuses to run otherwise) and `exit_code=0` last so MSan parses `0` and the binary exits cleanly on uninit hits.

The `.patch`, `in.tar.gz` and `sshd-rexec-dump` files are located in the directory of this README.
