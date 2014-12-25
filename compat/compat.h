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


#if defined(NEED_STRMODE) && !defined(HAVE_STRMODE)

void strmode(register mode_t, register char *);

#endif /* NEED_STRMODE && ! HAVE_STRMODE */


#ifdef NEED_STRSEP

# ifdef HAVE_STRSEP
#  define _LINUX_SOURCE_COMPAT		/* needed on AIX 4.3.3 */
# else

char *strsep(register char **, register const char *);

# endif

#endif /* NEED_STRSEP */


