#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "naltypes.h"

static const char* nal_type_desc[] = {
    "TRAIL_N",              // type 0
    "TRAIL_R",              // type 1
    "TSA_N",                // type 2
    "TSA_R",                // type 3
    "STSA_N",               // type 4
    "STSA_R",               // type 5
    "RADL_N",               // type 6
    "RADL_R",               // type 7
    "RASL_N",               // type 8
    "RASL_R",               // type 9
    "RSV_VCL_N10",          // type 10
    "RSV_VCL_R11",          // type 11
    "RSV_VCL_N12",          // type 12
    "RSV_VCL_R13",          // type 13
    "RSV_VCL_N14",          // type 14
    "RSV_VCL_R15",          // type 15
    "BLA_W_LP",             // type 16
    "BLA_W_RADL",           // type 17
    "BLA_N_LP",             // type 18
    "IDR_W_RADL",           // type 19
    "IDR_N_LP",             // type 20
    "CRA_NUT",              // type 21
    "RSV_IRAP_VCL22",       // type 22
    "RSV_IRAP_VCL23",       // type 23
    "RSV_VCL24",            // type 24
    "RSV_VCL25",            // type 25
    "RSV_VCL26",            // type 26
    "RSV_VCL27",            // type 27
    "RSV_VCL28",            // type 28
    "RSV_VCL29",            // type 29
    "RSV_VCL30",            // type 30
    "RSV_VCL31",            // type 31
    "VPS_NUT",              // type 32
    "SPS_NUT",              // type 33
    "PPS_NUT",              // type 34
    "AUD_NUT",              // type 35
    "EOS_NUT",              // type 36
    "EOB_NUT",              // type 37
    "FD_NUT",               // type 38
    "PREFIX_SEI_NUT",       // type 39
    "SUFFIX_SEI_NUT",       // type 40
    "RSV_NVCL41",           // type 41
    "RSV_NVCL42",           // type 42
    "RSV_NVCL43",           // type 43
    "RSV_NVCL44",           // type 44
    "RSV_NVCL45",           // type 45
    "RSV_NVCL46",           // type 46
    "RSV_NVCL47",           // type 47
    "UNSPEC48",             // type 48
    "UNSPEC49",             // type 49
    "UNSPEC50",             // type 50
    "UNSPEC51",             // type 51
    "UNSPEC52",             // type 52
    "UNSPEC53",             // type 53
    "UNSPEC54",             // type 54
    "UNSPEC55",             // type 55
    "UNSPEC56",             // type 56
    "UNSPEC57",             // type 57
    "UNSPEC58",             // type 58
    "UNSPEC59",             // type 59
    "UNSPEC60",             // type 60
    "UNSPEC61",             // type 61
    "UNSPEC62",             // type 62
    "UNSPEC63",             // type 63
};

const char* get_nal_type_desc(eNalType type) {
    return nal_type_desc[(int)type];
}
