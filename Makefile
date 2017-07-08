ifndef BUILD_TYPE
BUILD_TYPE=Debug
endif

ifndef VERBOSE
VERBOSE=0
endif

.SILENT:

all:
	mkdir -p build
	cd build; cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) ..; make VERBOSE=$(VERBOSE)

clean:
	rm -rf build
