#include "elfparser.hpp"
#include "../lib/hash-lib/md5.hpp"
#include "../lib/hash-lib/sha256.hpp"
#include "../lib/hash-lib/sha1.hpp"

#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <regex>
#include <set>
#include <stdexcept>
#include <boost/regex.hpp>
#include <iostream>


static double calcEntropyFunc(const unsigned int counted_bytes[256], const std::streamsize total_length)
{
    double entropy = 0.;
        
    double temp;

    for (int i = 0; i < 256; i++) {
        temp = static_cast<double>(counted_bytes[i]) / total_length;

        if (temp > 0.)
            entropy += temp * fabs(log2(temp));
    }

    return entropy;
}

std::size_t findFileSize(const std::string &p_file)
{
    std::ifstream in(p_file.c_str(), std::ios::binary | std::ios::ate);
    if (!in.is_open())
        throw std::runtime_error("Could not open " + p_file);

    if (!in.good())
        throw std::runtime_error("Error opening " + p_file);

    std::size_t return_value = in.tellg();
    in.close();

    return return_value;
}

ELFParser::ELFParser() : m_entropy(0),
    m_score(0),
    m_fileSize(0)
{
    
    m_searchValues.push_back(new SearchValue("UPX!", elf::k_packed, "UPX signature found"));
    m_searchValues.push_back(new SearchValue("the UPX Team. All Rights Reserved", elf::k_packed, "UPX copyright string found"));
    m_searchValues.push_back(new SearchValue("PRIVMSG ", elf::k_irc, "IRC command PRIVMSG found"));
    m_searchValues.push_back(new SearchValue("JOIN ", elf::k_irc, "IRC command JOIN found"));
    m_searchValues.push_back(new SearchValue("NOTICE ", elf::k_irc, "IRC command NOTICE found"));
    m_searchValues.push_back(new SearchValue("ustar\0", elf::k_compression, "Tar Archive signature found"));
    m_searchValues.push_back(new SearchValue("\x1f\x8b\x08", elf::k_compression, "Gzip signature found"));
    m_searchValues.push_back(new SearchValue("inflate 1.1.4 Copyright 1995-2002 Mark Adler", elf::k_compression, "Inflate 1.1.4 copyright string"));
    m_searchValues.push_back(new SearchValue("inflate 1.2.3 Copyright 1995-2005 Mark Adler", elf::k_compression, "Inflate 1.2.3 copyright string"));
    m_searchValues.push_back(new SearchValue("inflate 1.2.8 Copyright 1995-2013 Mark Adler", elf::k_compression, "Inflate 1.2.8 copyright string"));
    m_searchValues.push_back(new SearchValue("/proc/cpuinfo", elf::k_infoGathering, "Examines /proc/cpuinfo"));
    m_searchValues.push_back(new SearchValue("/proc/meminfo", elf::k_infoGathering, "Examines /proc/meminfo"));
    m_searchValues.push_back(new SearchValue("/proc/stat", elf::k_infoGathering, "Examines /proc/stat"));
    m_searchValues.push_back(new SearchValue("HISTFILE=", elf::k_envVariables, "Accesses the bash history file environment variable HISTFILE."));
    BOOST_FOREACH (SearchValue &value, m_searchValues)
    {
        m_searchEngine.addWord(value.m_search, &value);
    }

    m_searchEngine.compile();
}

ELFParser::~ELFParser()
{
    m_searchValues.clear();
    m_mapped_file.close();
}

boost::uint32_t ELFParser::getScore() const
{
    return m_score;
}

std::string ELFParser::getFilename() const
{
    return m_filename;
}

std::size_t ELFParser::getFileSize() const
{
    return m_fileSize;
}

std::string ELFParser::getSha1() const
{
    SHA1 sha1;
    return sha1(m_mapped_file.data(), m_fileSize);
}

std::string ELFParser::getSha256() const
{
    SHA256 sha256;
    return sha256(m_mapped_file.data(), m_fileSize);
}

std::string ELFParser::getMD5() const
{
    MD5 md5;
    return md5(m_mapped_file.data(), m_fileSize);
}

std::string ELFParser::getFamily() const
{
    return m_segments.determineFamily();
}

const std::vector<std::pair<boost::int32_t, std::string>> &ELFParser::getReasons() const
{
    return m_reasons;
}

double ELFParser::getEntropy()
{
    return m_entropy;
}

void ELFParser::parse(const std::string &p_file)
{
    m_fileSize = findFileSize(p_file); // get size of file
    m_mapped_file.open(p_file, m_fileSize); // map elf in memory
    m_filename.assign(p_file);    

    if (!m_mapped_file.is_open())
        throw std::runtime_error("Failed to memory map the file.");

    else if (p_file.empty())
        throw std::runtime_error("Parser given an empty file name.");


	// get infos elf
    const char* ptrDataMem = m_mapped_file.data();
    m_elfHeader.setHeader(ptrDataMem, m_fileSize);

    m_offset =  m_elfHeader.getProgramOffset();

    m_size =  m_elfHeader.getProgramSize();
    m_pc =  m_elfHeader.getProgramCount();

    if( m_offset <= m_fileSize && m_size <= m_fileSize )
    {
        m_programHeader.setHeaders(ptrDataMem + m_offset,
                               m_pc,
                               m_size,
							   m_elfHeader.is64(),
							   m_elfHeader.isLE());
    }

    m_offset =  m_elfHeader.getSectionOffset();
    m_size = m_elfHeader.getSectionSize();
    m_pc =  m_elfHeader.getSectionCount();

    if(m_offset <= m_fileSize && m_size <= m_fileSize)
    {

        m_sectionHeader.setHeaders(ptrDataMem, m_offset,
                               ptrDataMem,
							   m_fileSize,
                               m_pc,
							   m_size,
                               m_elfHeader.getStringTableIndex(),
							   m_elfHeader.is64(),
							   m_elfHeader.isLE(),
                               m_capabilities);
    }
    
    m_segments.setStart(ptrDataMem,
					    m_fileSize, m_elfHeader.is64(),
                        m_elfHeader.isLE(),
						m_elfHeader.getType() == "ET_DYN");


    // important to do section header first since it produces more complete data
    // create "segments" based off of the section header and program header
    m_sectionHeader.extractSegments(m_segments);
    m_programHeader.extractSegments(m_segments);
    m_segments.createDynamic();
    m_segments.generateSegments();

	// calculate entropy binary all
	calcEntropy(0, m_fileSize);
}

void ELFParser::evaluate()
{
    m_elfHeader.evaluate(m_reasons, m_capabilities);
    m_programHeader.evaluate(m_reasons);
    m_sectionHeader.evaluate(m_reasons, m_capabilities);
    m_segments.evaluate(m_reasons, m_capabilities);

    std::set<void *> results = m_searchEngine.search(m_mapped_file.data(), m_fileSize);
    BOOST_FOREACH (void *result, results)
    {
        SearchValue *converted = static_cast<SearchValue *>(result);
        m_capabilities[converted->m_type].insert(converted->m_info);
    }

    regexScan();
    findELF();

    for (auto &it : m_capabilities)
    {
        switch (it.first)
        {
        case elf::k_fileFunctions:
            break;
        case elf::k_networkFunctions:
            m_reasons.push_back(std::make_pair(1 * it.second.size(), std::string("Network functions")));
            break;
        case elf::k_processManipulation:
            m_reasons.push_back(std::make_pair(4 * it.second.size(), std::string("Process manipulation functions")));
            break;
        case elf::k_pipeFunctions:
            break;
        case elf::k_crypto:
            break;
        case elf::k_infoGathering:
            m_reasons.push_back(std::make_pair(1 * it.second.size(), std::string("Information gathering")));
            break;
        case elf::k_envVariables:
            m_reasons.push_back(std::make_pair(2 * it.second.size(), std::string("Environment variable manipulation")));
            break;
        case elf::k_permissions:
            break;
        case elf::k_syslog:
            m_reasons.push_back(std::make_pair(1 * it.second.size(), std::string("Syslog manipulation functions")));
            break;
        case elf::k_packetSniff:
            m_reasons.push_back(std::make_pair(10 * it.second.size(), std::string("Packet sniffing")));
            break;
        case elf::k_shell:
            m_reasons.push_back(std::make_pair(10 * it.second.size(), std::string("Shell commands")));
            break;
        case elf::k_packed:
            m_reasons.push_back(std::make_pair(25 * it.second.size(), std::string("Packed")));
            break;
        case elf::k_irc:
            m_reasons.push_back(std::make_pair(15 * it.second.size(), std::string("IRC functionality")));
            break;
        case elf::k_http:
            m_reasons.push_back(std::make_pair(4 * it.second.size(), std::string("HTTP functionality")));
            break;
        case elf::k_compression:
            m_reasons.push_back(std::make_pair(4 * it.second.size(), std::string("Compression")));
            break;
        case elf::k_ipAddress:
            m_reasons.push_back(std::make_pair(4 * it.second.size(), std::string("Hard coded IPv4 addresses")));
            break;
        case elf::k_url:
            break;
        case elf::k_hooking:
            m_reasons.push_back(std::make_pair(25 * it.second.size(), std::string("Function hooking")));
            break;
        case elf::k_antidebug:
            m_reasons.push_back(std::make_pair(10 * it.second.size(), std::string("Anti debug techniques")));
            break;
        case elf::k_filePath:
            m_reasons.push_back(std::make_pair(1 * it.second.size(), std::string("File paths")));
            break;
        case elf::k_dropper:
            m_reasons.push_back(std::make_pair(50 * it.second.size(), std::string("Dropper functionaltiy")));
            break;
        default:
            break;
        }
    }

    for (auto &it : m_reasons)
        m_score += it.first;
}

const AbstractElfHeader &ELFParser::getElfHeader() const
{
    return m_elfHeader;
}

const ProgramHeaders &ELFParser::getProgramHeaders() const
{
    return m_programHeader;
}

const SectionHeaders &ELFParser::getSectionHeaders() const
{
    return m_sectionHeader;
}

const AbstractSegments &ELFParser::getSegments() const
{
    return m_segments;
}

const DynamicSection &ELFParser::getDynamicSection() const
{
    return m_segments.getDynamicSection();
}

void ELFParser::printReasons() const
{
    std::cout << "---- Scoring Reasons ----" << std::endl;
    for (auto &it : m_reasons)
        std::cout << it.first << " . " << it.second << std::endl;
}

void ELFParser::printCapabilities() const
{
    std::cout << "---- Detected Capabilities ----" << std::endl;
    for (auto &it : m_capabilities)
    {
        switch (it.first)
        {
        case elf::k_fileFunctions:
            std::cout << "File Functions" << std::endl;
            break;
        case elf::k_networkFunctions:
            std::cout << "Network Functions" << std::endl;
            break;
        case elf::k_processManipulation:
            std::cout << "Process Manipulation" << std::endl;
            break;
        case elf::k_pipeFunctions:
            std::cout << "Pipe Functions" << std::endl;
            break;
        case elf::k_crypto:
            std::cout << "Random Functions" << std::endl;
            break;
        case elf::k_infoGathering:
            std::cout << "Information Gathering" << std::endl;
            break;
        case elf::k_envVariables:
            std::cout << "Environment Variables" << std::endl;
            break;
        case elf::k_permissions:
            std::cout << "Permissions" << std::endl;
            break;
        case elf::k_syslog:
            std::cout << "System Log" << std::endl;
            break;
        case elf::k_packetSniff:
            std::cout << "Packet Sniffing" << std::endl;
            break;
        case elf::k_shell:
            std::cout << "Shell" << std::endl;
            break;
        case elf::k_packed:
            std::cout << "Packed" << std::endl;
            break;
        case elf::k_irc:
            std::cout << "IRC" << std::endl;
            break;
        case elf::k_http:
            std::cout << "HTTP" << std::endl;
            break;
        case elf::k_compression:
            std::cout << "Compression" << std::endl;
            break;
        case elf::k_ipAddress:
            std::cout << "IP Addresses" << std::endl;
            break;
        case elf::k_url:
            std::cout << "URL" << std::endl;
            break;
        case elf::k_hooking:
            std::cout << "Function Hooking" << std::endl;
            break;
        case elf::k_antidebug:
            std::cout << "Anti-Debug" << std::endl;
            break;
        case elf::k_dropper:
            std::cout << "Dropper" << std::endl;
            break;
        case elf::k_filePath:
            std::cout << "File Path" << std::endl;
            break;
        default:
            std::cout << "Unassigned" << std::endl;
            break;
        }
        BOOST_FOREACH (const std::string &info, it.second)
        {
            std::cout << info << std::endl;
        }
    }
}

void ELFParser::printAll() const
{
    std::cout << "---- ELF Structures ----" << std::endl;
    std::cout << m_elfHeader.printToStdOut();
    std::cout << m_programHeader.printToStdOut();
    std::cout << m_sectionHeader.printToStdOut();
    std::cout << m_segments.printToStdOut() << std::endl;
}

const std::map<elf::Capabilties, std::set<std::string>> &ELFParser::getCapabilties() const
{
    return m_capabilities;
}

void ELFParser::regexScan()
{
    try
    {
        // ips
        boost::regex pattern("[1-2]?[0-9]?[0-9]\\.[1-2]?[0-9]?[0-9]\\.[1-2]?[0-9]?[0-9]\\.[1-2]?[0-9]?[0-9](?:[:0-9]{2,})*");
        const char *start = m_mapped_file.data();
        boost::cmatch m;
        while (boost::regex_search(start, m_mapped_file.data() + m_fileSize, m, pattern))
        {
            for (auto &x : m)
            {
                m_capabilities[elf::k_ipAddress].insert(x);
            }
            start = (m_mapped_file.data() + m_fileSize) - m.suffix().length();
        }

        // urls
        boost::regex urlPattern("(?:(?:http|https)://[A-Za-z0-9_./:%+?+)|(?:www.[A-Za-z0-9/:]+\\.com)");
        start = m_mapped_file.data();
        while (boost::regex_search(start, m_mapped_file.data() + m_fileSize, m, urlPattern))
        {
            for (auto &x : m)
            {
                m_capabilities[elf::k_url].insert(x);
            }
            start = (m_mapped_file.data() + m_fileSize) - m.suffix().length();
        }

        // commands
        boost::regex shellPattern("(?:(?:wget|chmod|killall|nohup|sed|insmod|echo) [[:print:]]+)|(?:tar -[[:print:]]+)");
        start = m_mapped_file.data();
        while (boost::regex_search(start, m_mapped_file.data() + m_fileSize, m, shellPattern))
        {
            for (auto &x : m)
            {
                m_capabilities[elf::k_shell].insert(x);
            }
            start = (m_mapped_file.data() + m_fileSize) - m.suffix().length();
        }

        // url request
        boost::regex urlRequest("(?:POST (?:/|%s)|GET (?:/|%s)|CONNECT (?:/|%s)|User-Agent:)[[:print:]]+");
        start = m_mapped_file.data();
        while (boost::regex_search(start, m_mapped_file.data() + m_fileSize, m, urlRequest))
        {
            for (auto &x : m)
            {
                m_capabilities[elf::k_http].insert(x);
            }
            start = (m_mapped_file.data() + m_fileSize) - m.suffix().length();
        }

        // file paths
        boost::regex filePaths("/(?:usr|etc|tmp|bin)/[a-zA-Z0-9/\\._\\-]+");
        start = m_mapped_file.data();
        while (boost::regex_search(start, m_mapped_file.data() + m_fileSize, m, filePaths))
        {
            for (auto &x : m)
            {
                m_capabilities[elf::k_filePath].insert(x);
            }
            start = (m_mapped_file.data() + m_fileSize) - m.suffix().length();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void ELFParser::findELF()
{
    SearchTree elfSearch;
    elfSearch.addWord("\x7f\x45\x4c\x46", this);
    elfSearch.compile();
    std::set<const char *> data = elfSearch.findOffsets(this->m_mapped_file.data() + 1, m_fileSize - 1);
    BOOST_FOREACH (const char *fib, data)
    {
        try
        {
            AbstractElfHeader newHeader;
            newHeader.setHeader(fib, fib - (m_mapped_file.data() + m_fileSize));
            if ((fib + newHeader.getProgramOffset()) < (m_mapped_file.data() + m_fileSize))
            {
                std::stringstream binaryFound;
                binaryFound << "Embedded ELF binary found at file offset 0x"
                            << std::hex << fib - m_mapped_file.data()
                            << " (" << std::dec << fib - m_mapped_file.data() << ")";
                m_capabilities[elf::k_dropper].insert(binaryFound.str());
            }
        }
        catch (std::exception &e)
        {
        }
    }
}

void ELFParser::calcEntropy(off_t p_offset, std::size_t p_fileSize)
{
	// calculate entry entropy binary
	unsigned char count;
	unsigned int counted_bytes[256] = {};

	for(std::size_t i = p_offset; i <= m_fileSize ; i++)
	{
			count = static_cast<unsigned char> (m_mapped_file.data()[i]);
			counted_bytes[count]++;
	}

	m_entropy =  calcEntropyFunc(counted_bytes, m_fileSize);
}