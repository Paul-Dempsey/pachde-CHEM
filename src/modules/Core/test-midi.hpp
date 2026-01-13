#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "services/HakenMidiOutput.hpp"
namespace pachde {

struct MidiEvent {
    float t;
    uint32_t msg;
};

extern MidiEvent test_midi_data[];

struct MidiPlayer
{
    MidiEvent* clip_start{nullptr};
    MidiEvent* clip{nullptr};
    HakenMidiOutput* output{nullptr};
    WallTimer timer;

    void init (HakenMidiOutput* out,  MidiEvent*clip);
    void rewind() { clip = clip_start; }
    bool playing() { return timer.running(); }
    void start(const rack::Module::ProcessArgs& args);
    void process(const rack::Module::ProcessArgs& args);
};

}
