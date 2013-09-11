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
}

hevcstream::~hevcstream()
{
    //dtor
    Close();
}

/**
 *
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
#if 1
    NAL_ENTRY*      nal = 0L;

    nal = new nalEntry(m_bIter, m_nal_count++);
    assert(nal != 0L);

    if (nal->isFirstFrameInSlice()) {
        m_picture_count++;
    }

    nal->set_picture_number( m_picture_count );

    m_nalVec.push_back( nal );
#else
    NAL_ENTRY       nal(m_bIter);

    nal.set_picture_number( m_picture_count );

    if (nal.isFirstFrameInSlice()) {
        m_picture_count++;
    }

    m_nalVec.push_back(nal);
#endif

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
 *
 */

void hevcstream::parse_bitstream() {
#ifdef  _DEBUG
    fprintf(stderr, "hevcstream::parse_bitstream()\n");
#endif

    assert(m_bitBuffer != 0L);

try {
    m_bIter = m_bitBuffer->begin();

    while (m_bIter != m_bitBuffer->end()) {
        //fprintf(stderr, "bitpos %ld 0x%lx\n", bIter.pos(), bIter.pos()/8);
        byte_stream_nal_unit( );
    }

    for (size_t x = 1 ; x < m_nalVec.size() ; x++) {
        uint64_t    bitDiff = m_nalVec[x]->offset() - m_nalVec[x-1]->offset();
        m_nalVec[x-1]->set_size(bitDiff / 8);
    }
    m_nalVec[m_nalVec.size() - 1]->set_size((m_lastbit - m_nalVec[m_nalVec.size() - 2]->offset()) / 8);
}
catch (bitBuffer::system_exception& except) {
    fprintf(stderr, "%s\n", except.m_desc.c_str());
    fprintf(stderr, "%s\n", except.strerror());
}
catch (std::exception& except) {
    fprintf(stderr, "ERROR: Caught exception!\n");
}
    return;
}

/**
 *
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
        parse_bitstream();
        bRes = true;
    } else {
        fprintf(stderr, "ERROR: File does not exist!\n");
    }

    return bRes;
}

/**
 *
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
    m_picture_count = 0;

    return;
}

//#ifdef  _DEBUG
void hevcstream::dump_nal_vector(FILE* oFP, nalEntry::DUMP_TYPE type) {
#ifdef  _DEBUG
    fprintf(stderr, "hevcstream::dump_nal_vector(%p)\n", oFP);
#endif
    if (m_nalVec.size() > 0) {

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
//#endif
