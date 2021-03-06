/**
 *
 */

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string>
#include "bitbuffer.h"
#include "naltypes.h"
#include "nalentry.h"

using namespace std;

string      sInputFilename;


NALENTRY_VECTOR     nalVec;
uint32_t            picture_count = 0;

bool parse_args(int argc, char* argv[]) {
    int     opt;
    bool    bRes = false;

    while ((opt = getopt(argc, argv, "i:")) != -1) {
        switch (opt) {
        case 'i':
            sInputFilename = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s [-i filename]\n", argv[0]);
            break;
        }
    }

    bRes = !sInputFilename.empty();

    return bRes;
}

/**
 *
 */

void byte_stream_nal_unit(bitBuffer::iterator& iter) {
#ifdef  _DEBUG
    fprintf(stderr, "byte_stream_nal_unit() bitpos %ld (0x%lx)\n", iter.pos(), iter.pos()/8);
#endif

    while ((iter.get_bits(24, false) != 0x000001) && (iter.get_bits(32,false) != 0x00000001)) {
        uint32_t leading_zero_bit = iter.get_bits(8);
#ifdef  _DEBUG
        fprintf(stderr, "leading_zero_bit %x\n", leading_zero_bit);
#endif
    }

    if (iter.get_bits(24, false) != 0x000001) {
        uint32_t zero_byte = iter.get_bits(8);
#ifdef  _DEBUG
        fprintf(stderr, "zero_byte %x\n", zero_byte);
#endif
    }

#ifdef _DEBUG
    fprintf(stderr, "24bit startcode starts at bitpos %ld (%lx)\n", iter.pos(), iter.pos()/8);
#endif

    uint32_t start_code_prefix_one_3bytes = iter.get_bits(24);

#ifdef  _DEBUG
    fprintf(stderr, "start_code_prefix_one_3bytes %06x\n", start_code_prefix_one_3bytes);
#endif

    //printf("STARTCODE @ bit %ld Byte 0x%lx!\n", iter.pos(), iter.pos()/8);
    NAL_ENTRY       nal(iter);

    nal.set_picture_number( picture_count );

    if (nal.isFirstFrameInSlice()) {
        picture_count++;
    }

    nalVec.push_back(nal);
try {
    while ((iter.get_bits(24, false) != 0x000001) && (iter.get_bits(32,false) != 0x00000001)) {
        uint32_t trailing_zero_bit = iter.get_bits(8);
    }
}
catch (bitBuffer::out_of_range& exception) {
    iter += 24; // Move past the end
}
    return;
}


int main(int argc, char* argv[]) {
    uint32_t    lastbit = 0;
    bitBuffer*  bitstream = 0L;

    if (!parse_args(argc, argv)) {
        return -10;
    }
    printf("Scanning file %s:\n", sInputFilename.c_str());

try {
//    bitBuffer               bitstream( sInputFilename );
    bitstream = new bitBuffer( sInputFilename );
    bitBuffer::iterator     bIter;

    printf("File size : %d Bytes / %d Bits\n", bitstream->size(), bitstream->bits());
    lastbit = bitstream->bits();

    bIter = bitstream->begin();

    while (bIter != bitstream->end()) {
        //fprintf(stderr, "bitpos %ld 0x%lx\n", bIter.pos(), bIter.pos()/8);
        byte_stream_nal_unit( bIter );
    }

    for (size_t x = 1 ; x < nalVec.size() ; x++) {
        uint64_t    bitDiff = nalVec[x].offset() - nalVec[x-1].offset();
        nalVec[x-1].set_size(bitDiff / 8);
    }
    nalVec[nalVec.size() - 1].set_size((lastbit - nalVec[nalVec.size() - 2].offset()) / 8);
}
catch (bitBuffer::system_exception& except) {
    fprintf(stderr, "%s\n", except.m_desc.c_str());
    fprintf(stderr, "%s\n", except.strerror());
}
catch (std::exception& except) {

}

    printf("Picture Count : %d\n", picture_count);

    if (nalVec.size() > 0) {
        NALENTRY_VECTOR_ITER    nIter;
        uint32_t                index = 0;
        for (nIter = nalVec.begin() ; nIter != nalVec.end() ; nIter++, index++) {
            fprintf(stdout, "NAL # %d:\n", index);
            (*nIter).display(stdout);
        }
    }
    FILE* oFP;
    oFP = fopen("/tmp/data.bin", "w");
    nalVec[4].copy_nal_to_file(*bitstream, oFP);
    fclose(oFP);

    delete bitstream;
    return 0;
}
