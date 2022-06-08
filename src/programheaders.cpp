#include "programheaders.hpp"
#include "abstract_segments.hpp"
#include "abstract_programheader.hpp"
#include "structures/programheader.hpp"

#include <boost/foreach.hpp>
#include <sstream>

ProgramHeaders::ProgramHeaders() : m_programHeaders()
{
}

ProgramHeaders::~ProgramHeaders()
{
}

void ProgramHeaders::setHeaders(const char *p_data, boost::uint16_t p_count,
                                boost::uint16_t p_size, bool p_is64, bool p_isLE)
{
    if (p_size == 0)
        exit(EXIT_FAILURE);

    for (std::size_t i = 0; i < p_count; ++i, p_data += p_size)
        m_programHeaders.emplace_back(p_data, p_size, p_is64, p_isLE);
}

void ProgramHeaders::extractSegments(AbstractSegments &p_segments)
{
    BOOST_FOREACH (auto &header, m_programHeaders)
        p_segments.makeSegmentFromProgramHeader(header);
}

const std::vector<AbstractProgramHeader> &ProgramHeaders::getProgramHeaders() const
{
    return m_programHeaders;
}

std::string ProgramHeaders::printToStdOut() const
{
    std::stringstream returnValue;
    std::size_t size = m_programHeaders.size();
    if (size > 0)
    {
        returnValue << "Program Header (count = " << size << ")\n";

        BOOST_FOREACH (auto &header, m_programHeaders)
        {
            returnValue << "\t Entry type=" << header.getName()
                        << "\t flags=" << std::dec << header.getFlags()
                        << "\t offset=0x" << std::hex << header.getOffset()
                        << "\t vaddr=0x" << header.getVirtualAddress()
                        << "\t paddr=0x" << header.getPhysicalAddress()
                        << "\t filesz=0x" << header.getFileSize()
                        << "\t memsz=0x" << header.getMemorySize()
                        << std::dec << std::endl;
        }
    }

    return returnValue.str();
}

void ProgramHeaders::evaluate(std::vector<std::pair<boost::int32_t, std::string>> &p_reasons) const
{
    bool found_load = false;
    std::size_t entry_count = 0;
    std::size_t load_count = 0;

    BOOST_FOREACH (auto &header, m_programHeaders)
    {
        if (header.getType() == elf::k_pload)
        {
            ++load_count;
            found_load = true;
        }
        ++entry_count;
    }

    if (load_count > 2)
        p_reasons.push_back(std::make_pair(30, std::string("Found 2+ PT_LOAD. Possible post-compilation addition of code (cryptor or packer)")));

    if (entry_count > 0 && !found_load)
        p_reasons.push_back(std::make_pair(5, std::string("Didn't find PT_LOAD in the program headers")));
}
