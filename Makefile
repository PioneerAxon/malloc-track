all: debug release

debug:
	mkdir debug
	cd debug; cmake -DCMAKE_BUILD_TYPE=Debug ..; make

release:
	mkdir release
	cd release; cmake -DCMAKE_BUILD_TYPE=Release ..; make

clean:
	rm -rf debug release
