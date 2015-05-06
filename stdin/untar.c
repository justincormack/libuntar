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

#include <libuntar.h>

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/param.h>
#include <stdlib.h>

# include <string.h>
# include <unistd.h>

#ifdef RUMP_UNMOUNT
#include <sys/param.h>
#include <sys/mount.h>
#endif

char *progname;

int use_chown = 0;

int
extract(char *rootdir)
{
	TAR *t;
	int options = (use_chown ? TAR_CHOWN : 0);

#ifdef DEBUG
	puts("opening tarfile...");
#endif
	if (tar_fdopen(&t, 0, NULL, NULL, O_RDONLY, 0, options) == -1)
	{
		fprintf(stderr, "tar_fdopen(): %s\n", strerror(errno));
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
	printf("Usage: %s [-C rootdir] [-z] [-x] [-p]\n", progname);
	free(rootdir);
	exit(-1);
}

int
main(int argc, char *argv[])
{
	char *rootdir = NULL;
	int c;
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
		case 'p':
			use_chown = 1;
			break;
		default:
			usage(rootdir);
		}

	if ((argc - optind) > 0)
	{
#ifdef DEBUG
		printf("argc - optind == %d\t\n", argc - optind);
#endif
		usage(rootdir);
	}

	return_code = extract(rootdir);

	free(rootdir);

#ifdef RUMP_UNMOUNT
	unmount("/", 0);
#endif

	return return_code;
}
