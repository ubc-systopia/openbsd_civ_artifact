## Email

Hello OpenBSD Team,  

We have discovered a heap buffer over-read bug  
in OpenBGPD's kroute while running on FreeBSD (14.2).  However, we  
believe that this bug also exists in the OpenBSD implementation.    

The bug occurs in the `fetchifs` function found in kroute-freebsd.c.    
On line 2763 `memcpy` reads past the boundary of the heap buffer  
`buf`.  

```c
kroute-freebsd.c  
2751     if ((buf = malloc(len*2)) == NULL) {  
2752         log_warn("%s", __func__);  
2753         return (-1);  
2754     }  
2755     if (sysctl(mib, 6, buf, &len, NULL, 0) == -1) {  
2756         log_warn("%s: sysctl2", __func__);  
2757         free(buf);  
2758         return (-1);  
2759     }  
2760       
2761     lim = buf + len;  
2762     for (next = buf; next < lim; next += ifm.ifm_msglen) {  
2763         memcpy(&ifm, next, sizeof(ifm));  
```

The issue occurs during the last iteration of  
the loop (line 2762) where the last RTM_NEWADDR message (64 bytes)  
in `buf` is less than `sizeof(ifm)` (168 bytes) causing a buffer  
over-read.  

[Proof-of-concept details removed.]  

We believe this bug also exists in OpenBSD's implementation of  
kroute.  To confirm this we examined the hexdump of `buf` in  
OpenBSD and confirmed that the contents were similar to those of  
FreeBSD.  In addition, the implementation of `fetchifs` in OpenBSD  
(kroute.c) is nearly identical to the one for FreeBSD  
(kroute-freebsd.c).    

Given that Address Sanitizer is not supported in OpenBSD, this may  
have also contributed to the bug being more difficult to detect.  

We are happy to provide any more information that may be of use for  
this bug.  Please don't hesitate to reach out, thanks!  

Regards,  

Shawn, Shibo, Hugo  
UBC Systopia Lab

---
## ASan Error

```
root@freebsd:~/openbgpd-portable/src/bgpd # ./bgpd -d
startup
=================================================================
==1657==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x5160000002c8 at pc 0x000000302062 bp 0x7fffffffe230 sp 0x7fffffffd9e8
READ of size 168 at 0x5160000002c8 thread T0
route decision engine ready
rtr engine ready
session engine ready
    #0 0x000000302061 in __asan_memcpy /usr/src/contrib/llvm-project/compiler-rt/lib/asan/asan_interceptors_memintrinsics.cpp:63:3
    #1 0x00000043fb1e in fetchifs /root/openbgpd-portable/src/bgpd/kroute-freebsd.c:2763:3
    #2 0x00000043f758 in kr_init /root/openbgpd-portable/src/bgpd/kroute-freebsd.c:250:6
    #3 0x000000338bc2 in main /root/openbgpd-portable/src/bgpd/bgpd.c:290:6
    #4 0x000800694e33 in __libc_start1 /usr/src/lib/libc/csu/libc_start1.c:157:7
    #5 0x000000292f83 in _start /usr/src/lib/csu/amd64/crt1_s.S:80

0x5160000002c8 is located 0 bytes after 584-byte region [0x516000000080,0x5160000002c8)
allocated by thread T0 here:
    #0 0x0000003039af in malloc /usr/src/contrib/llvm-project/compiler-rt/lib/asan/asan_malloc_linux.cpp:68:3
    #1 0x00000043fa1f in fetchifs /root/openbgpd-portable/src/bgpd/kroute-freebsd.c:2751:13
    #2 0x00000043f758 in kr_init /root/openbgpd-portable/src/bgpd/kroute-freebsd.c:250:6
    #3 0x000000338bc2 in main /root/openbgpd-portable/src/bgpd/bgpd.c:290:6
    #4 0x000800694e33 in __libc_start1 /usr/src/lib/libc/csu/libc_start1.c:157:7
    #5 0x000000292f83 in _start /usr/src/lib/csu/amd64/crt1_s.S:80
    #6 0x0008004e3007  (<unknown module>)

SUMMARY: AddressSanitizer: heap-buffer-overflow /root/openbgpd-portable/src/bgpd/kroute-freebsd.c:2763:3 in fetchifs
Shadow bytes around the buggy address:
  0x516000000000: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x516000000080: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x516000000100: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x516000000180: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x516000000200: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
=>0x516000000280: 00 00 00 00 00 00 00 00 00[fa]fa fa fa fa fa fa
  0x516000000300: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x516000000380: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x516000000400: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x516000000480: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x516000000500: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==1657==ABORTING
peer closed imsg connection
peer closed imsg connection
SE: Lost connection to parent
fatal in RTR: Lost connection to parent
peer closed imsg connection
session engine exiting
fatal in RDE: Lost connection to parent
```
