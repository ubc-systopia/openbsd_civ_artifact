## Setup
```sh
pkg install autoconf automake libtool libevent byacc
git clone https://github.com/openbgpd-portable/openbgpd-portable.git
cd openbgpd-portable/
git checkout 7518125c6930d0029901ca4c4e81f7073c0d8013
# replace PATH/TO with the path to the update.sh.patch file
patch < PATH/TO/update.sh.patch
./autogen.sh
./configure CPPFLAGS="-I/usr/local/include" LDFLAGS="-L/usr/local/lib"
pw groupadd _bgpd
pw useradd _bgpd -g _bgpd -s /sbin/nologin -d /var/empty -c 'OpenBGPD daemon'
```

## Compiling and Fuzzing
```sh
make
make install
cd src/bgpd
make clean
# replace PATH/TO with the path to the fuzz-bgpd.patch file
patch < PATH/TO/fuzz-bgpd.patch
mkdir seeds out 
dd if=/dev/urandom of=seeds/random_seed bs=1 count=16
AFL_USE_ASAN=1 make CC=afl-clang-lto CFLAGS='-g -O2'
afl-fuzz -i seeds -o out -- ./bgpd -d -f bgpd.conf
```

Fuzzing with `CFLAGS='-g -O0'` will result in the "(2025-07-04) Heap Buffer Over-Read in OpenBGPD's Kroute" bug.

The `.patch` files are located in the directory of this README.

Fuzzing the program for approximately 12 hours at around 200 execs/sec should produce all the bugs discovered.