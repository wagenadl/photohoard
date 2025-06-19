<img alt="Photohoard" src=https://github.com/wagenadl/photohoard/blob/main/docs/source/banner.svg width="100%">

# Photohoard - Photography Collection Management

## Introduction

Photohoard is a program to manage your collection of photographs.
Photohoard also contains lots of tools for basic editing.

## Installation

Binaries are available for Ubuntu Linux. To install from git source,
simply type:

    cmake -S . -B build
    cmake --build build
        
You may have to get some dependencies satisfied first. On Ubuntu, the
following will likely suffice:

        sudo apt install git qtbase5-dev qttools5-dev-tools asciidoc \
             docbook-xml docbook-xsl libexiv2-dev liblcms2-dev \
             libopencv-imgproc-dev libxcb-randr0-dev libx11-xcb-dev  
             
The photohoard binary may be run directly from the "build" folder. 
More conveniently, to install in /usr/local, type

    sudo make -C build install
    
or to build a .deb installation package, type

    cd build; cpack
    
the resulting .deb may then be installed with

    sudo dpkg -i ./photohoard_0.2.0-1_amd64.deb

## Acknowledgments

Photohoard is developed by Daniel A. Wagenaar and is distributed under the
terms of the GNU GENERAL PUBLIC LICENSE, Version 3.

The following items were obtained from other sources under license:

- The “Trash” icon: Icons made by <a href="https://www.flaticon.com/authors/smashicons" title="Smashicons">Smashicons</a> from <a href="https://www.flaticon.com/" title="Flaticon"> www.flaticon.com</a>.

- The open and slashed “Eye” icons: From Font Awesome. License terms are at: https://fontawesome.com/license.

