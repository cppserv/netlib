export CC=g++
export FLAGS=-fPIC -I include/ -Wall -Wextra -g -lpthread -pthread -O3 -Werror
export SSLCFLAGS=
export CFLAGS=$(FLAGS) $(SSLCFLAGS) -std=gnu++11

all: lib/libnetlib.so lib/netlib.o

lib/libnetlib.so: src/netlib.c src/netlib.cpp src/netlib_inline.c include/netlib.h include/netlib.hpp src/hptl.c 
	mkdir -p lib
	$(CXX) $(CFLAGS) -shared -o $@ src/netlib.c src/netlib.cpp src/hptl.c

lib/netlib.o: src/netlib.c src/netlib.cpp src/netlib_inline.c include/netlib.h include/netlib.hpp src/hptl.c 
	mkdir -p lib
	$(CXX) $(CFLAGS) -c -o $@ src/netlib.c src/netlib.cpp src/hptl.c
