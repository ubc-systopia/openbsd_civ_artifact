# mountd fuzzing harness

The source tree under `mountd-src/` is a port of OpenBSD's `mountd` to FreeBSD with the harness already applied. Uses BSD make (`bsd.prog.mk`); FreeBSD's base build framework is required.

## 1. Build the message-generator helper from `ptr_checker`

Place the `ptr_checker` directory next to this README, then build it.

```sh
cd ptr_checker
make
export BUFFER_CHECKER_ROOT=$PWD
cd ..
```

`BUFFER_CHECKER_ROOT` is read by `mountd-src/sbin/mountd/Makefile` to locate `msg_generator.h` and `libmsg_generator.a`.

## 2. Build mountd

```sh
cd mountd-src/sbin/mountd

export AFL_PATH=/path/to/AFLplusplus   # source dir, not install dir
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 make CC=afl-clang-lto

# Sanity check: empty stdin should exit 0.
touch /tmp/empty-exports
printf '' | ./mountd -d /tmp/empty-exports
```

`AFL_PATH` must point at the AFL++ source/build directory (the one that contains `afl-compiler-rt.o`).

## 3. Fuzz with ASan

```sh
mkdir -p in out_asan
dd if=/dev/urandom of=in/seed bs=128 count=4

afl-fuzz -i in -o out_asan -m none -- ./mountd -d /tmp/empty-exports
```

## 4. Fuzz with MSan

```sh
make clean
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'
AFL_USE_MSAN=1 make CC=afl-clang-lto

mkdir -p out_msan
afl-fuzz -i in -o out_msan -m none -- ./mountd -d /tmp/empty-exports
```

`-fsanitize-recover=memory` is baked into `sbin/mountd/Makefile` so MSan continues past the first uninit hit.

## Note

`libbuffer_check.so` is intentionally not loaded: `mountd` uses plain `fork(2)` for privsep, so parent and child share an ASLR layout and there is no cross-compartment boundary for the detector to flag.
