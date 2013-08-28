#ifndef __BITBUFFER_H__
#define __BITBUFFER_H__

#include <inttypes.h>
#include <string>
#include <cmath>
#include <exception>

class bitBuffer {
public:
    class out_of_range : public std::exception {
    public:
        const char* what() const throw() {
            return "out_of_range";
        }
    };

    class invalid_parameter: public std::exception {
    public:
        const char* whote() const throw() {
            return "invalid_parameter";
        }
    };

    class iterator {
    public:
        iterator() {}
        iterator(const iterator& copy) {}
        virtual ~iterator() {}
    };
    typedef enum _outputFormat {
        FORMAT_BINARY,
        FORMAT_HEX,
    } OUT_FMT;
    bitBuffer(std::string sFilename);
    bitBuffer(uint32_t bufLenBytes);
    bitBuffer(uint8_t* bufStart, uint32_t bufLenBytes);
    bitBuffer(const bitBuffer& copy);
    virtual ~bitBuffer();

    bool            is_bit_set(uint32_t bitpos);
    void            set_bit(uint32_t bitpos);
    void            set_bits(uint32_t bitStart, uint32_t bitEnd);
    void            clear_bit(uint32_t bitpos);
    void            clear_bits(uint32_t bitStart, uint32_t bitEnd);
    void            output_bits(FILE* oFp, OUT_FMT fmt = FORMAT_BINARY, uint32_t bitStart = 0, uint32_t bitEnd = 0);

private:
    uint8_t*        m_pBufStart;
    uint32_t        m_nBufLenBytes;
    uint32_t        m_nBufLenBits;
};

#define BITPOS_TO_BYTE_INDEX(x)     ((x/8))
#define BITPOS_TO_BIT_MASK(x)       (1 << (7 - (x%8)))


#endif // __BITBUFFER_H__

