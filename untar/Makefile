CFLAGS?=-O2 -Wall
SRC=untar.c
OBJ=$(SRC:.c=.o)
LIBS=../lib/libuntar.a
OUT=untar
INCLUDES=-I../lib

%.o:		%.c ${HEADERS}
		$(CC) -c $(CPPFLAGS) $(CFLAGS) ${INCLUDES} $< -o $@		

${OUT}:		${OBJ} ${LIBS}
		${CC} ${LDFLAGS} $^ -o $@

.PHONY:		clean
clean:		
		rm -f ${OBJ} ${OUT}
