#include <stdio.h>
#include "nalentry.h"

nalEntry::nalEntry()
:   m_bit_offset(-1),
    m_nalSize(0)
{
    //ctor
}

nalEntry::nalEntry(bitBuffer::iterator& bIter) {
    //ctor
    m_bit_offset = bIter.pos();
    m_nal_unit_header.forbidden_zero_bit    = bIter.get_bits(1);
    m_nal_unit_header.nal_unit_type         = (eNalType)bIter.get_bits(6);
    m_nal_unit_header.nuh_layer_id          = bIter.get_bits(6);
    m_nal_unit_header.nuh_temporal_id_plus1 = bIter.get_bits(3);

#if 0
    printf("Found Nal @ %ld (0x%lx) Type %s Layer %d tempId %d\n",
           m_bit_offset, m_bit_offset/8,
           get_nal_type_desc(m_nal_unit_header.nal_unit_type),
           m_nal_unit_header.nuh_layer_id,
           m_nal_unit_header.nuh_temporal_id_plus1 - 1);
#endif
}

nalEntry::nalEntry(const nalEntry& copy) {
    //ctor
    m_bit_offset        = copy.m_bit_offset;
    m_nalSize           = copy.m_nalSize;
    m_nal_unit_header   = copy.m_nal_unit_header;
}

nalEntry::~nalEntry()
{
    //dtor
}

void nalEntry::display(FILE* oFP) {
    fprintf(oFP, "  NAL TYPE : %-20s (0x%04x)\n", get_nal_type_desc(m_nal_unit_header.nal_unit_type), (int)m_nal_unit_header.nal_unit_type);
    fprintf(oFP, "  NAL OFF  : Bit %ld (0x%lx)\n", m_bit_offset, m_bit_offset/8);
    fprintf(oFP, "  LAYER ID : %d\n", m_nal_unit_header.nuh_layer_id);
    fprintf(oFP, "  TEMP ID  : %d\n", m_nal_unit_header.nuh_temporal_id_plus1 - 1);
}
