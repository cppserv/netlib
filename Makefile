export CC=gcc
export FLAGS=-fPIC -I include/ -Wall -Wextra -g -lpthread -pthread -O3 -Werror
export SSLCFLAGS=
export CFLAGS=$(FLAGS) $(SSLCFLAGS) -std=gnu11

all: lib/netlib.so

lib/netlib.so: src/netlib.c src/netlib_inline.c include/netlib.h
	mkdir -p lib
	$(CC) $(CFLAGS) -shared -o $@ $<
