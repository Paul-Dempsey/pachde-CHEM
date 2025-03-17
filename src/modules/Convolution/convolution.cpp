#include "convolution.hpp"

namespace pachde {

const uint8_t default_convolution_parameters[] = {
    0, 0, 0, 0,         // idx, mix, idx, mix
    Haken::cd_Wood,
    Haken::cd_MetalBright,
    Haken::cd_Fiber,
    Haken::cd_Wood,
    127,127,127,127,    // length
    64,64,64,64,        // shift (tuning)
    64,64,64,64,        // width
    127,127,127,127,    // left
    127,127,127,127,    // right
    1,                  // phase cancellation (what's the default?)
    0                   // pad
};

ConvolutionParams::ConvolutionParams() :
    in_conv_stream(false)
{
    memcpy(data, default_convolution_parameters, sizeof(data));
}

void ConvolutionParams::do_message(PackedMidiMessage msg)
{
    switch (msg.bytes.status_byte) {
    case PKP16:
        if (in_conv_stream) {
            data[msg.bytes.data1] = msg.bytes.data2;
        }
        break;

    case Haken::ccStat16:
        if (Haken::ccStream == midi_cc(msg)) {
            in_conv_stream = (Haken::s_Conv_Poke == midi_cc_value(msg));
        }
        break;

    default: break;
    }    
}

}