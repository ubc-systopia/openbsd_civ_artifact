# file privileged-compartment harness patch

This artifact intentionally contains a patch rather than a modified source
tree.  It targets brynet/file commit
`0ed35aae2d28d92820af7f49c43606a68ca5d0aa`.  The guarded patch retains the
project's original `main()` and fuzzes its privileged parent-side imsg handler.
This protocol has no message-type enum: production uses type `-1` for its ACK,
so the generator covers that value and `0` as an invalid type.

Prepare the source tree on FreeBSD without modifying this artifact directory:

```sh
git clone https://github.com/brynet/file.git
cd file
git checkout 0ed35aae2d28d92820af7f49c43606a68ca5d0aa
patch -p1 < /path/to/artifact/file/file-civ-fuzz.patch
cp -R /path/to/artifact/file/ptr_checker ./ptr_checker
./autogen.sh
./configure --enable-civ-fuzz CPPFLAGS=-I/usr/local/include \
  LDFLAGS=-L/usr/local/lib
cp -R /path/to/artifact/file/seeds ./seeds
```

Build and fuzz with ASan/UBSan plus pointer checking:

```sh
make -C ptr_checker clean
make -C ptr_checker ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
make clean
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 make CC=/usr/local/afl++-llvm/bin/afl-clang-lto
export LD_LIBRARY_PATH="$PWD:$PWD/ptr_checker"
export AFL_PRELOAD="$PWD/ptr_checker/libbuffer_check.so"
export ASAN_OPTIONS=verify_asan_link_order=0:abort_on_error=1:symbolize=0:detect_leaks=0
afl-fuzz -i seeds -o findings-asan -- ./file
```

For MSan, rebuild the checker and target with MSan flags:

```sh
make -C ptr_checker clean
make -C ptr_checker ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=1
make clean
AFL_USE_MSAN=1 make CC=/usr/local/afl++-llvm/bin/afl-clang-lto \
  CFLAGS='-O1 -g -fsanitize=memory -fsanitize-recover=memory' \
  LDFLAGS='-L/usr/local/lib -fsanitize=memory'
export LD_LIBRARY_PATH="$PWD:$PWD/ptr_checker"
export AFL_PRELOAD="$PWD/ptr_checker/libbuffer_check.so"
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'
afl-fuzz -i seeds -o findings-msan -- ./file
```
