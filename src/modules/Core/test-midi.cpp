#include "test-midi.hpp"
#include "em/midi-message.h"

namespace pachde {

MidiEvent test_midi_data[] = {
{ 0.f, MakeChannelPressure(2, 79).data }, // [01004fd5] ch6  ChP 79 0
{ 0.000076f, MakeNoteOn(2, 60, 127).data }, // ch6  Note 60 127
{ 0.000087f, MakeChannelPressure(2, 94).data }, // [01005ed5] ch6  ChP 94 0
{ 0.000096f, MakeChannelPressure(2, 109).data }, // [01006dd5] ch6  ChP 109 0
{ 0.000104f, MakeChannelPressure(2, 121).data }, // [010079d5] ch6  ChP 121 0
{ 0.015555f, MakeChannelPressure(2, 127).data }, // [01007fd5] ch6  ChP 127 0
{ 0.092885f, MakeChannelPressure(2, 125).data }, // [01007dd5] ch6  ChP 125 0
{ 0.092963f, MakeChannelPressure(2, 124).data }, // [01007cd5] ch6  ChP 124 0
{ 0.092974f, MakeChannelPressure(2, 123).data }, // [01007bd5] ch6  ChP 123 0
{ 0.108855f, MakeChannelPressure(2, 110).data }, // [01006ed5] ch6  ChP 110 0
{ 0.108912f, MakeChannelPressure(2, 100).data }, // [010064d5] ch6  ChP 100 0
{ 0.108978f, MakeChannelPressure(2, 93).data }, // [01005dd5] ch6  ChP 93 0
{ 0.108989f, MakeChannelPressure(2, 88).data }, // [010058d5] ch6  ChP 88 0
{ 0.109000f, MakeChannelPressure(2, 84).data }, // [010054d5] ch6  ChP 84 0
{ 0.110906f, MakeChannelPressure(2, 81).data }, // [010051d5] ch6  ChP 81 0
{ 0.110925f, MakeChannelPressure(2, 79).data }, // [01004fd5] ch6  ChP 79 0
{ 0.110928f, MakeChannelPressure(2, 86).data }, // [010056d6] ch7  ChP 86 0
{ 0.110932f, MakePitchBend(3, 0, 64).data }, // [014000e6] ch7  PB 0 64
{ 0.124030f, MakeNoteOn(3, 62, 127).data }, // ch7  Note 62 127
{ 0.124058f, MakeChannelPressure(2, 75).data }, // [01004bd5] ch6  ChP 75 0
{ 0.124063f, MakeChannelPressure(3, 104).data }, // [010068d6] ch7  ChP 104 0
{ 0.124067f, MakeChannelPressure(2, 72).data }, // [010048d5] ch6  ChP 72 0
{ 0.124071f, MakeChannelPressure(2, 70).data }, // [010046d5] ch6  ChP 70 0
{ 0.124074f, MakeChannelPressure(3, 120).data }, // [010078d6] ch7  ChP 120 0
{ 0.124078f, MakeChannelPressure(2, 65).data }, // [010041d5] ch6  ChP 65 0
{ 0.124082f, MakeChannelPressure(2, 61).data }, // [01003dd5] ch6  ChP 61 0
{ 0.124085f, MakeChannelPressure(3, 127).data }, // [01007fd6] ch7  ChP 127 0
{ 0.124089f, MakeChannelPressure(2, 59).data }, // [01003bd5] ch6  ChP 59 0
{ 0.139388f, MakeChannelPressure(2, 56).data }, // [010038d5] ch6  ChP 56 0
{ 0.139429f, MakeChannelPressure(2, 52).data }, // [010034d5] ch6  ChP 52 0
{ 0.139435f, MakeChannelPressure(2, 49).data }, // [010031d5] ch6  ChP 49 0
{ 0.139439f, MakeChannelPressure(2, 46).data }, // [01002ed5] ch6  ChP 46 0
{ 0.139443f, MakeChannelPressure(2, 45).data }, // [01002dd5] ch6  ChP 45 0
{ 0.139447f, MakeChannelPressure(2, 43).data }, // [01002bd5] ch6  ChP 43 0
{ 0.139451f, MakeNoteOff(2, 60, 0).data }, // ch6  Note off 60 0
{ 0.139454f, MakeChannelPressure(2, 0).data }, // [010000d5] ch6  ChP 0 0
{ 0.628741f, MakePitchBend(3, 1, 64).data }, // [014001e6] ch7  PB 1 64
{ 0.739265f, MakePitchBend(3, 2, 64).data }, // [014002e6] ch7  PB 2 64
{ 0.769417f, MakePitchBend(3, 3, 64).data }, // [014003e6] ch7  PB 3 64
{ 1.033550f, MakePitchBend(3, 4, 64).data }, // [014004e6] ch7  PB 4 64
{ 1.034289f, MakePitchBend(3, 5, 64).data }, // [014005e6] ch7  PB 5 64
{ 1.048801f, MakePitchBend(3, 2, 64).data }, // [014006e6] ch7  PB 2 64
{ 1.048826f, MakePitchBend(3, 8, 64).data }, // [014008e6] ch7  PB 8 64
{ 1.048831f, MakePitchBend(3, 11, 64).data }, // [01400be6] ch7  PB 11 64
{ 1.048836f, MakeChannelPressure(3, 125).data }, // [01007dd6] ch7  ChP 125 0
{ 1.048840f, MakePitchBend(3, 13, 64).data }, // [01400de6] ch7  PB 13 64
{ 1.240948f, MakeChannelPressure(3, 123).data }, // [01007bd6] ch7  ChP 123 0
{ 1.048845f, MakePitchBend(3, 14, 64).data }, // [01400ee6] ch7  PB 14 64
{ 1.048854f, MakeChannelPressure(3, 122).data }, // [01007ad6] ch7  ChP 122 0
{ 1.080271f, MakePitchBend(3, 21, 64).data }, // [014015e6] ch7  PB 21 64
{ 1.080318f, MakePitchBend(3, 24, 64).data }, // [014018e6] ch7  PB 24 64
{ 1.080322f, MakePitchBend(3, 26, 64).data }, // [01401ae6] ch7  PB 26 64
{ 1.080325f, MakePitchBend(3, 31, 64).data }, // [01401fe6] ch7  PB 31 64
{ 1.080329f, MakePitchBend(3, 33, 64).data }, // [014021e6] ch7  PB 33 64
{ 1.080333f, MakeChannelPressure(3, 107).data }, // [01006bd6] ch7  ChP 107 0
{ 1.080336f, MakePitchBend(3, 34, 64).data }, // [014022e6] ch7  PB 34 64
{ 1.080340f, MakeChannelPressure(3, 96).data }, // [010060d6] ch7  ChP 96 0
{ 1.080343f, MakePitchBend(3, 46, 64).data }, // [01402ee6] ch7  PB 46 64
{ 1.080346f, MakeChannelPressure(3, 88).data }, // [010058d6] ch7  ChP 88 0
{ 1.080350f, MakePitchBend(3, 51, 64).data }, // [014033e6] ch7  PB 51 64
{ 1.080353f, MakeChannelPressure(3, 81).data }, // [010051d6] ch7  ChP 81 0
{ 1.080356f, MakePitchBend(3, 53, 64).data }, // [014035e6] ch7  PB 53 64
{ 1.080360f, MakeChannelPressure(3, 73).data }, // [010049d6] ch7  ChP 73 0
{ 1.080363f, MakePitchBend(3, 33, 64).data }, // [014021e6] ch7  PB 33 64
{ 1.080367f, MakeChannelPressure(3, 66).data }, // [010042d6] ch7  ChP 66 0
{ 1.080370f, MakePitchBend(3, 24, 64).data }, // [014018e6] ch7  PB 24 64
{ 1.080374f, MakeChannelPressure(3, 62).data }, // [01003ed6] ch7  ChP 62 0
{ 1.080377f, MakePitchBend(3, 14, 64).data }, // [01400ee6] ch7  PB 14 64
{ 1.080381f, MakeChannelPressure(3, 58).data }, // [01003ad6] ch7  ChP 58 0
{ 1.080384f, MakePitchBend(3, 9, 64).data }, // [014009e6] ch7  PB 9 64
{ 1.080388f, MakeChannelPressure(3, 48).data }, // [010030d6] ch7  ChP 48 0
{ 1.080391f, MakePitchBend(3, 5, 64).data }, // [014005e6] ch7  PB 5 64
{ 1.080395f, MakeChannelPressure(3, 41).data }, // [010029d6] ch7  ChP 41 0
{ 1.080398f, MakePitchBend(3, 3, 64).data }, // [014003e6] ch7  PB 3 64
{ 1.080401f, MakeNoteOff(3, 62, 0).data }, // ch7  Note off 62 0
{ 0.f, 0x010000d6 }, //  ch7  ChP 0 0
{ 0.f, 0x014002e6 }, //  ch7  PB 2 64
{ INFINITY, 0 }
};

void MidiPlayer::init(HakenMidiOutput *out, MidiEvent *data) {
    output = out;
    clip = clip_start = data;
}

void MidiPlayer::start(const rack::Module::ProcessArgs &args)
{
    PackedMidiMessage msg;
    rewind();
    while (clip->msg && clip->t <= 0.f) {
        msg.data = clip->msg;
        output->do_message(msg);
        clip++;
    }
    output->dispatch(DISPATCH_NOW);
    timer.start(4.0);
}

void MidiPlayer::process(const rack::Module::ProcessArgs &args){
    if (!clip || !output) return;
    if (clip->msg) {
        PackedMidiMessage msg;
        while (clip->msg && (clip->t <= timer.elapsed())) {
            msg.data = clip->msg;
            output->do_message(msg);
            clip++;
        }
        output->dispatch(DISPATCH_NOW);
    } else {
        timer.stop();
    }
}

}