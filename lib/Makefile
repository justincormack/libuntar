CFLAGS?=-O2 -Wall
SRC=block.c decode.c extract.c handle.c util.c
HEADERS=libuntar.h
OBJ=$(SRC:.c=.o)
LIB=libuntar.a
INCLUDES=-I.

default:	$(LIB)

block.o:	block.c $(HEADERS)
		$(CC) -c $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -o $@ $<

decode.o:	decode.c $(HEADERS)
		$(CC) -c $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -o $@ $<

extract.o:	extract.c $(HEADERS)
		$(CC) -c $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -o $@ $<

handle.o:	handle.c $(HEADERS)
		$(CC) -c $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -o $@ $<

util.o:		util.c $(HEADERS)
		$(CC) -c $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -o $@ $<

$(LIB):		$(OBJ)
		ar rcs $(LIB) $(OBJ)

.PHONY:		clean
clean:		
		rm -f $(OBJ) $(LIB)
