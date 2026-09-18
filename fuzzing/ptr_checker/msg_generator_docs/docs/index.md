# CIV Fuzzing Framework

This framework generates IPC messages for fuzzing OpenBSD privilege-separated
programs. The harness runs the main compartment under test and sends messages
in place of the unprivileged compartments through their IPC interfaces. The
framework was used in *Characterizing and Detecting Bugs at the Interfaces of
OpenBSD Privilege-Separated Programs*.

## Build

The message generator has two source files:

```text
msg_generator.c    # implementation
msg_generator.h    # public declarations
```

Build each library in its source directory:

```sh
make libmsg_generator.a    # message generator
make libbuffer_check.so    # detector
```

## High-level overview

The example below shows a typical use of the framework in three steps:
setup, generate messages, and send EOMs. It uses one initialized imsg channel.

```c
#include "msg_generator.h"

struct msg_interface iface;
struct msg_data msg;

/* 1. Supply interface and protocol information. */
msg_interface_init(&iface, STDIN_FILENO, 1, N);
msg_interface_set_message_types_batch(&iface, 0, N, BASE_IMSG_TYPE);
msg_interface_set_eom_type(&iface, IMSG_EOM);
msg_interface_set_endpoint(&iface, 0, &ibuf);

/* 2. Generate and send messages until the input ends. */
while (msg_generate(&iface, &msg) == MSG_GEN_SUCCESS) {
    imsg_compose(&ibuf, msg.type, 0, 0, msg.fd,
        msg.payload, msg.actual_payload_size);
    imsgbuf_flush(&ibuf);
}

/* 3. Send an EOM after the input ends. */
struct msg_data eom;
if (msg_generate_eom(&iface, 0, &eom) == MSG_GEN_SUCCESS) {
    imsg_compose(&ibuf, eom.type, 0, 0, -1, NULL, 0);
    imsgbuf_flush(&ibuf);
}
```

1. **Setup.** Provide interface and protocol information in one shared
   `struct msg_interface`.
2. **Generate messages.** Generate messages automatically in a loop and send
   each one using the compartment's IPC functions.
3. **Send EOMs.** Signal the end of input so the run can terminate cleanly.

We will discuss each step in detail below.

## Functions

All declarations below are in `msg_generator.h`.

### msg_interface_init

Initialize interface and protocol information.

**Function signature**

```c
int msg_interface_init(struct msg_interface *iface, int fuzzer_fd,
    uint8_t num_compartments, uint8_t num_message_types);
```

**Description**

For each fuzzing campaign, `struct msg_interface` stores the interface and
protocol information needed to generate messages. `msg_interface_init()`
initializes this structure.

**Parameters**

- `iface`: Pointer to the structure to initialize.
- `fuzzer_fd`: File descriptor supplying fuzzer input, usually `STDIN_FILENO`.
- `num_compartments`: Number of unprivileged compartments replaced by the fuzzer.
- `num_message_types`: Total number of valid RPC message types. Values from
  1 through 255 specify that count; zero selects all 256 type indices.

**Return values**

`MSG_GEN_SUCCESS` on success. `MSG_GEN_ERROR` if `iface` is `NULL`.

### msg_interface_set_message_type and msg_interface_set_message_types_batch

Map fuzzer type indices to the program's message types.

**Function signature**

```c
int msg_interface_set_message_type(struct msg_interface *iface,
    uint8_t index, int value);
int msg_interface_set_message_types_batch(struct msg_interface *iface,
    uint8_t starting_index, uint8_t count, int16_t offset);
```

**Description**

Initialization supplies the number of message types. The framework also
needs their concrete values to generate the correct message types. These
values are usually defined in an enum: they may start at zero, start at
another number, or have gaps. These functions map the fuzzer's type indices
to those values for all endpoints in `msg_interface`.

The type count and mapping let the generator choose valid RPC types so
messages partly follow the protocol, as described in the paper. Call
`msg_interface_init()` before setting type mappings or endpoints.

- `msg_interface_set_message_type()` maps one `index` to `value`. Use it for
  types with arbitrary values.
- `msg_interface_set_message_types_batch()` maps `count` consecutive indices
  from `starting_index`. Each index `i` maps to `i + offset`. Use it for
  consecutive enum values.

For example, a batch with `starting_index = 0`, `count = 3`, and `offset = 100`
maps indices `0`, `1`, and `2` to types `100`, `101`, and `102`.

**Return values**

`MSG_GEN_SUCCESS` on success. Both functions return `MSG_GEN_ERROR` if `iface`
is `NULL`. The batch function also returns `MSG_GEN_ERROR` if
`starting_index + count` exceeds 256.

### msg_interface_set_endpoint

Register a compartment interface endpoint.

**Function signature**

```c
int msg_interface_set_endpoint(struct msg_interface *iface,
    uint8_t compartment_id, void *endpoint);
```

**Description**

A fuzzing campaign may have multiple endpoints when several unprivileged
compartments are replaced by the fuzzer. In the paper, each unprivileged
compartment has an endpoint for sending messages to the main compartment
under test.
`msg_interface_set_endpoint()` stores this information in `msg_interface`,
one endpoint per call.

**Parameters**

- `iface`: Pointer to the initialized `msg_interface` structure.
- `compartment_id`: User-defined ID for the unprivileged compartment assumed
  compromised: the malicious side replaced by the fuzzer. Assign IDs from zero
  through `num_compartments - 1`.
- `endpoint`: Program-specific data used to send messages. This may be a
  `struct imsgbuf *`, or a raw file descriptor stored as `(void *)(intptr_t)fd`.

**Return values**

`MSG_GEN_SUCCESS` on success. `MSG_GEN_ERROR` if `iface` is `NULL`.

### msg_generate

Read one input record and build a message.

**Function signature**

```c
int msg_generate(struct msg_interface *iface, struct msg_data *msg);
```

**Description**

`msg_generate()` is the main function of the framework. It uses the interface
and protocol information set in `iface` during setup, together with fuzzer
input, to generate one inter-compartment fuzzing message. It fills the
`struct msg_data` passed as `msg` with the information needed to send the
message.

**Important fields in the output**

| Field | Purpose |
| --- | --- |
| `compartment` | Compartment ID selecting the registered endpoint. |
| `endpoint` | The same endpoint value registered for this `compartment` ID with `msg_interface_set_endpoint()`; use it to send the message. |
| `type` | The program's RPC message type after applying the configured mapping. |
| `payload` | Message body to send. |
| `actual_payload_size` | Size of the payload. |
| `fd` | File descriptor to send, or `-1` if there is none. |
| `has_fd` | Whether the message includes a descriptor: 1 for yes, 0 for no. |
| `aux_data` | An additional 64 bytes the user can use for message fields the framework does not generate, such as a message ID. |

**Note:** Some messages require a file descriptor (FD) to be sent with the
message. In addition to the message payload, the framework generates both
the FD itself and the data sent through it.

If the input requests a descriptor, the function creates a UNIX socket
preloaded with the FD data. Its peer is closed before the function returns,
so the descriptor has no live peer for writes.

**Return values**

- `MSG_GEN_SUCCESS` — a message is ready.
- `MSG_GEN_EOF` — the input has ended. Stop generating messages and break
  out of the message-generation loop.
- `MSG_GEN_ERROR` — `iface` or `msg` is `NULL`, the header could not be read in
  full, or a socket operation failed.

### EOM functions

Most programs we fuzz are daemons. If fuzzing does not crash them, they may
keep running after the fuzzer's input ends. We use a special RPC message
type, EOM (end-of-message), to tell the receiving compartment to terminate
normally after processing all fuzzer-generated messages.

If the fuzzer replaces K unprivileged compartments, send one EOM through
each endpoint after sending its generated messages. Once the receiving
compartment has processed K EOMs, it has processed all the generated
messages and can terminate normally instead of hanging.

The framework generates the EOM messages. The framework user must implement
the EOM handler in the receiving compartment and terminate the program
after all expected EOMs arrive.

#### msg_interface_set_eom_type

Set the type used for EOM markers.

**Function signature**

```c
void msg_interface_set_eom_type(struct msg_interface *iface, int eom_type);
```

**Description**

`msg_interface_set_eom_type()` sets the message type used by
`msg_generate_eom()`. Pass the program's actual EOM type value. The
message-type mapping does not apply to EOMs. If `iface` is `NULL`, the function
returns without changing anything.

**Return values**

None.

#### msg_generate_eom

Build an EOM marker for one compartment ID.

**Function signature**

```c
int msg_generate_eom(struct msg_interface *iface, uint8_t compartment_id,
    struct msg_data *msg);
```

**Description**

`msg_generate_eom()` clears `msg` and sets its compartment ID, endpoint, and
EOM type. The message has no payload and `msg.fd` is `-1`. It reads no fuzzer
input.

After input ends, call this function once for each registered channel and send
the returned message through `msg.endpoint`.

**Return values**

`MSG_GEN_SUCCESS` on success. `MSG_GEN_ERROR` if `iface` or `msg` is `NULL`, or
if `compartment_id` is greater than or equal to `num_compartments`.

#### eom_counter_init and eom_counter_inc

Count received EOMs and exit when the expected count is reached.

**Function signature**

```c
void eom_counter_init(int expected_eom);
void eom_counter_inc(void);
```

**Description**

`eom_counter_init()` sets the expected EOM count and resets the received count
to zero. `eom_counter_inc()` adds one to the received count. If the expected
count is positive and the received count reaches it, the function calls
`exit(0)`.

Use these functions in the receiving process. Initialize the count to the
number of EOMs that process expects, then call `eom_counter_inc()` when it
receives an EOM. `msg_interface_init()` also initializes the counter in the
process that calls it.

The counter is global to each process. Separate processes have separate
counters, and threads share the same counter without synchronization.

**Return values**

None. `eom_counter_inc()` exits the process when the expected count is reached.

## Input format

Each record contains a 73-byte header, followed by the payload and optional FD
data. Two-byte fields use host byte order. The current decoder assumes a
little-endian host.

| Offset | Bytes | Field | How it is used |
| ---: | ---: | --- | --- |
| 0 | 1 | compartment | Reduced modulo `num_compartments` when nonzero. |
| 1 | 1 | instance | Reserved; unchanged. |
| 2 | 1 | type | Reduced modulo `num_message_types` when nonzero, then mapped. |
| 3 | 2 | size | Payload length, reduced modulo 8192. |
| 5 | 1 | has_fd | Reduced modulo 2. |
| 6 | 1 | fd_perm | `(value % 3) + 1`: read, write, or both. |
| 7 | 2 | fd_data_len | FD-data length, reduced modulo 8192. |
| 9 | 64 | aux_data | Harness-specific metadata; unchanged. |

Payload and FD-data lengths range from 0 through 8191 bytes
(`MAX_MESSAGE_LENGTH - 1`). After the header, the generator reads `size`
payload bytes. If `has_fd` is set, it then reads `fd_data_len` bytes to place in
the generated descriptor.

An incomplete header is an error, including an incomplete `aux_data` block.
A short payload or FD-data block returns a message on the current call and EOF
on the next call.
