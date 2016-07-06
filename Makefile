export CC=g++
export FLAGS=-fPIC -I include/ -Wall -Wextra -g -lpthread -pthread -O3 -Werror
export SSLCFLAGS=
export CFLAGS=$(FLAGS) $(SSLCFLAGS) -std=gnu++11

all: lib/netlib.so lib/netlib.o

lib/netlib.so: src/netlib.c src/netlib.cpp src/netlib_inline.c include/netlib.h include/netlib.hpp
	mkdir -p lib
	$(CXX) $(CFLAGS) -shared -o $@ $<

lib/netlib.o: src/netlib.c src/netlib.cpp src/netlib_inline.c include/netlib.h include/netlib.hpp
	mkdir -p lib
	$(CXX) $(CFLAGS) -c -o $@ $<
