## Setup
```sh
pkg install cmake libressl libevent bison
git clone https://github.com/openiked/openiked-portable.git
cd openiked-portable
git checkout f0da8188ae8b1778dbdd215f7bc30bd8f166aa5e
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
make install
pw groupadd _iked
pw useradd _iked -g _iked -s /sbin/nologin -d /var/empty -c 'IKEv2 Daemon'
```
## Creating the Config File
```sh
cd ..
cat > iked.conf << EOF
# \$OpenBSD: iked.conf,v 1.1 2014/07/11 21:20:10 deraadt Exp \$
#
# See iked.conf(5) for syntax and examples.

# Configure users for the Extensible Authentication Protocol (EAP)
user "user1" "password123"
user "user2" "password456"

# Configuration for clients connecting with EAP authentication.
# Remember to set up a PKI, see ikectl(8) for more information.
ikev2 "win7" passive esp \\
	from 10.1.0.0/24 to 10.2.0.0/24 \\
	local any peer any \\
	eap "mschap-v2" \\
	config address 10.2.0.1 \\
	config name-server 10.1.0.2 \\
	tag "\$name-\$id"

# Configuration for a client authenticating with a pre-shared key.
ikev2 esp \\
	from 10.3.0.0/24 to 10.1.0.0/24 \\
	from 10.5.0.0/24 to 10.1.0.0/24 \\
	from 10.5.0.0/24 to 172.16.1.0/24 \\
	local 192.168.1.1 peer 192.168.2.1 \\
	psk "you-should-not-use-psk-authentication!"
EOF
chmod 600 iked.conf
```
## Creating Certificates
```sh
mkdir /usr/local/etc/iked/
openssl genrsa -out /usr/local/etc/iked/private/local.key 4096
openssl req -new -key /usr/local/etc/iked/private/local.key -out /tmp/local.csr -subj "/C=CA/ST=State/L=City/O=MyOrg/CN=artifact.local"
openssl genrsa -out /usr/local/etc/iked/private/myca.key 4096
openssl req -x509 -new -nodes -key /usr/local/etc/iked/private/myca.key -sha256 -days 3650 -out /usr/local/etc/iked/ca/myca.crt -subj "/C=CA/O=MyCA/CN=My IKEv2 CA"
openssl x509 -req -in /tmp/local.csr -CA /usr/local/etc/iked/ca/myca.crt -CAkey /usr/local/etc/iked/private/myca.key -CAcreateserial -out /usr/local/etc/iked/certs/local.crt -days 365 -sha256
mv /usr/local/etc/iked/ca/myca.srl /usr/local/etc/iked/private/
```
## Compiling and Fuzzing
```sh
cd iked
# replace PATH/TO with the path to the fuzz-iked.patch file
patch < PATH/TO/fuzz-iked.patch
cd ../build 
rm -rf *
CC=afl-clang-lto cmake -DCMAKE_BUILD_TYPE=Debug ..
AFL_USE_ASAN=1 make
mkdir out
mkdir seeds
dd if=/dev/urandom of=seeds/random_seed bs=1 count=16
afl-fuzz -i seeds -o out -- iked/iked -d -f ../iked.conf
```

The `.patch` file is located in the directory of this README.

**Note:** 
Some of the crashes led us to finding more CIVs by generalising and reproducing the CIV in OpenBSD manually.  In that, not all CIVs reported were found by the fuzzer.  In those cases the fuzzer found a CIV which led us to look for similar CIVs manually. There are some reported CIVs that will not be detected by the fuzzer because we found them manually in the OpenBSD version of the program.  For example, the fuzzer is only fuzzing the portable version of the program in FreeBSD whereas some CIVs were specific to OpenBSD thus cannot be reproduced by out fuzzer.  
