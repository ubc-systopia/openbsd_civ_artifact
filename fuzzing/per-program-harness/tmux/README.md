## 1. Get `tmux`

```sh
git clone https://github.com/tmux/tmux.git
cd tmux
git checkout 86153fbba9cebaf4ecc95fa43a0853ef65c737e6
```

## 2. Set up the `ptr_checker` library

Place the `ptr_checker` directory next to this README, then build it.

```sh
cd ptr_checker
make ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0

export BUFFER_CHECKER_ROOT=$PWD
export AFL_PRELOAD="/lib/libthr.so.3:${BUFFER_CHECKER_ROOT}/libbuffer_check.so"
export LD_PRELOAD="$AFL_PRELOAD"
export LD_LIBRARY_PATH="${BUFFER_CHECKER_ROOT}"
cd -
```

`BUFFER_CHECKER_ROOT` must be exported before configuring `tmux`. `/lib/libthr.so.3` must come **first** in `AFL_PRELOAD` to avoid an ASan early-init `pthread_key_create()` CHECK abort on FreeBSD.

## 3. Apply the harness

```sh
# replace PATH/TO with the path to the respective file
patch -p1 < PATH/TO/fuzz-tmux.patch

pkg install autoconf automake pkgconf libevent ncurses

sh autogen.sh
./configure
```

Copy a seed corpus into place:

```sh
mkdir -p in
cp PATH/TO/in/random_seed in/
```

## 4. Fuzz with ASan and pointer detection

```sh
mkdir -p out_asan

AFL_USE_ASAN=1 AFL_USE_UBSAN=1 make CC=afl-clang-lto

afl-fuzz -i in -o out_asan -g 1024 -m none -- ./tmux -S ./tmux-socket
```

`AFL_PRELOAD`, `BUFFER_CHECKER_ROOT`, and `LD_LIBRARY_PATH` from step 2 must be in the environment. `-g 1024` matches the per-message budget in `msg_generator.h`.

## 5. Fuzz with MSan

```sh
cd $BUFFER_CHECKER_ROOT
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'
make clean
make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1

cd -
mkdir -p out_msan
make clean
AFL_USE_MSAN=1 make CC=afl-clang-lto
afl-fuzz -i in -o out_msan -g 1024 -m none -- ./tmux -S ./tmux-socket
```

The `fuzz-tmux.patch` file is located in the directory of this README.
