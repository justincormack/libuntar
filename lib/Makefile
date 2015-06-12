CFLAGS?=-O2 -Wall
SRC=extract.c handle.c
HEADERS=libuntar.h
OBJ=$(SRC:.c=.o)
LIB=libuntar.a
INCLUDES=-I.

default:	$(LIB)

extract.o:	extract.c $(HEADERS)
		$(CC) -c $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -o $@ $<

handle.o:	handle.c $(HEADERS)
		$(CC) -c $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -o $@ $<

$(LIB):		$(OBJ)
		ar rcs $(LIB) $(OBJ)

.PHONY:		clean
clean:		
		rm -f $(OBJ) $(LIB)
