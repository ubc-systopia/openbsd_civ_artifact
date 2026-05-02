- `bug-reports/` contains the bug reports sent to OpenBSD, including detailed descriptions of the identified bugs.
- `fuzzing/` contains the fuzzing framework and per-program fuzzing harnesses used in the evaluation.
- `llm_prompts/` contains the prompts used to guide LLM-based CIV discovery.

## Bug ID Mapping

Note: These reports do not include CIVs identified by the OpenBSD team.

- `B1`: [bug-reports/bgpd/(2025-07-10) Missing Null Terminator Check for pftable_msg.md](bug-reports/bgpd/%282025-07-10%29%20Missing%20Null%20Terminator%20Check%20for%20pftable_msg.md)
- `B2`: [bug-reports/bgpd/(2025-07-10) Stack Over-Read and Over-Write in log_evpnaddr.md](bug-reports/bgpd/%282025-07-10%29%20Stack%20Over-Read%20and%20Over-Write%20in%20log_evpnaddr.md)
- `B6`, `B10`, `B17`, `B21`, `B22`: [bug-reports/proc framework/(2025-07-10) Mid-Step Vector in Shared Privsep Framework.md](bug-reports/proc%20framework/%282025-07-10%29%20Mid-Step%20Vector%20in%20Shared%20Privsep%20Framework.md)
- `B18`: [bug-reports/iked/(2025-07-17) Null-Pointer Dereference via IMSG_OCSP_FD.md](bug-reports/iked/%282025-07-17%29%20Null-Pointer%20Dereference%20via%20IMSG_OCSP_FD.md)
- `B25`: [bug-reports/smtpd/(2025-07-23) Null-Pointer Dereference via Malformed String in mproc.md](bug-reports/smtpd/%282025-07-23%29%20Null-Pointer%20Dereference%20via%20Malformed%20String%20in%20mproc.md)
- `B7`: [bug-reports/relayd/(2025-07-23) Out-of-Bounds Array Access via Negative Index in IMSG_BINDANY.md](bug-reports/relayd/%282025-07-23%29%20Out-of-Bounds%20Array%20Access%20via%20Negative%20Index%20in%20IMSG_BINDANY.md)
- `B19`: [bug-reports/iked/(2025-07-24) OpenBSD Heap Over-Read and FreeBSD Stack-Heap Over-Read and Over-Write Due to insifficient verification on the size of struct sockaddr.md](bug-reports/iked/%282025-07-24%29%20OpenBSD%20Heap%20Over-Read%20and%20FreeBSD%20Stack-Heap%20Over-Read%20and%20Over-Write%20Due%20to%20insifficient%20verification%20on%20the%20size%20of%20struct%20sockaddr.md)
- `B29`: [bug-reports/rpki-client/(2025-07-29) Null Pointer Access via Malicious Parser Message.md](bug-reports/rpki-client/%282025-07-29%29%20Null%20Pointer%20Access%20via%20Malicious%20Parser%20Message.md)
- `B30`, `B31`: [bug-reports/tmux/(2025-08-16) Null Pointer Dereference and Use After Free via Client Messages.md](bug-reports/tmux/%282025-08-16%29%20Null%20Pointer%20Dereference%20and%20Use%20After%20Free%20via%20Client%20Messages.md)
- `B11`: [bug-reports/httpd/(2026-04-18) Heap Over-Read and Log Leak via IMSG_LOG_OPEN.md](bug-reports/httpd/%282026-04-18%29%20Heap%20Over-Read%20and%20Log%20Leak%20via%20IMSG_LOG_OPEN.md)
- `B32`, `B33`: [bug-reports/ntpd/Undefined Behavior via Out-of-Range Double to Integer Conversion.md](bug-reports/ntpd/Undefined%20Behavior%20via%20Out-of-Range%20Double%20to%20Integer%20Conversion.md)
- `B3`: [bug-reports/bgpd/Stack Over-Read via Unterminated IMSG_CTL_RELOAD Reason.md](bug-reports/bgpd/Stack%20Over-Read%20via%20Unterminated%20IMSG_CTL_RELOAD%20Reason.md)
- `B34`, `B35`: [bug-reports/radiusd/Buffer Over-Read via Zero-Length BSDAUTH Usercheck Strings.md](bug-reports/radiusd/Buffer%20Over-Read%20via%20Zero-Length%20BSDAUTH%20Usercheck%20Strings.md)
- `B36`: [bug-reports/dhcpleased/String Over-Read in IMSG_CONFIGURE_INTERFACE.md](bug-reports/dhcpleased/String%20Over-Read%20in%20IMSG_CONFIGURE_INTERFACE.md)
- `B38`: [bug-reports/mountd/(2025-09-23) Heap Over-Read Due to Missing Null-Terminators in Multiple Message Types.md](bug-reports/mountd/%282025-09-23%29%20Heap%20Over-Read%20Due%20to%20Missing%20Null-Terminators%20in%20Multiple%20Message%20Types.md)
- `B39`: [bug-reports/mountd/Stack Memory Leak via IMSG_GETFH_REQ Failure.md](bug-reports/mountd/Stack%20Memory%20Leak%20via%20IMSG_GETFH_REQ%20Failure.md)
- `B12`: [bug-reports/httpd/ASLR Pointer Leak via IMSG_CFG_AUTH.md](bug-reports/httpd/ASLR%20Pointer%20Leak%20via%20IMSG_CFG_AUTH.md)
- `B27`: [bug-reports/smtpd/Queue Deletion and Memory Exhaustion via Unverified Privsep Message Sender.md](bug-reports/smtpd/Queue%20Deletion%20and%20Memory%20Exhaustion%20via%20Unverified%20Privsep%20Message%20Sender.md)
- `B8`, `B9`: [bug-reports/relayd/Stack Over-Read via Unterminated IMSG_SCRIPT String.md](bug-reports/relayd/Stack%20Over-Read%20via%20Unterminated%20IMSG_SCRIPT%20String.md)
- `B13`, `B14`, `B15`, `B16`: [bug-reports/httpd/Pointer and Uninitialized Stack Data Leaks via Configuration Messages.md](bug-reports/httpd/Pointer%20and%20Uninitialized%20Stack%20Data%20Leaks%20via%20Configuration%20Messages.md)
- `B4`: [bug-reports/bgpd/Pointer Leak via IMSG_RECONF_CONF.md](bug-reports/bgpd/Pointer%20Leak%20via%20IMSG_RECONF_CONF.md)
- `B41`, `B42`, `B5`, `B20`, `B43`, `B44`, `B37`: [bug-reports/proc framework/Pointer and Uninitialized Memory Leaks via Unsanitized IPC Structures.md](bug-reports/proc%20framework/Pointer%20and%20Uninitialized%20Memory%20Leaks%20via%20Unsanitized%20IPC%20Structures.md)
- `B28`: [bug-reports/smtpd/Arbitrary MDA Command Execution via IMSG_MDA_FORK.md](bug-reports/smtpd/Arbitrary%20MDA%20Command%20Execution%20via%20IMSG_MDA_FORK.md)
- `B40`: [bug-reports/mountd/Arbitrary Export via Unsanitized Export Requests.md](bug-reports/mountd/Arbitrary%20Export%20via%20Unsanitized%20Export%20Requests.md)
