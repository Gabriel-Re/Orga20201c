CC = gcc
CFLAGS ?= -g -O2 -std=c11 -Wall -Wextra -Wvla -Wconversion -Wfloat-conversion
LDFLAGS := -lm
VFLAGS = --leak-check=full --track-origins=yes --show-reachable=yes
CFLAGSGDB = -g -O0 -std=c11 -Wall -Wextra -Wvla

cachesim:
	$(CC) *.c $(CFLAGS) $(LDFLAGS) -o cachesim

gdb:
	$(CC) *.c $(CFLAGSGDB) $(LDFLAGS) -o cachesim

pruebas:
	valgrind $(VFLAGS) ./cachesim adpcm.xex 2048 2 64

pruebas_verboso:
	valgrind $(VFLAGS) ./cachesim adpcm.xex 2048 2 64 -v 0 15000

clean:
	rm -f cachesim

.PHONY: all clean
