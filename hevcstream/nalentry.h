#ifndef __NALENTRY_H__
#define __NALENTRY_H__

#include <inttypes.h>
#include <vector>
#include "bitbuffer.h"
#include "naltypes.h"

struct sequence_parameter_set_struct {
    uint32_t    sps_video_parameter_set_id;
    uint32_t    sps_max_sub_layers_minus1;
    uint32_t    sps_temporal_id_nesting_flag;
    uint32_t    sps_seq_parameter_set_id;
    uint32_t    chroma_format_idc;
    uint32_t    seperate_color_plane_flag;
    uint32_t    pic_width_in_luma_samples;
    uint32_t    pic_height_in_luma_samples;
};

typedef class nalEntry {
public:
    nalEntry();
    nalEntry(const nalEntry& copy);
    nalEntry(bitBuffer::iterator& bIter, size_t nalNum);
    virtual ~nalEntry();

    typedef enum _dumpType {
        DUMP_SHORT,
        DUMP_LONG,
    } DUMP_TYPE;

    void        display(FILE* oFP, DUMP_TYPE type = DUMP_SHORT);

    bool        isFirstFrameInSlice();
    bool        isVCL();
    void        set_picture_number(int picnum);
    uint64_t    offset();
    void        set_size(uint32_t nalSize);

    bool        copy_nal_to_file(bitBuffer& buffer, FILE* oFP);

    size_t      nal_index_num() const;
    size_t      nal_picture_num() const;
    eNalType    nal_type() const;

protected:

    uint64_t    m_bit_offset;
    size_t      m_nalSize;
    int         m_first_slice_segment_in_pic_flag;
    size_t      m_nal_num;
    int         m_picture_num;

    NALU_HDR    m_nal_unit_header;

    bool        slice_segment_layer_rbsp(bitBuffer::iterator& bIter);
    bool        slice_segment_header(bitBuffer::iterator& bIter);

    bool        video_parameter_set_rbsp(bitBuffer::iterator& bIter);
    bool        sequence_parameter_set_rbsp(bitBuffer::iterator& bIter);
    bool        picture_parameter_set_rbsp(bitBuffer::iterator& bIter);

    bool        profile_tier_level(bitBuffer::iterator& bIter, uint32_t maxNumSubLayersMinus1);
} NAL_ENTRY;

typedef std::vector<NAL_ENTRY>              NALENTRY_VECTOR;
typedef std::vector<NAL_ENTRY*>             NALENTRY_PTR_VECTOR;
typedef std::vector<NAL_ENTRY>::iterator    NALENTRY_VECTOR_ITER;
typedef std::vector<NAL_ENTRY*>::iterator   NALENTRY_PTR_VECTOR_ITER;

#endif // __NALENTRY_H__
