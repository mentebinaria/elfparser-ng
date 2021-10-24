#ifndef SC_VERSION_HPP
#define SC_VERSION_HPP

#include <string>

std::string version_elf_parser()
{
    return std::string("Version: ELF Parser 1.4.0\nCopyright:\n  elfparser-ng (c) 2021 Mente Binária\n  elfparser  (c) 2014-2021 Jacob Baines");
}

#endif // !SC_VERSION_HPP