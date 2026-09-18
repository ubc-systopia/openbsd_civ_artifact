# acme-client fileproc harness

Build and run the ASan/UBSan plus pointer-checking configuration on FreeBSD:

```sh
make -C ptr_checker clean
make -C ptr_checker CC=clang ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
make clean
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 make CC=/usr/local/afl++-llvm/bin/afl-clang-lto
AFL_NO_UI=1 AFL_SKIP_CPUFREQ=1 ASAN_OPTIONS=abort_on_error=1:symbolize=0 \
  afl-fuzz -i seeds -o findings-asan -- ./acme-fileproc-harness
```

Build and run the MSan configuration:

```sh
make -C ptr_checker clean
make -C ptr_checker CC=clang ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1
make clean
AFL_USE_MSAN=1 make CC=/usr/local/afl++-llvm/bin/afl-clang-lto CHECKS=msan
AFL_NO_UI=1 AFL_SKIP_CPUFREQ=1 \
  MSAN_OPTIONS=handle_sigbus=0:exit_code=86:symbolize=0 \
  afl-fuzz -i seeds -o findings-msan -- ./acme-fileproc-harness
```

The only OpenBSD source modification is guarded by
`ACME_FILEPROC_FUZZ`.  With that macro disabled, a complete native OpenBSD
`acme-client` build succeeds unchanged.
