# snmpd privileged-compartment harness

This directory fuzzes the original privileged `snmpd` parent dispatcher under
the `SNMPD_PRIV_ONLY` guard.  Its vendored imsg implementation is built as
`libprivimsg.so`, making outbound compose operations dynamically visible to the
checker.  The generator covers every production imsg type and one invalid
type.  Trap payload parsing is excluded because production performs it only
in a forked child after dropping privileges to `_snmpd`.

Create a seed if needed, then run ASan/UBSan plus pointer checking on FreeBSD:

```sh
mkdir -p seeds
test -f seeds/seed || dd if=/dev/urandom of=seeds/seed bs=512 count=4
make -C ptr_checker clean
make -C ptr_checker ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
make clean
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 make CC=/usr/local/afl++-llvm/bin/afl-clang-lto
export LD_LIBRARY_PATH="$PWD:$PWD/ptr_checker"
export AFL_PRELOAD="$PWD/ptr_checker/libbuffer_check.so"
export ASAN_OPTIONS=verify_asan_link_order=0:abort_on_error=1:symbolize=0:detect_leaks=0
afl-fuzz -i seeds -o findings-asan -- ./snmpd-priv-harness
```

MSan:

```sh
make -C ptr_checker clean
make -C ptr_checker ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1
make clean
AFL_USE_MSAN=1 make CHECKS=msan CC=/usr/local/afl++-llvm/bin/afl-clang-lto
export LD_LIBRARY_PATH="$PWD:$PWD/ptr_checker"
export AFL_PRELOAD="$PWD/ptr_checker/libbuffer_check.so"
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'
afl-fuzz -i seeds -o findings-msan -- ./snmpd-priv-harness
```
