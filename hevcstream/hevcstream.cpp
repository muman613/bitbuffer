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
    Close();
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

    nal = new nalEntry(m_bIter, m_nal_count++);
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
    m_bIter = m_bitBuffer->begin();

#ifdef  _DEBUG
    fprintf(stderr, "-- SCANNING STARTCODES --\n");
#endif

    while (m_bIter != m_bitBuffer->end()) {
        //fprintf(stderr, "bitpos %ld 0x%lx\n", bIter.pos(), bIter.pos()/8);
        byte_stream_nal_unit( );
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

        } else if (nalType == NAL_SPS_NUT) {
            SEQ_PARAMETER_SET*      sps     = (SEQ_PARAMETER_SET*)m_nalVec[x]->info();
            int                     sps_id  = sps->sps_seq_parameter_set_id;

        } else if (nalType == NAL_PPS_NUT) {
            PIC_PARAMETER_SET*      pps     = (PIC_PARAMETER_SET*)m_nalVec[x]->info();
            int                     pps_id  = pps->pps_pic_parameter_set_id;
        }
    }
}
catch (bitBuffer::system_exception& except) {
    fprintf(stderr, "%s\n", except.m_desc.c_str());
    fprintf(stderr, "%s\n", except.strerror());
}
catch (std::exception& except) {
    fprintf(stderr, "ERROR: Caught exception [%s]!\n", except.what());
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

    if (m_bitBuffer != 0) {
        Close();
    }

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

    clear_parameter_set_arrays();

    return;
}

/**
 *  Dump the NAL info to a stream.
 */

void hevcstream::dump_nal_vector(FILE* oFP, nalEntry::DUMP_TYPE type) {
#ifdef  _DEBUG
    fprintf(stderr, "hevcstream::dump_nal_vector()\n");
#endif

    if (m_nalVec.size() > 0) {
        fprintf(oFP, "Input Stream  : %s\n", m_stream_path.c_str());
        fprintf(oFP, "NAL Count     : %ld\n", m_nalVec.size());
        fprintf(oFP, "Picture Count : %ld\n", m_picture_count + 1);

        if (type == nalEntry::DUMP_SHORT) {
//"  %-6d %-6d %-12s (0x%02x) %-8ld  %-8ld %-5s %s\n",
            fprintf(oFP, "  %-6s %-6s %-16s %-6s %-8s %-8s %-8s %-5s %s\n",
                    "NAL#", "PIC#", "TYPE", "TYP", "BITPOS", "BYTE","SIZE", "VCL", "FIRST");
        }
        NALENTRY_PTR_VECTOR_ITER    nIter;
        uint32_t                    index = 0;
        for (nIter = m_nalVec.begin() ; nIter != m_nalVec.end() ; nIter++, index++) {
            (*nIter)->display(oFP, type);
        }
    }
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
        m_nalVec[x-1]->set_size(bitDiff / 8);
    }
//    fprintf(stderr, "-- calculate final nal --\n");
//    fprintf(stderr, " m_lastbit = %ld\n", m_lastbit);
//    fprintf(stderr, " last nal offset = %ld\n", m_nalVec[m_nalVec.size() - 1]->offset());
    m_nalVec[m_nalVec.size() - 1]->set_size((m_lastbit - m_nalVec[m_nalVec.size() - 1]->offset()) / 8);

    return;
}
