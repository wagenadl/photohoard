# This makefile is for Linux and MacOS building
# Windows uses QCreator.

# Unix installation
ifdef DESTDIR
  # Debian uses this
  INSTALLPATH = $(DESTDIR)/usr
  SHAREPATH = $(DESTDIR)/usr/share
else
  INSTALLPATH = /usr/local
  SHAREPATH = /usr/local/share
endif

UNAME=$(shell uname -s)

ifeq (, $(shell which qmake-qt5))
  QMAKE=qmake
else
  QMAKE=qmake-qt5
endif

ifeq ($(UNAME),Linux)
  # Linux
  SELECTQT=QT_SELECT=5
else
  ifeq ($(UNAME),Darwin)
    # Mac OS
    QROOT=/Users/wagenaar/Qt-5.7/5.7
    QBINPATH=$(QROOT)/clang_64/bin
    QMAKE=$(QBINPATH)/qmake
  else
    $(error Unknown operating system. This Makefile is for Mac or Linux.)
  endif
endif

DOCPATH = $(SHAREPATH)/doc/photohoard

# Linux and Mac building
all: bin man

update:
	res/updatesources.sh

clean:
	+rm -rf build
	+rm -rf build-doc
	+rm -rf .ndocproject ndoc
	+rm -f test/Makefile*

bin: prep
	+make -C build release

prep:
	mkdir -p build
	( cd build; $(SELECTQT) $(QMAKE) ../src/photohoard.pro )

debug: prep
	+make -C build debug

native:
	mkdir -p build
	( cd build; PHOTOHOARD_CXXFLAGS='-march=native' $(SELECTQT) $(QMAKE) ../src/photohoard.pro )
	+make -C build release

# Unix installation
install: all
	install -d $(INSTALLPATH)/bin
	install -d $(SHAREPATH)/man/man1
	install -d $(SHAREPATH)/applications
	install -d $(DOCPATH)
	install -d $(SHAREPATH)/pixmaps
	install build/photohoard $(INSTALLPATH)/bin/photohoard
	cp build-doc/photohoard.1 $(SHAREPATH)/man/man1/photohoard.1
	cp res/photohoard.svg $(SHAREPATH)/pixmaps/photohoard.svg
	install res/photohoard.desktop $(SHAREPATH)/applications/photohoard.desktop

	cp README.md $(DOCPATH)/readme
	gzip -9 $(DOCPATH)/readme
	cp CHANGELOG $(DOCPATH)/changelog
	gzip -9 $(DOCPATH)/changelog

man:
	mkdir -p build-doc
	cp doc/Makefile build-doc/
	+make -C build-doc photohoard.1

# Tar preparation
tar: all
	git archive -o ../photohoard.tar.gz --prefix=photohoard/ HEAD

.PHONY: src all clean tar bin man install prep

NDOC:; naturaldocs -i src -o HTML ndoc -p .ndocproject

