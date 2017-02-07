all: $(wildcard src/*c src/*cpp include/*h include/*hpp) | build 
	cd build && cmake ..
	cd build && ${MAKE} --no-print-directory

build:
	mkdir -p build
