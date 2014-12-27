CFLAGS?=-O2 -Wall
SRC=untar.c
OBJ=$(SRC:.c=.o)
LIBS=../lib/libuntar.a
OUT=untar
INCLUDES=-I../lib
HEADERS=../lib/libuntar.h

default:	$(OUT)

untar.o:	untar.c $(HEADERS)
		$(CC) -c $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -o $@ $<

$(OUT):		$(OBJ) $(LIBS)
		$(CC) $(LDFLAGS) $(OBJ) $(LIBS) -o $@

.PHONY:		clean
clean:		
		rm -f $(OBJ) $(OUT)
