.PHONY: all clean


CROSS_COMPILE=

IDIR =../include
CC=$(CROSS_COMPILE)gcc
CFLAGS=-I$(IDIR)

ODIR=.
LDIR =../lib
LIBS=-lm

_DEPS = writer.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = writer.o
OBJ = $(_OBJ)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: writer

writer: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~ writer
