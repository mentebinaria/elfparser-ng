#include "gtest/gtest.h"
#include "../../src/elfparser.hpp"
#include "../../src/abstract_programheader.hpp"
#include "../../src/abstract_sectionheader.hpp"
#include "../../src/abstract_segments.hpp"
#include "../../src/structures/sectionheader.hpp"
#include "../../src/segment_types/segment_type.hpp"
#include "../../src/dynamicsection.hpp"
#include "../../src/symbols.hpp"

#include <boost/foreach.hpp>

class TinyTest : public testing::Test
{
protected:
    
    virtual void SetUp(){ }
    
    ELFParser m_parser;
};

/*
 * the true.asm binary from muppet labs.
 */
TEST_F(TinyTest, true_binary)
{
    m_parser.parse("../tests/test_files/true");
    EXPECT_FALSE(m_parser.getElfHeader().is64());
    EXPECT_TRUE(m_parser.getElfHeader().isLE());
    EXPECT_STREQ("7fELF", m_parser.getElfHeader().getMagic().c_str());
    EXPECT_EQ(0x2549001a, m_parser.getElfHeader().getEntryPoint());
    EXPECT_EQ("0x2549001a", m_parser.getElfHeader().getEntryPointString());
    EXPECT_EQ(4, m_parser.getElfHeader().getProgramOffset());
    EXPECT_EQ(0xAEF25F5B, m_parser.getElfHeader().getSectionOffset());
    EXPECT_EQ(32, m_parser.getElfHeader().getProgramSize());
    EXPECT_EQ(1, m_parser.getElfHeader().getProgramCount());
    EXPECT_EQ(0, m_parser.getElfHeader().getSectionSize());
    EXPECT_EQ(0, m_parser.getElfHeader().getSectionCount());
    EXPECT_EQ(0, m_parser.getElfHeader().getStringTableIndex());
    EXPECT_EQ(1, m_parser.getProgramHeaders().getProgramHeaders().size());
    EXPECT_EQ(0, m_parser.getSectionHeaders().getSections().size());
    EXPECT_EQ(0, m_parser.getSectionHeaders().getStringTableIndex());
    EXPECT_STREQ(m_parser.getProgramHeaders().getProgramHeaders()[0].getName().c_str(),"PT_LOAD");
    EXPECT_EQ(m_parser.getProgramHeaders().getProgramHeaders()[0].getVirtualAddress(), 0x25490000);
}

/*
 * the bf.asm binary from muppet labs.
 */
TEST_F(TinyTest, bf_binary)
{
    m_parser.parse("../tests/test_files/bf");
    EXPECT_FALSE(m_parser.getElfHeader().is64());
    EXPECT_TRUE(m_parser.getElfHeader().isLE());
    EXPECT_STREQ("7fELF", m_parser.getElfHeader().getMagic().c_str());
    EXPECT_EQ(0x45e9b095, m_parser.getElfHeader().getEntryPoint());
    EXPECT_EQ("0x45e9b095", m_parser.getElfHeader().getEntryPointString());
    EXPECT_EQ(44, m_parser.getElfHeader().getProgramOffset());
    EXPECT_EQ(693735845, m_parser.getElfHeader().getSectionOffset());
    EXPECT_EQ(32, m_parser.getElfHeader().getProgramSize());
    EXPECT_EQ(1, m_parser.getElfHeader().getProgramCount());
    EXPECT_EQ(0, m_parser.getElfHeader().getSectionSize());
    EXPECT_EQ(0, m_parser.getElfHeader().getSectionCount());
    EXPECT_EQ(0, m_parser.getElfHeader().getStringTableIndex());
    EXPECT_EQ(1, m_parser.getProgramHeaders().getProgramHeaders().size());
    EXPECT_EQ(0, m_parser.getSectionHeaders().getSections().size());
    EXPECT_EQ(0, m_parser.getSectionHeaders().getStringTableIndex());
    EXPECT_STREQ(m_parser.getProgramHeaders().getProgramHeaders()[0].getName().c_str(),"PT_LOAD");
    EXPECT_EQ(m_parser.getProgramHeaders().getProgramHeaders()[0].getVirtualAddress(), 0x45e9b000);
}
