# tcpdump fuzzing harness

The source tree under `tcpdump-src/` is a port of OpenBSD's `tcpdump` to FreeBSD with the harness already applied. Uses BSD make (`bsd.prog.mk`); FreeBSD's base build framework is required.

## 1. Build the `ptr_checker` library

Place the `ptr_checker` directory next to this README, then build it with the pointer-leak detector enabled.

```sh
cd ptr_checker
make ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
export BUFFER_CHECKER_ROOT=$PWD
export AFL_PRELOAD="${BUFFER_CHECKER_ROOT}/libbuffer_check.so"
cd ..
```

`BUFFER_CHECKER_ROOT` is read by `tcpdump-src/usr.sbin/tcpdump/Makefile` to locate the headers and link `libmsg_generator.a` and `libbuffer_check.so`.

## 2. Build libpcap and tcpdump

```sh
cd tcpdump-src/lib/libpcap
make
cd ../../usr.sbin/tcpdump

# clean fuzz build (ASan + pointer-leak detection)
make clean
export AFL_PATH=/path/to/AFLplusplus   # source dir, not install dir
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    make CC=afl-clang-lto

# Sanity check: empty stdin should exit 0.
printf '' | ./tcpdump -P

mkdir -p seeds
dd if=/dev/urandom of=seeds/seed bs=512 count=8

mkdir -p out
afl-fuzz -i seeds -o out -m none \
    -- ./tcpdump -P
```

`AFL_PATH` must point at the AFL++ source/build directory (the one that contains `afl-compiler-rt.o`). The `-P` flag tells `tcpdump` to enter the privileged-side path (`priv_exec`) directly.

## 3. Fuzz with MSan

```sh
cd $BUFFER_CHECKER_ROOT
make clean
make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'

cd /PATH/TO/tcpdump-src/usr.sbin/tcpdump
make clean
export AFL_PATH=/path/to/AFLplusplus
AFL_USE_MSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    make CC=afl-clang-lto

mkdir -p out_msan
afl-fuzz -i seeds -o out_msan -m none \
    -- ./tcpdump -P
```
