libuntar - C library for extracting tar files
======

libuntar is a very simple library for extracting tar files from within C programs.

It is based on libtar from http://repo.or.cz/w/libtar.git

The idea was to remove all code for creating and appending to tarfiles,
add tests, and remove all support for obsolete platforms, and simplify
the build process, so there is just a very minimal library to extract
tar files, optionally with decompression. Tests coming soon.

The library is currently just under 10k stripped on amd64.

It requires a modern Posix environment: Linux, NetBSD 7, OpenBSD 5.6 as it
uses the \*at(2) family of functions. FreeBSD 10.1 is still missing
utimensat(2) so not curently working (please fix FreeBSD!).

Build instructions:
```
   set CFLAGS, LDFLAGS if required
   make
```
There is an example untar program using the library.

Compile time options:
   -DUSE\_SYMBOLIC\_IDS use symbolic names from tarfiles, default is numeric
   -DDEBUG verbose debug
   -DHAVE\_LIBZ for untar, use libz
