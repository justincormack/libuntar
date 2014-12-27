SRC=untar.c
OBJ=$(SRC:.c=.o)
LIBS=../lib/libuntar.a
OUT=untar
INCLUDES=-I../lib

${OUT}:		${OBJ} ${LIBS}
		${CC} ${LDFLAGS} $^ -o $@

.PHONY:		clean
clean:		
		rm -f ${OBJ} ${OUT}
