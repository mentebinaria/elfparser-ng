#include "abstract_elfheader.hpp"

AbstractElfHeader::AbstractElfHeader()
{
    m_is64 = false;
    m_fileSize = 0;
    m_header32 = nullptr;
    m_header64 = nullptr;
}

AbstractElfHeader::~AbstractElfHeader()
{  }

void AbstractElfHeader::setHeader(const char* p_data, std::size_t p_size)
{
    if (p_size < 45) // see size of true.asm from muppet labs
        throw std::runtime_error("The file is too small to be an ELF binary");

    m_fileSize = p_size;
    const elf::elf_header_32* header = reinterpret_cast<const elf::elf_header_32*>(p_data);

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
        m_header64 = reinterpret_cast<const elf::elf_header_64*>(p_data);
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
        type = (isLE()) ? m_header64->m_type : ntohs(m_header64->m_type);
    else
        type = (isLE()) ? m_header32->m_type : ntohs(m_header32->m_type);

    switch (type)
    {
        case elf::k_etnone:
            return std::string("ET_NONE");
        case elf::k_etrel:
            return std::string("ET_REL");
        case elf::k_etexec:
            return std::string("ET_EXEC");
        case elf::k_etdyn:
            return std::string("ET_DYN");
        case elf::k_etcore:
            return std::string("ET_CORE");
        default:
            return std::string("UNKNOWN");       
    }
    return CEXIT_SUCCESS;
}

std::string AbstractElfHeader::getOSABI() const
{
    boost::uint8_t os = m_is64 ? m_header64->m_os : m_header32->m_os;
    switch (os)
    {
        case 0x00:
            return std::string("System V");
        case 0x01:
            return std::string("HP-UX");
        case 0x02:
            return std::string("NetBSD");
        case 0x03:
            return std::string("Linux");
        case 0x06:
            return std::string("Solaris");
        case 0x07:
            return std::string("AIX");
        case 0x08:
            return std::string("IRIX");
        case 0x09:
            return std::string("FreeBSD");
        case 0x0c:
            return std::string("OpenBSD");
        default:
        {
            std::stringstream result;
            result << "0x" << std::hex << static_cast<int>(os);
            return result.str();
        }
    }
    return CEXIT_SUCCESS;
}

boost::uint64_t AbstractElfHeader::getEntryPoint() const
{
    assert(m_header32 != NULL || m_header64 != NULL);
    if (m_is64)
    {
        return isLE() ? m_header64->m_entry : htobe64(m_header64->m_entry);
    }
    return isLE() ? m_header32->m_entry : ntohl(m_header32->m_entry);
}

std::string AbstractElfHeader::getEntryPointString() const
{
    std::stringstream result;
    result << "0x" << std::hex << getEntryPoint();
    return result.str();
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
    assert(m_header32 != NULL || m_header64 != NULL);
    if (m_is64)
        return isLE() ? m_header64->m_phoff : htobe64(m_header64->m_phoff);

    return isLE() ? m_header32->m_phoff : ntohl(m_header32->m_phoff);
}

boost::uint16_t AbstractElfHeader::getProgramCount() const
{
    assert(m_header32 != NULL || m_header64 != NULL);
    if (m_is64)
        return isLE() ? m_header64->m_phnum : ntohs(m_header64->m_phnum);

    return isLE() ? m_header32->m_phnum : ntohs(m_header32->m_phnum);
}

boost::uint16_t AbstractElfHeader::getProgramSize() const
{
    assert(m_header32 != NULL || m_header64 != NULL);
    if (m_is64)
        return isLE() ? m_header64->m_phentsize : ntohs(m_header64->m_phentsize);

    return isLE() ? m_header32->m_phentsize : ntohs(m_header32->m_phentsize);
}

boost::uint32_t AbstractElfHeader::getSectionOffset() const
{
    assert(m_header32 != NULL || m_header64 != NULL);
    if (m_is64)
        return isLE() ? m_header64->m_shoff : htobe64(m_header64->m_shoff);

    return isLE() ? m_header32->m_shoff : ntohl(m_header32->m_shoff);
}

boost::uint16_t AbstractElfHeader::getSectionSize() const
{
    assert(m_header32 != NULL || m_header64 != NULL);

    if (m_is64)
        return isLE() ? m_header64->m_shentsize : ntohs(m_header64->m_shentsize);

    return isLE() ? m_header32->m_shentsize : ntohs(m_header32->m_shentsize);
}

boost::uint16_t AbstractElfHeader::getSectionCount() const
{
    assert(m_header32 != NULL || m_header64 != NULL);
    if (m_is64)
        return isLE() ? m_header64->m_shnum : ntohs(m_header64->m_shnum);

    return isLE() ? m_header32->m_shnum : ntohs(m_header32->m_shnum);
}

boost::uint16_t AbstractElfHeader::getStringTableIndex() const
{
    assert(m_header32 != NULL || m_header64 != NULL);
    if (m_is64)
        return isLE() ? m_header64->m_shtrndx : ntohs(m_header64->m_shtrndx);

    return isLE() ? m_header32->m_shtrndx : ntohs(m_header32->m_shtrndx);
}

boost::uint32_t AbstractElfHeader::getStringTableOffset(const char* p_start) const
{
    if (getStringTableIndex() == 0)
        return 0;

    if (m_is64)
    {
        boost::uint32_t offset = getSectionOffset() + (getStringTableIndex() * sizeof(elf::section_header_64));
        const elf::section_header_64* strTab =
            reinterpret_cast<const elf::section_header_64*>(p_start + offset);
        return strTab->m_offset;
    }
    else
    {
        boost::uint32_t offset = getSectionOffset() + (getStringTableIndex() * sizeof(elf::section_header_32));
        const elf::section_header_32* strTab =
            reinterpret_cast<const elf::section_header_32*>(p_start + offset);
        return strTab->m_offset;
    }
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
    if (!isLE())
        machine = ntohs(machine);

    switch (machine)
    {
        case elf::k_em386:
            return std::string("x86");
        case elf::k_emARM:
            return std::string("ARM");
        case elf::k_emMIPS:
            return std::string("MIPS");
        case elf::k_emPPC:
            return std::string("PowerPC");
        case elf::k_emx8664:
            return std::string("x86_64");
        case elf::k_emnone:
            return std::string("None");
        default:
            return std::string("Unknown");
    }
}

std::string AbstractElfHeader::getEncoding() const
{
    boost::uint8_t encoding = m_header64 != NULL ? m_header64->m_encoding : m_header32->m_encoding;
    std::string str;
    switch (encoding)
    {
        case 0:
            str = "Invalid";
            break;
        case 1:
            str =  "Little Endian";
            break;
        case 2:
            str = "Big Endian";
            break;
        default:
            str = "Unknown";
            break;
    }
    return str;
}

std::string AbstractElfHeader::getABIVersion() const
{
    std::stringstream result;
    if (m_is64)
        result << static_cast<int>(m_header64->m_abi);
    else
        result << static_cast<int>(m_header32->m_abi);

    return result.str();
}

std::string AbstractElfHeader::getClass() const
{
    std::string str = (m_is64) ? "64-bit" :  "32-bit";
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
        value = isLE() ? m_header64->m_ehsize : ntohs(m_header64->m_ehsize);
    else
        value = isLE() ? m_header32->m_ehsize : ntohs(m_header32->m_ehsize);

    return value;
}

std::string AbstractElfHeader::getFlags() const
{
    assert(m_header32 != NULL || m_header64 != NULL);
    boost::uint32_t value = 0;
    if (m_is64)
        value = isLE() ? m_header64->m_flags : ntohl(m_header64->m_flags);

    else
        value = isLE() ? m_header32->m_flags : ntohl(m_header32->m_flags);

    std::stringstream result;
    result << "0x" << std::hex << value;
    return result.str();
}

std::string AbstractElfHeader::printToStdOut() const
{
    std::stringstream return_value;
    return_value << "ELF Header\n\tclass=";
    return_value << getClass() << std::endl;
    return_value << "\tencoding=" << getEncoding() << std::endl;
    return_value << "\tfileversion=" << getFileVersion() << std::endl;
    return_value << "\tos=" << getOSABI() << std::endl;
    return_value << "\tabi=" << getABIVersion() << std::endl;
    return_value << "\ttype=" << getType() << std::endl;
    return_value << "\tmachine=" << getMachine() << std::endl;
    return_value << "\tversion=" << getVersion() << std::endl;
    return_value << "\tentryPoint=" << getEntryPointString() << std::endl;
    return_value << "\tphoffset=" << "0x" << std::hex << getProgramOffset() << std::endl;
    return_value << "\tshoffset=" << "0x" << std::hex << getSectionOffset() << std::endl;
    return_value << "\tflags=" << getFlags() << std::endl;
    return_value << "\tehsize=" << getEHSize() << std::endl;
    return_value << "\tphentsize=" << getProgramSize() << std::endl;
    return_value << "\tphnum=" << getProgramCount() << std::endl;
    return_value << "\tshentsize=" << getSectionSize() << std::endl;
    return_value << "\tshnum=" << getSectionCount() << std::endl;
    return_value << "\tshtrndx=" << getStringTableIndex() << std::endl;
    return return_value.str();
}

void AbstractElfHeader::evaluate(std::vector<std::pair<boost::int32_t, std::string> >&,
                                 std::map<elf::Capabilties, std::set<std::string> >& p_capabilities) const
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
        {
            p_capabilities[elf::k_antidebug].insert("Possible compact ELF: invalid class in ELF header");
        }
    }
    else
    {
        if (m_header64->m_class != 1 && m_header64->m_class != 2)
        {
            p_capabilities[elf::k_antidebug].insert("Possible compact ELF: invalid class in ELF header");
        }
    }
}
