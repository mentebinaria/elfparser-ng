# elfparser-ng

Multiplatform CLI and GUI tool to show information about ELF files.

![gui](assets/elfparser-ng.png)
![cli](assets/elfparser-ng-cli.png)

This project aims to keep the great [elfparser](https://github.com/jacob-baines/elfparser) alive. Plans include updating and adding new features to it.

## What problem are you trying to solve?

Keep one of the few multiplatform graphical tool to parse ELF files up to date and improve it with new features.

## Features

* Parse ELF headers and segments.
* Show symbols.
* Hex Dump.
* Calculate file's entropy.

## Installing

You can just download a pre-compiled release from the [Releases](https://github.com/mentebinaria/elfparser-ng/releases) section and use it.

## Building

If you want to build elfparser-ng yourself, here's an useful tutorial:

[![asciicast](https://asciinema.org/a/444072.svg)](https://asciinema.org/a/444072)

To build on Windows, set the `option(windows "Enable Windows build." OFF` to `ON` in [CMakeFiles](CMakeLists.txt) file. Then, download and install CMake - and all the dependencies - and compile it using Visual Studio.

### Compile Targets

ELF Parser has a number of compilation targets that can be configured by CMakeLists.txt. The targets are:

* Unit tests
* CLI build
* GUI build

### Dependencies

[Boost](http://robots.uc3m.es/installation-guides/install-boost.html#install-boost-windows)
> sudo apt-get install libboost-all-dev

[Qt5](https://www.qt.io/download-open-source)
> sudo apt-get install build-essential qtcreator qt5-default  qtdeclarative5-dev
