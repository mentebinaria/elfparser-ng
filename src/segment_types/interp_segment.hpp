#ifndef INTERP_SEGMENT_HPP
#define INTERP_SEGMENT_HPP

#include "segment_type.hpp"

#include <string>
#include <boost/cstdint.hpp>

/*!
 * Stores the interpreter in a string. Example of the content:
 * "/lib64/ld-linux-x86-64.so.2"
 */
class InterpSegment : public SegmentType
{
public:

    /*
     * puts the interpreter into m_interpreter
     * p_start the start of the binary
     * p_offset the offset to this segment
     * p_size the size of this segment
     * p_type elf::k_progbits
     */
    InterpSegment(const char* start, boost::uint32_t p_offset,
                  boost::uint32_t p_size, elf::section_type p_type);

    // nothing of note
    ~InterpSegment();

    // return the string representation of the interp segment
    virtual std::string printToStdOut() const;

private:

    // disable evil things
    InterpSegment(const InterpSegment& p_rhs);
    InterpSegment& operator=(const InterpSegment& p_rhs);

    // the name of the interpreter
    std::string m_interpreter;
};

#endif
