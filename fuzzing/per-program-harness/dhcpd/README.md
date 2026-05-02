# dhcpd fuzzing harness

## 1. Build the `ptr_checker` helper

Place the `ptr_checker` directory next to this README, then build it. Only `libmsg_generator.a` is consumed by the harness.

```sh
cd ptr_checker
make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=0
export BUFFER_CHECKER_ROOT=$PWD
cd ..
```

`BUFFER_CHECKER_ROOT` is read by the patched `src/usr.sbin/dhcpd/Makefile` to locate `msg_generator.h` and `libmsg_generator.a`.

## 2. Install build dependencies (FreeBSD)

```sh
pkg install git
kldload pf                              # /dev/pf must exist for pftable_handler
echo 'subnet 192.168.122.0 netmask 255.255.255.0 {
  range 192.168.122.100 192.168.122.200;
  option domain-name "home.example";
  option domain-name-servers 8.8.8.8, 8.8.4.4;
  option routers 192.168.122.1;
}' > /etc/dhcpd.conf
```

`pftable_handler` opens `/dev/pf` at startup; without `kldload pf` the binary exits before the harness loop runs.

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
export AFL_PATH=/path/to/AFLplusplus    # source dir, not install dir
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    make CC=afl-clang-lto

printf '' | ./dhcpd -d
echo $?
```

Expected: `1 number of eoms received!, 1 expected` followed by `EXIT=0`.

## 5. Fuzz

```sh
mkdir -p seeds out
dd if=/dev/urandom of=seeds/seed bs=512 count=8

afl-fuzz -i seeds -o out -m none -- ./dhcpd -d
```

## Note

`libbuffer_check.so` is not used here: upstream `dhcpd` privsep uses plain `fork(2)` (no `exec`), so parent and child share an ASLR layout and pointer-shaped IPC values are valid on the receiving side. Fuzz with ASan + UBSan only.
