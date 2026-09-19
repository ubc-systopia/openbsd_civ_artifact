# dhcpd fuzzing harness

## 1. Build the `ptr_checker` helper

Place the `ptr_checker` directory next to this README, then build it. The harness links both artifacts it produces: `libmsg_generator.a` (the stdin-driven message generator) and `libbuffer_check.so` (which `libmsg_generator.a` depends on for `ptr_check_skip()`).

```sh
cd ptr_checker
make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=0
export BUFFER_CHECKER_ROOT=$PWD
cd ..
```

`BUFFER_CHECKER_ROOT` is read by the patched `src/usr.sbin/dhcpd/Makefile` to locate `msg_generator.h`, `libmsg_generator.a`, and `libbuffer_check.so`. The resulting `dhcpd` binary carries an rpath to this directory, so no `LD_LIBRARY_PATH` is needed at run time — but do not move or delete `ptr_checker` after building.

## 2. Install build dependencies (FreeBSD)

```sh
pkg install git afl++-llvm
export PATH=/usr/local/afl++-llvm/bin:$PATH   # pkg installs AFL++ outside the default PATH
kldload pf                              # /dev/pf must exist for pftable_handler
echo 'subnet 192.168.122.0 netmask 255.255.255.0 {
  range 192.168.122.100 192.168.122.200;
  option domain-name "home.example";
  option domain-name-servers 8.8.8.8, 8.8.4.4;
  option routers 192.168.122.1;
}' > /etc/dhcpd.conf
```

`pftable_handler` opens `/dev/pf` at startup; without `kldload pf` the binary exits before the harness loop runs. `dhcpd` also requires the `_dhcp` user, which is part of the FreeBSD base system.

Do not set `AFL_PATH` to an AFL++ source tree: the packaged `afl-cc` locates its runtime objects under `/usr/local/afl++-llvm/lib/afl` on its own.

## 3. Clone upstream and apply the harness patch

```sh
git clone https://github.com/koue/dhcpd.git
cd dhcpd
git checkout 43fc6d3cb6cac3c8bac8bf6f24dcbb764ced614b
patch -p1 < PATH/TO/fuzz-dhcpd.patch
```

## 4. Sanity-check the harness wiring

```sh
cd src/usr.sbin/dhcpd
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    make CC=afl-clang-lto

printf '' | ./dhcpd -d
echo $?
```

Expected: a `Listening on <iface> (<addr>).` line, then `1 number of eoms received!, 1 expected`, and an exit status of `0`.

If the link step fails with `undefined symbol: ptr_check_skip`, the patched Makefile is not linking `libbuffer_check.so` — re-apply `fuzz-dhcpd.patch` and check that `BUFFER_CHECKER_ROOT` points at the built `ptr_checker` directory.

## 5. Fuzz

```sh
mkdir -p seeds out
dd if=/dev/urandom of=seeds/seed bs=512 count=8

afl-fuzz -i seeds -o out -m none -- ./dhcpd -d
```

## Note

`libbuffer_check.so` is linked into the binary only to satisfy `libmsg_generator.a`'s reference to `ptr_check_skip()`; it is not preloaded and, with `ENABLE_PTR_CHECK=0`, its `check_buffer()` is a no-op. Pointer checking is deliberately off: upstream `dhcpd` privsep uses plain `fork(2)` (no `exec`), so parent and child share an ASLR layout and pointer-shaped IPC values are valid on the receiving side. Fuzz with ASan + UBSan only.
