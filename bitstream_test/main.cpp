#include <stdio.h>
#include <iostream>
#include <string.h>
#include <inttypes.h>
#include "bitbuffer.h"

using namespace std;


int main()
{
    bitBuffer       buffer(32);

try {
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
}
catch (std::exception& except) {
    printf("Caught %s exception!\n", except.what());
}

    return 0;
}
