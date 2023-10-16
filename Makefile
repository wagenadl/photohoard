# This Makefile is for convenience only. The project actually uses cmake

release:;
	cmake -S . -B build
	+cmake --build build

debug:;
	cmake  -DCMAKE_BUILD_TYPE=Debug -S . -B debug
	+cmake --build debug 

clean:; rm -rf build

.PHONY: release debug
