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
    buffer.clear_bit(0);
    printf("\n");
    buffer.output_bits(stdout, bitBuffer::FORMAT_BINARY);

    bitBuffer::iterator iter;

}
catch (bitBuffer::out_of_range& except) {
    printf("Caught %s exception!\n", except.what());
}
    return 0;
}
