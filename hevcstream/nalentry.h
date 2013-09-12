#ifndef __NALENTRY_H__
#define __NALENTRY_H__

#include <inttypes.h>
#include <vector>
#include "bitbuffer.h"
#include "naltypes.h"

#define ENABLE_RBSP_SAVE            1

#define MAX_SUBLAYERS_MINUS1         8

class hevcstream;

typedef struct _video_parameter_set_struct {
    uint32_t        vps_video_parameter_set_id;
    uint32_t        vps_reserved_three_2bits;
    uint32_t        vps_max_layers_minus1;
    uint32_t        vps_max_sub_layers_minus1;
    uint32_t        vps_temporal_id_nesting_flag;
    uint32_t        vps_reserved_0xffff_16bits;
} VIDEO_PARAMETER_SET;

typedef struct _profile_tier_level_struct {
    uint32_t        general_profile_space;
    uint32_t        general_tier_flag;
    uint32_t        general_profile_idc;
    uint32_t        general_profile_compatibility_flag[32];
    uint32_t        general_progressive_source_flag;
    uint32_t        general_interlaced_source_flag;
    uint32_t        general_non_packed_constraint_flag;
    uint32_t        general_frame_only_constraint_flag;
    uint32_t        general_reserved_zero_44bits[2];
    uint32_t        general_level_idc;

    uint32_t        sub_layer_profile_present_flag[MAX_SUBLAYERS_MINUS1];
    uint32_t        sub_layer_level_present_flag[MAX_SUBLAYERS_MINUS1];
    uint32_t        sub_layer_level_idc[MAX_SUBLAYERS_MINUS1];
    uint32_t        sub_layer_profile_space[MAX_SUBLAYERS_MINUS1];
    uint32_t        sub_layer_tier_flag[MAX_SUBLAYERS_MINUS1];
    uint32_t        sub_layer_profile_idc[MAX_SUBLAYERS_MINUS1];
    uint32_t        sub_layer_profile_compatibility_flag[MAX_SUBLAYERS_MINUS1][ 32 ];
    uint32_t        sub_layer_progressive_source_flag[MAX_SUBLAYERS_MINUS1];
    uint32_t        sub_layer_interlaced_source_flag[MAX_SUBLAYERS_MINUS1];
    uint32_t        sub_layer_non_packed_constraint_flag[MAX_SUBLAYERS_MINUS1];
    uint32_t        sub_layer_frame_only_constraint_flag[MAX_SUBLAYERS_MINUS1];
    uint32_t        sub_layer_reserved_zero_44bits[MAX_SUBLAYERS_MINUS1][2];
} PROFILE_TIER_LEVEL;

typedef struct _sequence_parameter_set_struct {
    uint32_t            sps_video_parameter_set_id;
    uint32_t            sps_max_sub_layers_minus1;
    uint32_t            sps_temporal_id_nesting_flag;
    PROFILE_TIER_LEVEL  sps_profile;
    uint32_t            sps_seq_parameter_set_id;
    uint32_t            chroma_format_idc;
    uint32_t            seperate_color_plane_flag;
    uint32_t            pic_width_in_luma_samples;
    uint32_t            pic_height_in_luma_samples;
/* ... */
} SEQ_PARAMETER_SET;

typedef struct _picture_parameter_set_struct {
    uint32_t            pps_pic_parameter_set_id;
    uint32_t            pps_seq_parameter_set_id;
    uint32_t            dependent_slice_segments_enabled_flag;
    uint32_t            output_flag_present_flag;
    uint32_t            num_extra_slice_header_bits;
    uint32_t            sign_data_hiding_enabled_flag;
    uint32_t            cabac_init_present_flag;
} PIC_PARAMETER_SET;

typedef struct _slice_segment_header {
    uint32_t            first_slice_segment_in_pic_flag;
    uint32_t            no_output_of_prior_pics_flag;
    uint32_t            slice_pic_parameter_set_id;
/* ... */
} SLICE_SEGMENT_HDR;

/**
 *
 */

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
    void*       info() const;

#if defined(_DEBUG) && defined(ENABLE_RBSP_SAVE)
    void        save_rbsp_buffer(bitBuffer* pBuffer);
#endif

protected:
    friend class hevcstream;

    bool        parse_nal(bitBuffer* pBuffer);

    bool        slice_segment_layer_rbsp(bitBuffer::iterator& bIter);
    bool        slice_segment_header(bitBuffer::iterator& bIter);

    bool        video_parameter_set_rbsp(bitBuffer::iterator& bIter);
    bool        sequence_parameter_set_rbsp(bitBuffer::iterator& bIter);
    bool        picture_parameter_set_rbsp(bitBuffer::iterator& bIter);

    bool        profile_tier_level(bitBuffer::iterator& bIter, uint32_t maxNumSubLayersMinus1);

    uint64_t    m_bit_offset;
    size_t      m_nalSize;
    int         m_first_slice_segment_in_pic_flag;
    size_t      m_nal_num;
    int         m_picture_num;

    NALU_HDR    m_nal_unit_header;

    void*       m_pNalInfo;
    size_t      m_nalInfo_size;
} NAL_ENTRY;

typedef std::vector<NAL_ENTRY>              NALENTRY_VECTOR;
typedef std::vector<NAL_ENTRY*>             NALENTRY_PTR_VECTOR;
typedef std::vector<NAL_ENTRY>::iterator    NALENTRY_VECTOR_ITER;
typedef std::vector<NAL_ENTRY*>::iterator   NALENTRY_PTR_VECTOR_ITER;

#endif // __NALENTRY_H__
