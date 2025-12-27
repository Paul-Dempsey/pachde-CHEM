#include "wxyz.hpp"
#include "em/wrap-HakenMidi.hpp"

void MusicMidiToCV::do_message(PackedMidiMessage msg) {
    uint8_t channel = midi_channel(msg);
    uint8_t status = midi_status(msg);

    if (Haken::ch16 == channel) return;

    switch (status) {
    case MidiStatus_NoteOn:
        nn[channel]= midi_note(msg);
        w[channel] = 1;
        break;

    case Haken::chanPres:
        z[channel] = ((uint16_t)msg.bytes.data1 << 7) + consume_frac(channel);
        break;

    case Haken::pitchWheel: {
        uint32_t bend = ((uint32_t)msg.bytes.data2 << 14) + ((uint32_t)msg.bytes.data1 << 7) + consume_frac(channel);
        this->bend[channel] = ((((double)bend - (double)Haken::extBendOffset)) * inv_extBendOffset) * em->get_bend_range();
    } break;

    case MidiStatus_CC:{
        uint8_t cc = midi_cc(msg);

        switch (cc) {
        case Haken::ccFracXYZ:
            fracXYZ[channel] = midi_cc_value(msg);
            break;

        default:
            if (cc == em->get_y_assign()) {
                y[channel] = (midi_cc_value(msg) << 7) + consume_frac(channel);
            }
            else if (cc == em->get_z_assign()) {
                z[channel] = (midi_cc_value(msg) << 7) + consume_frac(channel);
            }
            break;
        }
    } break;

    case MidiStatus_NoteOff:
        w[channel] = 0;
        if (zero_xyz) {
            nn[channel] = 0;
            bend[channel] = 0;
            y[channel] = 0;
            z[channel] = 0;
        }
        break;
    }

}