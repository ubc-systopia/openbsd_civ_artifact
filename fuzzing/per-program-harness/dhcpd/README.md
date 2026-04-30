## Getting `dhcpd`

Clone the repository, enter the source tree, and check out the revision used for this artifact:

```sh
git clone https://github.com/koue/dhcpd.git
cd dhcpd
git checkout 43fc6d3cb6cac3c8bac8bf6f24dcbb764ced614b
```

Set up the `ptr_checker` library:

```sh
cp -r /path/to/ptr_checker ./ptr_checker
cd ptr_checker

make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=0
export BUFFER_CHECKER_ROOT=$PWD
export AFL_PRELOAD="${BUFFER_CHECKER_ROOT}/libbuffer_check.so"
cd ..
```

Create `/etc/dhcpd.conf`, add the following content:

```txt
subnet 192.168.122.0 netmask 255.255.255.0 {
  range 192.168.122.100 192.168.122.200;
  option domain-name "home.ridgway.io";
  option domain-name-servers 8.8.8.8, 8.8.4.4;
  option routers 192.168.122.1;
}
```

Apply the fuzzing harness patch, compile with `asan` and `ubsan`, and start fuzzing:

```sh
patch -p2 < PATH/TO/fuzz-dhcpd.patch
cd src/usr.sbin/dhcpd
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 make CC=afl-clang-lto

mkdir seeds
dd if=/dev/urandom of=seeds/random_seed bs=1 count=16

mkdir out

afl-fuzz -i seeds -o out -- ./dhcpd -d
```

The `.patch` file is located in the directory of this README.

Note: The privileged compartment is input-only and does not send messages. Because of that, there is no need to fuzz with MSan or `ptr_checker`, since they only check outgoing messages from the privileged compartment.
