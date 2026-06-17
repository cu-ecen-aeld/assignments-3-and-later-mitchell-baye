.PHONY: all clean

CROSS_COMPILE=

CC=$(CROSS_COMPILE)gcc
USE_AESD_CHAR_DEVICE ?= 1
CFLAGS=-Wall -Wextra
CFLAGS += -DUSE_AESD_CHAR_DEVICE=$(USE_AESD_CHAR_DEVICE)
LIBS=-lpthread

_OBJ = aesdsocket.o
OBJ = $(_OBJ)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

all: aesdsocket

aesdsocket: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIBS)

clean:
	rm -f *.o *~ core aesdsocket

