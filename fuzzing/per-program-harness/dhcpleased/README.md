# dhcpleased privileged-compartment harness

This directory fuzzes the original `dhcpleased` parent dispatcher through two
synthetic imsg endpoints representing the unprivileged frontend and engine.
The guarded harness remains in the original `main()` and preserves the
parent-to-child setup messages.  Vendored imsg is built as `libprivimsg.so` so
the checker can dynamically intercept `imsg_compose()` and `imsg_composev()`.
The generator covers the complete `enum imsg_type` range and one value beyond
that range; the privileged dispatchers decide which messages they accept.

On FreeBSD, build and fuzz the ASan/UBSan plus pointer-checking configuration:

```sh
make -C ptr_checker clean
make -C ptr_checker ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
make clean
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 make CC=/usr/local/afl++-llvm/bin/afl-clang-lto
export LD_LIBRARY_PATH="$PWD:$PWD/ptr_checker"
export AFL_PRELOAD="$PWD/ptr_checker/libbuffer_check.so"
export ASAN_OPTIONS=verify_asan_link_order=0:abort_on_error=1:symbolize=0:detect_leaks=0
afl-fuzz -i seeds -o findings-asan -- ./dhcpleased-priv-harness
```

Build and fuzz with MSan in the artifact's documented recover mode:

```sh
make -C ptr_checker clean
make -C ptr_checker ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1
make clean
AFL_USE_MSAN=1 make CHECKS=msan CC=/usr/local/afl++-llvm/bin/afl-clang-lto
export LD_LIBRARY_PATH="$PWD:$PWD/ptr_checker"
export AFL_PRELOAD="$PWD/ptr_checker/libbuffer_check.so"
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'
afl-fuzz -i seeds -o findings-msan -- ./dhcpleased-priv-harness
```
