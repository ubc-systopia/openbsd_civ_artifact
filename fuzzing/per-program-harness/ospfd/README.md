# ospfd privileged-compartment harness

This directory fuzzes the original privileged `ospfd` parent handlers using
synthetic imsg traffic from the unprivileged compartments.  The harness is
guarded by `OSPFD_PRIV_ONLY`; vendored imsg remains dynamically interposable.
The generator covers the complete `enum imsg_type` range and one value beyond
that range; the privileged parent decides which messages each peer may send.
Sanitizer terminations are findings and must be triaged against OpenBSD rather
than patched merely to keep AFL running.

ASan/UBSan plus pointer checking on FreeBSD:

```sh
make -C ptr_checker clean
make -C ptr_checker ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
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
make -C ptr_checker ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1
make clean
AFL_USE_MSAN=1 make CHECKS=msan CC=/usr/local/afl++-llvm/bin/afl-clang-lto
export LD_LIBRARY_PATH="$PWD:$PWD/ptr_checker"
export AFL_PRELOAD="$PWD/ptr_checker/libbuffer_check.so"
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'
afl-fuzz -i seeds -o findings-msan -- ./ospfd-priv-harness
```
