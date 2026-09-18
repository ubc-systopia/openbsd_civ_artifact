#include <sys/param.h>
#include <sys/types.h>

#include <err.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

struct fuzz_metadata {
	uint8_t	 compartment;
	uint8_t	 instance;
	uint8_t	 type;
	uint16_t size;
	uint8_t	 has_fd;
	uint8_t	 fd_perm;
	uint16_t fd_data_len;
	uint8_t	 aux_data[64];
} __attribute__((packed));

struct module_file_params {
	int	 debug;
	char	 path[PATH_MAX];
};

static void
write_all(const void *data, size_t len)
{
	const uint8_t *p = data;
	ssize_t n;

	while (len != 0) {
		if ((n = write(STDOUT_FILENO, p, len)) == -1)
			err(1, "write");
		if (n == 0)
			errx(1, "short write");
		p += n;
		len -= n;
	}
}

int
main(void)
{
	struct fuzz_metadata meta;
	struct module_file_params params;
	uint8_t username = '\0';

	memset(&meta, 0, sizeof(meta));
	memset(&params, 0, sizeof(params));

	/* The startup exchange must contain a complete PARAMS message. */
	meta.type = 2;
	meta.size = sizeof(params);
	write_all(&meta, sizeof(meta));
	write_all(&params, sizeof(params));

	/* Follow it with the smallest valid USERINFO request. */
	meta.type = 3;
	meta.size = sizeof(username);
	write_all(&meta, sizeof(meta));
	write_all(&username, sizeof(username));

	return (0);
}
