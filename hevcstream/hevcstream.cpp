#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include "hevcstream.h"
#include "bitbuffer.h"
#include "nalentry.h"

hevcstream::hevcstream()
:   m_bitBuffer(0L),
    m_picture_count(-1),
    m_nal_count(0L)
{
    //ctor
    clear_parameter_set_arrays();
}

hevcstream::~hevcstream()
{
    //dtor
#ifdef  _DEBUG
    fprintf(stderr, "hevcstream::~hevcstream()\n");
#endif
    Close();
}

size_t hevcstream::picture_count() const {
    return m_picture_count;
}

size_t hevcstream::nal_count() const {
    return m_nal_count;
}

/**
 *
 */

void hevcstream::clear_parameter_set_arrays() {
#ifdef  _DEBUG
    fprintf(stderr, "hevcstream::clear_parameter_set_arrays()\n");
#endif

    memset(&m_vps, 0, sizeof(PARM_SET_ARRAY));
    memset(&m_sps, 0, sizeof(PARM_SET_ARRAY));
    memset(&m_pps, 0, sizeof(PARM_SET_ARRAY));

    return;
}

/**
 *  Return true if the hevc stream is open.
 */

 bool hevcstream::IsOpen() {
    return (m_bitBuffer != 0)?true:false;
 }

/**
 *
 */

void hevcstream::byte_stream_nal_unit() {
    bool bLongSC = false;

#ifdef  _DEBUG
    fprintf(stderr, "byte_stream_nal_unit() bitpos %ld (0x%lx)\n", m_bIter.pos(), m_bIter.pos()/8);
#endif

    while ((m_bIter.get_bits(24, false) != 0x000001) && (m_bIter.get_bits(32,false) != 0x00000001)) {
        uint32_t leading_zero_bit = m_bIter.get_bits(8);
#ifdef  _DEBUG
        fprintf(stderr, "leading_zero_bit %x\n", leading_zero_bit);
#endif
    }

    if (m_bIter.get_bits(24, false) != 0x000001) {
        uint32_t zero_byte = m_bIter.get_bits(8);
#ifdef  _DEBUG
        fprintf(stderr, "zero_byte %x\n", zero_byte);
#endif
        bLongSC = true;
    }

#ifdef _DEBUG
    fprintf(stderr, "24bit startcode starts at bitpos %ld (%lx)\n", m_bIter.pos(), m_bIter.pos()/8);
#endif

    uint32_t start_code_prefix_one_3bytes = m_bIter.get_bits(24);

#ifdef  _DEBUG
    fprintf(stderr, "start_code_prefix_one_3bytes %06x\n", start_code_prefix_one_3bytes);
#endif

    //printf("STARTCODE @ bit %ld Byte 0x%lx!\n", m_bIter.pos(), m_bIter.pos()/8);
    NAL_ENTRY*      nal = 0L;

    nal = new nalEntry(m_bIter, bLongSC, m_nal_count++);
    assert(nal != 0L);

    m_nalVec.push_back( nal );

try {
    while ((m_bIter.get_bits(24, false) != 0x000001) && (m_bIter.get_bits(32,false) != 0x00000001)) {
        uint32_t trailing_zero_bit = m_bIter.get_bits(8);
    }
}
catch (bitBuffer::out_of_range& exception) {
    m_bIter += 24; // Move past the end
}
    return;
}

/**
 *  Perform parsing of all NAL units.
 */

void hevcstream::parse_bitstream() {
#ifdef  _DEBUG
    fprintf(stderr, "hevcstream::parse_bitstream()\n");
#endif

    assert(m_bitBuffer != 0L);

try {
    try {
        m_bIter = m_bitBuffer->begin();

    #ifdef  _DEBUG
        fprintf(stderr, "-- SCANNING STARTCODES --\n");
    #endif

        while (m_bIter != m_bitBuffer->end()) {
            //fprintf(stderr, "bitpos %ld 0x%lx\n", bIter.pos(), bIter.pos()/8);
            byte_stream_nal_unit( );
        }
    }
    catch (std::exception& except) {
        //fprintf(stderr, "ERROR: Caught exception [%s] during scanning!\n", except.what());
    }

    calculate_nal_sizes();

#ifdef  _DEBUG
    fprintf(stderr, "-- PARSING NAL UNITS --\n");
#endif

    for (size_t x = 0 ; x < m_nalVec.size() ; x++) {
        m_nalVec[x]->parse_nal(m_bitBuffer);

        /* advance picture count if first frame in slice */
        if (m_nalVec[x]->isFirstFrameInSlice()) {
            m_picture_count++;
        }
        m_nalVec[x]->set_picture_number( ((int)m_picture_count > 0)?m_picture_count:0 );

        eNalType nalType = m_nalVec[x]->nal_type();

        if (nalType == NAL_VPS_NUT) {
            VIDEO_PARAMETER_SET*    vps     = (VIDEO_PARAMETER_SET*)m_nalVec[x]->info();
            int                     vps_id  = vps->vps_video_parameter_set_id;

            m_vps.psa[vps_id] = m_nalVec[x];
        } else if (nalType == NAL_SPS_NUT) {
            SEQ_PARAMETER_SET*      sps     = (SEQ_PARAMETER_SET*)m_nalVec[x]->info();
            int                     sps_id  = sps->sps_seq_parameter_set_id;

            m_sps.psa[sps_id] = m_nalVec[x];
        } else if (nalType == NAL_PPS_NUT) {
            PIC_PARAMETER_SET*      pps     = (PIC_PARAMETER_SET*)m_nalVec[x]->info();
            int                     pps_id  = pps->pps_pic_parameter_set_id;

            m_pps.psa[pps_id] = m_nalVec[x];
        }
    }
}
catch (bitBuffer::system_exception& except) {
    fprintf(stderr, "%s\n", except.m_desc.c_str());
    fprintf(stderr, "%s\n", except.strerror());
}
catch (std::exception& except) {
    fprintf(stderr, "ERROR: Caught exception [%s] during parsing!\n", except.what());
}
    return;
}

/**
 *  Open an HEVC stream.
 */

bool hevcstream::Open(std::string sInputFilename) {
    bool bRes = false;
    struct stat statbuf;

#ifdef  _DEBUG
    fprintf(stderr, "hevcstream::Open(%s)\n", sInputFilename.c_str());
#endif

    /* If hevc object already opened, close it... */
    if (m_bitBuffer != 0) {
        Close();
    }

    /* Make sure the file exists... */
    if (stat(sInputFilename.c_str(), &statbuf) == 0) {
        m_stream_path = sInputFilename;
        m_bitBuffer = new bitBuffer( m_stream_path );
        assert(m_bitBuffer != 0L);
        m_lastbit = m_bitBuffer->bits();
#ifdef  _DEBUG
        fprintf(stderr, "m_lastbit = %ld (Byte offset %08lx)\n", m_lastbit, m_lastbit/8);
#endif
        parse_bitstream();
        bRes = true;
    } else {
        fprintf(stderr, "ERROR: File does not exist!\n");
    }

    return bRes;
}

/**
 *  Close the hevc stream and release all resources.
 */

void hevcstream::Close() {
#ifdef  _DEBUG
    fprintf(stderr, "hevcstream::Close()\n");
#endif

    if (m_bitBuffer != 0) {
        delete m_bitBuffer;
        m_bitBuffer = 0L;
    }

    for (size_t x = 0 ; x < m_nalVec.size() ; x++) {
        delete m_nalVec[x];
    }
    m_nalVec.clear();

    m_picture_count = -1;
    m_nal_count     = 0;

    clear_parameter_set_arrays();

    return;
}

/**
 *  Dump the NAL info to a stream.
 */

void hevcstream::dump_nal_vector(FILE* oFP, int type) {
#ifdef  _DEBUG
    fprintf(stderr, "hevcstream::dump_nal_vector()\n");
#endif

    if (m_nalVec.size() > 0) {
        fprintf(oFP, "Input Stream  : %s\n", m_stream_path.c_str());
        fprintf(oFP, "NAL Count     : %ld\n", m_nalVec.size());
        fprintf(oFP, "Picture Count : %ld\n", m_picture_count + 1);

        if ((type & nalEntry::DUMP_SHORT) != 0) {
//"  %-6d %-6d %-12s (0x%02x) %-8ld  %-8ld %-5s %s\n",
            fprintf(oFP, "  %-6s %-6s %-16s %-6s %-8s %-8s %-8s %-5s %s\n",
                    "NAL#", "PIC#", "TYPE", "TYP", "BITPOS", "BYTE","SIZE", "VCL", "FIRST");
        }
        NALENTRY_PTR_VECTOR_ITER    nIter;
        uint32_t                    index = 0;
        for (nIter = m_nalVec.begin() ; nIter != m_nalVec.end() ; nIter++, index++) {
            (*nIter)->display(oFP, type & 0x1);
        }
    }

#if 0
    if ((type & nalEntry::DUMP_EXTRA) != 0) {
        for (size_t i = 0 ; i < MAX_PARMSET_ID ; i++) {
            if (m_vps.psa[i] != 0) {
                fprintf(oFP, "vps[%2ld] = %p\n", i, m_vps.psa[i]);
                m_vps.psa[i]->display(oFP, nalEntry::DUMP_LONG);
            }
            if (m_sps.psa[i] != 0) {
                fprintf(oFP, "sps[%2ld] = %p\n", i, m_sps.psa[i]);
                m_sps.psa[i]->display(oFP, nalEntry::DUMP_LONG);
            }
            if (m_pps.psa[i] != 0) {
                fprintf(oFP, "pps[%2ld] = %p\n", i, m_pps.psa[i]);
                m_pps.psa[i]->display(oFP, nalEntry::DUMP_LONG);
            }
        }
    }
#endif

    return;
}

/**
 *  Calculate the NAL size.
 */

void hevcstream::calculate_nal_sizes() {
    uint64_t    bitDiff;
#ifdef  _DEBUG
    fprintf(stderr, "-- CALCULATING NAL SIZES...\n");
#endif

    /* Calculate NAL sizes */
    for (size_t x = 1 ; x < m_nalVec.size() ; x++) {
        bitDiff = m_nalVec[x]->offset() - m_nalVec[x-1]->offset();
        m_nalVec[x-1]->set_size(bitDiff / 8 + (m_nalVec[x]->m_long_sc?0:1));
    }
//    fprintf(stderr, "-- calculate final nal --\n");
//    fprintf(stderr, " m_lastbit = %ld\n", m_lastbit);
//    fprintf(stderr, " last nal offset = %ld\n", m_nalVec[m_nalVec.size() - 1]->offset());
    m_nalVec[m_nalVec.size() - 1]->set_size((m_lastbit - m_nalVec[m_nalVec.size() - 1]->offset()) / 8);

    return;
}

/**
 *
 */

bool hevcstream::save_parm_set(FILE* oFP) {
    bool bRes = false;

    if (IsOpen()) {
        for (size_t i = 0 ; i < 32 ; i++) {
            if (m_vps.psa[i] != 0) {
                m_vps.psa[i]->copy_nal_to_file(*m_bitBuffer, oFP);
            }
            if (m_sps.psa[i] != 0) {
                m_sps.psa[i]->copy_nal_to_file(*m_bitBuffer, oFP);
            }
        }
    }
    return bRes;
}

bool hevcstream::save_nal_to_file(size_t nalindex, FILE* oFP) {

    if ((nalindex >= 0) && (nalindex <= nal_count())) {
        m_nalVec[nalindex]->copy_nal_to_file(*m_bitBuffer, oFP);
    }
    return false;
}

bool hevcstream::get_ps_to_frame(size_t frame,
                                 PARM_SET_ARRAY& vpsAr,
                                 PARM_SET_ARRAY& spsAr,
                                 PARM_SET_ARRAY& ppsAr)
{
#ifdef  _DEBUG
    fprintf(stderr, "hevcstream::get_ps_to_frame(%ld, ...)\n", frame);
#endif

    assert(frame <= picture_count());

    memset(&vpsAr, 0, sizeof(PARM_SET_ARRAY));
    memset(&spsAr, 0, sizeof(PARM_SET_ARRAY));
    memset(&ppsAr, 0, sizeof(PARM_SET_ARRAY));

    for (size_t x = 0 ; ((x < m_nalVec.size()) && (m_nalVec[x]->nal_picture_num() <= frame)) ; x++) {
        nalEntry*   pNal = m_nalVec[x];
        assert(pNal != 0L);
        eNalType    nalType = pNal->nal_type();

        if (nalType == NAL_VPS_NUT) {
            VIDEO_PARAMETER_SET*    vps     = (VIDEO_PARAMETER_SET*)pNal->info();
            int                     vps_id  = vps->vps_video_parameter_set_id;

            vpsAr.psa[vps_id] = pNal;
        } else if (nalType == NAL_SPS_NUT) {
            SEQ_PARAMETER_SET*      sps     = (SEQ_PARAMETER_SET*)pNal->info();
            int                     sps_id  = sps->sps_seq_parameter_set_id;

            spsAr.psa[sps_id] = pNal;
        } else if (nalType == NAL_PPS_NUT) {
            PIC_PARAMETER_SET*      pps     = (PIC_PARAMETER_SET*)pNal->info();
            int                     pps_id  = pps->pps_pic_parameter_set_id;

            ppsAr.psa[pps_id] = pNal;
        }
    }

    return true;
}


/**
 *
 */

bool hevcstream::get_nals_to_frame(size_t framenum, NALENTRY_PTR_VECTOR& nalVec) {
    nalEntry            *pFirstIndex = 0L,
                        *pThisIndex = 0L;
    NALENTRY_PTR_VECTOR frameVec;
    eNalType        type;

#ifdef  _DEBUG
    fprintf(stderr, "hevcstream::get_nals_to_frame(%ld, ...)\n", framenum);
#endif

    for (size_t x = 0 ; x < m_nalVec.size() ; x++) {
        if (m_nalVec[x]->isFirstFrameInSlice()) {
            frameVec.push_back( m_nalVec[x] );
        }
    }

#ifdef  _DEBUG
    fprintf(stderr, "FRAME VECTOR:\n");
    for (size_t x = 0 ; x < frameVec.size() ; x++) {
        fprintf(stderr, "FRAME %ld starts at NAL %ld\n", x, frameVec[x]->nal_index_num());
    }
#endif

    pThisIndex = frameVec[framenum];

    for (size_t frame = framenum ; frame >= 0 ; frame--) {
        pFirstIndex = frameVec[frame];
        type        = pFirstIndex->nal_type();

        if ((type >= NAL_BLA_W_LP) && (type <= NAL_CRA_NUT)) {
            break;
        }
    }

#ifdef  _DEBUG
    fprintf(stderr, "Starting frame : Index %ld Frame %ld\n",
            pFirstIndex->nal_index_num(), pFirstIndex->nal_picture_num());
#endif

    if (pFirstIndex != pThisIndex) {
        for (size_t x = pFirstIndex->nal_index_num() ; x < pThisIndex->nal_index_num() ; x++) {
#ifdef  _DEBUG
            fprintf(stderr, "Copy NAL # %ld\n", x);
#endif
            if (m_nalVec[x]->isVCL())
                nalVec.push_back( m_nalVec[x] );
        }
    }
    for (size_t x = pThisIndex->nal_index_num() ; ((x < m_nalVec.size()) && (m_nalVec[x]->nal_picture_num() <= (framenum + 1))) ; x++) {
#ifdef  _DEBUG
        fprintf(stderr, "Copy NAL # %ld\n", x);
#endif
        if (m_nalVec[x]->isVCL())
            nalVec.push_back( m_nalVec[x] );
    }
    return true;
}
