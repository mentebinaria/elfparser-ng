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
{}

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
    std::stringstream str;
    switch (p_tag)
    {
        case elf::dynamic::k_needed:
            str << "NEEDED";
            break;
        case elf::dynamic::k_pltrelsz:
            str << "PLTRELSZ";
            break;
        case elf::dynamic::k_pltgot:
            str << "PLTGOT";
            break;        
        case elf::dynamic::k_hash:
            str << "HASH";
            break;
        case elf::dynamic::k_strtab:
            str << "STRTAB";
            break;
        case elf::dynamic::k_symtab:
            str << "SYMTAB";
            break;
        case elf::dynamic::k_rela:
            str << "RELA";
            break;
        case elf::dynamic::k_relasz:
            str << "RELASZ";
            break;
        case elf::dynamic::k_relaent:
            str << "RELAENT";
            break;
        case elf::dynamic::k_strsz:
            str << "STRSZ";
            break;
        case elf::dynamic::k_syment:
            str << "SYMENT";
            break;
        case elf::dynamic::k_init:
            str << "INIT";
            break;
        case elf::dynamic::k_fini:
            str << "FINI";
            break;
        case elf::dynamic::k_soname:
            str << "SONAME";
            break;
        case elf::dynamic::k_rpath:
            str << "RPATH";
            break;
        case elf::dynamic::k_symbolic:
            str << "SYMBOLIC";
            break;
        case elf::dynamic::k_rel:
            str << "REL";
            break;
        case elf::dynamic::k_relsz:
            str << "RELSZ";
            break;
        case elf::dynamic::k_relent:
            str << "RELENT";
            break;
        case elf::dynamic::k_pltrel:
            str << "PLTREL";
            break;
        case elf::dynamic::k_debug:
            str << "DEBUG";
            break;
        case elf::dynamic::k_textrel:
            str << "TEXTREL";
            break;
        case elf::dynamic::k_jmprel:
            str << "JMPREL";
            break;
        case elf::dynamic::k_bindnow:
            str << "BIND_NOW";
            break;
        case elf::dynamic::k_initarray:
            str << "INIT_ARRAY";
            break;
        case elf::dynamic::k_finiarray:
            str << "FINI_ARRAY";
            break;
        case elf::dynamic::k_init_arraysz:
            str << "INIT_ARRAYSZ";
            break;
        case elf::dynamic::k_fini_arraysz:
            str << "FINI_ARRAYSZ";
            break;
        case elf::dynamic::k_gnuhash:
            str << "GNU_HASH";
            break;
    default:
        str << std::hex << "0x" << p_tag << std::dec;

    }
    return str.str();
}

std::string AbstractDynamicEntry::createValue(boost::uint64_t p_tag, boost::uint64_t p_value, const char *p_strTab) const
{
    std::string str;
    switch (p_tag)
    {
    case elf::dynamic::k_needed:
    case elf::dynamic::k_soname:
    case elf::dynamic::k_rpath:
        if (p_strTab != __null)
            str = p_strTab + p_value;
    }
    return str;
}

boost::uint64_t AbstractDynamicEntry::getTag() const
{
    return m_tag;
}

boost::uint64_t AbstractDynamicEntry::getValue() const
{
    return m_value;
}

const std::string AbstractDynamicEntry::getString() const
{
    return m_stringValue;
}

std::string AbstractDynamicEntry::printToStdOut() const 
{
    std::stringstream valueString;
    valueString << "\t tag = " << createTag(m_tag) << ", value = ";
    if (validString())
        valueString << m_stringValue << std::endl;
    else
        valueString << "0x" << std::hex << m_value << std::endl;
    
    return valueString.str();
}