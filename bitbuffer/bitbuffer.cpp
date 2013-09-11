#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include "bitbuffer.h"

using namespace std;

bitBuffer::bitBuffer()
:   m_pBufStart(0L),
    m_nBufLenBytes(0L),
    m_nBufLenBits(0L)
{
    // ctor
}
/**
 *
 */

bitBuffer::bitBuffer(string sFilename)
:   m_pBufStart(0L),
    m_nBufLenBytes(0L),
    m_nBufLenBits(0L)
{
    // ctor
    m_fd = open( sFilename.c_str(), O_RDONLY );

    if (m_fd == -1) {
        throw system_exception("Error opening file for reading");
    }

    m_nBufLenBytes = lseek(m_fd, 0, SEEK_END);
    m_nBufLenBits  = m_nBufLenBytes * 8;

    m_pBufStart = (uint8_t*)mmap(0, m_nBufLenBytes, PROT_READ, MAP_SHARED, m_fd, 0);
    if (m_pBufStart == MAP_FAILED) {
        close(m_fd);
        throw system_exception("Error calling mmap");
    }

    return;
}

/**
 *  Allocate a zero-filled buffer of bufLenBytes.
 */

bitBuffer::bitBuffer(uint32_t bufLenBytes)
:   m_pBufStart(0L),
    m_nBufLenBytes(0L),
    m_fd(-1)
{
    m_pBufStart     = new uint8_t[bufLenBytes];
    memset(m_pBufStart, 0, bufLenBytes);
    m_nBufLenBytes  = bufLenBytes;
    m_nBufLenBits   = bufLenBytes * 8;

//  fprintf(stderr, "bitBuffer::bitBuffer(%d) lenBits %d\n", bufLenBytes, m_nBufLenBits);
}

/**
 *  Copy buffer from a memory pointer.
 */

bitBuffer::bitBuffer(uint8_t* bufStart, uint32_t bufLenBytes)
:   m_pBufStart(0L),
    m_nBufLenBytes(0L),
    m_nBufLenBits(0L),
    m_fd(-1)
{
    m_pBufStart     = new uint8_t[bufLenBytes];
    memcpy(m_pBufStart, bufStart, bufLenBytes);
    m_nBufLenBytes  = bufLenBytes;
    m_nBufLenBits   = bufLenBytes * 8;
}

bitBuffer::~bitBuffer() {
    // dtor
#if 1
    destroy();
#else
    if (m_fd != -1) {
        if (munmap(m_pBufStart, m_nBufLenBytes) == -1) {
            throw system_exception("Error unmapping file");
        }
        close(m_fd);
    } else {
        if (m_pBufStart != 0L) {
            delete [] m_pBufStart;
            m_pBufStart = 0L;
            m_nBufLenBytes = m_nBufLenBits = 0L;
        }
    }
#endif
}

void bitBuffer::destroy() {
    fprintf(stderr, "bitBuffer::destroy()\n");

    if (m_fd != -1) {
        if (munmap(m_pBufStart, m_nBufLenBytes) == -1) {
            throw system_exception("Error unmapping file");
        }
        close(m_fd);
        m_fd = -1;
    } else {
        if (m_pBufStart != 0L) {
            delete [] m_pBufStart;
        }
    }
    m_pBufStart = 0L;
    m_nBufLenBits = m_nBufLenBytes = 0L;

    return;
}

bool bitBuffer::is_bit_set(uint32_t bitpos) {
    if (bitpos < m_nBufLenBits) {
        uint32_t        byteIndex   = BITPOS_TO_BYTE_INDEX(bitpos);
        uint8_t         bitMask     = BITPOS_TO_BIT_MASK(bitpos);
        uint8_t         data        = m_pBufStart[byteIndex];

//      fprintf(stderr, "is_bit_set(%d) byteIndex %d bitMask %02x\n", bitpos, byteIndex, bitMask);

        return ((data & bitMask) != 0L);
    } else {
        throw out_of_range();
    }

}

uint32_t bitBuffer::size() const {
    return m_nBufLenBytes;
}

uint32_t bitBuffer::bits() const {
    return m_nBufLenBits;
}

void bitBuffer::output_bits(FILE* oFp, OUT_FMT fmt, uint32_t bitStart, uint32_t bitEnd) {
    uint32_t cnt = 0;

    if (bitEnd == 0) {
        bitEnd = m_nBufLenBits;
    }

    //fprintf(stderr, "output_bits(..., %d, %d)\n", bitStart, bitEnd);

    if (fmt == FORMAT_BINARY) {
        char ch;
        for (uint32_t index = bitStart ; index < bitEnd ; index++) {
            ch = is_bit_set(index)?'1':'0';
            fprintf(oFp, "%c", ch);
            cnt++;
            if ((cnt > 0) && ((cnt % 8) == 0)) {
                fprintf(oFp, " ");
                if ((cnt%64) == 0)
                    fprintf(oFp, "%c", '\n');
            }
        }

        if ((cnt%8)!= 0)
            fprintf(oFp, "\n");
    } else if (fmt == FORMAT_HEX) {
        for (uint32_t index = 0 ; index < m_nBufLenBytes ; index++) {
            fprintf(oFp, "%02x ", m_pBufStart[index]);
            cnt++;
            if ((cnt > 0) && ((cnt % 16) == 0)) {
                fprintf(oFp, "\n");
            }
        }

        if ((cnt%8)!= 0)
            fprintf(oFp, "\n");
    } else {
        throw invalid_parameter();
    }

    return;
}

void bitBuffer::set_bit(uint32_t bitpos) {
    if (bitpos < m_nBufLenBits) {
        uint32_t        byteIndex   = BITPOS_TO_BYTE_INDEX(bitpos);
        uint8_t         bitMask     = BITPOS_TO_BIT_MASK(bitpos);

        m_pBufStart[byteIndex] |= bitMask;
    } else {
        throw out_of_range();
    }
    return;
}

void bitBuffer::set_bits(uint32_t bitStart, uint32_t bitEnd) {
    for (uint32_t index = bitStart ; index < bitEnd ; index++) {
        set_bit(index);
    }
    return;
}

void bitBuffer::clear_bit(uint32_t bitpos) {
    if (bitpos < m_nBufLenBits) {
        uint32_t        byteIndex   = BITPOS_TO_BYTE_INDEX(bitpos);
        uint8_t         bitMask     = BITPOS_TO_BIT_MASK(bitpos);

        m_pBufStart[byteIndex] &= ~bitMask;
    } else {
        throw out_of_range();
    }
    return;
}

void bitBuffer::clear_bits(uint32_t bitStart, uint32_t bitEnd) {
    for (uint32_t index = bitStart ; index < bitEnd ; index++) {
        clear_bit(index);
    }
    return;
}


bitBuffer::iterator bitBuffer::begin() const {
    iterator    biter;

    biter.m_pBitBuffer = (bitBuffer*)this;
    biter.m_nBitPos    = 0L;

    return biter;
}

bitBuffer::iterator bitBuffer::end() const {
    iterator    biter;

    biter.m_pBitBuffer = (bitBuffer*)this;
    biter.m_nBitPos    = (m_nBufLenBits);

    return biter;
}


/*----------------------------------------------------------------------------*/

bitBuffer::iterator::iterator()
:   m_pBitBuffer(0L),
    m_nBitPos(-1)
{
    //ctor
}

/**
 *
 */

bitBuffer::iterator::iterator(const iterator& copy)
:   m_pBitBuffer(copy.m_pBitBuffer),
    m_nBitPos(copy.m_nBitPos)
{
    //ctor
}

/**
 *
 */

bitBuffer::iterator::~iterator()
{
    //dtor
}

/**
 *
 */

bitBuffer::iterator& bitBuffer::iterator::operator =(const bitBuffer::iterator& copy) {

    m_pBitBuffer = copy.m_pBitBuffer;
    m_nBitPos    = copy.m_nBitPos;

    return *this;
}

/**
 *
 */

int         bitBuffer::iterator::operator *() {

    if ((m_nBitPos < 0) || (m_nBitPos >= m_pBitBuffer->m_nBufLenBits)) {
        throw bitBuffer::out_of_range();
    }

    return (m_pBitBuffer->is_bit_set(m_nBitPos)?1:0);
}

/**
 *  Advance (prefix) bit position iterator.
 */

bitBuffer::iterator&   bitBuffer::iterator::operator++() {
    m_nBitPos++;

    if (m_nBitPos > m_pBitBuffer->m_nBufLenBits) {
        throw bitBuffer::out_of_range();
    }

    return *this;
}

/**
 *  Advance (postfix) bit position iterator.
 */

bitBuffer::iterator&   bitBuffer::iterator::operator++(int postfix) {
    m_nBitPos++;

    if (m_nBitPos > m_pBitBuffer->m_nBufLenBits) {
        throw bitBuffer::out_of_range();
    }

    return *this;
}

/**
 *  Decrement bit position iterator.
 */

bitBuffer::iterator&   bitBuffer::iterator::operator--() {
    m_nBitPos--;

//    if (m_nBitPos < 0) {
//        throw bitBuffer::out_of_range();
//    }

    return *this;
}

/**
 *  Decrement bit position iterator.
 */

bitBuffer::iterator&   bitBuffer::iterator::operator--(int postfix) {
    m_nBitPos--;

//    if (m_nBitPos < 0) {
//        throw bitBuffer::out_of_range();
//    }

    return *this;
}
/**
 *
 */

bool bitBuffer::iterator::operator == (const iterator& compare) {
    int bRes = false;
    if ((m_pBitBuffer == compare.m_pBitBuffer) && (m_nBitPos == compare.m_nBitPos)) {
        bRes = true;
    }
    return bRes;
}

/**
 *
 */

bool bitBuffer::iterator::operator != (const iterator& compare) {
    int bRes = false;
    if ((m_pBitBuffer != compare.m_pBitBuffer) || (m_nBitPos != compare.m_nBitPos)) {
        bRes = true;
    }
    return bRes;

}

bool bitBuffer::iterator::operator <= (const iterator& compare) {
    int bRes = false;
    if ((m_pBitBuffer == compare.m_pBitBuffer) && (m_nBitPos <= compare.m_nBitPos)) {
        bRes = true;
    }
    return bRes;
}

bool bitBuffer::iterator::operator >= (const iterator& compare) {
    int bRes = false;
    if ((m_pBitBuffer == compare.m_pBitBuffer) && (m_nBitPos >= compare.m_nBitPos)) {
        bRes = true;
    }
    return bRes;
}


bitBuffer::iterator   bitBuffer::iterator::operator- (int value) {
    bitBuffer::iterator iter;

    iter.m_pBitBuffer = m_pBitBuffer;
    iter.m_nBitPos    = m_nBitPos - value;

    return iter;
}

bitBuffer::iterator   bitBuffer::iterator::operator+ (int value) {
    bitBuffer::iterator iter;

    iter.m_pBitBuffer = m_pBitBuffer;
    iter.m_nBitPos    = m_nBitPos + value;

    return iter;
}

bitBuffer::iterator& bitBuffer::iterator::operator+=(int value) {
    m_nBitPos += value;
    return *this;
}

bitBuffer::iterator& bitBuffer::iterator::operator-=(int value) {
    m_nBitPos -= value;
    return *this;
}

/**
 *
 */

uint32_t bitBuffer::iterator::get_bits(int bitCount, bool bConsume) {
    uint32_t    lastBit = m_nBitPos + bitCount;
    uint32_t    bits = 0L;

    if (lastBit > m_pBitBuffer->m_nBufLenBits) {
        throw out_of_range();
    }

    for (uint32_t index = m_nBitPos ; index < lastBit ; index++) {
        uint32_t bit = m_pBitBuffer->is_bit_set(index);
        bits = (bits << 1) | bit;
    }

    if (bConsume)
        m_nBitPos += bitCount;

    return bits;
}

/**
 *
 */

uint32_t bitBuffer::iterator::ue(bool bConsume) {
    uint32_t    codeNum = 0;
    int         leadingZeroBits = -1;
    int         b;
    bitBuffer::iterator oldPos = *this;

    for (b = 0 ; (b == 0) ; leadingZeroBits++) {
        b = get_bits(1);
    }

    codeNum = ((1 << leadingZeroBits) - 1) + get_bits(leadingZeroBits);

    if (bConsume == false) {
        *this = oldPos;
    }

    return codeNum;
}

/**
 *
 */

void bitBuffer::iterator::byte_align() {
    if ((m_nBitPos % 8) != 0) {
        m_nBitPos += (8 - (m_nBitPos % 8));
    }
}

bitBuffer bitBuffer::operator+(bitBuffer& operand) {
    bitBuffer result;

    return result;
}
