# syslogd fuzzing harness

The source tree under `syslogd-src/` is a self-contained snapshot of an OpenBSD `syslogd` port to FreeBSD with the harness already applied.

Three sibling source directories share one harness layout:

- `usr.sbin/syslogd-asan/` — fuzzed with `AFL_USE_ASAN=1 AFL_USE_UBSAN=1`.
- `usr.sbin/syslogd-normal/` — non-sanitized build, used as AFL `-S` secondaries.
- `usr.sbin/syslogd-coverage/` — `--coverage` build for line-coverage measurement.

## 1. Install build dependencies and create the privsep user

```sh
pkg install libressl
pw useradd _syslogd -d /var/empty -s /usr/sbin/nologin -c 'syslogd privsep'
```

`libressl` provides `<tls.h>`/`libtls.so.*`. `_syslogd` must exist because `priv_exec` calls `getpwnam("_syslogd")` at startup.

## 2. Build the message-generator helper from `ptr_checker`

Place `ptr_checker` next to this README, then build it.

```sh
cd ptr_checker
make
export BUFFER_CHECKER_ROOT=$PWD
cd ..
```

`BUFFER_CHECKER_ROOT` is read by each `usr.sbin/syslogd-*` Makefile to locate `msg_generator.h` and link `libmsg_generator.a`.

## 3. Build the bundled libevent

```sh
cd syslogd-src/libevent/src/lib/libevent
make
cd -
```

## 4. Fuzz with ASan

```sh
cd syslogd-src/usr.sbin/syslogd-asan
export AFL_PATH=/path/to/AFLplusplus     # source dir, not install dir
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    make CC=afl-clang-lto

# Sanity check: empty stdin should print "quit event loop!" and exit 0.
printf '' | ./syslogd -P 1234

mkdir -p in out_asan
dd if=/dev/urandom of=in/seed bs=512 count=8

afl-fuzz -i in -o out_asan -m none -- ./syslogd -P 1234
```

`AFL_PATH` must point at the AFL++ source/build directory (the one that contains `afl-compiler-rt.o`). The `-P 1234` flag steers `main()` directly into `priv_exec` so both privsep sides stay in one process.

## 5. Fuzz with MSan

```sh
make clean
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'
AFL_USE_MSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    make CC=afl-clang-lto

mkdir -p out_msan
afl-fuzz -i in -o out_msan -m none -- ./syslogd -P 1234
```

`-fsanitize-recover=memory` is baked into each Makefile so MSan continues past the first uninit hit.
