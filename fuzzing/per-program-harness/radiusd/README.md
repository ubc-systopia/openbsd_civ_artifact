# radiusd root-helper harnesses

This directory fuzzes the two root helper receivers.  The generic `radiusd`
main process is not a target: it drops to `_radiusd` and is not the most
privileged compartment.  Run the BSDAUTH and file helpers separately.

Copy the `ptr_checker` directory next to this README before building either
helper.

## Root BSDAUTH helper

This target exercises only the root `radiusd_bsdauth` receiver.  It retains
the helper's original `main()` and dispatch loop.  The generator covers the
complete contiguous helper enum from `IMSG_BSDAUTH_OK` through
`IMSG_BSDAUTH_GROUPCHECK`, plus one invalid type.

ASan/UBSan plus pointer checking:

```sh
mkdir -p seeds-bsdauth
cp seeds/seed seeds-bsdauth/seed
make -C ptr_checker clean
make -C ptr_checker INTERCEPT_SENDMSG=0 INTERCEPT_IMSG_COMPOSE=1 \
    INTERCEPT_IMSG_COMPOSEV=1 ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
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
make -C ptr_checker INTERCEPT_SENDMSG=0 INTERCEPT_IMSG_COMPOSE=1 \
    INTERCEPT_IMSG_COMPOSEV=1 ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1
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

This target exercises only the root `radiusd_file` receiver.  The generator
maps the complete contiguous enum range from `IMSG_RADIUSD_FILE_OK` through
`IMSG_RADIUSD_FILE_USERINFO`, plus one invalid type.  Its first generated
message is consumed by the production startup check, and subsequent generated
messages enter the normal dispatcher.  This includes the child-controlled
`IMSG_RADIUSD_FILE_PARAMS` startup boundary instead of synthesizing trusted
parameters.

ASan/UBSan plus pointer checking:

```sh
cc -Wall -Wextra -o generate-file-seed generate_file_seed.c
mkdir -p seeds-file
./generate-file-seed > seeds-file/seed
make -C ptr_checker clean
make -C ptr_checker INTERCEPT_SENDMSG=0 INTERCEPT_IMSG_COMPOSE=1 \
    INTERCEPT_IMSG_COMPOSEV=1 ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
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
make -C ptr_checker INTERCEPT_SENDMSG=0 INTERCEPT_IMSG_COMPOSE=1 \
    INTERCEPT_IMSG_COMPOSEV=1 ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1
make HELPER=file clean
AFL_USE_MSAN=1 make HELPER=file CHECKS=msan \
    CC=/usr/local/afl++-llvm/bin/afl-clang-lto
export LD_LIBRARY_PATH="$PWD:$PWD/ptr_checker"
export AFL_PRELOAD="$PWD/ptr_checker/libbuffer_check.so"
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'
afl-fuzz -i seeds-file -o findings-file-msan -- \
  ./radiusd-file-priv-harness
```
