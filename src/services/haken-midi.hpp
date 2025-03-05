// Copyright (C) Paul Chase Dempsey
#pragma once
#include "../chem-id.hpp"
#include "../em/PresetId.hpp"
#include "../em/wrap-HakenMidi.hpp"
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

    void control_change(MidiTag tag, uint8_t channel, uint8_t cc, uint8_t value);
    void key_pressure(MidiTag tag, uint8_t channel, uint8_t note, uint8_t pressure);

    void begin_stream(MidiTag tag, uint8_t stream);
    void stream_data(MidiTag tag, uint8_t d1, uint8_t d2);
    void end_stream(MidiTag tag);

    void select_preset(MidiTag tag, PresetId id);
    void editor_present(MidiTag tag);
    void request_configuration(MidiTag tag);
    void request_con_text(MidiTag tag);
    void request_updates(MidiTag tag);
    void request_user(MidiTag tag);
    void request_system(MidiTag tag);
    void midi_rate(MidiTag tag, HakenMidiRate rate);
    void remake_mahling(MidiTag tag);
    void previous_system_preset(MidiTag tag);
    void next_system_preset(MidiTag tag);
    void reset_calibration(MidiTag tag);
    void refine_calibration(MidiTag tag);
    void factory_calibration(MidiTag tag);
    void surface_alignment(MidiTag tag);

    void disable_recirculator(MidiTag tag, bool disable);
    void compressor_option(MidiTag tag, bool tanh);
    void keep_pedals(MidiTag tag, bool keep);
    void keep_midi(MidiTag tag, bool keep);
    void keep_surface(MidiTag tag, bool keep);

};

    
}