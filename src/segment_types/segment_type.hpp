#ifndef SEGMENTTYPE_HPP
#define SEGMENTTYPE_HPP

#include <set>
#include <map>
#include <string>
#include <vector>
#include <boost/cstdint.hpp>

#include "../structures/sectionheader.hpp"
#include "../structures/capabilities.hpp"

/*
 * the segment pure virtual base class. abstracts away some basic things like
 * size and offset. future uses includes entropy and hashing.
 */
class SegmentType
{
    protected:
        // The type of underlying segment
        elf::section_type m_type;

        // The offset to this segment
        boost::uint32_t m_offset;

        // The size of this segment
        boost::uint32_t m_size;

        // A pointer to a string segment
        SegmentType *m_strings;

    public:
        /*
        *  initializes the member variables
        *  p_start a pointer to the start of the mmaped file
        *  p_offset the offset to this segment
        *  p_size the size of this segment
        *  p_type the type of segment being created
        *  p_start is currently not used, but is there for future use
        */
        SegmentType(const char *p_start, boost::uint32_t p_offset,
                    boost::uint32_t p_size, elf::section_type p_type);

        virtual ~SegmentType();

        // return the type of this segment
        elf::section_type getType() const;

        // return the offset to this segment
        boost::uint32_t getOffset() const;

        /*
        * stores this segments corresponding string table if one exists and the
        * segment needs to make use of it
        * p_strSegment a pointer to a StringTableSegment
        */
        void setStrSegment(SegmentType *p_strSegment);

        /*!
        * The default evaluate function. Segments that wish to contribute to the
        * scoring override this
        * unused the reasons vector
        * unused the capabilities vector
        */
        virtual void evaluate(std::vector<std::pair<boost::int32_t, std::string>> &,
                            std::map<elf::Capabilties, std::set<std::string>> &) const
        {
        }

        //! Every segment must have a function that creates a string representation
        virtual std::string printToStdOut() const = 0;
};

#endif
