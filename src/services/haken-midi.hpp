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
    bool osmose_target;

    HakenMidi(const HakenMidi&) = delete; // no copy constructor
    HakenMidi() : log(nullptr), doer(nullptr), tick_tock(true) {}

    void set_handler(IDoMidi* handler) { doer = handler; }
    void set_logger(MidiLog* logger) { log = logger; }

    void send_message(PackedMidiMessage msg) { doer->do_message(msg); }

    void control_change(ChemId tag, uint8_t channel, uint8_t cc, uint8_t value);
    void key_pressure(ChemId tag, uint8_t channel, uint8_t note, uint8_t pressure);

    void begin_stream(ChemId tag, uint8_t stream);
    void stream_data(ChemId tag, uint8_t d1, uint8_t d2);
    void end_stream(ChemId tag);

    void select_preset(ChemId tag, PresetId id);
    void editor_present(ChemId tag);
    void request_configuration(ChemId tag);
    void request_archive_0(ChemId tag);
    void request_con_text(ChemId tag);
    void request_updates(ChemId tag);
    void request_user(ChemId tag);
    void request_system(ChemId tag);
    void midi_rate(ChemId tag, HakenMidiRate rate);
    void remake_mahling(ChemId tag);
    void previous_system_preset(ChemId tag);
    void next_system_preset(ChemId tag);
    void reset_calibration(ChemId tag);
    void refine_calibration(ChemId tag);
    void factory_calibration(ChemId tag);
    void surface_alignment(ChemId tag);

    void disable_recirculator(ChemId tag, bool disable);
    void recirculator_type(ChemId tag, uint8_t kind);
    void compressor_option(ChemId tag, bool tanh);
    void keep_pedals(ChemId tag, bool keep);
    void keep_midi(ChemId tag, bool keep);
    void keep_surface(ChemId tag, bool keep);

};

    
}