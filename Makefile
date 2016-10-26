export CC=g++
export FLAGS=-fPIC -I include/ -Wall -Wextra -g -lpthread -pthread -O3 -Werror
export CFLAGS=$(FLAGS)
export CXXFLAGS=$(CFLAGS) -std=gnu++11
export SSLCFLAGS=$(CXXFLAGS) -L/home/rafael/git/wormhole/dependencies/compiled/libressl/usr/local/lib/ -lssl -lcrypto

all: lib/libnetlib.so lib/netlib.o #examples
examples: bin/cli bin/srv

lib/libnetlib.so: src/netlib.c src/netlibpp.cpp src/netlib_inline.c include/netlib.h include/netlib.hpp src/hptl.c
	mkdir -p lib
	$(CXX) $(CXXFLAGS) -shared -o $@ src/netlib.c src/netlibpp.cpp src/hptl.c

lib/netlib.o: src/netlib.c src/netlibpp.cpp src/netlib_inline.c include/netlib.h include/netlib.hpp src/hptl.c
	mkdir -p lib
	$(CXX) $(CXXFLAGS) -c src/netlib.c -include src/netlibpp.cpp -include src/hptl.c -o $@

clean:
	rm -rf bin lib

bin/cli: lib/netlib.o src/examples/cli.cpp
	mkdir -p bin
	$(CXX) $(SSLCFLAGS) $^ -o $@

bin/srv: lib/netlib.o src/examples/srv.cpp
	mkdir -p bin
	$(CXX) $(SSLCFLAGS) $^ -o $@
