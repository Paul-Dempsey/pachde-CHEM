// Copyright (C) Paul Chase Dempsey
#pragma once
#include "midi-io.hpp"

namespace pachde {

enum class HakenMidiRate : uint8_t {
    Full = Haken::midiTxFull,
    Third = Haken::midiTxThird,
    Twentieth = Haken::midiTxTweenth
};

struct HakenMidi
{
    MidiLog* log;
    IDoMidi* doer;
    bool tick_tock;

    HakenMidi(const HakenMidi&) = delete; // no copy constructor
    HakenMidi() : log(nullptr), doer(nullptr), tick_tock(true) {}

    void init(MidiLog* log, IDoMidi* handler)
    {
        this->log = log;
        doer = handler;
    }

    void send_message(PackedMidiMessage msg) { doer->doMessage(msg); }

    void control_change(uint8_t channel, uint8_t cc, uint8_t value);
    void key_pressure(uint8_t channel, uint8_t note, uint8_t pressure);

    void begin_stream(uint8_t stream);
    void stream_data(uint8_t d1, uint8_t d2);
    void end_stream();

    void select_preset(PresetId id);
    void editor_present();
    void request_configuration();
    void request_con_text();
    void request_updates();
    void request_user();
    void request_system();
    void midi_rate(HakenMidiRate rate);
    void remake_mahling();
    void previous_system_preset();
    void next_system_preset();
    void reset_calibration();
    void refine_calibration();
    void factory_calibration();
    void surface_alignment();

    void disable_recirculator(bool disable);
    void compressor_option(bool tanh);
    void keep_pedals(bool keep);
    void keep_midi(bool keep);
    void keep_surface(bool keep);

};

    
}