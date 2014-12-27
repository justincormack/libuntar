CFLAGS?=-O2 -Wall
SRC=block.c decode.c extract.c handle.c util.c
HEADERS=libtar.h
OBJ=$(SRC:.c=.o)
LIB=libuntar.a

default:	${LIB}

${LIB}:		${OBJ} ${HEADERS}
		ar rcs ${LIB} ${OBJ}

.PHONY:		clean
clean:		
		rm -f ${OBJ} ${LIB}
