/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <libuntar.h>

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <pwd.h>
#include <grp.h>

/*
**  util.c - miscellaneous utility code for libtar
*/

/* calculate header checksum */
int
th_crc_calc(TAR *t)
{
	int i, sum = 0;

	for (i = 0; i < T_BLOCKSIZE; i++)
		sum += ((unsigned char *)(&(t->th_buf)))[i];
	for (i = 0; i < 8; i++)
		sum += (' ' - (unsigned char)t->th_buf.chksum[i]);

	return sum;
}


/* calculate a signed header checksum */
int
th_signed_crc_calc(TAR *t)
{
	int i, sum = 0;

	for (i = 0; i < T_BLOCKSIZE; i++)
		sum += ((signed char *)(&(t->th_buf)))[i];
	for (i = 0; i < 8; i++)
		sum += (' ' - (signed char)t->th_buf.chksum[i]);

	return sum;
}


/* string-octal to integer conversion */
int
oct_to_int(char *oct)
{
	int i;

	return sscanf(oct, "%o", &i) == 1 ? i : 0;
}

/*
**  block.c - libtar code to handle tar archive header blocks
*/

#define BIT_ISSET(bitmask, bit) ((bitmask) & (bit))


/* read a header block */
/* FIXME: the return value of this function should match the return value
	  of tar_block_read(), which is a macro which references a prototype
	  that returns a ssize_t.  So far, this is safe, since tar_block_read()
	  only ever reads 512 (T_BLOCKSIZE) bytes at a time, so any difference
	  in size of ssize_t and int is of negligible risk.  BUT, if
	  T_BLOCKSIZE ever changes, or ever becomes a variable parameter
	  controllable by the user, all the code that calls it,
	  including this function and all code that calls it, should be
	  fixed for security reasons.
	  Thanks to Chris Palmer for the critique.
*/
int
th_read_internal(TAR *t)
{
	int i;
	int num_zero_blocks = 0;

#ifdef DEBUG
	printf("==> th_read_internal\n");
#endif

	while ((i = tar_block_read(t, &(t->th_buf))) == T_BLOCKSIZE)
	{
		/* two all-zero blocks mark EOF */
		if (t->th_buf.name[0] == '\0')
		{
			num_zero_blocks++;
			if (!BIT_ISSET(t->options, TAR_IGNORE_EOT)
			    && num_zero_blocks >= 2)
				return 0;	/* EOF */
			else
				continue;
		}

		/* verify magic and version */
		if (BIT_ISSET(t->options, TAR_CHECK_MAGIC)
		    && strncmp(t->th_buf.magic, TMAGIC, TMAGLEN - 1) != 0)
		{
#ifdef DEBUG
			puts("!!! unknown magic value in tar header");
#endif
			return -2;
		}

		if (BIT_ISSET(t->options, TAR_CHECK_VERSION)
		    && strncmp(t->th_buf.version, TVERSION, TVERSLEN) != 0)
		{
#ifdef DEBUG
			puts("!!! unknown version value in tar header");
#endif
			return -2;
		}

		/* check chksum */
		if (!BIT_ISSET(t->options, TAR_IGNORE_CRC)
		    && !th_crc_ok(t))
		{
#ifdef DEBUG
			puts("!!! tar header checksum error");
#endif
			return -2;
		}

		break;
	}

#ifdef DEBUG
	printf("<== th_read_internal(): returning %d\n", i);
#endif
	return i;
}


/* wrapper function for th_read_internal() to handle GNU extensions */
int
th_read(TAR *t)
{
	int i;
	size_t sz, j, blocks;
	char *ptr;

#ifdef DEBUG
	printf("==> th_read(t=0x%lx)\n", t);
#endif

	if (t->th_buf.gnu_longname != NULL)
		free(t->th_buf.gnu_longname);
	if (t->th_buf.gnu_longlink != NULL)
		free(t->th_buf.gnu_longlink);
	memset(&(t->th_buf), 0, sizeof(struct tar_header));

	i = th_read_internal(t);
	if (i == 0)
		return 1;
	else if (i != T_BLOCKSIZE)
	{
		if (i != -1)
			errno = EINVAL;
		return -1;
	}

	/* check for GNU long link extention */
	if (TH_ISLONGLINK(t))
	{
		sz = th_get_size(t);
		blocks = (sz / T_BLOCKSIZE) + (sz % T_BLOCKSIZE ? 1 : 0);
		if (blocks > ((size_t)-1 / T_BLOCKSIZE))
		{
			errno = E2BIG;
			return -1;
		}
#ifdef DEBUG
		printf("    th_read(): GNU long linkname detected "
		       "(%ld bytes, %d blocks)\n", sz, blocks);
#endif
		t->th_buf.gnu_longlink = (char *)malloc(blocks * T_BLOCKSIZE);
		if (t->th_buf.gnu_longlink == NULL)
			return -1;

		for (j = 0, ptr = t->th_buf.gnu_longlink; j < blocks;
		     j++, ptr += T_BLOCKSIZE)
		{
#ifdef DEBUG
			printf("    th_read(): reading long linkname "
			       "(%d blocks left, ptr == %ld)\n", blocks-j, ptr);
#endif
			i = tar_block_read(t, ptr);
			if (i != T_BLOCKSIZE)
			{
				if (i != -1)
					errno = EINVAL;
				return -1;
			}
#ifdef DEBUG
			printf("    th_read(): read block == \"%s\"\n", ptr);
#endif
		}
#ifdef DEBUG
		printf("    th_read(): t->th_buf.gnu_longlink == \"%s\"\n",
		       t->th_buf.gnu_longlink);
#endif

		i = th_read_internal(t);
		if (i != T_BLOCKSIZE)
		{
			if (i != -1)
				errno = EINVAL;
			return -1;
		}
	}

	/* check for GNU long name extention */
	if (TH_ISLONGNAME(t))
	{
		sz = th_get_size(t);
		blocks = (sz / T_BLOCKSIZE) + (sz % T_BLOCKSIZE ? 1 : 0);
		if (blocks > ((size_t)-1 / T_BLOCKSIZE))
		{
			errno = E2BIG;
			return -1;
		}
#ifdef DEBUG
		printf("    th_read(): GNU long filename detected "
		       "(%ld bytes, %d blocks)\n", sz, blocks);
#endif
		t->th_buf.gnu_longname = (char *)malloc(blocks * T_BLOCKSIZE);
		if (t->th_buf.gnu_longname == NULL)
			return -1;

		for (j = 0, ptr = t->th_buf.gnu_longname; j < blocks;
		     j++, ptr += T_BLOCKSIZE)
		{
#ifdef DEBUG
			printf("    th_read(): reading long filename "
			       "(%d blocks left, ptr == %ld)\n", blocks-j, ptr);
#endif
			i = tar_block_read(t, ptr);
			if (i != T_BLOCKSIZE)
			{
				if (i != -1)
					errno = EINVAL;
				return -1;
			}
#ifdef DEBUG
			printf("    th_read(): read block == \"%s\"\n", ptr);
#endif
		}
#ifdef DEBUG
		printf("    th_read(): t->th_buf.gnu_longname == \"%s\"\n",
		       t->th_buf.gnu_longname);
#endif

		i = th_read_internal(t);
		if (i != T_BLOCKSIZE)
		{
			if (i != -1)
				errno = EINVAL;
			return -1;
		}
	}

#if 0
	/*
	** work-around for old archive files with broken typeflag fields
	** NOTE: I fixed this in the TH_IS*() macros instead
	*/

	/*
	** (directories are signified with a trailing '/')
	*/
	if (t->th_buf.typeflag == AREGTYPE
	    && t->th_buf.name[strlen(t->th_buf.name) - 1] == '/')
		t->th_buf.typeflag = DIRTYPE;

	/*
	** fallback to using mode bits
	*/
	if (t->th_buf.typeflag == AREGTYPE)
	{
		mode = (mode_t)oct_to_int(t->th_buf.mode);

		if (S_ISREG(mode))
			t->th_buf.typeflag = REGTYPE;
		else if (S_ISDIR(mode))
			t->th_buf.typeflag = DIRTYPE;
		else if (S_ISFIFO(mode))
			t->th_buf.typeflag = FIFOTYPE;
		else if (S_ISCHR(mode))
			t->th_buf.typeflag = CHRTYPE;
		else if (S_ISBLK(mode))
			t->th_buf.typeflag = BLKTYPE;
		else if (S_ISLNK(mode))
			t->th_buf.typeflag = SYMTYPE;
	}
#endif

	return 0;
}

/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  decode.c - libtar code to decode tar header blocks
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

/* determine full path name */
char *
th_get_pathname(TAR *t)
{
	if (t->th_buf.gnu_longname)
		return t->th_buf.gnu_longname;

	/* allocate the th_pathname buffer if not already */
	if (t->th_pathname == NULL)
	{
		t->th_pathname = malloc(MAXPATHLEN * sizeof(char));
		if (t->th_pathname == NULL)
			/* out of memory */
			return NULL;
	}

	if (t->th_buf.prefix[0] == '\0')
	{
		snprintf(t->th_pathname, MAXPATHLEN, "%.100s", t->th_buf.name);
	}
	else
	{
		snprintf(t->th_pathname, MAXPATHLEN, "%.155s/%.100s",
			 t->th_buf.prefix, t->th_buf.name);
	}

	/* will be deallocated in tar_close() */
	return t->th_pathname;
}


uid_t
th_get_uid(TAR *t)
{
	int uid;
#ifdef USE_SYMBOLIC_IDS
	struct passwd *pw;

	pw = getpwnam(t->th_buf.uname);
	if (pw != NULL)
		return pw->pw_uid;

	/* if the password entry doesn't exist */
#endif
	sscanf(t->th_buf.uid, "%o", &uid);
	return uid;
}


gid_t
th_get_gid(TAR *t)
{
	int gid;
#ifdef USE_SYMBOLIC_IDS
	struct group *gr;

	gr = getgrnam(t->th_buf.gname);
	if (gr != NULL)
		return gr->gr_gid;

	/* if the group entry doesn't exist */
#endif
	sscanf(t->th_buf.gid, "%o", &gid);
	return gid;
}


mode_t
th_get_mode(TAR *t)
{
	mode_t mode;

	mode = (mode_t)oct_to_int(t->th_buf.mode);
	if (! (mode & S_IFMT))
	{
		switch (t->th_buf.typeflag)
		{
		case SYMTYPE:
			mode |= S_IFLNK;
			break;
		case CHRTYPE:
			mode |= S_IFCHR;
			break;
		case BLKTYPE:
			mode |= S_IFBLK;
			break;
		case DIRTYPE:
			mode |= S_IFDIR;
			break;
		case FIFOTYPE:
			mode |= S_IFIFO;
			break;
		case AREGTYPE:
			if (t->th_buf.name[strlen(t->th_buf.name) - 1] == '/')
			{
				mode |= S_IFDIR;
				break;
			}
			/* FALLTHROUGH */
		case LNKTYPE:
		case REGTYPE:
		default:
			mode |= S_IFREG;
		}
	}

	return mode;
}


