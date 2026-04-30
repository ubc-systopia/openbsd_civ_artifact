## Email

Dear OpenBSD Security Team,  

This email reports a mid-step vector found in bgpd.  

A stack ***over-read*** and ***over-write*** can be triggered in the privileged  
bgpd process when an `IMSG_KROUTE_CHANGE` message is sent from the  
unprivileged rde process.  

When the `IMSG_KROUTE_CHANGE` message is received by the bgpd  
process, `imsg_get_data` is called and populates the  
`struct kroute_full kf` stack variable. Within `struct kroute_full`  
are two `struct bgpd_addr` structures which contain the following two  
elements:  

```c
bgpd.h  
 231     uint8_t     labellen;   /* size of the labelstack */  
 232     uint8_t     labelstack[18]; /* max that makes sense */  
```

Given that the value of `labellen` is determined by the data within  
an `IMSG_KROUTE_CHANGE` message (and that this length is not  
appropriately checked), it is possible for a compromised rde process  
to  set `labellen`to a value beyond the size of `labelstack`. When  
`labellen` is then used to access `labelstack`, an over-read occurs. We  
see this happening in the `log_evpnaddr` function found in `util.c`.  
Specifically, on lines 97 and 110:  

```c
util.c  
  87 const char *  
  88 log_evpnaddr(const struct bgpd_addr *addr, struct sockaddr *sa,  
  89     socklen_t salen)  
  90 {  
  91     static char buf[138];  
  92     uint32_t    vni;  
  93     uint8_t     len;  
  94  
  95     switch (addr->evpn.type) {  
  96     case EVPN_ROUTE_TYPE_2:  
  97         memcpy(&vni, addr->labelstack, addr->labellen);  
  98         snprintf(buf, sizeof(buf), "[2]:[%s]:[%s]:[%d]:[48]:[%s]",  
  99             log_rd(addr->rd), log_esi(addr->evpn.esi), htonl(vni) >> 8,  
 100             log_mac(addr->evpn.mac));  
 101         if (sa != NULL) {  
 102             len = strlen(buf);  
 103             snprintf(buf+len, sizeof(buf)-len, ":[%d]:[%s]",  
 104                 sa->sa_family == AF_INET ? 32 : 128,  
 105                 log_sockaddr(sa, salen));  
 106         }  
 107         break;  
 108     case EVPN_ROUTE_TYPE_3:  
 109         if (sa != NULL) {  
 110             memcpy(&vni, addr->labelstack, addr->labellen);  
 111             snprintf(buf, sizeof(buf), "[3]:[%s]:[%d]:[%s]",  
 112                 log_rd(addr->rd),  
 113                 sa->sa_family == AF_INET ? 32 : 128,  
 114                 log_sockaddr(sa, salen));  
 115         }  
 116         break;  
 117     default:  
 118         break;  
 119     }  
 120     return (buf);  
 121 }  
 ```

Furthermore, because `memcpy`’s (line 97 and line 110) length is also  
controlled by the un-checked `labellen`, a compromised rde compartment  
could also over-write the stack variable `vni`, possibly over-writing  
the return address on the stack, gaining arbitrary code execution.  
Moreover, because `addr->labelstack` points into the middle of the IPC  
message, an attacker can precisely control the stack contents by placing  
their chosen data from `addr->labelstack` to the end of the message.  

[Proof-of-concept details removed.]

We acknowledge that gaining arbitrary code execution by over-writing  
the return address can be prevented by ASLR, and stack canaries.  

To prevent the over-read, a check could be added to restrict  
`labellen` to be within the size of `labelstack`. Yet, this does not  
prevent the over-write.  Even if `labellen < sizeof(labelstack)`,  
`labellen` could still be larger than `sizeof(vni)` thus causing an  
over-write despite the additional check.  

We believe that it may also be possible to remove line 110 entirely.  

Once again this mid-step vector was discovered with AFL++ while  
fuzzing the OpenBGPD portable with ASan on FreeBSD. We have carefully  
reviewed this bug to verify that it also occurs on OpenBSD and that  
it is not a false positive.  

We will continue to report the mid-step vectors we find. Please let us  
know how we can improve our reports and if you would like any more  
information. Thank you!  

Regards,  
Researcher C, Researcher A, Researcher B  
Research Group X
