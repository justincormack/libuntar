/* prototypes for borrowed "compatibility" code */

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <stdarg.h>
#include <stddef.h>

#ifdef HAVE_LIBGEN_H
# include <libgen.h>
#endif


#if defined(NEED_DIRNAME) && !defined(HAVE_DIRNAME)

char *openbsd_dirname(const char *);
# define dirname openbsd_dirname

#endif /* NEED_DIRNAME && ! HAVE_DIRNAME */


#ifdef NEED_FNMATCH
# ifndef HAVE_FNMATCH

#  define FNM_NOMATCH	1	/* Match failed. */

#  define FNM_NOESCAPE	0x01	/* Disable backslash escaping. */
#  define FNM_PATHNAME	0x02	/* Slash must be matched by slash. */
#  define FNM_PERIOD	0x04	/* Period must be matched by period. */

#  define FNM_LEADING_DIR 0x08	/* Ignore /<tail> after Imatch. */
#  define FNM_CASEFOLD	0x10	/* Case insensitive search. */
#  define FNM_IGNORECASE FNM_CASEFOLD
#  define FNM_FILE_NAME FNM_PATHNAME

int openbsd_fnmatch(const char *, const char *, int);
#  define fnmatch openbsd_fnmatch

# else /* HAVE_FNMATCH */

#  ifdef HAVE_FNMATCH_H
#   include <fnmatch.h>
#  endif

# endif /* ! HAVE_FNMATCH */
#endif /* NEED_FNMATCH */


#ifdef NEED_MAKEDEV

# ifdef MAJOR_IN_MKDEV
#  include <sys/mkdev.h>
# else
#  ifdef MAJOR_IN_SYSMACROS
#   include <sys/sysmacros.h>
#  endif
# endif

/*
** On most systems makedev() has two args.
** Some weird systems, like QNX6, have makedev() functions that expect
** an extra first argument for "node", which can be 0 for a local
** machine.
*/

# ifdef MAKEDEV_THREE_ARGS
#  define compat_makedev(maj, min)	makedev(0, maj, min)
# else
#  define compat_makedev		makedev
# endif

#endif /* NEED_MAKEDEV */


#if defined(NEED_SNPRINTF) && !defined(HAVE_SNPRINTF)

int mutt_snprintf(char *, size_t, const char *, ...);
int mutt_vsnprintf(char *, size_t, const char *, va_list);
#define snprintf mutt_snprintf
#define vsnprintf mutt_vsnprintf

#endif /* NEED_SNPRINTF && ! HAVE_SNPRINTF */


#if defined(NEED_STRLCAT) && !defined(HAVE_STRLCAT)

size_t strlcat(char *, const char *, size_t);

#endif /* NEED_STRLCAT && ! HAVE_STRLCAT */


#if defined(NEED_STRLCPY) && !defined(HAVE_STRLCPY)

size_t strlcpy(char *, const char *, size_t);

#endif /* NEED_STRLCPY && ! HAVE_STRLCPY */


#if defined(NEED_STRDUP) && !defined(HAVE_STRDUP)

char *openbsd_strdup(const char *);
# define strdup openbsd_strdup

#endif /* NEED_STRDUP && ! HAVE_STRDUP */


#if defined(NEED_STRMODE) && !defined(HAVE_STRMODE)

void strmode(register mode_t, register char *);

#endif /* NEED_STRMODE && ! HAVE_STRMODE */


#if defined(NEED_STRRSTR) && !defined(HAVE_STRRSTR)

char *strrstr(char *, char *);

#endif /* NEED_STRRSTR && ! HAVE_STRRSTR */


#ifdef NEED_STRSEP

# ifdef HAVE_STRSEP
#  define _LINUX_SOURCE_COMPAT		/* needed on AIX 4.3.3 */
# else

char *strsep(register char **, register const char *);

# endif

#endif /* NEED_STRSEP */


