/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  output.c - libtar code to print out tar header blocks
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <internal.h>

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <limits.h>
#include <sys/param.h>
#include <string.h>


#ifndef _POSIX_LOGIN_NAME_MAX
# define _POSIX_LOGIN_NAME_MAX	9
#endif


void
th_print(TAR *t)
{
	puts("\nPrinting tar header:");
	printf("  name     = \"%.100s\"\n", t->th_buf.name);
	printf("  mode     = \"%.8s\"\n", t->th_buf.mode);
	printf("  uid      = \"%.8s\"\n", t->th_buf.uid);
	printf("  gid      = \"%.8s\"\n", t->th_buf.gid);
	printf("  size     = \"%.12s\"\n", t->th_buf.size);
	printf("  mtime    = \"%.12s\"\n", t->th_buf.mtime);
	printf("  chksum   = \"%.8s\"\n", t->th_buf.chksum);
	printf("  typeflag = \'%c\'\n", t->th_buf.typeflag);
	printf("  linkname = \"%.100s\"\n", t->th_buf.linkname);
	printf("  magic    = \"%.6s\"\n", t->th_buf.magic);
	/*printf("  version  = \"%.2s\"\n", t->th_buf.version); */
	printf("  version[0] = \'%c\',version[1] = \'%c\'\n",
	       t->th_buf.version[0], t->th_buf.version[1]);
	printf("  uname    = \"%.32s\"\n", t->th_buf.uname);
	printf("  gname    = \"%.32s\"\n", t->th_buf.gname);
	printf("  devmajor = \"%.8s\"\n", t->th_buf.devmajor);
	printf("  devminor = \"%.8s\"\n", t->th_buf.devminor);
	printf("  prefix   = \"%.155s\"\n", t->th_buf.prefix);
	printf("  padding  = \"%.12s\"\n", t->th_buf.padding);
	printf("  gnu_longname = \"%s\"\n",
	       (t->th_buf.gnu_longname ? t->th_buf.gnu_longname : "[NULL]"));
	printf("  gnu_longlink = \"%s\"\n",
	       (t->th_buf.gnu_longlink ? t->th_buf.gnu_longlink : "[NULL]"));
}

