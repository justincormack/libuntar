CFLAGS?=-O2 -Wall
SRC=untar.c
HEADERS=libuntar.h
OBJ=$(SRC:.c=.o)
LIB=libuntar.a
INCLUDES=-I.

default:	$(LIB)

untar.o:	untar.c $(HEADERS)
		$(CC) -c $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -o $@ $<

$(LIB):		$(OBJ)
		ar rcs $(LIB) $(OBJ)

.PHONY:		clean
clean:		
		rm -f $(OBJ) $(LIB)
