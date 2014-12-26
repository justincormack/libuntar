#include <string.h>

char *openbsd_dirname(const char *);
char *openbsd_basename(const char *);
#ifndef __OpenBSD__
size_t strlcpy(char *, const char *, size_t);
#endif

