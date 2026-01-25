#include "convolution.hpp"

namespace eaganmatrix {

const uint8_t default_convolution_parameters[30] = {
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

ConvolutionParams::ConvolutionParams()
{
    set_default();
}

void ConvolutionParams::set_default()
{
    std::memcpy(data, default_convolution_parameters, sizeof(data));
}

void ConvolutionParams::do_message(PackedMidiMessage msg)
{
    switch (msg.bytes.status_byte) {
    case PKP16:
        if (in_conv_stream) {
            data[stream_counter++] = msg.bytes.data1;
            data[stream_counter++] = msg.bytes.data2;
            assert(stream_counter <= 30);
        } else if (in_conv_poke) {
            assert(msg.bytes.data1 < 30);
            data[msg.bytes.data1] = msg.bytes.data2;
        }
        break;

    case Haken::ctlChg16:
        if (Haken::ccStream == midi_cc(msg)) {
            stream_counter = 0;
            auto cc_value = midi_cc_value(msg);
            switch (cc_value) {
            case Haken::s_Conv:
                in_conv_stream = true;
                break;
            case Haken::s_Conv_Poke:
                in_conv_poke = true;
                break;
            case Haken::s_StreamEnd:
            default:
                in_conv_stream = in_conv_poke = false;
                break;
            }
        }
        break;

    default:
        break;
    }
}

}