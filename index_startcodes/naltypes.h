#ifndef __NALTYPES_H__
#define __NALTYPES_H__

#include <inttypes.h>

enum eNalType   {
    NAL_TRAIL_N,              // type 0
    NAL_TRAIL_R,              // type 1
    NAL_TSA_N,                // type 2
    NAL_TSA_R,                // type 3
    NAL_STSA_N,               // type 4
    NAL_STSA_R,               // type 5
    NAL_RADL_N,               // type 6
    NAL_RADL_R,               // type 7
    NAL_RASL_N,               // type 8
    NAL_RASL_R,               // type 9
    NAL_RSV_VCL_N10,          // type 10
    NAL_RSV_VCL_R11,          // type 11
    NAL_RSV_VCL_N12,          // type 12
    NAL_RSV_VCL_R13,          // type 13
    NAL_RSV_VCL_N14,          // type 14
    NAL_RSV_VCL_R15,          // type 15
    NAL_BLA_W_LP,             // type 16
    NAL_BLA_W_RADL,           // type 17
    NAL_BLA_N_LP,             // type 18
    NAL_IDR_W_RADL,           // type 19
    NAL_IDR_N_LP,             // type 20
    NAL_CRA_NUT,              // type 21
    NAL_RSV_IRAP_VCL22,       // type 22
    NAL_RSV_IRAP_VCL23,       // type 23
    NAL_RSV_VCL24,            // type 24
    NAL_RSV_VCL25,            // type 25
    NAL_RSV_VCL26,            // type 26
    NAL_RSV_VCL27,            // type 27
    NAL_RSV_VCL28,            // type 28
    NAL_RSV_VCL29,            // type 29
    NAL_RSV_VCL30,            // type 30
    NAL_RSV_VCL31,            // type 31
    NAL_VPS_NUT,              // type 32
    NAL_SPS_NUT,              // type 33
    NAL_PPS_NUT,              // type 34
    NAL_AUD_NUT,              // type 35
    NAL_EOS_NUT,              // type 36
    NAL_EOB_NUT,              // type 37
    NAL_FD_NUT,               // type 38
    NAL_PREFIX_SEI_NUT,       // type 39
    NAL_SUFFIX_SEI_NUT,       // type 40
    NAL_RSV_NVCL41,           // type 41
    NAL_RSV_NVCL42,           // type 42
    NAL_RSV_NVCL43,           // type 43
    NAL_RSV_NVCL44,           // type 44
    NAL_RSV_NVCL45,           // type 45
    NAL_RSV_NVCL46,           // type 46
    NAL_RSV_NVCL47,           // type 47
    NAL_UNSPEC48,             // type 48
    NAL_UNSPEC49,             // type 49
    NAL_UNSPEC50,             // type 50
    NAL_UNSPEC51,             // type 51
    NAL_UNSPEC52,             // type 52
    NAL_UNSPEC53,             // type 53
    NAL_UNSPEC54,             // type 54
    NAL_UNSPEC55,             // type 55
    NAL_UNSPEC56,             // type 56
    NAL_UNSPEC57,             // type 57
    NAL_UNSPEC58,             // type 58
    NAL_UNSPEC59,             // type 59
    NAL_UNSPEC60,             // type 60
    NAL_UNSPEC61,             // type 61
    NAL_UNSPEC62,             // type 62
    NAL_UNSPEC63,             // type 63
};

typedef struct nal_unit_header {
    int         forbidden_zero_bit;
    eNalType    nal_unit_type;
    int         nuh_layer_id;
    int         nuh_temporal_id_plus1;
} NALU_HDR;

extern const char* get_nal_type_desc(eNalType type);


#endif // __NALTYPES_H__
