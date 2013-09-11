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

    bool        isFirstFrameInSlice();
    bool        isVCL();
    void        set_picture_number(int picnum);
    uint64_t    offset();
    void        set_size(uint32_t nalSize);

    bool        copy_nal_to_file(bitBuffer& buffer, FILE* oFP);

protected:

    uint64_t    m_bit_offset;
    size_t      m_nalSize;
    int         m_first_slice_segment_in_pic_flag;
    int         m_picture_num;

    NALU_HDR    m_nal_unit_header;

    bool        slice_segment_layer_rbsp(bitBuffer::iterator& bIter);
    bool        slice_segment_header(bitBuffer::iterator& bIter);

    bool        video_parameter_set_rbsp(bitBuffer::iterator& bIter);
    bool        sequence_parameter_set_rbsp(bitBuffer::iterator& bIter);
    bool        picture_parameter_set_rbsp(bitBuffer::iterator& bIter);

} NAL_ENTRY;

typedef std::vector<NAL_ENTRY>              NALENTRY_VECTOR;
typedef std::vector<NAL_ENTRY>::iterator    NALENTRY_VECTOR_ITER;

#endif // __NALENTRY_H__
