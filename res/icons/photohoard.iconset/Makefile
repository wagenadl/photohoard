all: photohoard_16x16.png photohoard_24x24.png photohoard_32x32.png photohoard_48x48.png	\
     photohoard_64x64.png photohoard_128x128.png photohoard_256x256.png photohoard_512x512.png

photohoard_%.png: photohoard.svg
	inkscape --export-width  $(shell echo $*|sed 's/x.*//') \
		 --export-height $(shell echo $*|sed 's/.*x//') \
		 --export-filename $@ $<

.PHONY: all
