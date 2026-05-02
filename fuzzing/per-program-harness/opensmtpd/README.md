# OpenSMTPD fuzzing harness

Anchored on OpenSMTPD-portable commit `4d397dd2` (release 7.7.0p0).

## 1. Build the `ptr_checker` library (pointer-leak mode)

Place the `ptr_checker` directory next to this README and build it with the runtime pointer-leak detector enabled. This harness runs the detector productively under AFL — `ENABLE_PTR_CHECK=1` is the intended mode for the regular fuzz pass.

```sh
cd ptr_checker
make ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0
export BUFFER_CHECKER_ROOT=$PWD
export AFL_PRELOAD="${BUFFER_CHECKER_ROOT}/libbuffer_check.so"
export LD_PRELOAD="$AFL_PRELOAD"
export LD_LIBRARY_PATH="${BUFFER_CHECKER_ROOT}"
cd ..
```

`BUFFER_CHECKER_ROOT` is read by the patched `mk/smtpd/Makefile.am` for headers and library paths. `AFL_PRELOAD` covers `afl-fuzz -- ./smtpd …` runs; `LD_PRELOAD` covers direct `./smtpd …` invocations.

## 2. Get OpenSMTPD and apply the harness

```sh
git clone https://github.com/OpenSMTPD/OpenSMTPD.git
cd OpenSMTPD
git checkout 4d397dd2
patch -p1 < /PATH/TO/fuzz-opensmtpd.patch

pkg install autoconf automake libtool libevent libasr bison
./bootstrap
./configure CPPFLAGS=-I/usr/local/include LDFLAGS=-L/usr/local/lib
```

`CPPFLAGS` / `LDFLAGS` are required on FreeBSD because libevent, LibreSSL, and libasr live under `/usr/local/`.

Provide a minimal config (this overwrites any sendmail config at the same path):

```sh
mkdir -p /etc/mail
cat > /etc/mail/smtpd.conf <<'EOF'
table aliases file:/etc/mail/aliases
action "local" mbox alias <aliases>
match for local action "local"
EOF
echo 'root: nobody' > /etc/mail/aliases
```

The `-f /etc/mail/smtpd.conf` flag below is required so `smtpd` doesn't pick up an unrelated `/usr/local/etc/smtpd.conf`.

## 3. Fuzz with ASan + pointer-leak detection

```sh
cd mk/smtpd

make clean
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    make CC=afl-clang-lto

mkdir -p seeds
dd if=/dev/urandom of=seeds/seed bs=512 count=8

mkdir -p out_asan
afl-fuzz -i seeds -o out_asan -m none -- ./smtpd -d -f /etc/mail/smtpd.conf
```

`AFL_PRELOAD` is already exported from step 1. The pointer-leak detector catches addresses that escape into `imsg` payloads across the in-process compartment boundary.

## 4. Fuzz with MSan

Rebuild `ptr_checker` with both flags off (the binary's own MSan instrumentation handles uninit detection) and unset the preloads so the previous step's ptr-leak library doesn't carry over.

```sh
cd $BUFFER_CHECKER_ROOT
make clean
make ENABLE_PTR_CHECK=0 ENABLE_MSAN_CHECK=0
unset AFL_PRELOAD LD_PRELOAD
export MSAN_OPTIONS='handle_sigbus=0:exit_code=86:symbolize=0:exit_code=0'

cd /PATH/TO/OpenSMTPD-portable/mk/smtpd
make clean
AFL_USE_MSAN=1 BUFFER_CHECKER_ROOT=$BUFFER_CHECKER_ROOT \
    make CC=afl-clang-lto

mkdir -p out_msan
afl-fuzz -i seeds -o out_msan -m none -- ./smtpd -d -f /etc/mail/smtpd.conf
```

`MSAN_OPTIONS` lists `exit_code=86` first (AFL refuses to run otherwise) and `exit_code=0` last so MSan parses `0` and the binary exits cleanly on uninit hits, combined with `-fsanitize-recover=memory` baked into `mk/smtpd/Makefile.am`.
