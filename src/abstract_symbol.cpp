#include "abstract_symbol.hpp"

#include "structures/symtable_entry.hpp"

#include <boost/lexical_cast.hpp>
#include <sstream>

#if WINDOWS || __APPLE__
#include "endian.hpp"
#else
#include <arpa/inet.h>
#endif

std::string getSymBinding(boost::uint8_t p_info)
{
    std::string str;
    switch ((p_info >> 4) & 0x0f)
    {
    case elf::symbol::k_local:
        str = "STB_LOCAL";
    case elf::symbol::k_global:
        str = "STB_GLOBAL";
    case elf::symbol::k_weak:
        str = "STB_WEAK";
    default:
        str = boost::lexical_cast<std::string>((p_info >> 4) & 0x0f);
    }
    return str;
}

std::string getSymType(boost::uint8_t p_info)
{
    std::string str;
    switch (p_info & 0x0f)
    {
    case elf::symbol::k_notype:
        str = "STT_NOTYPE";
    case elf::symbol::k_object:
        str = "STT_OBJECT";
    case elf::symbol::k_function:
        str = "STT_FUNC";
    case elf::symbol::k_section:
        str = "STT_SECTION";
    case elf::symbol::k_file:
        str = "STT_FILE";
    case elf::symbol::k_common:
        str = "STT_COMMON";
    case elf::symbol::k_tls:
        str = "STT_TLS";
    default:
        str = boost::lexical_cast<std::string>(p_info & 0x0f);
    }

    return str;
}

AbstractSymbol::AbstractSymbol(const char *p_data, boost::uint32_t p_offset,
                               bool p_is64, bool p_isLE) : m_symbol32(),
                                                           m_symbol64(),
                                                           m_name(),
                                                           m_is64(p_is64),
                                                           m_isLE(p_isLE)
{
    if (m_is64)
        m_symbol64 = reinterpret_cast<const elf::symbol::symtable_entry64 *>(p_data + p_offset);
    else
        m_symbol32 = reinterpret_cast<const elf::symbol::symtable_entry32 *>(p_data + p_offset);

    std::stringstream value;
    value << "0x" << std::hex << getValue();
    m_name.assign(value.str());
}

AbstractSymbol::AbstractSymbol(const AbstractSymbol &p_rhs)
{
    m_symbol32 = p_rhs.m_symbol32;
    m_symbol64 = p_rhs.m_symbol64;
    m_name = p_rhs.m_name;
    m_is64 = p_rhs.m_is64;
    m_isLE = p_rhs.m_isLE;
}

AbstractSymbol::~AbstractSymbol()
{
}

boost::uint32_t AbstractSymbol::getStructSize() const
{
    boost::uint32_t size;
    if (m_is64)
        size = sizeof(elf::symbol::symtable_entry64);
    else
        size = sizeof(elf::symbol::symtable_entry32);

    return size;
}

boost::uint8_t AbstractSymbol::getType() const
{
    boost::uint8_t type;
    if (m_is64)
        type = m_symbol64->m_info & 0x0f;
    else 
        type = m_symbol32->m_info & 0x0f;

    return type;
}

boost::uint8_t AbstractSymbol::getInfo() const
{
    boost::uint8_t info;
    if (m_is64)
        info = m_symbol64->m_info;
    else
        info = m_symbol32->m_info;

    return info;
}

std::string AbstractSymbol::getTypeName() const
{
    return getSymType(getInfo());
}

std::string AbstractSymbol::getBinding() const
{
    return getSymBinding(getInfo());
}

boost::uint64_t AbstractSymbol::getValue() const
{
    boost::uint64_t value;
    if (m_is64)
        value = m_isLE ? m_symbol64->m_address : htobe64(m_symbol64->m_address);
    else 
        value = m_isLE ? m_symbol32->m_address : ntohl(m_symbol32->m_address);

    return value;
}

boost::uint32_t AbstractSymbol::getNameIndex() const
{
    boost::uint32_t name;
    if (m_is64)
        name = m_isLE ? m_symbol64->m_name : ntohl(m_symbol64->m_name);
    else
        name = m_isLE ? m_symbol32->m_name : ntohl(m_symbol32->m_name);

    return name;
}

boost::uint16_t AbstractSymbol::getSectionIndex() const
{
    boost::uint16_t index;
    if (m_is64)
        index = m_isLE ? m_symbol64->m_shndx : ntohs(m_symbol64->m_shndx);
    else 
        index = m_isLE ? m_symbol32->m_shndx : ntohs(m_symbol32->m_shndx);

    return index;
}

const std::string &AbstractSymbol::getName() const
{
    return m_name;
}

void AbstractSymbol::setName(const std::string &p_name)
{
    if (!p_name.empty())
        m_name.assign(p_name);
}