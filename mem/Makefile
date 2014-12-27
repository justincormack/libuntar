CFLAGS?=-O2 -Wall
SRC=untar.c
OBJ=$(SRC:.c=.o)
LIBS=../lib/libuntar.a
OUT=untar
INCLUDES=-I../lib
HEADERS=../lib/libtar.h

TARFILE=image.tar
TOUT=$(TARFILE:.tar=.tar.obj)
OBJCOPY?=objcopy

default:	$(OUT)

untar.o:	untar.c $(HEADERS)
		$(CC) -c $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -o $@ $<

$(OUT):		$(OBJ) $(LIBS) $(TOUT)
		$(CC) $(LDFLAGS) $(OBJ) $(TOUT) $(LIBS) -o $@

$(TOUT):	$(TARFILE)
		$(LD) -r -o $(TOUT) -z noexecstack --format=binary $(TARFILE)
		$(OBJCOPY) --rename-section .data=.rodata,alloc,load,readonly,data,contents $(TOUT)

.PHONY:		clean
clean:		
		rm -f $(OUT) $(TOUT) $(OBJ)
