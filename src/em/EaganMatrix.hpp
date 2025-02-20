#pragma once
#include <stdint.h>
#include "wrap-HakenMidi.hpp"
#include "midi-message.h"
#include "em-hardware.h"
#include "preset.hpp"
#include "FixedStringBuffer.hpp"

namespace pachde {

struct IHandleEmEvents {
    virtual void onLoopDetect(uint8_t cc, uint8_t value) = 0;
    virtual void onEditorReply(uint8_t reply) = 0;
    virtual void onHardwareChanged(uint8_t hardware, uint16_t firmware_version) = 0;
    virtual void onPresetChanged() = 0;
    virtual void onUserComplete() = 0;
    virtual void onSystemComplete() = 0;
    virtual void onTaskMessage(uint8_t code) = 0;
    virtual void onLED(uint8_t led) = 0;
};

struct ChannelData
{
    uint8_t cc[128]{0};
    uint8_t pedal_fraction() { return cc[Haken::ccFracPed]; }
    uint8_t pedal_fraction_ex() { return cc[Haken::ccFracPedEx]; }
    void clear() { std::memset(cc, 0, 128); }
};

struct EaganMatrix {
    bool ready;
    uint16_t firmware_version;
    //uint16_t cvc_version;
    uint8_t hardware;
    //uint8_t middle_c;
    bool reverse_surface;

    bool broken_midi;
    bool in_preset;
    bool in_user;
    bool in_system;
    bool pending_EditorReply;
    bool pending_config;
    bool frac_hi;
    uint8_t frac_lsb;
    int data_stream;
    
    ChannelData ch1;
    ChannelData ch2;
    ChannelData ch16;
    uint16_t macro[90];

    float get_macro_voltage(int id);

    FixedStringBuffer<32> name_buffer;
    FixedStringBuffer<256> text_buffer;
    PresetDescription preset;

    void reset();

    uint8_t get_led() { return ch16.cc[Haken::ccEdState] & Haken::sLedBits; }
    uint8_t get_actuation() { return ch16.cc[Haken::ccActuation]; }
    uint8_t get_traditional_polyphony() { return ch16.cc[Haken::ccPolyTrad]; }
    uint8_t get_dsp_polyphony() { return ch16.cc[Haken::ccPolyDsp]; }
    uint8_t get_cvc_polyphony() { return ch16.cc[Haken::ccPolyCvc]; }
    uint8_t get_tuning() { return ch16.cc[Haken::ccGrid]; }

    uint8_t get_octave_shift() { return ch1.cc[Haken::ccOctShift]; }
    uint8_t get_mono_switch() { return ch1.cc[Haken::ccMonoSwitch]; }
    uint8_t get_fine_tune() { return ch1.cc[Haken::ccFineTune]; }

    uint8_t get_pre() { return ch1.cc[Haken::ccPre]; } // 14-bit?
    uint8_t get_post() { return ch1.cc[Haken::ccPost]; } // 14-bit?
    uint8_t get_attenuation() { return ch1.cc[Haken::ccAtten]; }

    uint8_t get_round_rate() { return ch1.cc[Haken::ccRoundRate]; }
    uint8_t get_round_initial() { return ch1.cc[Haken::ccRndIni]; }

    uint8_t get_sustain() { return ch1.cc[Haken::ccSus]; }
    uint8_t get_sos() { return ch1.cc[Haken::ccSos]; }
    uint8_t get_sos2() { return ch1.cc[Haken::ccSos2]; }
    uint8_t get_stretch() { return ch1.cc[Haken::ccStretch]; }

    uint8_t get_headphone_level() { return ch1.cc[Haken::ccHpLevel]; }
    uint8_t get_line_level() { return ch1.cc[Haken::ccLineLevel]; }

    uint8_t get_pedal_1_min() { return ch1.cc[Haken::ccMin1]; }
    uint8_t get_pedal_1_max() { return ch1.cc[Haken::ccMax1]; }
    uint8_t get_pedal_2_min() { return ch1.cc[Haken::ccMin2]; }
    uint8_t get_pedal_2_max() { return ch1.cc[Haken::ccMax2]; }

    uint8_t get_eq_title() { return ch1.cc[Haken::ccEqTilt]; }
    uint8_t get_eq_freq() { return ch1.cc[Haken::ccEqFreq]; }
    uint8_t get_eq_mix() { return ch1.cc[Haken::ccEqMix]; }
    uint8_t get_thresh_drive() { return ch1.cc[Haken::ccThrDrv]; }
    uint8_t get_attack() { return ch1.cc[Haken::ccAtkCut]; }
    uint8_t get_ratio_makeup() { return ch1.cc[Haken::ccRatMkp]; }
    uint8_t get_cotan_mix() { return ch1.cc[Haken::ccCoThMix]; }

    uint8_t get_r1() { return ch1.cc[Haken::ccReci1]; }
    uint8_t get_r2() { return ch1.cc[Haken::ccReci2]; }
    uint8_t get_r3() { return ch1.cc[Haken::ccReci3]; }
    uint8_t get_r4() { return ch1.cc[Haken::ccReci4]; }
    uint8_t get_r5() { return ch1.cc[Haken::ccReci5]; }
    uint8_t get_r6() { return ch1.cc[Haken::ccReci6]; }
    uint8_t get_r_mix() { return ch1.cc[Haken::ccReciMix]; }

    //uint8_t get_xx() { return ch1.cc[Haken::]; }

    std::vector<IHandleEmEvents*> clients;
    void clearClients() { clients.clear(); }
    void subscribeEMEvents(IHandleEmEvents* client);
    void unsubscribeEMEvents(IHandleEmEvents* client);

    void notifyLoopDetect(uint8_t cc, uint8_t value);
    void notifyEditorReply(uint8_t editor_reply);
    void notifyHardwareChanged();
    void notifyPresetChanged();
    void notifyUserComplete();
    void notifySystemComplete();
    void notifyTaskMessage(uint8_t code);
    void notifyLED(uint8_t led);

    EaganMatrix();

    void begin_preset();
    void clear_preset(bool notify);

    bool handle_macro_cc(uint8_t cc, uint8_t value);
    void onChannelOneCC(uint8_t cc, uint8_t value);
    void onChannelTwoCC(uint8_t cc, uint8_t value);
    void onChannel16CC(uint8_t cc, uint8_t value);
    void onMessage(PackedMidiMessage msg);

};

}