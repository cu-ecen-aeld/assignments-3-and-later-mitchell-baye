.PHONY: all clean

CROSS_COMPILE=

IDIR =../include
CC=$(CROSS_COMPILE)gcc
CFLAGS=-I$(IDIR) -Wall -Wextra
LIBS=-lpthread

ODIR= .
LDIR =../lib

_DEPS = aesdsocket.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = aesdsocket.o
OBJ = $(_OBJ)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: aesdsocket

aesdsocket: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~ aesdsocket

