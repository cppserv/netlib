export CC=gcc
export CXX=g++
export FLAGS=-fPIC -I include/ -Wall -Wextra -g -pthread -O3 -Werror -DNL_NOHPTL -I/usr/local/opt/openssl/include
export CFLAGS=$(FLAGS) -std=gnu99
export CXXFLAGS=$(CFLAGS) -std=gnu++11
export LDFLAGS=-lpthread -L/home/rafael/git/wormhole/dependencies/compiled/libressl/usr/local/lib/ -lssl -lcrypto

all: lib/libnetlib.so lib/netlib.o #examples
examples: bin/cli bin/srv

lib/libnetlib.so: src/netlib.c src/netlibpp.cpp src/netlib_inline.c include/netlib.h include/netlib.hpp src/hptl.c
	mkdir -p lib
	$(CXX) $(CXXFLAGS) -shared -o $@ src/netlib.c src/netlibpp.cpp src/hptl.c

lib/netlib.o: lib/netlibc.o lib/netlibpp.o lib/netlib_inline.o
	mkdir -p lib
	ld -r -o $@ $^

lib/netlibc.o: src/netlib.c include/netlib.h
	$(CC) $(CFLAGS) -c -o $@ $<

lib/netlib_inline.o: src/netlib_inline.c include/netlib.h
	$(CC) $(CFLAGS) -c -o $@ $<

lib/netlibpp.o: src/netlibpp.cpp include/netlib.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

lib/hptl.o: src/hptl.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf bin lib

bin/cli: lib/netlib.o src/examples/cli.cpp
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

bin/srv: lib/netlib.o src/examples/srv.cpp
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
