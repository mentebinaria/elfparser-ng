#include "abstract_programheader.hpp"
#include "structures/programheader.hpp"
#include "abstract_segments.hpp"
#include <sstream>
#include <stdexcept>
#include <boost/foreach.hpp>
#include <iostream>

#if __APPLE__ || WINDOWS
#include "endian.h"
#else
#include <arpa/inet.h>
#endif

AbstractProgramHeader::AbstractProgramHeader(const char *p_data, boost::uint16_t p_size, bool p_is64, bool p_isLE)
{
    m_is64 = p_is64;
    m_isLE = p_isLE;

    if (p_is64)
    {
        if (p_size != sizeof(elf::program_header_64))
            throw std::runtime_error("Unexpected program header size");

        m_program_header64 = reinterpret_cast<const elf::program_header_64 *>(p_data);
    }
    else
    {
        if (p_size != sizeof(elf::program_header_32))
        {
            std::cout << p_size << std::endl;
            throw std::runtime_error("Unexpected program header size.");
        }

        m_program_header32 = reinterpret_cast<const elf::program_header_32 *>(p_data);
    }
}

AbstractProgramHeader::AbstractProgramHeader(const AbstractProgramHeader &p_rhs)
{
    m_program_header32 = p_rhs.m_program_header32;
    m_program_header64 = p_rhs.m_program_header64;
    m_is64 = p_rhs.m_is64;
    m_isLE = p_rhs.m_isLE;
}

AbstractProgramHeader::~AbstractProgramHeader()
{  }

bool AbstractProgramHeader::is64() const
{
    return m_is64;
}

bool AbstractProgramHeader::isLE() const
{
    return m_isLE;
}

bool AbstractProgramHeader::isExecutable() const
{
    return (getFlags() & elf::k_pfexec) != 0;
}

bool AbstractProgramHeader::isWritable() const
{
    return (getFlags() & elf::k_pfwrite) != 0;
}

std::string AbstractProgramHeader::getFlagsString() const
{
    std::string returnValue;
    if (getFlags() & elf::k_pfexec)
    {
        returnValue.append("Exec");
    }
    else if (getFlags() & elf::k_pfwrite)
    {
        if (!returnValue.empty())
        {
            returnValue.append(", ");
        }
        returnValue.append("Write");
    }
    else if (getFlags() & elf::k_pfread)
    {
        if (!returnValue.empty())
            returnValue.append(", ");
        returnValue.append("Read");
    }
    return returnValue;
}

std::string AbstractProgramHeader::getName() const
{
    switch (getType())
    {
    case elf::k_pnull:
        return std::string("PT_NULL");
    case elf::k_pload:
        return std::string("PT_LOAD");
    case elf::k_pdynamic:
        return std::string("PT_DYNAMIC");
    case elf::k_pinterp:
        return std::string("PT_INTERP");
    case elf::k_pnote:
        return std::string("PT_NOTE");
    case elf::k_pshlib:
        return std::string("PT_SHLIB");
    case elf::k_pphdr:
        return std::string("PT_PHDR");
    case elf::k_ptls:
        return std::string("PT_TLS");
    case elf::k_gnuEh:
        return std::string("GNU_EH_FRAME");
    case elf::k_gnuStack:
        return std::string("GNU_STACK");
    case elf::k_gnuRelRo:
        return std::string("GNU_RELRO");
    case elf::k_reginfo:
        return std::string("REGINFO");
    case elf::k_exidx:
        return std::string("EXIDX");
    default:
        std::stringstream return_this;
        return_this << "0x" << std::hex << getType();
        return return_this.str();
    }
}

boost::uint32_t AbstractProgramHeader::getType() const
{
    if (m_is64)
        return (m_isLE) ? m_program_header64->m_type : ntohl(m_program_header64->m_type);

    return (m_isLE) ? m_program_header32->m_type : ntohl(m_program_header32->m_type);
}

boost::uint64_t AbstractProgramHeader::getOffset() const
{
    if (m_is64)
        return m_isLE ? m_program_header64->m_offset : htobe64(m_program_header64->m_offset);

    return m_isLE ? m_program_header32->m_offset : ntohl(m_program_header32->m_offset);
}

boost::uint64_t AbstractProgramHeader::getVirtualAddress() const
{
    if (m_is64)
        return (m_isLE) ? m_program_header64->m_vaddr : htobe64(m_program_header64->m_vaddr);

    return (m_isLE) ? m_program_header32->m_vaddr : ntohl(m_program_header32->m_vaddr);
}

std::string AbstractProgramHeader::getVirtualAddressString() const
{
    std::stringstream result;
    result << "0x" << std::hex << getVirtualAddress();
    return result.str();
}

boost::uint64_t AbstractProgramHeader::getPhysicalAddress() const
{
    if (m_is64)
        return m_isLE ? m_program_header64->m_paddr : htobe64(m_program_header64->m_paddr);

    return m_isLE ? m_program_header32->m_paddr : ntohl(m_program_header32->m_paddr);
}

std::string AbstractProgramHeader::getPhysicalAddressString() const
{
    std::stringstream result;
    result << "0x" << std::hex << getPhysicalAddress();
    return result.str();
}

boost::uint64_t AbstractProgramHeader::getFileSize() const
{
    if (m_is64)
        return m_isLE ? m_program_header64->m_filesz : htobe64(m_program_header64->m_filesz);

    return m_isLE ? m_program_header32->m_filesz : ntohl(m_program_header32->m_filesz);
}

boost::uint64_t AbstractProgramHeader::getMemorySize() const
{
    if (m_is64)
        return m_isLE ? m_program_header64->m_memsz : htobe64(m_program_header64->m_memsz);

    return m_isLE ? m_program_header32->m_memsz : ntohl(m_program_header32->m_memsz);
}

boost::uint32_t AbstractProgramHeader::getFlags() const
{
    if (m_is64)
        return m_isLE ? m_program_header64->m_flags : ntohl(m_program_header64->m_flags);

    return m_isLE ? m_program_header32->m_flags : ntohl(m_program_header32->m_flags);
}