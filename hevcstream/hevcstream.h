#ifndef __HEVCSTREAM_H__
#define __HEVCSTREAM_H__

#include "bitbuffer.h"
#include "nalentry.h"


typedef struct _parameter_set_array {
    nalEntry*       pps[32];
} PARM_SET_ARRAY;

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
        void                    clear_parameter_set_arrays();
        void                    calculate_nal_sizes();

        std::string             m_stream_path;
        bitBuffer*              m_bitBuffer;
        bitBuffer::iterator     m_bIter;
        size_t                  m_picture_count;
        size_t                  m_nal_count;
        NALENTRY_PTR_VECTOR     m_nalVec;
        size_t                  m_lastbit;

        PARM_SET_ARRAY          m_vps;      ///< Video parameter set array
        PARM_SET_ARRAY          m_sps;      ///< Sequence parameter set array
        PARM_SET_ARRAY          m_pps;      ///< Picture parameter set array
};

#endif // __HEVCSTREAM_H__
