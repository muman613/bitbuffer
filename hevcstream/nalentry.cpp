/**
 *  @file       nalentry.cpp
 *  @author     Michael A. Uman
 *  @date       September 11, 2013 (Do not forget Jonathan J Uman)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "nalentry.h"

/**
 *
 */

nalEntry::nalEntry()
:   m_bit_offset(-1),
    m_nalSize(0),
//    m_first_slice_segment_in_pic_flag(0),
    m_nal_num(0),
    m_picture_num(0),
    m_long_sc(0),
    m_pNalInfo(0L),
    m_nalInfo_size(0)
{
    //ctor
}

/**
 *
 */

nalEntry::nalEntry(bitBuffer::iterator& bIter, bool longsc, size_t nalNum)
:   m_nal_num(nalNum),
    m_picture_num(0),
    m_long_sc(longsc),
    m_pNalInfo(0L),
    m_nalInfo_size(0)
{
    //ctor
    m_bit_offset = bIter.pos();

    /* parse nal header */
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
//    m_first_slice_segment_in_pic_flag   = copy.m_first_slice_segment_in_pic_flag;
    m_nal_num                           = copy.m_nal_num;
    m_picture_num                       = copy.m_picture_num;
    m_long_sc                           = copy.m_long_sc;

    /* copy associated nal info data */
    m_pNalInfo = malloc(copy.m_nalInfo_size);
    assert(m_pNalInfo != 0L);
    memcpy(m_pNalInfo, copy.m_pNalInfo, copy.m_nalInfo_size);
    m_nalInfo_size                      = copy.m_nalInfo_size;
}

/**
 *
 */

nalEntry::~nalEntry()
{
    //dtor
#ifdef  _DEBUG
    fprintf(stderr, "nalEntry::~nalEntry()\n");
#endif
    if (m_pNalInfo != 0) {
#ifdef  _DEBUG
        fprintf(stderr, "-- freeing %s nal info @ %p !\n",
                get_nal_type_desc(m_nal_unit_header.nal_unit_type),
                m_pNalInfo);
#endif
        free(m_pNalInfo);
        m_pNalInfo = 0L;
    }
}


void* nalEntry::info() const {
    return m_pNalInfo;
}


/**
 *
 */

bool nalEntry::isFirstFrameInSlice() {
#if 1
    bool bRes = false;
    eNalType nalType = m_nal_unit_header.nal_unit_type;

    if (((nalType >= NAL_TRAIL_N) && (nalType <= NAL_RASL_R)) ||
        ((nalType >= NAL_BLA_W_LP) && (nalType <= NAL_CRA_NUT)))
    {
        SLICE_SEGMENT_HDR*  shdr = (SLICE_SEGMENT_HDR*)m_pNalInfo;
        assert(shdr != 0L);
        bRes = (shdr->first_slice_segment_in_pic_flag == 1)?true:false;
    }

    return bRes;
#else
    return (m_first_slice_segment_in_pic_flag == 1)?true:false;
#endif
}

/**
 *
 */

bool nalEntry::isVCL() {
    return ((m_nal_unit_header.nal_unit_type >= NAL_TRAIL_N) &&
            (m_nal_unit_header.nal_unit_type <= NAL_RSV_VCL31))?true:false;
}

void nalEntry::set_picture_number(int picnum) {
#ifdef  _DEBUG
    fprintf(stderr, "nalEntry::set_picture_number(%d)\n"
                    "Setting nal %ld picture # to %d\n",
                    picnum, m_nal_num, picnum);
#endif

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

void nalEntry::display(FILE* oFP, int type) {
    if ((type & DUMP_SHORT) != 0) {
        fprintf(oFP, "  %-6ld %-6d %-16s (0x%02x) %-8ld %08lx %-8ld %-5s %-4s %d %s\n",
                m_nal_num, m_picture_num,
                get_nal_type_desc(m_nal_unit_header.nal_unit_type), (int)m_nal_unit_header.nal_unit_type,
                m_bit_offset, m_bit_offset/8, m_nalSize,
                isVCL()?"Yes":"No",
                isFirstFrameInSlice()?"Yes":"No",
                m_nal_unit_header.nuh_temporal_id_plus1 - 1,
                m_long_sc?"32":"24");
    } else if ((type & DUMP_LONG) != 0) {
        fprintf(oFP, "NAL # %ld:\n", m_nal_num);
        fprintf(oFP, "  NAL TYPE    : %-20s (0x%04x)\n", get_nal_type_desc(m_nal_unit_header.nal_unit_type), (int)m_nal_unit_header.nal_unit_type);
        fprintf(oFP, "  NAL OFF     : Bit %ld (0x%lx)\n", m_bit_offset, m_bit_offset/8);
        fprintf(oFP, "  NAL SIZE    : %ld\n", m_nalSize);
        fprintf(oFP, "  VCL         : %s\n", isVCL()?"Yes":"No");
        fprintf(oFP, "  PICTURE #   : %d\n", m_picture_num);
        fprintf(oFP, "  LAYER ID    : %d\n", m_nal_unit_header.nuh_layer_id);
        fprintf(oFP, "  TEMP ID     : %d\n", m_nal_unit_header.nuh_temporal_id_plus1 - 1);
        fprintf(oFP, "  FIRST SLICE : %s\n", isFirstFrameInSlice()?"Yes":"No");
    }
}

/**
 *
 */

bool nalEntry::video_parameter_set_rbsp(bitBuffer::iterator& bIter) {
#ifdef  _DEBUG
    fprintf(stderr, "nalEntry::video_parameter_set()\n");
#endif

    VIDEO_PARAMETER_SET*    vps;

    vps = (VIDEO_PARAMETER_SET*)calloc(1, sizeof(VIDEO_PARAMETER_SET));
    assert(vps != 0L);

    m_pNalInfo      = (void*)vps;
    m_nalInfo_size  = sizeof(VIDEO_PARAMETER_SET);

    vps->vps_video_parameter_set_id      = bIter.get_bits(4);
    vps->vps_reserved_three_2bits        = bIter.get_bits(2);
    vps->vps_max_layers_minus1           = bIter.get_bits(6);
    vps->vps_max_sub_layers_minus1       = bIter.get_bits(3);
    vps->vps_temporal_id_nesting_flag    = bIter.get_bits(1);
    vps->vps_reserved_0xffff_16bits      = bIter.get_bits(16);

#ifdef _DEBUG
    fprintf(stderr, "vps_video_parameter_set_id   : %x\n", vps->vps_video_parameter_set_id);
    fprintf(stderr, "vps_reserved_three_2bits     : %x\n", vps->vps_reserved_three_2bits);
    fprintf(stderr, "vps_max_layers_minus1        : %x\n", vps->vps_max_layers_minus1);
    fprintf(stderr, "vps_max_sub_layers_minus1    : %x\n", vps->vps_max_sub_layers_minus1);
    fprintf(stderr, "vps_temporal_id_nesting_flag : %x\n", vps->vps_temporal_id_nesting_flag);
    fprintf(stderr, "vps_reserved_0xffff_16bits   : %x\n", vps->vps_reserved_0xffff_16bits);
#endif

    return true;
}

/**
 *
 */

bool nalEntry::sequence_parameter_set_rbsp(bitBuffer::iterator& bIter) {
#ifdef  _DEBUG
    fprintf(stderr, "nalEntry::sequence_parameter_set()\n");
#endif

    SEQ_PARAMETER_SET*      sps = 0L;

    sps = (SEQ_PARAMETER_SET*)calloc(1, sizeof(SEQ_PARAMETER_SET));
    assert(sps != 0L);

    m_pNalInfo     = (void*)sps;
    m_nalInfo_size = sizeof(SEQ_PARAMETER_SET);

    sps->sps_video_parameter_set_id         = bIter.get_bits(4);
    sps->sps_max_sub_layers_minus1          = bIter.get_bits(3);
    sps->sps_temporal_id_nesting_flag       = bIter.get_bits(1);

    profile_tier_level( bIter, sps->sps_max_sub_layers_minus1 );

    sps->sps_seq_parameter_set_id           = bIter.ue();
    sps->chroma_format_idc                  = bIter.ue();

    if (sps->chroma_format_idc == 3) {
        sps->seperate_color_plane_flag = bIter.get_bits(1);
    }

    sps->pic_width_in_luma_samples          = bIter.ue();
    sps->pic_height_in_luma_samples         = bIter.ue();

#ifdef  _DEBUG
    fprintf(stderr, "sps_video_parameter_set_id         : %x\n", sps->sps_video_parameter_set_id);
    fprintf(stderr, "sps_max_sub_layers_minus1          : %x\n", sps->sps_max_sub_layers_minus1);
    fprintf(stderr, "sps_temporal_id_nesting_flag       : %x\n", sps->sps_temporal_id_nesting_flag);
    fprintf(stderr, "sps_seq_parameter_set_id           : %x\n", sps->sps_seq_parameter_set_id);
    fprintf(stderr, "chroma_format_idc                  : %x\n", sps->chroma_format_idc);
    fprintf(stderr, "pic_width_in_luma_samples          : %d\n", sps->pic_width_in_luma_samples);
    fprintf(stderr, "pic_height_in_luma_samples         : %d\n", sps->pic_height_in_luma_samples);
#endif

    bIter.byte_align();
    return true;
}

/**
 *
 */

bool nalEntry::picture_parameter_set_rbsp(bitBuffer::iterator& bIter) {
#ifdef  _DEBUG
    fprintf(stderr, "nalEntry::picture_parameter_set()\n");
#endif

    PIC_PARAMETER_SET*          pps = 0L;

    pps = (PIC_PARAMETER_SET *)calloc(1, sizeof(PIC_PARAMETER_SET));
    assert(pps != 0L);

    m_pNalInfo     = (void*)pps;
    m_nalInfo_size = sizeof(PIC_PARAMETER_SET);

    pps->pps_pic_parameter_set_id               = bIter.ue();
    pps->pps_seq_parameter_set_id               = bIter.ue();
    pps->dependent_slice_segments_enabled_flag  = bIter.get_bits(1);
    pps->output_flag_present_flag               = bIter.get_bits(1);
    pps->num_extra_slice_header_bits            = bIter.get_bits(3);
    pps->sign_data_hiding_enabled_flag          = bIter.get_bits(1);
    pps->cabac_init_present_flag                = bIter.get_bits(1);

#ifdef  _DEBUG
    fprintf(stderr, "pps_pic_parameter_set_id                : %x\n", pps->pps_pic_parameter_set_id);
    fprintf(stderr, "pps_seq_parameter_set_id                : %x\n", pps->pps_seq_parameter_set_id);
    fprintf(stderr, "dependent_slice_segments_enabled_flag   : %x\n", pps->dependent_slice_segments_enabled_flag);
    fprintf(stderr, "output_flag_present_flag                : %x\n", pps->output_flag_present_flag);
    fprintf(stderr, "num_extra_slice_header_bits             : %x\n", pps->num_extra_slice_header_bits);
    fprintf(stderr, "sign_data_hiding_enabled_flag           : %x\n", pps->sign_data_hiding_enabled_flag);
    fprintf(stderr, "cabac_init_present_flag                 : %x\n", pps->cabac_init_present_flag);
#endif

    bIter.byte_align();

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
#ifdef  _DEBUG
    fprintf(stderr, "nalEntry::slice_segment_header()\n");
#endif

    SLICE_SEGMENT_HDR*      shdr = 0L;

    shdr = (SLICE_SEGMENT_HDR*)calloc(1, sizeof(SLICE_SEGMENT_HDR));
    assert(shdr != 0L);

    m_pNalInfo     = (void*)shdr;
    m_nalInfo_size = sizeof(SLICE_SEGMENT_HDR);

    shdr->first_slice_segment_in_pic_flag = bIter.get_bits(1);

    if ((m_nal_unit_header.nal_unit_type >= NAL_BLA_W_LP) && (m_nal_unit_header.nal_unit_type <= NAL_RSV_IRAP_VCL23))
        shdr->no_output_of_prior_pics_flag = bIter.get_bits(1);

    shdr->slice_pic_parameter_set_id = bIter.ue();

#ifdef  _DEBUG
    fprintf(stderr, "first_slice_segment_in_pic_flag : %x\n", shdr->first_slice_segment_in_pic_flag);
    fprintf(stderr, "no_output_of_prior_pics_flag    : %x\n", shdr->no_output_of_prior_pics_flag);
    fprintf(stderr, "slice_pic_parameter_set_id      : %x\n", shdr->slice_pic_parameter_set_id);
#endif

    bIter.byte_align();

    return true;
}

/**
 *
 */

bool nalEntry::profile_tier_level(bitBuffer::iterator& bIter, uint32_t maxNumSubLayersMinus1) {
#ifdef  _DEBUG
    fprintf(stderr, "nalEntry::profile_tier_level(..., %d)\n", maxNumSubLayersMinus1);
#endif

    PROFILE_TIER_LEVEL*     ptl = &(((SEQ_PARAMETER_SET *)m_pNalInfo)->sps_profile);

    ptl->general_profile_space = bIter.get_bits(2);
    ptl->general_tier_flag     = bIter.get_bits(1);
    ptl->general_profile_idc   = bIter.get_bits(5);

    for (int j = 0 ; j < 32 ; j++) {
        ptl->general_profile_compatibility_flag[j] = bIter.get_bits(1);
    }

    ptl->general_progressive_source_flag     = bIter.get_bits(1);
    ptl->general_interlaced_source_flag      = bIter.get_bits(1);
    ptl->general_non_packed_constraint_flag  = bIter.get_bits(1);
    ptl->general_frame_only_constraint_flag  = bIter.get_bits(1);
    ptl->general_reserved_zero_44bits[0]     = bIter.get_bits(32);
    ptl->general_reserved_zero_44bits[1]     = bIter.get_bits(12);
    ptl->general_level_idc                   = bIter.get_bits(8);

    for (size_t i = 0 ; i < maxNumSubLayersMinus1 ; i++) {
        ptl->sub_layer_profile_present_flag[i] = bIter.get_bits(1);
        ptl->sub_layer_level_present_flag[i]   = bIter.get_bits(1);
    }

    if (maxNumSubLayersMinus1 > 0) {
        for (size_t i = maxNumSubLayersMinus1 ; i < 8 ; i++) {
#ifdef  _DEBUG
            uint32_t reserved_zero_2bits = bIter.get_bits(2);
            fprintf(stderr,"reserved_zero_2bits                : %x\n", reserved_zero_2bits);
#else
            bIter.get_bits(2);
#endif
        }
    }

    for (size_t i = 0 ; i < maxNumSubLayersMinus1 ; i++) {
        if (ptl->sub_layer_profile_present_flag[i] == 1) {
            ptl->sub_layer_profile_space[i]              = bIter.get_bits(2);
            ptl->sub_layer_tier_flag[i]                  = bIter.get_bits(1);
            ptl->sub_layer_profile_idc[i]                = bIter.get_bits(5);

            for (size_t j = 0 ; j < 32 ; j++)
                ptl->sub_layer_profile_compatibility_flag[i][j] = bIter.get_bits(1);

            ptl->sub_layer_progressive_source_flag[i]    = bIter.get_bits(1);
            ptl->sub_layer_interlaced_source_flag[i]     = bIter.get_bits(1);
            ptl->sub_layer_non_packed_constraint_flag[i] = bIter.get_bits(1);
            ptl->sub_layer_frame_only_constraint_flag[i] = bIter.get_bits(1);
            ptl->sub_layer_reserved_zero_44bits[i][0]    = bIter.get_bits(32);
            ptl->sub_layer_reserved_zero_44bits[i][1]    = bIter.get_bits(12);
        }

        if (ptl->sub_layer_level_present_flag[i] == 1) {
            ptl->sub_layer_level_idc[i] = bIter.get_bits(8);
        }
    }
#ifdef _DEBUG
    fprintf(stderr, "general_profile_space                 : %x\n", ptl->general_profile_space);
    fprintf(stderr, "general_tier_flag                     : %x\n", ptl->general_tier_flag);
    fprintf(stderr, "general_profile_idc                   : %x\n", ptl->general_profile_idc);
    for (int j = 0 ; j < 32 ; j++)
        fprintf(stderr,"general_profile_compatibility_flag[%d] : %x\n", j, ptl->general_profile_compatibility_flag[j]);
    fprintf(stderr,"general_progressive_source_flag        : %x\n", ptl->general_progressive_source_flag);
    fprintf(stderr,"general_interlaced_source_flag         : %x\n", ptl->general_interlaced_source_flag);
    fprintf(stderr,"general_non_packed_constraint_flag     : %x\n", ptl->general_non_packed_constraint_flag);
    fprintf(stderr,"general_frame_only_constraint_flag     : %x\n", ptl->general_frame_only_constraint_flag);
    fprintf(stderr,"general_reserved_zero_44bits[0]        : %x\n", ptl->general_reserved_zero_44bits[0]);
    fprintf(stderr,"general_reserved_zero_44bits[1]        : %x\n", ptl->general_reserved_zero_44bits[1]);
    fprintf(stderr,"general_level_idc                      : %x\n", ptl->general_level_idc);
    for (size_t i = 0 ; i < maxNumSubLayersMinus1 ; i++) {
        fprintf(stderr, "sub_layer_profile_present_flag[%ld]   : %x\n", i, ptl->sub_layer_profile_present_flag[i]);
        fprintf(stderr, "sub_layer_level_present_flag[%ld]     : %x\n", i, ptl->sub_layer_level_present_flag[i]);
    }
#endif

    return true;
}

/**
 *
 */

bool nalEntry::copy_nal_to_file(bitBuffer& buffer, FILE* oFP) {
    uint8_t* pBuffer = buffer.m_pBufStart + (m_bit_offset / 8) - (m_long_sc?4:3);
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

/**
 *
 */

bool nalEntry::parse_nal(bitBuffer* pBuffer) {
#ifdef  _DEBUG
    fprintf(stderr, "nalEntry::parse_nal()\n");
#endif

    if ((m_nal_unit_header.nal_unit_type == NAL_EOS_NUT) || (m_nal_unit_header.nal_unit_type == NAL_EOB_NUT)) {
        return true;
    }

    if (m_nalSize < 4) {
        fprintf(stderr, "ERROR: NAL too small to parse rbsp [type %s]...\n", get_nal_type_desc(m_nal_unit_header.nal_unit_type));
        return false;
    }

    bitBuffer*      rbsp_buffer = pBuffer->get_rbsp(m_bit_offset + 16, m_nalSize - 3);
    bitBuffer::iterator bIter = rbsp_buffer->begin(); // = pBuffer->bit_iterator( m_bit_offset ) + 16;

#ifdef  _DEBUG
#ifdef  ENABLE_RBSP_SAVE
    save_rbsp_buffer(rbsp_buffer);
#endif  // ENABLE_RBSP_SAVE
#endif  // _DEBUG

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
        case NAL_RASL_N:
        case NAL_RASL_R:
            slice_segment_layer_rbsp(bIter);
            break;
        case NAL_RADL_N:
        case NAL_RADL_R:
            slice_segment_layer_rbsp(bIter);
            break;
        default:
#ifdef  _DEBUG
            fprintf(stderr, "WARNING : Unhandled NAL type [%s]!\n", get_nal_type_desc(m_nal_unit_header.nal_unit_type));
#endif
            break;
    }

    delete rbsp_buffer;

    return true;
}

#if defined(_DEBUG) && defined(ENABLE_RBSP_SAVE)

void nalEntry::save_rbsp_buffer(bitBuffer* pBuffer) {
    FILE*   oFP = 0L;
    static char fname[128];
#ifdef  _DEBUG
    fprintf(stderr, "nalEntry::save_rbsp_buffer()\n");
#endif

    snprintf(fname, 128, "buffers/rbsp_%02ld.bin", m_nal_num);
    oFP = fopen(fname, "w");

    if (oFP != 0) {
#ifdef  _DEBUG
        fprintf(stderr, "Saving to file %s\n", fname);
#endif
        fwrite(pBuffer->m_pBufStart, pBuffer->m_nBufLenBytes, 1, oFP);
        fclose(oFP);
    }
}

#endif  // ENABLE_RBSP_SAVE
