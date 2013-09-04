#include <stdio.h>
#include <iostream>
#include <string.h>
#include <inttypes.h>
#include "bitbuffer.h"
#include "random.h"

using namespace std;

//#define FILE_TEST
#define MEDIA_FILE "/media/elementary/hevc/ateme/TestATEME4KforTektro_1616.265/TestATEME4KforTektro_1616.265"

void basic_tests() {
    printf("---BASIC TESTS---\n");
try {
    bitBuffer       buffer(32);
    buffer.set_bit(0);
    buffer.set_bits(2,4);
    buffer.set_bits(64, 64+9);
    buffer.set_bit(250);
    buffer.set_bit(255);
    buffer.output_bits(stdout, bitBuffer::FORMAT_HEX);
    //buffer.clear_bit(0);
    printf("\n");
    buffer.output_bits(stdout, bitBuffer::FORMAT_BINARY);

    bitBuffer::iterator iter;

    printf("Forward bits:\n");
    for (iter = buffer.begin() ; iter != buffer.end() ; iter++) {
        printf("%d", *iter);
    }
    printf("\n");
    printf("Reverse bits:\n");
    for (iter = (buffer.end() - 1) ; iter >= buffer.begin() ; iter--) {
        printf("%d", *iter);
    }
    printf("\n");

    iter = buffer.begin();

    uint32_t bits = iter.get_bits(6, true);

    printf("%0x\n", bits);
}
catch (std::exception& except) {
    printf("Caught %s exception!\n", except.what());
}
    return;
}

void file_tests() {
    printf("---FILE TESTS---\n");
try {
    bitBuffer           buffer(MEDIA_FILE);
    bitBuffer::iterator iter, start, end;

    start = buffer.begin();
    end   = start + 32 + 16;

    printf("Buffer size = %d\n", buffer.size());
    buffer.output_bits(stdout, bitBuffer::FORMAT_BINARY);
    for (iter = start ; iter != end ; iter++) {
        printf("%d", *iter);
    }
    printf("\n");
}
catch (std::exception& except) {
    printf("Caught %s exception!\n", except.what());
}
    return;
}

void byte_stream_nal_unit(bitBuffer::iterator& iter) {
    //fprintf(stderr, "byte_stream_nal_unit()\n");

    while ((iter.get_bits(24, false) != 0x000001) && (iter.get_bits(32,false) != 0x00000001)) {
        uint32_t leading_zero_bit = iter.get_bits(8);
    }
    if (iter.get_bits(24, false) != 0x000001) {
        uint32_t zero_byte = iter.get_bits(8);
    }
    uint32_t start_code_prefix_one_3bytes = iter.get_bits(24);

    printf("STARTCODE @ bit %ld Byte 0x%lx!\n", iter.pos(), iter.pos()/8);

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

void buffer_tests() {
    printf("---BUFFER TESTS---\n");
try {
//    bitBuffer           buffer(random, random_len);
    bitBuffer           buffer("mobile_qcif_b.hevc");

    //buffer.output_bits(stdout, bitBuffer::FORMAT_HEX);

    bitBuffer::iterator     bIter = buffer.begin();

#if 1
    printf("bitpos end %ld 0x%lx\n", buffer.end().pos(), buffer.end().pos()/8);

    while (bIter != buffer.end()) {
        //fprintf(stderr, "bitpos %ld 0x%lx\n", bIter.pos(), bIter.pos()/8);
        byte_stream_nal_unit( bIter );
    }
#else
    for (bIter = buffer.begin() ; bIter != buffer.end() ; ) {
        uint32_t bits = bIter.get_bits(28, false);

        if (bits == 0x00000001) {
            printf("STARTCODE @ bitpos %ld (0x%lx)!\n", bIter.pos(), (bIter.pos()/8));
            bIter += 28;
        } else {
            bIter += 1;
        }
    }
#endif
}
catch (std::exception& except) {
    printf("Exception! (%s)\n", except.what());
}
    return;
}

int main()
{

    basic_tests();

#ifdef  FILE_TEST
    file_tests();
#endif

    buffer_tests();

    return 0;
}
