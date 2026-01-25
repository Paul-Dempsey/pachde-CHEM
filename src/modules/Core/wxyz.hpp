#pragma once
#include "em/EaganMatrix.hpp"

struct MusicMidiToCV
{
    uint8_t fracXYZ[16]{0};
    uint8_t w[16]{0};
    uint8_t nn[16]{0};
    uint16_t y[16]{0};
    uint16_t z[16]{0};
    float bend[16]{0};
    bool zero_xyz{false};
    bool mpe_channels{true}; // process only ch 2-15

    void set_mpe_channels (bool mpe) {
        mpe_channels = mpe;
    }

    void silence();

    eaganmatrix::EaganMatrix * em{nullptr};

    uint8_t consume_frac(uint8_t channel) {
        uint8_t r = fracXYZ[channel];
        fracXYZ[channel] = 0;
        return r;
    }
    void do_message(PackedMidiMessage message);
};