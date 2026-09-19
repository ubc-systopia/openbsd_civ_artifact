# ospfd privileged-compartment harness

Copy the `ptr_checker` directory next to this README before building.

ASan/UBSan plus pointer checking on FreeBSD:

```sh
make -C ptr_checker clean
make -C ptr_checker INTERCEPT_SENDMSG=0 INTERCEPT_IMSG_COMPOSE=1 \
    INTERCEPT_IMSG_COMPOSEV=1 ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
make clean
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 make CC=/usr/local/afl++-llvm/bin/afl-clang-lto
export LD_LIBRARY_PATH="$PWD:$PWD/ptr_checker"
export AFL_PRELOAD="$PWD/ptr_checker/libbuffer_check.so"
export ASAN_OPTIONS=verify_asan_link_order=0:abort_on_error=1:symbolize=0:detect_leaks=0
afl-fuzz -i seeds -o findings-asan -- ./ospfd-priv-harness
```

MSan:

```sh
make -C ptr_checker clean
make -C ptr_checker INTERCEPT_SENDMSG=0 INTERCEPT_IMSG_COMPOSE=1 \
    INTERCEPT_IMSG_COMPOSEV=1 ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1
make clean
AFL_USE_MSAN=1 make CHECKS=msan CC=/usr/local/afl++-llvm/bin/afl-clang-lto
export LD_LIBRARY_PATH="$PWD:$PWD/ptr_checker"
export AFL_PRELOAD="$PWD/ptr_checker/libbuffer_check.so"
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'
afl-fuzz -i seeds -o findings-msan -- ./ospfd-priv-harness
```
