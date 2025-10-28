# Makefile - Part of Photohoard

# Actual build process is through cmake, but typing "make"
# is easier than "cmake -S . -B build" etc.

release: prep-release
	+cmake --build build --config Release

prep-release:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release  

debug: prep-debug
	+cmake --build build-debug --config Debug

prep-debug:
	cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug 

# linux only:
deb:	release
	(cd build; cpack )

# mac only:
dmg:	release
	+make -C build dmg

tar:;	git archive -o ../photohoard.tar.gz --prefix=photohoard/ HEAD

clean:; rm -rf build build-debug

.PHONY: release debug tar deb dmg
