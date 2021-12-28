#include "abstract_programheader.hpp"
#include "structures/programheader.hpp"
#include "abstract_segments.hpp"
#include <sstream>
#include <stdexcept>
#include <boost/foreach.hpp>
#include <iostream>

#if __APPLE__ || WINDOWS
#include "endian.hpp"
#else
#include <arpa/inet.h>
#endif

AbstractProgramHeader::AbstractProgramHeader(const char *p_data, boost::uint16_t p_size, bool p_is64, bool p_isLE)
{
    m_isLE = p_isLE;
    m_is64 = p_is64;

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
{
}

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
        returnValue.append("Exec");

    else if (getFlags() & elf::k_pfwrite)
    {
        if (!returnValue.empty())
            returnValue.append(", ");

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
    std::stringstream str;
    switch (getType())
    {
    case elf::k_pnull:
        str << "PT_NULL";
        break;
    case elf::k_pload:
        str << "PT_LOAD";
        break;
    case elf::k_pdynamic:
        str << "PT_DYNAMIC";
        break;
    case elf::k_pinterp:
        str << "PT_INTERP";
        break;
    case elf::k_pnote:
        str << "PT_NOTE";
        break;
    case elf::k_pshlib:
        str << "PT_SHLIB";
        break;
    case elf::k_pphdr:
        str << "PT_PHDR";
        break;
    case elf::k_ptls:
        str << "PT_TLS";
        break;
    case elf::k_gnuEh:
        str << "GNU_EH_FRAME";
        break;
    case elf::k_gnuStack:
        str << "GNU_STACK";
        break;
    case elf::k_gnuRelRo:
        str << "GNU_RELRO";
        break;
    case elf::k_reginfo:
        str << "REGINFO";
        break;
    case elf::k_exidx:
        str << "EXIDX";
        break;
    default:
        str << "0x" << std::hex << getType();

    }
    return str.str();
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