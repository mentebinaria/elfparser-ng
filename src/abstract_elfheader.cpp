#include "abstract_elfheader.hpp"

AbstractElfHeader::AbstractElfHeader()
{
    m_is64 = false;
    m_fileSize = 0;
    m_header32 = nullptr;
    m_header64 = nullptr;
}

AbstractElfHeader::~AbstractElfHeader()
{
}

void AbstractElfHeader::setHeader(const char *p_data, std::size_t p_size)
{
    if (p_size < 45) // see size of true.asm from muppet labs
        throw std::runtime_error("The file is too small to be an ELF binary");

    m_fileSize = p_size;
    const elf::elf_header_32 *header = reinterpret_cast<const elf::elf_header_32 *>(p_data);

    if (header->m_magic0 != 0x7f || memcmp(header->m_ident + 1, "ELF", 3) != 0)
        throw std::runtime_error("The file does not start with ELF magic.");

    // we can't trust header->m_class. See muppet lab's
    if (header->m_phentsize == sizeof(elf::program_header_32) || header->m_phentsize == ntohs(sizeof(elf::program_header_32)))
    {
        m_is64 = false;
        m_header32 = header;
    }
    else
    {
        m_is64 = true;
        m_header64 = reinterpret_cast<const elf::elf_header_64 *>(p_data);
        if (m_header64->m_phentsize != sizeof(elf::program_header_64) && m_header64->m_phentsize != ntohs(sizeof(elf::program_header_64)))
        {
            throw std::runtime_error("Unable to determine 32 bit vs. 64 bit");
        }
    }
}

bool AbstractElfHeader::is64() const
{
    assert(m_header32 != NULL || m_header64 != NULL);
    return m_is64;
}

std::string AbstractElfHeader::getMagic() const
{
    std::stringstream result;
    result << std::hex << static_cast<int>((m_is64 ? m_header64->m_magic0 : m_header32->m_magic0)) << " ";
    result << std::hex << static_cast<int>((m_is64 ? m_header64->m_magic1 : m_header32->m_magic1)) << " ";
    result << std::hex << static_cast<int>((m_is64 ? m_header64->m_magic2 : m_header32->m_magic2)) << " ";
    result << std::hex << static_cast<int>((m_is64 ? m_header64->m_magic3 : m_header32->m_magic3));
    return result.str();
}

std::string AbstractElfHeader::getType() const
{
    boost::uint16_t type = 0;
    if (m_is64)
        type = ((isLE())) ? m_header64->m_type : ntohs(m_header64->m_type);
    else
        type = ((isLE())) ? m_header32->m_type : ntohs(m_header32->m_type);

    const std::string type_str =[&](){
    switch(type)
    {
        case elf::k_etnone:
            return "ET_NONE";
        case elf::k_etrel:
            return "ET_REL";
        case elf::k_etexec:
            return "ET_EXEC";
        case elf::k_etdyn:
            return "ET_DYN";
        case elf::k_etcore:
            return "ET_CORE";
        default:
            return "UNKNOWN";
        }
    }();

    return type_str;
}

std::string AbstractElfHeader::getOSABI() const
{
    std::string os_str;
    boost::uint8_t os = m_is64 ? m_header64->m_os : m_header32->m_os;
    switch (os)
    {
    case 0x00:
        os_str = "System V";
        break;
    case 0x01:
        os_str = "HP-UX";
        break;
    case 0x02:
        os_str = "NetBSD";
        break;
    case 0x03:
        os_str = "Linux";
        break;
    case 0x06:
        os_str = "Solaris";
        break;
    case 0x07:
        os_str = "AIX";
        break;
    case 0x08:
        os_str = "IRIX";
        break;
    case 0x09:
        os_str = "FreeBSD";
        break;
    case 0x0c:
        os_str = "OpenBSD";
        break;
    default:
    {
        std::stringstream result;
        result << "0x" << std::hex << static_cast<int>(os);
        return result.str();
    }
    }
    return os_str;
}

boost::uint64_t AbstractElfHeader::getEntryPoint() const
{
    boost::uint64_t entryPoint;
    assert(m_header32 != NULL || m_header64 != NULL);

    if (m_is64)
        entryPoint = ((isLE())) ? m_header64->m_entry : htobe64(m_header64->m_entry);
    else
        entryPoint = ((isLE())) ? m_header32->m_entry : ntohl(m_header32->m_entry);

    return entryPoint;
}

std::string AbstractElfHeader::getEntryPointString() const
{
    std::stringstream entryPoint;
    entryPoint << "0x" << std::hex << getEntryPoint();
    return entryPoint.str();
}

std::string AbstractElfHeader::getVersion() const
{
    std::stringstream result;
    if (m_is64)
        result << (isLE() ? m_header64->m_version : ntohl(m_header64->m_version));
    else
        result << (isLE() ? m_header32->m_version : ntohl(m_header32->m_version));

    return result.str();
}

boost::uint32_t AbstractElfHeader::getProgramOffset() const
{
    boost::uint32_t offset;
    assert(m_header32 != NULL || m_header64 != NULL);

    if (m_is64)
        offset = (isLE()) ? m_header64->m_phoff : htobe64(m_header64->m_phoff);
    else
        offset = (isLE()) ? m_header32->m_phoff : ntohl(m_header32->m_phoff);

    return offset;
}

boost::uint16_t AbstractElfHeader::getProgramCount() const
{
    boost::uint16_t PC;
    assert(m_header32 != NULL || m_header64 != NULL);

    if (m_is64)
        PC = (isLE()) ? m_header64->m_phnum : ntohs(m_header64->m_phnum);
    else
        PC = (isLE()) ? m_header32->m_phnum : ntohs(m_header32->m_phnum);

    return PC;
}

boost::uint16_t AbstractElfHeader::getProgramSize() const
{
    boost::uint16_t PS;
    assert(m_header32 != NULL || m_header64 != NULL);

    if (m_is64)
        PS = (isLE()) ? m_header64->m_phentsize : ntohs(m_header64->m_phentsize);
    else
        PS = (isLE()) ? m_header32->m_phentsize : ntohs(m_header32->m_phentsize);

    return PS;
}

boost::uint32_t AbstractElfHeader::getSectionOffset() const
{
    boost::uint32_t offset;
    assert(m_header32 != NULL || m_header64 != NULL);

    if (m_is64)
        offset = (isLE()) ? m_header64->m_shoff : htobe64(m_header64->m_shoff);
    else
        offset = (isLE()) ? m_header32->m_shoff : ntohl(m_header32->m_shoff);

    return offset;
}

boost::uint16_t AbstractElfHeader::getSectionSize() const
{
    boost::uint16_t size;
    assert(m_header32 != NULL || m_header64 != NULL);

    if (m_is64)
        size = (isLE()) ? m_header64->m_shentsize : ntohs(m_header64->m_shentsize);
    else
        size = (isLE()) ? m_header32->m_shentsize : ntohs(m_header32->m_shentsize);

    return size;
}

boost::uint16_t AbstractElfHeader::getSectionCount() const
{
    assert(m_header32 != NULL || m_header64 != NULL);
    if (m_is64)
        return (isLE()) ? m_header64->m_shnum : ntohs(m_header64->m_shnum);

    return (isLE()) ? m_header32->m_shnum : ntohs(m_header32->m_shnum);
}

boost::uint16_t AbstractElfHeader::getStringTableIndex() const
{
    assert(m_header32 != NULL || m_header64 != NULL);
    if (m_is64)
        return (isLE()) ? m_header64->m_shtrndx : ntohs(m_header64->m_shtrndx);

    return (isLE()) ? m_header32->m_shtrndx : ntohs(m_header32->m_shtrndx);
}

boost::uint32_t AbstractElfHeader::getStringTableOffset(const char *p_start) const
{
    boost::uint32_t offset_return;
    if (getStringTableIndex() == 0)
        return 0;

    if (m_is64)
    {
        boost::uint32_t offset = getSectionOffset() + (getStringTableIndex() * sizeof(elf::section_header_64));
        const elf::section_header_64 *strTab = reinterpret_cast<const elf::section_header_64 *>(p_start + offset);

        offset_return = strTab->m_offset;
    }
    else
    {
        boost::uint32_t offset = getSectionOffset() + (getStringTableIndex() * sizeof(elf::section_header_32));
        const elf::section_header_32 *strTab = reinterpret_cast<const elf::section_header_32 *>(p_start + offset);

        offset_return = strTab->m_offset;
    }
    return offset_return;
}

bool AbstractElfHeader::isLE() const
{
    // this is hack to get around abuse of the encoding field
    assert(m_header32 != NULL || m_header64 != NULL);
    if (m_is64)
        return (m_header64->m_phentsize == sizeof(elf::program_header_64) ? true : false);

    return (m_header32->m_phentsize == sizeof(elf::program_header_32) ? true : false);
}

std::string AbstractElfHeader::getMachine() const
{
    boost::uint16_t machine = m_is64 ? m_header64->m_machine : m_header32->m_machine;
    if (!(isLE()))
        machine = ntohs(machine);

    const std::string machine_str = [&](){
        switch (machine)
        {
        case elf::k_em386:
            return "x86";
        case elf::k_emARM:
            return "ARM";
        case elf::k_emMIPS:
            return "MIPS";
        case elf::k_emPPC:
            return "PowerPC";
        case elf::k_emx8664:
            return "x86_64";
        case elf::k_emnone:
            return "None";
        default:
            return "Unknown";
        }
    }();

    return machine_str;
}

std::string AbstractElfHeader::getEncoding() const
{
    boost::uint8_t encoding = m_header64 != NULL ? m_header64->m_encoding : m_header32->m_encoding;
    const std::string str = [&](){
        switch (encoding)
        {
        case 0:
            return "Invalid";
        case 1:
            return "Little Endian";
        case 2:
            return "Big Endian";
        default:
            return "Unknown";
        }
    }();

    return str;
}

std::string AbstractElfHeader::getABIVersion() const
{
    std::stringstream result;
    (m_is64) ? result << static_cast<int>(m_header64->m_abi) : result << static_cast<int>(m_header32->m_abi);

    return result.str();
}

std::string AbstractElfHeader::getClass() const
{
    std::string str = (m_is64) ? "64-bit" : "32-bit";
    return str;
}

std::string AbstractElfHeader::getFileVersion() const
{
    std::stringstream result;
    result << static_cast<int>(m_is64 ? m_header64->m_fileversion : m_header32->m_fileversion);

    return result.str();
}

boost::uint16_t AbstractElfHeader::getEHSize() const
{
    assert(m_header32 != NULL || m_header64 != NULL);
    boost::uint16_t value = 0;

    if (m_is64)
        value = (isLE()) ? m_header64->m_ehsize : ntohs(m_header64->m_ehsize);
    else
        value = (isLE()) ? m_header32->m_ehsize : ntohs(m_header32->m_ehsize);

    return value;
}

std::string AbstractElfHeader::getFlags() const
{
    assert(m_header32 != NULL || m_header64 != NULL);
    boost::uint32_t value = 0;
    if (m_is64)
        value = (isLE()) ? m_header64->m_flags : ntohl(m_header64->m_flags);

    else
        value = (isLE()) ? m_header32->m_flags : ntohl(m_header32->m_flags);

    std::stringstream result;
    result << "0x" << std::hex << value;
    return result.str();
}

std::string AbstractElfHeader::printToStdOut() const
{
    std::stringstream return_value;
    return_value << "ELF Header\n\tclass=";
    return_value << getClass() << std::endl;
    return_value << "\t encoding=" << getEncoding() << std::endl;
    return_value << "\t fileversion=" << getFileVersion() << std::endl;
    return_value << "\t os=" << getOSABI() << std::endl;
    return_value << "\t abi=" << getABIVersion() << std::endl;
    return_value << "\t type=" << getType() << std::endl;
    return_value << "\t machine=" << getMachine() << std::endl;
    return_value << "\t version=" << getVersion() << std::endl;
    return_value << "\t entryPoint=" << getEntryPointString() << std::endl;
    return_value << "\t phoffset="
                 << "0x" << std::hex << getProgramOffset() << std::endl;
    return_value << "\t shoffset="
                 << "0x" << std::hex << getSectionOffset() << std::endl;
    return_value << "\t flags=" << getFlags() << std::endl;
    return_value << "\t ehsize=" << getEHSize() << std::endl;
    return_value << "\t phentsize=" << getProgramSize() << std::endl;
    return_value << "\t phnum=" << getProgramCount() << std::endl;
    return_value << "\t shentsize=" << getSectionSize() << std::endl;
    return_value << "\t shnum=" << getSectionCount() << std::endl;
    return_value << "\t shtrndx=" << getStringTableIndex() << std::endl;
    return return_value.str();
}

void AbstractElfHeader::evaluate(std::vector<std::pair<boost::int32_t, std::string>> &,
                                 std::map<elf::Capabilties, std::set<std::string>> &p_capabilities) const
{
    if (getProgramOffset() != 0 && getProgramOffset() < 45)
        p_capabilities[elf::k_antidebug].insert("Possible compact ELF: program header overlaps with ELF header");

    else if (getEncoding() == "Invalid")
        p_capabilities[elf::k_antidebug].insert("Possible compact ELF: invalid encoding in ELF header");

    else if (getSectionOffset() != 0 && getSectionCount() == 0)
        p_capabilities[elf::k_antidebug].insert("Possible compact ELF: section count 0 but section offset non-zero");

    else if (m_header32 != NULL)
    {
        if (m_header32->m_class != 1 && m_header32->m_class != 2)
            p_capabilities[elf::k_antidebug].insert("Possible compact ELF: invalid class in ELF header");
    }
    else
    {
        if (m_header64->m_class != 1 && m_header64->m_class != 2)
            p_capabilities[elf::k_antidebug].insert("Possible compact ELF: invalid class in ELF header");
    }
}