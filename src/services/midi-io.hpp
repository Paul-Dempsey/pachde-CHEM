// Copyright (C) Paul Chase Dempsey
#pragma once
#include <rack.hpp>
#include "midi-log.hpp"
//#include "../chem-core.hpp"
#include "../chem-id.hpp"

#include "../em/midi-message.h"
using namespace ::rack;
namespace pachde {

midi::Message rackFromPacked(PackedMidiMessage source);
inline PackedMidiMessage Tag(PackedMidiMessage msg, MidiTag tag) {
    msg.bytes.tag = as_u8(tag);
    return msg;
}

constexpr const float MIDI_RATE = 0.05f;
constexpr const float DISPATCH_NOW = MIDI_RATE;

struct MidiInput : midi::Input
{
    MidiTag my_tag;
    IDoMidi* target;
    uint64_t message_count;
    MidiLog* log;
    std::string source_name;
    bool music_pass_filter;

    rack::dsp::RingBuffer<PackedMidiMessage, 1024> ring;
    void queueMessage(PackedMidiMessage msg);
    void dispatch(float sampleTime);
    void drop(int count);
    rack::dsp::Timer midi_timer;

    MidiInput(const MidiInput &) = delete; // no copy constructor
    MidiInput(MidiTag tag) : my_tag(tag), message_count(0), log(nullptr), music_pass_filter(false) { this->channel = -1; }

    uint64_t count() { return message_count; }
    void clear()
    {
        //target = nullptr;
        reset();
        this->setDeviceId(-1);
        this->channel = -1;
    }
    void setTarget(IDoMidi* out) { target = out; }
    void setLogger(const std::string& source, MidiLog* logger);
    void setMusicPassFilter(bool pass_music) { music_pass_filter = pass_music; }
    void onMessage(const midi::Message& message) override;
};

}
