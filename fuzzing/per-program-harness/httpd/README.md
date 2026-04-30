## Setting Up the `ptr_checker` Library
```sh
cp -r /path/to/ptr_checker ./ptr_checker
cd ptr_checker

make ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0

export BUFFER_CHECKER_ROOT=$PWD
export AFL_PRELOAD="${BUFFER_CHECKER_ROOT}/libbuffer_check.so"
cd ..
```

## Setting Up `httpd`
```sh
cd /path/to/httpd-source-tree
git checkout 342cb1e
git submodule update --init

# replace PATH/TO with the path to the fuzz-httpd.patch file
patch -p1 < PATH/TO/fuzz-httpd.patch

make -C libimsg/src/lib/libutil
make -C libevent/src/lib/libevent

pw useradd www -d /var/www -s /usr/sbin/nologin -c "WWW user" || true

mkdir -p /var/www/htdocs /var/www/logs

cd src/usr.sbin/httpd
```

Create `httpd.conf` in `src/usr.sbin/httpd/`:
```txt
prefork 1

server "default" {
	listen on * port 80
}
```

## Fuzzing With ASan and Pointer Detection
```sh
mkdir seeds out_asan
dd if=/dev/urandom of=seeds/random_seed bs=1 count=16

AFL_USE_ASAN=1 make CC=afl-clang-lto
afl-fuzz -i seeds -o out_asan -- ./httpd -d -f httpd.conf
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
afl-fuzz -i seeds -o out_msan -- ./httpd -d -f httpd.conf
```

The `.patch` file is located in the directory of this README.

**Note:**
The fuzzing harness runs `httpd` without forking child processes: the server and logger compartments execute in-process alongside the parent. The fuzzer reads a structured input stream from stdin where each message is prefixed by a 5-byte header (1-byte compartment, 1-byte instance, 1-byte type, 2-byte payload length) followed by the payload. The parent dispatches messages to the server (compartment 1) or logger (compartment 2) via `imsg`. Fuzzing ends when stdin is exhausted and both compartments echo back `IMSG_EOM`.
