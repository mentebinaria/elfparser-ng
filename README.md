# ELF Parser
Multiplatform CLI and GUI tool to show information about ELF files
This project is a tentative to keep [elfparser](https://github.com/mentebinaria/elfparser-ng) alive. Plans include update it and add new features.

> [License](LICENSE)

> Made in C++ lang
## What problem are you trying to solve?
I was looking for ELF based malware on https://malwr.com and couldn't help but notice how little analysis gets done on them. More surprising, to me, is that even well known malware like Kaiten gets flagged by very few AV (https://malwr.com/analysis/NThiZTU0MWUwZGI2NDAzYWI5YWU2ZjkzNTJmYTNjZTY/). ELF Parser attempts to move ELF malware analysis forward by quickly providing basic information and static analysis of the binary. The end goal of ELF Parser is to indicate to the analyst if it thinks the binary is malicious / dangerous and if so why.

## Help

ELF Parser can be compiled on Windows, OS X, or Linux (demangling and unit tests don't work on Windows) Compiling on Linux goes like this:

[![asciicast](https://asciinema.org/a/444072.svg)](https://asciinema.org/a/444072)


>To compile windows, just go to [CMakeFiles](CMakeLists.txt), change `windows` to `on`
![Windows](assets/compiler_windows.png) Open Visual Studio 2019 command `ctrl+f5`

## Compile Targets
ELF Parser has a number of compilation targets that can be configured by CMakeLists.txt. The targets are:
* Unit tests
* CLI build
* GUI build

## Dependencies

[boost C++](hhttp://robots.uc3m.es/installation-guides/install-boost.html#install-boost-windows)
```
sudo apt-get install boost
```

[Qt5](https://www.qt.io/download-open-source)
``` 
sudo apt-get install build-essential \
qtcreator qt5-default \
qtcreator qtdeclarative5-dev \
```
## Features
features that elf parser currently has

* ELF Header 
* Program Header
* Section Headers
* Dynamic Section
* Symbols
* Note Segment
* Read Only Segment
* Debug Link Segment
* String Table