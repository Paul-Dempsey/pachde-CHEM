#pragma once
#include "midi-message.h"

namespace eaganmatrix {

struct EmBatchState: IDoMidi
{
    bool in_user{false};
    bool in_system{false};
    bool in_mahling{false};

    bool busy() {
        return in_system
            || in_user 
            || in_mahling
            ;
    }

    void do_message(PackedMidiMessage message) override
    {
        message.bytes.tag = 0;
        switch (message.data) {
        case 0x00366dbf: in_user = true; return;
        case 0x00376dbf: in_user = false; return;
        case 0x00316dbf: in_system = true; return;
        case 0x00286dbf: in_system = false; return;
        case 0x003c6dbf: in_mahling = true; return;
        case 0x00686dbf: in_mahling = false; return;
        }
    }
};

}