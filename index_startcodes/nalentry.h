#ifndef __NALENTRY_H__
#define __NALENTRY_H__

#include <inttypes.h>
#include <vector>
#include "bitbuffer.h"
#include "naltypes.h"

typedef class nalEntry {
public:
    nalEntry();
    nalEntry(const nalEntry& copy);
    nalEntry(bitBuffer::iterator& bIter);
    virtual ~nalEntry();

    void        display(FILE* oFP);

    uint64_t    m_bit_offset;
    size_t      m_nalSize;

    NALU_HDR    m_nal_unit_header;
} NAL_ENTRY;

typedef std::vector<NAL_ENTRY>              NALENTRY_VECTOR;
typedef std::vector<NAL_ENTRY>::iterator    NALENTRY_VECTOR_ITER;

#endif // __NALENTRY_H__
