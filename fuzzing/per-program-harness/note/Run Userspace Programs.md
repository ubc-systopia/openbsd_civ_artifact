### 0. Introduction

This note explains the process of building and running individual OpenBSD userland programs. All operations assume an OpenBSD environment. I've tried using both QEMU and VMware to run OpenBSD VMs, and both work very well.

### 1. Getting the source code

It is common practice to put source code under `/usr/src`. Use the following command to acquire the source code. CVS should come with OpenBSD by default.

``` bash
cd /usr
export CVSROOT=anoncvs@obsdacvs.cs.toronto.edu:/cvs # use UofT CVS server
cvs -q checkout -P src
```

This could take a while (~5 minutes on my end).

If you're experiencing network issues try the following:
```sh
echo 'inet autoconf` > /etc/hostname.vio0
```
Change `vio0` to the appropriate network interface.

### 2. Compiling Libraries

From now on, I will use httpd (`/usr/src/usr.sbin/httpd`) as an example. The same rules apply to other programs.

Most applications require `libutil` to run, so we need to first generate the necessary `.so` files.

``` bash
cd /usr/src/lib/libutil
make
```

Verify that the `.so` files are generated in the same directory.

It is possible that some programs require other libraries; compile them as required (httpd only needs libutil).

Add the directory containing `.so` files to the dynamic linker search directory. For libutil, append the following to the end of `~/.profile` (similar to .bashrc, but for ksh).

``` bash
export LD_LIBRARY_PATH=/usr/src/lib/libutil:$LD_LIBRARY_PATH
```

#### Including Debugging Information

For debugging with GDB we can append the following lines of code to the end of the `/usr/src/lib/libutil/Makefile` file.

```bash
# For debugging
CFLAGS+= -g -O0 -fno-omit-frame-pointer
```

- `-g`: includes debugging symbols
- `-O0`: compiles without optimisations (for ease of debugging)
- `-fno-omit-frame-point`: includes the stack base pointer for backtraces

Then we can simple run:
```bash
make
```


### 3. Compiling Individual Userspace Programs

``` bash
cd /usr/src/usr.sbin/httpd
make CFLAGS="${CFLAGS} -I/usr/src/lib/libutil" LDFLAGS="${LDFLAGS} -L/usr/src/lib/libutil"
```

Two arguments:

- CFLAGS: include directory, which contains all the header files of our needed library.
- LDFLAGS: linking directory, which contains all the `.so` files. This is for compilation time, while the above `LD_LIBRARY_PATH` is for runtime. They have the same value, as expected.

Because we use the same directory for header and `.so` files, the path is the same. If additional libraries are required, add them to the arguments accordingly.

#### Including Debugging Information

We can also modify the `Makefile` of a specific userspace program to include debugging symbols which we can use with GDB. For example, for `httpd` we can append the following lines of code to the end of the `/usr/src/usr.sbin/httpd/Makefile` file.

```bash
# Linking libutil
CFLAGS+= -I/usr/src/lib/libutil
LDFLAGS+= -L/usr/src/lib/libutil

# For debugging
CFLAGS+= -g -O0 -fno-omit-frame-pointer
```

Note that for some programs they may have their own debugging modes.  Usually, these modes will be commented out in the `Makefile` such as in `/usr/src/usr.sbin/httpd/Makefile`:

```bash
#DEBUG=		-g -DDEBUG=3 -O0
```

So we can includes these additional debugging flags in the `Makefile` as well.  In total, we append the following to the end of `/usr/src/usr.sbin/httpd/Makefile`:

```bash
# Linking libutil
CFLAGS+= -I/usr/src/lib/libutil
LDFLAGS+= -L/usr/src/lib/libutil

# For debugging
CFLAGS+= -g -DDEBUG=3 -O0 -fno-omit-frame-pointer
```

Again, we can simple run:
```bash
make
```


### 4. Run!

If the compilation is successful, the binary should appear in the same folder.

Note, that when debugging with GDB to get the source code to display prior to running the code it may be necessary to first run `layout asm` then `layout src` in GDB.
