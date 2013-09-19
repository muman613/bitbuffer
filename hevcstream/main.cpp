/**
 *  @file           main.cpp
 *  @author         Michael A. Uman
 *  @date           September 18, 2013
 *  @brief          Tool to cut HEVC encoded files.
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "hevc-cut.h"
#include "hevcstream.h"

using namespace std;

/**
 *
 */

USER_OPTIONS    user_opt = {
    "",                         // Input filename
    "",                         // Output filename
    "cut.info",                 // Report filename
    -1,                         // First picture number
    -1,                         // Last picture number
    false                       // Dump option
};

void display_usage(char* sAppName) {
    fprintf(stderr, "Usage: %s [-d] -i inputfile [-f first_picture_num] [-l last_picture_num] [-o outputfile] [-r reportfile]\n", sAppName);
    return;
}

/**
 *  Parse commandline arguments
 */

bool parse_args(int argc, char* argv[]) {
    int     opt;
    bool    bRes = false;

    while ((opt = getopt(argc, argv, "i:o:f:l:r:dh")) != -1) {
        switch (opt) {
        case 'i':
            user_opt.sInputFilename = optarg;
            break;

        case 'o':
            user_opt.sOutputFilename = optarg;
            break;

        case 'f':
            user_opt.first_picture_num = atoi( optarg );
            break;

        case 'l':
            user_opt.last_picture_num = atoi( optarg );
            break;

        case 'd':
            user_opt.bDump = true;
            break;

        case 'r':
            user_opt.sReportFilename = optarg;
            break;

        case 'h':
        default:
            display_usage(argv[0]);
            exit(-1);
        }
    }

//    bRes = !sInputFilename.empty() && !sOutputFilename.empty();

    bRes = !user_opt.sInputFilename.empty();
    if (!user_opt.sOutputFilename.empty() && (user_opt.first_picture_num == (size_t)-1)) {
        bRes = false;
    }
    if (user_opt.last_picture_num == (size_t)-1) {
        user_opt.last_picture_num = user_opt.first_picture_num + 1;
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
 *  Print a short header identifying the application.
 */

void display_header() {
    printf("hevc-cut : Version %d.%d Release %d built %s\n",
           TOOL_VERSION_MAJOR, TOOL_VERSION_MINOR, TOOL_VERSION_RELEASE,
           __TIMESTAMP__);
    return;
}

void generate_report(hevcstream& hevcObj) {
    FILE* oFP = 0L;

#ifdef  _DEBUG
    fprintf(stderr, "generate_report()\n");
#endif

    oFP = fopen(user_opt.sReportFilename.c_str(), "w");

    if (oFP) {
        hevcObj.dump_nal_vector( oFP, nalEntry::DUMP_SHORT );
        fclose(oFP);
    }

    return;
}

/**
 *
 */

int main(int argc, char* argv[]) {
    int             nRes = 0;
    hevcstream      hevcObj;

    if (!parse_args(argc, argv)) {
        fprintf(stderr, "Exiting due to invalid arguments!\n");
        return -10;
    }

    display_header();

#ifdef  _DEBUG
    fprintf(stderr, "sInputFilename  = %s\n"
                    "sOutputFilename = %s\n"
                    "picture_num     = %d\n",
                    sInputFilename.c_str(),
                    sOutputFilename.c_str(),
                    picture_num);
#endif

    if (hevcObj.Open( user_opt.sInputFilename )) {
        if (user_opt.bDump)
            hevcObj.dump_nal_vector( stdout, nalEntry::DUMP_SHORT | nalEntry::DUMP_EXTRA );

        if (!user_opt.sReportFilename.empty()) {
            generate_report( hevcObj );
        }

        /* Generate cut file */
        if (!user_opt.sOutputFilename.empty()) {
            PARM_SET_ARRAY          vpsAr, spsAr, ppsAr;
            NALENTRY_PTR_VECTOR     nalVec;
            size_t                  actual_first, actual_last;

            hevcObj.get_ps_to_frame( user_opt.first_picture_num, vpsAr, spsAr, ppsAr );
            hevcObj.get_nals_to_frame( user_opt.first_picture_num,
                                       user_opt.last_picture_num,
                                       nalVec,
                                       actual_first,
                                       actual_last );


            printf("Copying from frame %ld to frame %ld...\n", actual_first, actual_last);

#ifdef  _DEBUG
            dump_parm_set(stdout, "VIDEO PARAMETER SETS:",    vpsAr);
            dump_parm_set(stdout, "SEQUENCE PARAMETER SETS:", spsAr);
            dump_parm_set(stdout, "PICTURE PARAMETER SETS:",  ppsAr);
#endif

            FILE* oFP = fopen(user_opt.sOutputFilename.c_str(), "w");

            if (oFP != 0) {
                save_parm_set(hevcObj, oFP, vpsAr, spsAr, ppsAr);
                save_nals(hevcObj, oFP, nalVec);
                fclose(oFP);
            } else {
                /* */
                fprintf(stderr, "ERROR: Unable to open output file!\n");
                nRes = -10;
            }
        }
    }

    return nRes;
}
