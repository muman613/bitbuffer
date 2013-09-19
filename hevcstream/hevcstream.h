#ifndef __HEVCSTREAM_H__
#define __HEVCSTREAM_H__

#include "bitbuffer.h"
#include "nalentry.h"

#define MAX_PARMSET_ID      64

typedef struct _parameter_set_array {
    nalEntry*       psa[MAX_PARMSET_ID];
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
        void                    dump_nal_vector(FILE* oFP, int type = nalEntry::DUMP_SHORT);
//#endif

        size_t                  picture_count() const;
        size_t                  nal_count() const;

        bool                    save_parm_set(FILE* oFP);
        bool                    save_nal_to_file(size_t nalindex, FILE* oFP);

        bool                    get_ps_to_frame(size_t frame,
                                                PARM_SET_ARRAY& vps,
                                                PARM_SET_ARRAY& sps,
                                                PARM_SET_ARRAY& pps);

        bool                    get_nals_to_frame(size_t first_frame,
                                                  size_t last_frame,
                                                  NALENTRY_PTR_VECTOR& nalVec,
                                                  size_t& actual_first,
                                                  size_t& actual_last);
    protected:
        void                    parse_bitstream();
        void                    byte_stream_nal_unit();

#ifdef  _DEBUG
        void                    save_all_nals(std::string sOutDir);
#endif

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
