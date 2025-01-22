//#include <assert.h>
#include "midi-message.h"


PackedMidiMessage MakeRawBase(uint8_t status_byte, uint8_t value1, uint8_t value2)
{
    PackedMidiMessage m = {0};
    m.bytes.status_byte = status_byte;
    m.bytes.data1 = value1;
    m.bytes.data2 = value2;
    return m;
}

PackedMidiMessage MakeRaw(uint8_t status, uint8_t channel, uint8_t value1, uint8_t value2)
{
    //assert(0 == (status & CHANNEL_MASK));
    //assert(channel < 16);
    //assert(!hiBit(value1));
    //assert(!hiBit(value2));
    PackedMidiMessage m = {0};
    m.bytes.status_byte = status | channel;
    m.bytes.data1 = value1;
    m.bytes.data2 = value2;
    return m;
}

