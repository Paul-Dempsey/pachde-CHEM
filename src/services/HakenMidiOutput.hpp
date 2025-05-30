#pragma once
#include "midi-io.hpp"
#include "../em/wrap-HakenMidi.hpp"
#include "../em/EaganMatrix.hpp"

namespace pachde {

struct HakenMidiOutput : IDoMidi
{
    midi::Output output;
    uint64_t message_count;
    MidiLog* log;

    rack::dsp::RingBuffer<PackedMidiMessage, 1024> ring;
    void queueMessage(PackedMidiMessage msg);
    void dispatch(float sampleTime);
    rack::dsp::Timer midi_timer;
    
    HakenMidiOutput(const HakenMidiOutput&) = delete; // no copy constructor
    HakenMidiOutput() : message_count(0), log(nullptr) {
        output.setChannel(-1);
    }

    midi::Output& midi_out() { return output; }

    void clear() {
        message_count = 0;
        ring.clear();
        midi_timer.reset();
    }
    uint64_t count() { return message_count; }
    bool pending() { return ring.size() > 0; }

    void set_logger(MidiLog* logger) {
        log = logger;
    }

    // IDoMidi
    void do_message(PackedMidiMessage message) override;
};

}