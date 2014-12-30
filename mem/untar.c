
#include <libuntar.h>

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/param.h>
#include <stdlib.h>

# include <string.h>
# include <unistd.h>

char *progname;

int use_chown = 0;

extern const char _binary_image_tar_start[];
extern const char _binary_image_tar_end[];

static int offset = 0;
static size_t size = 0;

static int
memopen(const char *filename, int flags, mode_t mode)
{
	offset = 0;
	size = (size_t) (_binary_image_tar_end - _binary_image_tar_start);
	return 0;
}

static int
memclose(int fd)
{
	return 0;
}

static ssize_t
memread(int fd, void *buf, size_t len)
{
	if (offset + len > size)
		len = size - offset;
	if (len > 0)
		memcpy(buf, _binary_image_tar_start + offset, len);
	offset += len;
	return len;
}

tartype_t memtype = { (openfunc_t)memopen, (closefunc_t)memclose, (readfunc_t)memread };

int
extract(char *tarfile, char *rootdir)
{
	TAR *t;
	int options = (use_chown ? TAR_CHOWN : 0);

	if (tar_open(&t, tarfile, &memtype,
		     O_RDONLY, 0, options) == -1)
	{
		fprintf(stderr, "tar_open(): %s\n", strerror(errno));
		return -1;
	}

	if (tar_extract_all(t, rootdir) != 0)
	{
		fprintf(stderr, "tar_extract_all(): %s\n", strerror(errno));
		tar_close(t);
		return -1;
	}

	if (tar_close(t) != 0)
	{
		fprintf(stderr, "tar_close(): %s\n", strerror(errno));
		return -1;
	}

	return 0;
}
