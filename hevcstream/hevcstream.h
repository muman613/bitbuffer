#ifndef __HEVCSTREAM_H__
#define __HEVCSTREAM_H__

#include "bitbuffer.h"
#include "nalentry.h"

class hevcstream
{
    public:
        /** Default constructor */
        hevcstream();
        /** Default destructor */
        virtual ~hevcstream();

        bool                    Open(std::string sFilename);
        void                    Close();
        bool                    IsOpen();

//#ifdef  _DEBUG
        void                    dump_nal_vector(FILE* oFP, nalEntry::DUMP_TYPE type = nalEntry::DUMP_SHORT);
//#endif

    protected:
        void                    parse_bitstream();
        void                    byte_stream_nal_unit();

    private:
        void                    calculate_nal_sizes();

        std::string             m_stream_path;
        bitBuffer*              m_bitBuffer;
        bitBuffer::iterator     m_bIter;
        size_t                  m_picture_count;
        size_t                  m_nal_count;
        NALENTRY_PTR_VECTOR     m_nalVec;
        size_t                  m_lastbit;
};

#endif // __HEVCSTREAM_H__
