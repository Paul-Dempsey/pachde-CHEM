#pragma once
#include "midi-io.hpp"
#include "../em/wrap-haken-midi.hpp"
#include "../em/EaganMatrix.hpp"

namespace pachde {

enum class HakenMidiRate : uint8_t {
    Full = Haken::midiTxFull,
    Third = Haken::midiTxThird,
    Twentieth = Haken::midiTxTweenth
};


struct HakenMidiOutput : IDoMidi
{
    midi::Output output;
    uint64_t message_count;
    bool tick_tock;
    MidiLog* log;
    EaganMatrix* em;

    rack::dsp::RingBuffer<PackedMidiMessage, 1024> ring;
    void queueMessage(PackedMidiMessage msg);
    void dispatch(float sampleTime);
    rack::dsp::Timer midi_timer;
    
    HakenMidiOutput(const HakenMidiOutput&) = delete; // no copy constructor
    HakenMidiOutput() : message_count(0), tick_tock(true), log(nullptr), em(nullptr) {
        output.channel = -1;
    }

    midi::Output& midi_out() { return output; }

    void clear() {
        message_count = 0;
        tick_tock = true;
        ring.clear();
        midi_timer.reset();
    }
    uint64_t count() { return message_count; }

    void setLogger(MidiLog* logger) {
        log = logger;
    }
    void setEm(EaganMatrix* matrix) {
        em = matrix;
    }

    void sendControlChange(uint8_t channel, uint8_t cc, uint8_t value);
    void sendKeyPressure(uint8_t channel, uint8_t note, uint8_t pressure);
    void sendEditorPresent();
    void sendRequestConfiguration();
    void sendRequestConText();
    void sendRequestUpdates();
    void sendRequestUser();
    void sendRequestSystem();
    void sendMidiRate(HakenMidiRate rate);

    // IDoMidi
    void doMessage(PackedMidiMessage message) override;
};

}