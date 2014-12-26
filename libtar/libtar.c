/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  libtar.c - demo driver program for libtar
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <config.h>
#include <libtar.h>

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/param.h>
#include <stdlib.h>

# include <string.h>
# include <unistd.h>

#ifdef HAVE_LIBZ
# include <zlib.h>
#endif

#include <compat.h>


char *progname;

#ifdef HAVE_LIBZ

int use_zlib = 0;

int
gzopen_frontend(char *pathname, int oflags, int mode)
{
	char *gzoflags;
	gzFile gzf;
	int fd;

	switch (oflags & O_ACCMODE)
	{
	case O_WRONLY:
		gzoflags = "wb";
		break;
	case O_RDONLY:
		gzoflags = "rb";
		break;
	default:
	case O_RDWR:
		errno = EINVAL;
		return -1;
	}

	fd = open(pathname, oflags, mode);
	if (fd == -1)
		return -1;

	if ((oflags & O_CREAT) && fchmod(fd, mode))
	{
		close(fd);
		return -1;
	}

	gzf = gzdopen(fd, gzoflags);
	if (!gzf)
	{
		errno = ENOMEM;
		return -1;
	}

	/* This is a bad thing to do on big-endian lp64 systems, where the
	   size and placement of integers is different than pointers.
	   However, to fix the problem 4 wrapper functions would be needed and
	   an extra bit of data associating GZF with the wrapper functions.  */
	return (int)gzf;
}

tartype_t gztype = { (openfunc_t) gzopen_frontend, (closefunc_t) gzclose,
	(readfunc_t) gzread, (writefunc_t) gzwrite
};

#endif /* HAVE_LIBZ */

int
extract(char *tarfile, char *rootdir)
{
	TAR *t;

#ifdef DEBUG
	puts("opening tarfile...");
#endif
	if (tar_open(&t, tarfile,
#ifdef HAVE_LIBZ
		     (use_zlib ? &gztype : NULL),
#else
		     NULL,
#endif
		     O_RDONLY, 0, 0) == -1)
	{
		fprintf(stderr, "tar_open(): %s\n", strerror(errno));
		return -1;
	}

#ifdef DEBUG
	puts("extracting tarfile...");
#endif
	if (tar_extract_all(t, rootdir) != 0)
	{
		fprintf(stderr, "tar_extract_all(): %s\n", strerror(errno));
		tar_close(t);
		return -1;
	}

#ifdef DEBUG
	puts("closing tarfile...");
#endif
	if (tar_close(t) != 0)
	{
		fprintf(stderr, "tar_close(): %s\n", strerror(errno));
		return -1;
	}

	return 0;
}


void
usage(void *rootdir)
{
	printf("Usage: %s [-C rootdir] [-g] [-z] -x|-t filename.tar\n",
	       progname);
	free(rootdir);
	exit(-1);
}


#define MODE_LIST	1
#define MODE_CREATE	2
#define MODE_EXTRACT	3

int
main(int argc, char *argv[])
{
	char *tarfile = NULL;
	char *rootdir = NULL;
	int c;
	libtar_list_t *l;
	int return_code = -2;

	progname = argv[0];

	while ((c = getopt(argc, argv, "C:xz")) != -1)
		switch (c)
		{
		case 'C':
			rootdir = strdup(optarg);
			break;
		case 'x':
			break;
#ifdef HAVE_LIBZ
		case 'z':
			use_zlib = 1;
			break;
#endif /* HAVE_LIBZ */
		default:
			usage(rootdir);
		}

	if ((argc - optind) < 1)
	{
#ifdef DEBUG
		printf("argc - optind == %d\t\n", argc - optind);
#endif
		usage(rootdir);
	}

	return_code = extract(argv[optind], rootdir);

	free(rootdir);
	return return_code;
}


