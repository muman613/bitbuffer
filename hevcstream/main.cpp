#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "hevcstream.h"

using namespace std;


string      sInputFilename;
string      sOutputFilename;
size_t      picture_num = -1;
bool        bDump = false;

/**
 *
 */

bool parse_args(int argc, char* argv[]) {
    int     opt;
    bool    bRes = false;

    while ((opt = getopt(argc, argv, "i:o:f:d")) != -1) {
        switch (opt) {
        case 'i':
            sInputFilename = optarg;
            break;

        case 'o':
            sOutputFilename = optarg;
            break;

        case 'f':
            picture_num = atoi( optarg );
            break;

        case 'd':
            bDump = true;
            break;

        default:
            fprintf(stderr, "Usage: %s [-i filename] [-f picture_num] [-d]\n", argv[0]);
            break;
        }
    }

//    bRes = !sInputFilename.empty() && !sOutputFilename.empty();

    bRes = !sInputFilename.empty();
    if (!sOutputFilename.empty() && (picture_num == (size_t)-1)) {
        bRes = false;
    }

    return bRes;
}

#ifdef  _DEBUG
void dump_parm_set(FILE* oFP, const char* sMsg, PARM_SET_ARRAY& parms) {
    fprintf(oFP, "%s\n", sMsg);

    for (size_t x = 0 ; x < MAX_PARMSET_ID ; x++) {
        if (parms.psa[x] != 0L) {
            fprintf(oFP, "PARM SET ID %ld @ NAL %ld\n", x, parms.psa[x]->nal_index_num());
        }
    }
}
#endif  // _DEBUG

void save_parm_set(hevcstream& hevcObj, FILE* oFP, PARM_SET_ARRAY& vpsAr, PARM_SET_ARRAY& spsAr, PARM_SET_ARRAY& ppsAr) {
    for (size_t x = 0 ; x < MAX_PARMSET_ID ; x++) {
        nalEntry    *vps, *sps, *pps;

        vps = vpsAr.psa[x];
        sps = spsAr.psa[x];
        pps = ppsAr.psa[x];

        if (vps != 0) {
            hevcObj.save_nal_to_file(vps->nal_index_num(), oFP);
        }
        if (sps != 0) {
            hevcObj.save_nal_to_file(sps->nal_index_num(), oFP);
        }
        if (pps != 0) {
            hevcObj.save_nal_to_file(pps->nal_index_num(), oFP);
        }
    }
}

void save_nals(hevcstream& hevcObj, FILE* oFP, NALENTRY_PTR_VECTOR& nalVec) {
#ifdef  _DEBUG
    fprintf(stderr, "save_nals()\n");
#endif
    for (size_t x = 0 ;x < nalVec.size(); x++) {
        hevcObj.save_nal_to_file(nalVec[x]->nal_index_num(), oFP);
    }

    return;
}
/**
 *
 */

int main(int argc, char* argv[]) {
    hevcstream      hevcObj;

    if (!parse_args(argc, argv)) {
        fprintf(stderr, "ERROR: Must specify input [-i] and output [-o] filenames!\n");
        return -10;
    }

#ifdef  _DEBUG
    fprintf(stderr, "sInputFilename  = %s\n"
                    "sOutputFilename = %s\n"
                    "picture_num     = %d\n",
                    sInputFilename.c_str(),
                    sOutputFilename.c_str(),
                    picture_num);
#endif

    if (hevcObj.Open( sInputFilename )) {
        if (bDump)
            hevcObj.dump_nal_vector( stdout, nalEntry::DUMP_SHORT | nalEntry::DUMP_EXTRA );

        if (!sOutputFilename.empty()) {
            PARM_SET_ARRAY          vpsAr, spsAr, ppsAr;
            NALENTRY_PTR_VECTOR     nalVec;

            hevcObj.get_ps_to_frame( picture_num, vpsAr, spsAr, ppsAr );
            hevcObj.get_nals_to_frame( picture_num, nalVec );

#ifdef  _DEBUG
            dump_parm_set(stdout, "VIDEO PARAMETER SETS:",    vpsAr);
            dump_parm_set(stdout, "SEQUENCE PARAMETER SETS:", spsAr);
            dump_parm_set(stdout, "PICTURE PARAMETER SETS:",  ppsAr);
#endif

            FILE* oFP = fopen(sOutputFilename.c_str(), "w");

            if (oFP != 0) {

                save_parm_set(hevcObj, oFP, vpsAr, spsAr, ppsAr);
                save_nals(hevcObj, oFP, nalVec);

                fclose(oFP);
            } else {
                /* */
            }
        }
    }

    return 0;
}
