# radiusd root-helper harnesses

This directory fuzzes the two root helper receivers. Run the BSDAUTH and file helpers separately.

## Root BSDAUTH helper

ASan/UBSan plus pointer checking:

```sh
mkdir -p seeds-bsdauth
cp seeds/seed seeds-bsdauth/seed
make -C ptr_checker clean
make -C ptr_checker ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
make HELPER=bsdauth clean
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 make HELPER=bsdauth \
    CC=/usr/local/afl++-llvm/bin/afl-clang-lto
export LD_LIBRARY_PATH="$PWD:$PWD/ptr_checker"
export AFL_PRELOAD="$PWD/ptr_checker/libbuffer_check.so"
export ASAN_OPTIONS=verify_asan_link_order=0:abort_on_error=1:symbolize=0:detect_leaks=0
afl-fuzz -i seeds-bsdauth -o findings-bsdauth-asan -- \
    ./radiusd-bsdauth-priv-harness
```

MSan:

```sh
make -C ptr_checker clean
make -C ptr_checker ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1
make HELPER=bsdauth clean
AFL_USE_MSAN=1 make HELPER=bsdauth CHECKS=msan \
    CC=/usr/local/afl++-llvm/bin/afl-clang-lto
export LD_LIBRARY_PATH="$PWD:$PWD/ptr_checker"
export AFL_PRELOAD="$PWD/ptr_checker/libbuffer_check.so"
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'
afl-fuzz -i seeds-bsdauth -o findings-bsdauth-msan -- \
    ./radiusd-bsdauth-priv-harness
```

## Root file helper

ASan/UBSan plus pointer checking:

```sh
cc -Wall -Wextra -o generate-file-seed generate_file_seed.c
mkdir -p seeds-file
./generate-file-seed > seeds-file/seed
make -C ptr_checker clean
make -C ptr_checker ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
make HELPER=file clean
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 make HELPER=file \
    CC=/usr/local/afl++-llvm/bin/afl-clang-lto
export LD_LIBRARY_PATH="$PWD:$PWD/ptr_checker"
export AFL_PRELOAD="$PWD/ptr_checker/libbuffer_check.so"
export ASAN_OPTIONS=verify_asan_link_order=0:abort_on_error=1:symbolize=0:detect_leaks=0
afl-fuzz -i seeds-file -o findings-file-asan -- \
    ./radiusd-file-priv-harness
```

MSan:

```sh
make -C ptr_checker clean
make -C ptr_checker ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1
make HELPER=file clean
AFL_USE_MSAN=1 make HELPER=file CHECKS=msan \
    CC=/usr/local/afl++-llvm/bin/afl-clang-lto
export LD_LIBRARY_PATH="$PWD:$PWD/ptr_checker"
export AFL_PRELOAD="$PWD/ptr_checker/libbuffer_check.so"
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'
afl-fuzz -i seeds-file -o findings-file-msan -- \
  ./radiusd-file-priv-harness
```
