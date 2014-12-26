/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  extract.c - libtar code to extract a file from a tar archive
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <internal.h>

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <utime.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <unistd.h>

static int tar_extract_file(TAR *t, char *realname);

static int tar_extract_dir(TAR *t, char *filename);
static int tar_extract_hardlink(TAR *t, char *filename);
static int tar_extract_symlink(TAR *t, char *filename);
static int tar_extract_chardev(TAR *t, char *filename);
static int tar_extract_blockdev(TAR *t, char *filename);
static int tar_extract_fifo(TAR *t, char *filename);
static int tar_extract_regfile(TAR *t, char *filename);

int
tar_extract_all(TAR *t, char *prefix)
{
	char *filename;
	int i;

#ifdef DEBUG
	printf("==> tar_extract_all(TAR *t, \"%s\")\n",
	       (prefix ? prefix : "(null)"));
#endif

	if (prefix) {
		t->dirfd = open(prefix, O_RDONLY | O_DIRECTORY);
		if (t->dirfd == -1)
			return -1;
	}

	while ((i = th_read(t)) == 0)
	{
#ifdef DEBUG
		puts("    tar_extract_all(): calling th_get_pathname()");
#endif
		filename = th_get_pathname(t);
#ifdef DEBUG
		printf("    tar_extract_all(): calling tar_extract_file(t, "
		       "\"%s\")\n", filename);
#endif
		if (tar_extract_file(t, filename) != 0)
			return -1;
	}

	if (t->dirfd >= 0) {
		close(t->dirfd);
	}

	return (i == 1 ? 0 : -1);
}

/*
** mkdirhier() - create all directories needed for a given filename
** returns:
**	0			success
**	1			all directories already exist
**	-1 (and sets errno)	error
*/
static int
mkdirhier(TAR *t, char *filename)
{
	char src[MAXPATHLEN], dst[MAXPATHLEN] = "";
	char *dirp, *nextp = src;
	int retval = 1;
	char *path = openbsd_dirname(filename);

	if (strlcpy(src, path, sizeof(src)) > sizeof(src))
	{
		errno = ENAMETOOLONG;
		return -1;
	}

	if (path[0] == '/')
		strcpy(dst, "/");

	while ((dirp = strsep(&nextp, "/")) != NULL)
	{
		if (*dirp == '\0')
			continue;

		if (dst[0] != '\0')
			strcat(dst, "/");
		strcat(dst, dirp);

		if (mkdirat(t->dirfd, dst, 0777) == -1)
		{
			if (errno != EEXIST)
				return -1;
		}
		else
			retval = 0;
	}

	return retval;
}

static int
tar_set_file_perms(TAR *t, char *filename)
{
	mode_t mode = th_get_mode(t);
	uid_t uid = th_get_uid(t);
	gid_t gid = th_get_gid(t);
	time_t mtime = th_get_mtime(t);
	const struct timespec ut[] = {{mtime, 0}, {mtime, 0}};

	/* change owner/group */
	if (t->options & TAR_CHOWN)
		if (fchownat(t->dirfd, filename, uid, gid, t->atflags) == -1)
		{
#ifdef DEBUG
			fprintf(stderr, "fchownat(\"%s\", %d, %d): %s\n",
				filename, uid, gid, strerror(errno));
#endif
			return -1;
		}

	/* change access/modification time */
	if (!TH_ISSYM(t))
		if (utimensat(t->dirfd, filename, ut, t->atflags) == -1)
		{
#ifdef DEBUG
			perror("utimensat()");
#endif
			return -1;
		}

	/* change permissions */
	if (!TH_ISSYM(t))
		if (fchmodat(t->dirfd, filename, mode, 0) == -1)
	{
#ifdef DEBUG
		perror("fchmodat()");
#endif
		return -1;
	}

	return 0;
}


/* switchboard */
static int
tar_extract_file(TAR *t, char *realname)
{
	int i;
	char *lnp;
	int pathname_len;
	int realname_len;

	if (t->options & TAR_NOOVERWRITE)
	{
		struct stat s;

		if (fstatat(t->dirfd, realname, &s, t->atflags) == 0 || errno != ENOENT)
		{
			errno = EEXIST;
			return -1;
		}
	}

	if (TH_ISDIR(t))
		i = tar_extract_dir(t, realname);
	else if (TH_ISLNK(t))
		i = tar_extract_hardlink(t, realname);
	else if (TH_ISSYM(t))
		i = tar_extract_symlink(t, realname);
	else if (TH_ISCHR(t))
		i = tar_extract_chardev(t, realname);
	else if (TH_ISBLK(t))
		i = tar_extract_blockdev(t, realname);
	else if (TH_ISFIFO(t))
		i = tar_extract_fifo(t, realname);
	else if (TH_ISREG(t))
		i = tar_extract_regfile(t, realname);
	else
		return -1;

	if (i != 0)
		return i;

	i = tar_set_file_perms(t, realname);
	if (i != 0)
		return i;

	pathname_len = strlen(th_get_pathname(t)) + 1;
	realname_len = strlen(realname) + 1;
	lnp = (char *)calloc(1, pathname_len + realname_len);
	if (lnp == NULL)
		return -1;
	strcpy(&lnp[0], th_get_pathname(t));
	strcpy(&lnp[pathname_len], realname);
#ifdef DEBUG
	printf("tar_extract_file(): calling libtar_hash_add(): key=\"%s\", "
	       "value=\"%s\"\n", th_get_pathname(t), realname);
#endif
	if (libtar_hash_add(t->h, lnp) != 0)
		return -1;

	return 0;
}


/* extract regular file */
static int
tar_extract_regfile(TAR *t, char *filename)
{
	mode_t mode;
	size_t size;
	uid_t uid;
	gid_t gid;
	int fdout;
	int i, k;
	char buf[T_BLOCKSIZE];

#ifdef DEBUG
	printf("==> tar_extract_regfile(t=0x%lx, realname=\"%s\")\n", t,
	       filename);
#endif

	mode = th_get_mode(t);
	size = th_get_size(t);
	uid = th_get_uid(t);
	gid = th_get_gid(t);

	if (mkdirhier(t, filename) == -1)
		return -1;

#ifdef DEBUG
	printf("  ==> extracting: %s (mode %04o, uid %d, gid %d, %d bytes)\n",
	       filename, mode, uid, gid, size);
#endif
	fdout = openat(t->dirfd, filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fdout == -1)
	{
#ifdef DEBUG
		perror("openat()");
#endif
		return -1;
	}

#if 0
	/* change the owner.  (will only work if run as root) */
	if (fchown(fdout, uid, gid) == -1 && errno != EPERM)
	{
#ifdef DEBUG
		perror("fchown()");
#endif
		return -1;
	}

	/* make sure the mode isn't inheritted from a file we're overwriting */
	if (fchmod(fdout, mode & 07777) == -1)
	{
#ifdef DEBUG
		perror("fchmod()");
#endif
		return -1;
	}
#endif

	/* extract the file */
	for (i = size; i > 0; i -= T_BLOCKSIZE)
	{
		k = tar_block_read(t, buf);
		if (k != T_BLOCKSIZE)
		{
			if (k != -1)
				errno = EINVAL;
			close(fdout);
			return -1;
		}

		/* write block to output file */
		if (write(fdout, buf,
			  ((i > T_BLOCKSIZE) ? T_BLOCKSIZE : i)) == -1)
		{
			close(fdout);
			return -1;
		}
	}

	/* close output file */
	if (close(fdout) == -1)
		return -1;

#ifdef DEBUG
	printf("### done extracting %s\n", filename);
#endif

	return 0;
}


/* hardlink */
static int
tar_extract_hardlink(TAR * t, char *filename)
{
	char *linktgt = NULL;
	char *lnp;
	libtar_hashptr_t hp;

	if (mkdirhier(t, filename) == -1)
		return -1;
	libtar_hashptr_reset(&hp);
	if (libtar_hash_getkey(t->h, &hp, th_get_linkname(t),
			       (libtar_matchfunc_t)libtar_str_match) != 0)
	{
		lnp = (char *)libtar_hashptr_data(&hp);
		linktgt = &lnp[strlen(lnp) + 1];
	}
	else
		linktgt = th_get_linkname(t);

#ifdef DEBUG
	printf("  ==> extracting: %s (link to %s)\n", filename, linktgt);
#endif
	if (linkat(t->dirfd, linktgt, t->dirfd, filename, t->atflags) == -1)
	{
#ifdef DEBUG
		perror("linkat()");
#endif
		return -1;
	}

	return 0;
}


/* symlink */
static int
tar_extract_symlink(TAR *t, char *filename)
{

	if (mkdirhier(t, filename) == -1)
		return -1;

	if (unlink(filename) == -1 && errno != ENOENT)
		return -1;

#ifdef DEBUG
	printf("  ==> extracting: %s (symlink to %s)\n",
	       filename, th_get_linkname(t));
#endif
	if (symlinkat(th_get_linkname(t), t->dirfd, filename) == -1)
	{
#ifdef DEBUG
		perror("symlinkat()");
#endif
		return -1;
	}

	return 0;
}


/* character device */
static int
tar_extract_chardev(TAR *t, char *filename)
{
	mode_t mode = th_get_mode(t);
	unsigned long devmaj = th_get_devmajor(t);
	unsigned long devmin = th_get_devminor(t);

	if (mkdirhier(t, filename) == -1)
		return -1;

#ifdef DEBUG
	printf("  ==> extracting: %s (character device %ld,%ld)\n",
	       filename, devmaj, devmin);
#endif
	if (mknodat(t->dirfd, filename, mode | S_IFCHR,
		  makedev(devmaj, devmin)) == -1)
	{
#ifdef DEBUG
		perror("mknodat()");
#endif
		return -1;
	}

	return 0;
}


/* block device */
static int
tar_extract_blockdev(TAR *t, char *filename)
{
	mode_t mode = th_get_mode(t);
	unsigned long devmaj = th_get_devmajor(t);
	unsigned long devmin = th_get_devminor(t);

	if (mkdirhier(t, filename) == -1)
		return -1;

#ifdef DEBUG
	printf("  ==> extracting: %s (block device %ld,%ld)\n",
	       filename, devmaj, devmin);
#endif
	if (mknodat(t->dirfd, filename, mode | S_IFBLK,
		  makedev(devmaj, devmin)) == -1)
	{
#ifdef DEBUG
		perror("mknodat()");
#endif
		return -1;
	}

	return 0;
}


/* directory */
static int
tar_extract_dir(TAR *t, char *filename)
{
	mode_t mode;

	mode = th_get_mode(t);

	if (mkdirhier(t, filename) == -1)
		return -1;

#ifdef DEBUG
	printf("  ==> extracting: %s (mode %04o, directory)\n", filename,
	       mode);
#endif
	if (mkdirat(t->dirfd, filename, mode) == -1)
	{
		if (errno == EEXIST)
		{
			if (fchmodat(t->dirfd, filename, mode, 0) == -1)
			{
#ifdef DEBUG
				perror("fchmodat()");
#endif
				return -1;
			}
			else
			{
#ifdef DEBUG
				puts("  *** using existing directory");
#endif
				return 0;
			}
		}
		else
		{
#ifdef DEBUG
			perror("mkdirat()");
#endif
			return -1;
		}
	}

	return 0;
}


/* FIFO */
static int
tar_extract_fifo(TAR *t, char *filename)
{
	mode_t mode;

	mode = th_get_mode(t);

	if (mkdirhier(t, filename) == -1)
		return -1;

#ifdef DEBUG
	printf("  ==> extracting: %s (fifo)\n", filename);
#endif
	if (mkfifoat(t->dirfd, filename, mode) == -1)
	{
#ifdef DEBUG
		perror("mkfifoat()");
#endif
		return -1;
	}

	return 0;
}
