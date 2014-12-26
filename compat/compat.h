/* prototypes for borrowed "compatibility" code */

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <stdarg.h>
#include <stddef.h>

#ifdef HAVE_LIBGEN_H
# include <libgen.h>
#endif


char *openbsd_dirname(const char *);
char *openbsd_basename(const char *);

#if defined(NEED_STRLCPY) && !defined(HAVE_STRLCPY)

size_t strlcpy(char *, const char *, size_t);

#endif /* NEED_STRLCPY && ! HAVE_STRLCPY */


