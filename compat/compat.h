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


#if defined(NEED_STRLCAT) && !defined(HAVE_STRLCAT)

size_t strlcat(char *, const char *, size_t);

#endif /* NEED_STRLCAT && ! HAVE_STRLCAT */


#if defined(NEED_STRLCPY) && !defined(HAVE_STRLCPY)

size_t strlcpy(char *, const char *, size_t);

#endif /* NEED_STRLCPY && ! HAVE_STRLCPY */


