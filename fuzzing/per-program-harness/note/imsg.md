## 0. Introduction

**imsg** is an OpenBSD tool for inter-process message passing usually for compartmentalized software. It defines a message format with a header and content, supporting data transfer (including file descriptors) through a UNIX socket and a queuing system for asynchronous communication. This note highlights key APIs. For detailed documentation, see `man imsg_init(3)` and `man ibuf_add(3)`.

The core structures of **imsg** are the following:
- **`struct ibuf`**: a single buffer containing dynamic data, used to store the header an payload of an `imsg` message
- **`struct msgbuf`**: used to queue output buffers (`ibuf`) for transmission via a file descriptor
- **`struct imsg`**: higher-level abstraction of a single message passed between processes using `ibuf`, and consists of the message header and pointer to the payload in `ibuf`
- **`struct imsgbuf`**: higher-level abstraction used to interface with `msgbuf` and a file descriptor to send and receive `imsg` messages 

Relevant source code:
- `src/lib/libutil/imsg.c`
- `src/lib/libutil/imsg.h`
- `src/lib/libutil/imsg-buffer.c`

## 1. struct ibuf

```c
struct ibuf {
	TAILQ_ENTRY(ibuf)	 entry;
	unsigned char		*buf;
	size_t			 size;
	size_t			 max;
	size_t			 wpos;
	size_t			 rpos;
	int			 fd;
};
```

- `entry`: a doubly linked tail queue used to link `ibuf`'s together to make a `msgbuf`
- `buf`: buffer to store data as raw bytes
- `size`: size of the buffer 
- `max`: maximum size the buffer can grow to
- `wpos`: write position in the buffer (offset from the start of the buffer that the next write should start at)
- `rpos`: read position (offset from the start of the buffer that the next read should start at)
- `fd`: file descriptor that is passed with the message
## 2. struct msgbuf

```c
struct msgbuf {
	TAILQ_HEAD(, ibuf)	 bufs;
	TAILQ_HEAD(, ibuf)	 rbufs;
	uint32_t		 queued;
	char			*rbuf;
	struct ibuf		*rpmsg;
	struct ibuf		*(*readhdr)(struct ibuf *, void *, int *);
	void			*rarg;
	size_t			 roff;
	size_t			 hdrsize;
};
```

- `bufs`: outgoing queue of `ibuf`'s ready for transmission
- `rbufs`: incoming queue of `ibuf`'s waiting to be processed
- `queued`: number of `ibuf`'s in the `bufs` queue
- `rbuf`: raw data received from a file descriptor
- `rpmsg`: partial `ibuf` assembled from `rbuf`, once the `rpmsg` is a full it is enqueued onto `rbufs` and set to `NULL`
- `readhdr`: callback function used to parse the header and return a new `ibuf` of the size of the full message.  Can take ownership of the of the file descriptor passed in its `int *` argument
- `rarg`: an argument that is passed to `readhdr()`
- `roff`: offset of data read from the file descriptor but not yet processed, the amount of remaining data to be parsed
- `hdrsize`: defines the size of the `ibuf` passed to the `readhdr` callback

```c
int ibuf_read_process(struct msgbuf *msgbuf, int fd)
```

The `ibuf_read_process()` function is responsible for transforming the raw bytes from `msgbuf->rbuf`  into structured `ibuf` messages which are then added to `msgbuf->rbufs`.  The file descriptor argument `fd` will be closed once the function returns.
1. If a partial message is not already being processed (`msgbuf->rpmsg == NULL`)  and the data in `rbuf` is at least `msgbuf->hdrsize` then call `msgbuf->readhdr()` and store the returned partial `ibuf` message in `msgbuf->rpmsg`.  
2. If there is no partial message (`msgbuf->rpmsg == NULL`)  and the data in `rbuf` is less than `msgbuf->hdrsize` then go to step 6.
3. Add the remaining bytes from `rbuf` into `msgbuf->rpmsg` until `msgbuf->rpmsg` is full or until there is no more data left in `rbuf`.
4. If `msgbuf->rpmsg` is full (a complete message) enqueue it to `msgbuf->rbufs` and indicate that a partial message is no longer being processed (`msgbuf->rpmsg = NULL`).
5. Repeat steps 1-4 until `rbuf` has no more data left to process
6. Shift any remaining data to be processed to the start of `msg->rbuf` and set `msgbuf->off` to the amount of data left to be processed
## 3. struct imsg

```c
struct imsg_hdr {
	uint32_t	 type;
	uint32_t	 len;
	uint32_t	 peerid;
	uint32_t	 pid;
};

struct imsg {
	struct imsg_hdr	 hdr;
	void		*data;
	struct ibuf	*buf;
};
```

 In `imsg_hdr` both `type` and `peerid` can be application-specific. 

- `type`: determines the message format and how to interpret the message data
- `len`: length of the message (including the header)
- `peerid`: can be used to differentiate multiple sessions and responses to a single process (`pid`)
- `pid`: process ID of the sender, if `pid == 0` it is set to the `pid` of `imsgbuf` during the `imsg_compose_ibuf()` or `imsg_create()` functions
- `data`: message payload (see commit `4658a15`)
- `buf`: the `ibuf` buffer where the message (header + payload) is actually stored
## 4. struct imsgbuf

```c
struct imsgbuf {
	struct msgbuf		*w;
	pid_t			 pid;
	uint32_t		 maxsize;
	int			 fd;
	int			 flags;
};
```

Each `imsgbuf` is initialized with a UNIX socket descriptor, representing a communication endpoint. You can create a pair of connected sockets with `socketpair(2)`—one for the parent process, one for the child.

`MAX_IMSGSIZE` defines the maximum size of a single `imsg`, currently 16384 bytes.

- `w`: `msgbuf` used to write/read data 
- `pid`: process ID of the calling process, set during the `imsgbuf_init()` function
- `maxsize`: the maximum number of bytes of a single `imsg` message that the `imsgbuf` can handle
- `fd`: the file descriptor used to send and receive messages
- `flags`: bitmask used to enable/disable certain behaviours (ie. `IMSG_ALLOW_FDPASS` determines whether file descriptors can be passed)
## 5. Message Types

On reception, the object is not payload raw bytes but an struct instance; payload extraction procedure slightly differs depending the type of payload:

### 5.1 Fixed-Length Data
If the payload size is known (struct instance), extract the bytes directly.

This is quite commonly used because it allows the message format to be simply defined as a struct.

```c
int imsg_get_data(struct imsg *imsg, void *data, size_t len)
```

`imsg_get_data()` extract the payload of an `imsg` when the payload structure is known and can be extract in one go.


### 5.2 Variable-Length Data
Extract the payload size first, then process the data. This could be used to pass arrays.

```c
int imsg_get_ibuf(struct imsg *imsg, struct ibuf *ibuf)
```

`imsg_get_ibuf()` initializes the `ibuf` argument to hold the payload from `imsg->buf`
### 5.3 File Descriptor 
The receiving process calls the relevant function to extract `fd` number. The `fd` will be automatically added to receiving process's `fd` table.

```c
imsgbuf_allow_fdpass(struct imsgbuf *imsgbuf)
```

Passing file descriptors via `imsg` is disabled by default.  `imsg_allow_fdpass()` simply adds the `IMSG_ALLOW_FDPASS` flag (`0x01`) to `imsgbuf->flags`

To avoid accidentally forwarding file descriptors to an unintended the recipient the `imsg_forward()` function closes any file descriptors it has.  Furthermore, `imsg_compose_ibuf()` cannot forward file descriptors.

## 6 Typical Usage

### 6.1 Function Calls
- `imsgbuf_init()`
	- `msgbuf_new_reader()`
- `imsg_compose()`
	- `imsg_create()`
	- `imsg_add()`
	- `ibuf_fd_set()`
	- `imsg_close()`
- `imsgbuf_write()`
	- `msgbuf_write()` (if `IMSG_ALLOW_FDPASS` flag is set)
	- `ibuf_write()` (if `IMSG_ALLOW_FDPASS` flag is not set) 
- `dispatch_imsg()`
	- `imsgbuf_read()`
		- `msgbuf_read()` (if `IMSG_ALLOW_FDPASS` flag is set) 
		- `ibuf_read()` (if `IMSG_ALLOW_FDPASS` flag is not set) 
	- `imsg_get()`
		- `msgbuf_get()`
		- `ibuf_get()`
	- `switch (imsg_get_type())`
		- `imsg_get_data()`
			- `ibuf_get()`
	- `imsg_free()`
		- `ibuf_free()`

### 6.2 Example Diagram
![imsg diagram](imsg_example.png)