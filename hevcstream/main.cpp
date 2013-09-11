#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "hevcstream.h"

using namespace std;


string      sInputFilename;
int         picture_num = 0;

/**
 *
 */

bool parse_args(int argc, char* argv[]) {
    int     opt;
    bool    bRes = false;

    while ((opt = getopt(argc, argv, "i:f:")) != -1) {
        switch (opt) {
        case 'i':
            sInputFilename = optarg;
            break;

        case 'f':
            picture_num = atoi( optarg );
            break;

        default:
            fprintf(stderr, "Usage: %s [-i filename] [-f picture_num]\n", argv[0]);
            break;
        }
    }

    bRes = !sInputFilename.empty();

    return bRes;
}

/**
 *
 */

int main(int argc, char* argv[]) {
    hevcstream      hevcObj;

    if (!parse_args(argc, argv)) {
        return -10;
    }

#ifdef  _DEBUG
    fprintf(stderr, "sInputFilename = %s picture_num = %d\n", sInputFilename.c_str(), picture_num);
#endif

    if (hevcObj.Open( sInputFilename )) {
        hevcObj.dump_nal_vector( stdout, nalEntry::DUMP_SHORT );
    }
    return 0;
}
