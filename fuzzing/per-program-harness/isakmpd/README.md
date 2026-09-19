# isakmpd privileged-monitor fuzzing harness

ASan/UBSan plus pointer checking:

```sh
make -C ptr_checker clean
make -C ptr_checker CC=clang ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
make clean
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 \
  make CC=/usr/local/afl++-llvm/bin/afl-clang-lto
cpuset -l 0 env AFL_SKIP_CPUFREQ=1 AFL_NO_AFFINITY=1 \
  ASAN_OPTIONS=abort_on_error=1:symbolize=0 \
  afl-fuzz -i seeds -o findings-asan -- ./isakmpd-priv-harness
```

MSan:

```sh
make -C ptr_checker clean
make -C ptr_checker CC=clang ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1
make clean
AFL_USE_MSAN=1 \
  make CC=/usr/local/afl++-llvm/bin/afl-clang-lto CHECKS=msan
cpuset -l 0 env AFL_SKIP_CPUFREQ=1 \
  MSAN_OPTIONS=handle_sigbus=0:exit_code=86:symbolize=0 \
  afl-fuzz -i seeds -o findings-msan -- ./isakmpd-priv-harness
```
