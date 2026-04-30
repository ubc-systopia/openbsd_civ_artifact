## Email

Dear OpenBSD Security Team,  

This email reports a privsep interface bug found in `mountd`, which  
causes a heap over-read in the privileged child process.  The bug  
can be triggered by the unprivileged parent compartment by sending an  
`IMSG_GETFH_REQ`, `IMSG_EXPORT_REQ`, or `IMSG_DELEXPORT` message with  
an un-terminated string in the payload.  

```c
mountd.c  
 478             case IMSG_DELEXPORT:  
 479                 if (size != MNAMELEN) {  
 480                     syslog(LOG_ERR, "Invalid message size");  
 481                     break;  
 482                 }  
 483                 path = imsg.data;  
 484                 if (statfs(path, &sfb) == -1) {  
 485                     syslog(LOG_ERR, "statfs: %m");  
 486                     break;  
 487                 }  
```

[Proof-of-concept details removed.]

Regards,  

Researcher C, Researcher A, Researcher B  
Research Group X
