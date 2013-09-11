#include <stdio.h>
#include "nalentry.h"

/**
 *
 */

nalEntry::nalEntry()
:   m_bit_offset(-1),
    m_nalSize(0),
    m_first_slice_segment_in_pic_flag(0),
    m_nal_num(0),
    m_picture_num(0)
{
    //ctor
}

/**
 *
 */

nalEntry::nalEntry(bitBuffer::iterator& bIter, size_t nalNum)
:   m_first_slice_segment_in_pic_flag(0),
    m_nal_num(nalNum),
    m_picture_num(0)
{
    //ctor
    m_bit_offset = bIter.pos();
    m_nal_unit_header.forbidden_zero_bit    = bIter.get_bits(1);
    m_nal_unit_header.nal_unit_type         = (eNalType)bIter.get_bits(6);
    m_nal_unit_header.nuh_layer_id          = bIter.get_bits(6);
    m_nal_unit_header.nuh_temporal_id_plus1 = bIter.get_bits(3);

#ifdef _DEBUG
    fprintf(stderr, "Found Nal @ %ld (0x%lx) Type %s Layer %d tempId %d\n",
           m_bit_offset, m_bit_offset/8,
           get_nal_type_desc(m_nal_unit_header.nal_unit_type),
           m_nal_unit_header.nuh_layer_id,
           m_nal_unit_header.nuh_temporal_id_plus1 - 1);
#endif

    switch (m_nal_unit_header.nal_unit_type) {
        case NAL_VPS_NUT:
            video_parameter_set_rbsp(bIter);
            break;
        case NAL_SPS_NUT:
            sequence_parameter_set_rbsp(bIter);
            break;
        case NAL_PPS_NUT:
            picture_parameter_set_rbsp(bIter);
            break;
        case NAL_IDR_W_RADL:
        case NAL_IDR_N_LP:
            slice_segment_layer_rbsp(bIter);
            break;
        case NAL_TRAIL_N:
        case NAL_TRAIL_R:
            slice_segment_layer_rbsp(bIter);
            break;
        case NAL_TSA_N:
        case NAL_TSA_R:
            slice_segment_layer_rbsp(bIter);
            break;
        case NAL_CRA_NUT:
            slice_segment_layer_rbsp(bIter);
            break;
        default:
            break;
    }

    return;
}

/**
 *
 */

nalEntry::nalEntry(const nalEntry& copy) {
    //ctor
    m_bit_offset                        = copy.m_bit_offset;
    m_nalSize                           = copy.m_nalSize;
    m_nal_unit_header                   = copy.m_nal_unit_header;
    m_first_slice_segment_in_pic_flag   = copy.m_first_slice_segment_in_pic_flag;
    m_picture_num                       = copy.m_picture_num;
}

/**
 *
 */

nalEntry::~nalEntry()
{
    //dtor
}

/**
 *
 */

bool nalEntry::isFirstFrameInSlice() {
    return (m_first_slice_segment_in_pic_flag == 1)?true:false;
}

/**
 *
 */

bool nalEntry::isVCL() {
    return ((m_nal_unit_header.nal_unit_type >= NAL_TRAIL_N) &&
            (m_nal_unit_header.nal_unit_type <= NAL_RSV_VCL31))?true:false;
}

void nalEntry::set_picture_number(int picnum) {
    m_picture_num = picnum;
}

uint64_t nalEntry::offset() {
    return m_bit_offset;
}

void nalEntry::set_size(uint32_t size) {
    m_nalSize = size;
    return;
}

/**
 *
 */

void nalEntry::display(FILE* oFP, DUMP_TYPE type) {
    if (type == DUMP_SHORT) {
        fprintf(oFP, "  %-6ld %-6d %-16s (0x%02x) %-8ld %08lx %-8ld %-5s %s\n",
                m_nal_num, m_picture_num,
                get_nal_type_desc(m_nal_unit_header.nal_unit_type), (int)m_nal_unit_header.nal_unit_type,
                m_bit_offset, m_bit_offset/8, m_nalSize,
                isVCL()?"Yes":"No",
                (m_first_slice_segment_in_pic_flag == 1)?"Yes":"No");
    } else if (type == DUMP_LONG) {
        fprintf(oFP, "NAL # %ld:\n", m_nal_num);
        fprintf(oFP, "  NAL TYPE    : %-20s (0x%04x)\n", get_nal_type_desc(m_nal_unit_header.nal_unit_type), (int)m_nal_unit_header.nal_unit_type);
        fprintf(oFP, "  NAL OFF     : Bit %ld (0x%lx)\n", m_bit_offset, m_bit_offset/8);
        fprintf(oFP, "  NAL SIZE    : %ld\n", m_nalSize);
        fprintf(oFP, "  VCL         : %s\n", isVCL()?"Yes":"No");
        fprintf(oFP, "  PICTURE #   : %d\n", m_picture_num);
        fprintf(oFP, "  LAYER ID    : %d\n", m_nal_unit_header.nuh_layer_id);
        fprintf(oFP, "  TEMP ID     : %d\n", m_nal_unit_header.nuh_temporal_id_plus1 - 1);
        fprintf(oFP, "  FIRST SLICE : %s\n", (m_first_slice_segment_in_pic_flag == 1)?"Yes":"No");
    }
}

/**
 *
 */

bool nalEntry::video_parameter_set_rbsp(bitBuffer::iterator& bIter) {
    uint32_t vps_video_parameter_set_id;
    uint32_t vps_reserved_three_2bits;
    uint32_t vps_max_layers_minus1;
    uint32_t vps_max_sub_layers_minus1;
    uint32_t vps_temporal_id_nesting_flag;
    uint32_t vps_reserved_0xffff_16bits;

#ifdef  _DEBUG
    fprintf(stderr, "nalEntry::video_parameter_set()\n");
#endif

    vps_video_parameter_set_id      = bIter.get_bits(4);
    vps_reserved_three_2bits        = bIter.get_bits(2);
    vps_max_layers_minus1           = bIter.get_bits(6);
    vps_max_sub_layers_minus1       = bIter.get_bits(3);
    vps_temporal_id_nesting_flag    = bIter.get_bits(1);
    vps_reserved_0xffff_16bits      = bIter.get_bits(16);

#ifdef _DEBUG
    fprintf(stderr, "vps_video_parameter_set_id   : %x\n", vps_video_parameter_set_id);
    fprintf(stderr, "vps_reserved_three_2bits     : %x\n", vps_reserved_three_2bits);
    fprintf(stderr, "vps_max_layers_minus1        : %x\n", vps_max_layers_minus1);
    fprintf(stderr, "vps_max_sub_layers_minus1    : %x\n", vps_max_sub_layers_minus1);
    fprintf(stderr, "vps_temporal_id_nesting_flag : %x\n", vps_temporal_id_nesting_flag);
    fprintf(stderr, "vps_reserved_0xffff_16bits   : %x\n", vps_reserved_0xffff_16bits);
#endif

    return true;
}

/**
 *
 */

bool nalEntry::sequence_parameter_set_rbsp(bitBuffer::iterator& bIter) {
    uint32_t    sps_video_parameter_set_id;
    uint32_t    sps_max_sub_layers_minus1;
    uint32_t    sps_temporal_id_nesting_flag;
    uint32_t    sps_seq_parameter_set_id;
    uint32_t    chroma_format_idc;
    uint32_t    seperate_color_plane_flag;
    uint32_t    pic_width_in_luma_samples,
                pic_height_in_luma_samples;

#ifdef  _DEBUG
    fprintf(stderr, "nalEntry::sequence_parameter_set()\n");
#endif

    sps_video_parameter_set_id      = bIter.get_bits(4);
    sps_max_sub_layers_minus1       = bIter.get_bits(3);
    sps_temporal_id_nesting_flag    = bIter.get_bits(1);

#if 0
    profile_tier_level( bIter, sps_max_sub_layers_minus1  );

    sps_seq_parameter_set_id        = bIter.ue();
    chroma_format_idc               = bIter.ue();

    if (chroma_format_idc == 3) {
        seperate_color_plane_flag = bIter.get_bits(1);
    }

    pic_width_in_luma_samples = bIter.ue();
    pic_height_in_luma_samples = bIter.ue();
#endif

#ifdef  _DEBUG
    fprintf(stderr, "sps_video_parameter_set_id   : %x\n", sps_video_parameter_set_id);
    fprintf(stderr, "sps_max_sub_layers_minus1    : %x\n", sps_max_sub_layers_minus1);
    fprintf(stderr, "sps_temporal_id_nesting_flag : %x\n", sps_temporal_id_nesting_flag);
#endif

    bIter.byte_align();
    return true;
}

/**
 *
 */

bool nalEntry::picture_parameter_set_rbsp(bitBuffer::iterator& bIter) {
    uint32_t pps_pic_parameter_set_id;
    uint32_t pps_seq_parameter_set_id;

#ifdef  _DEBUG
    fprintf(stderr, "nalEntry::picture_parameter_set()\n");
#endif

    pps_pic_parameter_set_id = bIter.ue();
    pps_seq_parameter_set_id = bIter.ue();

    bIter.byte_align();

#ifdef  _DEBUG
    fprintf(stderr, "pps_pic_parameter_set_id   : %x\n", pps_pic_parameter_set_id);
    fprintf(stderr, "pps_seq_parameter_set_id   : %x\n", pps_seq_parameter_set_id);
#endif

    return true;
}

/**
 *
 */

bool nalEntry::slice_segment_layer_rbsp(bitBuffer::iterator& bIter) {
#ifdef  _DEBUG
    fprintf(stderr, "nalEntry::slice_segment_layer_rbsp()\n");
#endif

    slice_segment_header(bIter);

    return true;
}

/**
 *
 */

bool nalEntry::slice_segment_header(bitBuffer::iterator& bIter) {
    //uint32_t first_slice_segment_in_pic_flag;
    uint32_t no_output_of_prior_pics_flag = 0;

#ifdef  _DEBUG
    fprintf(stderr, "nalEntry::slice_segment_header()\n");
#endif

    m_first_slice_segment_in_pic_flag = bIter.get_bits(1);

    if ((m_nal_unit_header.nal_unit_type >= NAL_BLA_W_LP) && (m_nal_unit_header.nal_unit_type <= NAL_RSV_IRAP_VCL23))
        no_output_of_prior_pics_flag = bIter.get_bits(1);

    bIter.byte_align();

#ifdef  _DEBUG
    fprintf(stderr, "first_slice_segment_in_pic_flag : %x\n", m_first_slice_segment_in_pic_flag);
    fprintf(stderr, "no_output_of_prior_pics_flag    : %x\n", no_output_of_prior_pics_flag);
#endif

    return true;
}

/**
 *
 */

bool nalEntry::profile_tier_level(bitBuffer::iterator& bIter, uint32_t maxNumSubLayersMinus1) {
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
    uint32_t        sub_layer_profile_present_flag[maxNumSubLayersMinus1];
    uint32_t        sub_layer_level_present_flag[maxNumSubLayersMinus1];

#ifdef  _DEBUG
    fprintf(stderr, "nalEntry::profile_tier_level(..., %d)\n", maxNumSubLayersMinus1);
#endif

    general_profile_space = bIter.get_bits(2);
    general_tier_flag     = bIter.get_bits(1);
    general_profile_idc   = bIter.get_bits(5);

    for (int j = 0 ; j < 32 ; j++) {
        general_profile_compatibility_flag[j] = bIter.get_bits(1);
#ifdef _DEBUG
        fprintf(stderr, "general_profile_compatibility_flag[%d] = %d\n", j, general_profile_compatibility_flag[j]);
#endif
    }

    general_progressive_source_flag     = bIter.get_bits(1);
    general_interlaced_source_flag      = bIter.get_bits(1);
    general_non_packed_constraint_flag  = bIter.get_bits(1);
    general_frame_only_constraint_flag  = bIter.get_bits(1);
    general_reserved_zero_44bits[0]     = bIter.get_bits(32);
    general_reserved_zero_44bits[1]     = bIter.get_bits(12);
    general_level_idc                   = bIter.get_bits(8);

    for (size_t i = 0 ; i < maxNumSubLayersMinus1 ; i++) {
        sub_layer_profile_present_flag[i] = bIter.get_bits(1);
        sub_layer_level_present_flag[i]   = bIter.get_bits(1);
    }

    if (maxNumSubLayersMinus1 > 0) {
        for (size_t i = 0 ; i < 8 ; i++) {
            uint32_t reserved_zero_2bits = bIter.get_bits(2);
        }
    }

    for (size_t i = 0 ; i < maxNumSubLayersMinus1 ; i++) {
        if (sub_layer_profile_present_flag[i] == 1) {

        }
        if (sub_layer_level_present_flag[i] == 1) {

        }
    }

    return true;
}

/**
 *
 */

bool nalEntry::copy_nal_to_file(bitBuffer& buffer, FILE* oFP) {
    uint8_t* pBuffer = buffer.m_pBufStart + (m_bit_offset / 8) - 4;
    fwrite(pBuffer, m_nalSize, 1, oFP);

    return true;
}

/**
 *  Return NAL index number.
 */

size_t nalEntry::nal_index_num() const {
    return m_nal_num;
}

/**
 *  Return NAL picture number.
 */

size_t nalEntry::nal_picture_num() const {
    return m_picture_num;
}

/**
 *
 */

eNalType nalEntry::nal_type() const {
    return m_nal_unit_header.nal_unit_type;
}
