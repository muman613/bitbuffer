#ifndef __BITBUFFER_H__
#define __BITBUFFER_H__

#include <inttypes.h>
#include <string.h>
#include <string>
#include <cmath>
#include <exception>
#include <errno.h>

class nalEntry;

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
        const char* what() const throw() {
            return "invalid_parameter";
        }
    };

    class system_exception: public std::exception {
    public:
        system_exception(std::string desc) : m_errno(errno),m_desc(desc) { }
        virtual ~system_exception() throw() {}
        const char* what() const throw() {
            return "system_exception";
        }
        const char* strerror() { return ::strerror(m_errno); }
        int             m_errno;
        std::string     m_desc;
    };

    /**
     *  Iterator class.
     */

    class iterator {
    public:
        iterator();
        iterator(const iterator& copy);
        virtual ~iterator();

        uint32_t    get_bits(int bitCount, bool bConsume = true);
        uint32_t    ue(bool bConsume = true);
        void        byte_align();

        iterator& operator =(const iterator& copy);
        bool operator == (const iterator& compare);
        bool operator != (const iterator& compare);
        bool operator <= (const iterator& compare);
        bool operator >= (const iterator& compare);

        int         operator *();
        iterator&   operator++();
        iterator&   operator++(int postfix);
        iterator&   operator--();
        iterator&   operator--(int postfix);
        iterator    operator- (int value);
        iterator    operator+ (int value);
        iterator&   operator+=(int value);
        iterator&   operator-=(int value);

        uint64_t    pos() const { return m_nBitPos; }

    private:
        friend class bitBuffer;
        bitBuffer*  m_pBitBuffer;
        int64_t     m_nBitPos;
    };

    typedef enum _outputFormat {
        FORMAT_BINARY,
        FORMAT_HEX,
    } OUT_FMT;
    bitBuffer(std::string sFilename);
    bitBuffer(uint32_t bufLenBytes);
    bitBuffer(uint8_t* bufStart, uint32_t bufLenBytes);
    bitBuffer(const bitBuffer& copy);
    bitBuffer();
    virtual ~bitBuffer();

    uint32_t        size() const;
    uint32_t        bits() const;

    iterator        begin() const;
    iterator        end() const;
    iterator        bit_iterator(uint32_t bitpos);

    bool            is_bit_set(uint32_t bitpos);
    void            set_bit(uint32_t bitpos);
    void            set_bits(uint32_t bitStart, uint32_t bitEnd);
    void            clear_bit(uint32_t bitpos);
    void            clear_bits(uint32_t bitStart, uint32_t bitEnd);
    void            output_bits(FILE* oFp, OUT_FMT fmt = FORMAT_BINARY, uint32_t bitStart = 0, uint32_t bitEnd = 0);

    bitBuffer       operator + (bitBuffer& operand);

private:
    friend class nalEntry;

    void            destroy();

    uint8_t*        m_pBufStart;
    uint32_t        m_nBufLenBytes;
    uint32_t        m_nBufLenBits;
    int             m_fd;
};

#define BITPOS_TO_BYTE_INDEX(x)     ((x/8))
#define BITPOS_TO_BIT_MASK(x)       (1 << (7 - (x%8)))


#endif // __BITBUFFER_H__

