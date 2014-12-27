/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  handle.c - libtar code for initializing a TAR handle
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <libuntar.h>

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

#include <stdlib.h>

static tartype_t default_type = { open, close, read };


static int
tar_init(TAR **t, tartype_t *type,
	 int oflags, int mode, int options)
{
	if ((oflags & O_ACCMODE) == O_RDWR)
	{
		errno = EINVAL;
		return -1;
	}

	*t = (TAR *)calloc(1, sizeof(TAR));
	if (*t == NULL)
		return -1;

	(*t)->options = options;
	(*t)->type = (type ? type : &default_type);
	(*t)->oflags = oflags;
	(*t)->dirfd = AT_FDCWD;
	(*t)->atflags = AT_SYMLINK_NOFOLLOW;

	return 0;
}


/* open a new tarfile handle */
int
tar_open(TAR **t, const char *pathname, tartype_t *type,
	 int oflags, int mode, int options)
{
	if (tar_init(t, type, oflags, mode, options) == -1)
		return -1;

	if ((options & TAR_NOOVERWRITE) && (oflags & O_CREAT))
		oflags |= O_EXCL;

	(*t)->fd = (*((*t)->type->openfunc))(pathname, oflags, mode);
	if ((*t)->fd == -1)
	{
		free(*t);
		return -1;
	}

	return 0;
}


int
tar_fdopen(TAR **t, int fd, const char *pathname, tartype_t *type,
	   int oflags, int mode, int options)
{
	if (tar_init(t, type, oflags, mode, options) == -1)
		return -1;

	(*t)->fd = fd;
	return 0;
}


int
tar_fd(TAR *t)
{
	return t->fd;
}


/* close tarfile handle */
int
tar_close(TAR *t)
{
	int i;

	i = (*(t->type->closefunc))(t->fd);

	if (t->th_pathname != NULL)
		free(t->th_pathname);
	free(t);

	return i;
}
