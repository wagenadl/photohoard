ALL: photohoard NDOC

photohoard: src/Makefile
	+make -C src

debug: src/Makefile
	+make -C src debug

NDOC:; naturaldocs -i src -o HTML ndoc -p .ndocproject

#icons:;
#	mkdir -p res/icons
#	( cd res; ./spliticons.pl )
# - For whatever reason, spliticons.pl doesn't work right now.

src/Makefile: src/photohoard.pro
	( cd src; qmake -qt=qt5 )

clean:;
	make -C src clean
	rm -f src/*/*~

.PHONY: ALL photohoard debug icons NDOC
