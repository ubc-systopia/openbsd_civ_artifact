## Setting Up the `ptr_checker` Library
```sh
cp -r /path/to/ptr_checker ./ptr_checker
cd ptr_checker

make ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0

export BUFFER_CHECKER_ROOT=$PWD
export AFL_PRELOAD="${BUFFER_CHECKER_ROOT}/libbuffer_check.so"
cd ..
```
## Setting Up `openssh`
```sh
pkg install autoconf automake libressl
git clone https://github.com/openssh/openssh-portable.git
cd openssh-portable
git checkout ae44cd74f3a4ac711152f50b2712803ccf785593

# replace PATH/TO with the path to the respective file
patch -p1 < PATH/TO/fuzz-sshd.patch
tar -xvf PATH/TO/in.tar.gz
mv PATH/TO/sshd-rexec-dump .

adduser
# Create a user with the following information (and all other fields can be left as their defaults):
# Username    : fuzzuser
# Password    : example-password

autoreconf
CC=afl-clang-lto ./configure --with-kerberos5 --with-pam 
```
## Fuzzing With ASan and Pointer Detection
```sh
mkdir out_asan

AFL_USE_ASAN=1 make
afl-fuzz -i in -o out_asan -g 4000 -m none -- $(pwd)/sshd-session -R
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
AFL_USE_MSAN=1 make
afl-fuzz -i in -o out_msan -g 4000 -m none -- $(pwd)/sshd-session -R
```

The `.patch`, `in.tar.gz` and `sshd-rexec-dump` files are all located in the directory of this README.
