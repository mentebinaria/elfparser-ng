#include "abstract_dynamic.hpp"
#include "structures/dynamicstruct.hpp"

AbstractDynamicEntry::AbstractDynamicEntry(boost::uint64_t p_tag, boost::uint64_t p_value)
{
    m_tag = p_tag;
    m_value = p_value;
}

AbstractDynamicEntry::AbstractDynamicEntry(const AbstractDynamicEntry &p_rhs)
{
    m_tag = p_rhs.m_tag;
    m_value = p_rhs.m_value;
    m_stringValue = p_rhs.m_stringValue;
}

AbstractDynamicEntry::~AbstractDynamicEntry()
{  }

bool AbstractDynamicEntry::validString() const
{
    return !m_stringValue.empty();
}

void AbstractDynamicEntry::createString(const char *p_strOffset)
{
    m_stringValue.assign(createValue(m_tag, m_value, p_strOffset));
}

std::string AbstractDynamicEntry::createTag(boost::uint64_t p_tag) const
{
    switch (p_tag)
    {
        case elf::dynamic::k_needed:
            return std::string("NEEDED");
        case elf::dynamic::k_pltrelsz:
            return std::string("PLTRELSZ");
        case elf::dynamic::k_pltgot:
            return std::string("PLTGOT");
        case elf::dynamic::k_hash:
            return std::string("HASH");
        case elf::dynamic::k_strtab:
            return std::string("STRTAB");
        case elf::dynamic::k_symtab:
            return std::string("SYMTAB");
        case elf::dynamic::k_rela:
            return std::string("RELA");
        case elf::dynamic::k_relasz:
            return std::string("RELASZ");
        case elf::dynamic::k_relaent:
            return std::string("RELAENT");
        case elf::dynamic::k_strsz:
            return std::string("STRSZ");
        case elf::dynamic::k_syment:
            return std::string("SYMENT");
        case elf::dynamic::k_init:
            return std::string("INIT");
        case elf::dynamic::k_fini:
            return std::string("FINI");
        case elf::dynamic::k_soname:
            return std::string("SONAME");
        case elf::dynamic::k_rpath:
            return std::string("RPATH");
        case elf::dynamic::k_symbolic:
            return std::string("SYMBOLIC");
        case elf::dynamic::k_rel:
            return std::string("REL");
        case elf::dynamic::k_relsz:
            return std::string("RELSZ");
        case elf::dynamic::k_relent:
            return std::string("RELENT");
        case elf::dynamic::k_pltrel:
            return std::string("PLTREL");
        case elf::dynamic::k_debug:
            return std::string("DEBUG");
        case elf::dynamic::k_textrel:
            return std::string("TEXTREL");
        case elf::dynamic::k_jmprel:
            return std::string("JMPREL");
        case elf::dynamic::k_bindnow:
            return std::string("BIND_NOW");
        case elf::dynamic::k_initarray:
            return std::string("INIT_ARRAY");
        case elf::dynamic::k_finiarray:
            return std::string("FINI_ARRAY");
        case elf::dynamic::k_init_arraysz:
            return std::string("INIT_ARRAYSZ");
        case elf::dynamic::k_fini_arraysz:
            return std::string("FINI_ARRAYSZ");
        case elf::dynamic::k_gnuhash:
            return std::string("GNU_HASH");
    default:
        std::stringstream valueString;
        valueString << std::hex << "0x" << p_tag << std::dec;
        return valueString.str();
    }
    return CEXIT_SUCCESS;
}

std::string AbstractDynamicEntry::createValue(boost::uint64_t p_tag, boost::uint64_t p_value, const char *p_strTab) const
{
    switch (p_tag)
    {
    case elf::dynamic::k_needed:
    case elf::dynamic::k_soname:
    case elf::dynamic::k_rpath:
        if (p_strTab != NULL)
            return std::string(p_strTab + p_value);
    }
    return CEXIT_SUCCESS;
}

boost::uint64_t AbstractDynamicEntry::getTag() const
{
    return m_tag;
}

boost::uint64_t AbstractDynamicEntry::getValue() const
{
    return m_value;
}

const std::string &AbstractDynamicEntry::getString() const
{
    return m_stringValue;
}

std::string AbstractDynamicEntry::printToStdOut() const 
{
    std::stringstream valueString;
    valueString << "\t tag = " << createTag(m_tag) << ", value = ";
    if (validString())
        valueString << m_stringValue;
    else
        valueString << "0x" << std::hex << m_value;
    valueString << "\n";

    return valueString.str();
}