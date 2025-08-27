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
inline PackedMidiMessage Tag(PackedMidiMessage msg, ChemId tag) {
    msg.bytes.tag = as_u8(tag);
    return msg;
}
inline PackedMidiMessage MakeStreamData(ChemId tag, uint8_t id, uint8_t value) {
    return Tag(MakePolyKeyPressure(15, id, value), tag);
}

constexpr const float DEFAULT_MIDI_RATE = 0.001f;
extern float MIDI_RATE;
#define DISPATCH_NOW (MIDI_RATE)
void InitMidiRate();

struct MidiInput : midi::Input
{
    ChemId my_tag;
    IDoMidi* target;
    uint64_t message_count;
    MidiLog* log;
    std::string source_name;
    bool music_pass_filter;
    bool channel_reflect;
    bool mute;

    rack::dsp::RingBuffer<PackedMidiMessage, 1024> ring;
    void queueMessage(PackedMidiMessage msg);
    void dispatch(float sampleTime);
    void drop(int count);
    rack::dsp::Timer midi_timer;
    MidiInput(const MidiInput &) = delete; // no copy constructor
    MidiInput(ChemId tag);

    uint64_t count() { return message_count; }
    void clear()
    {
        reset();
        this->setDeviceId(-1);
        this->channel = -1;
        mute = false;
        message_count = 0;
    }
    void set_target(IDoMidi* out) { target = out; }
    void set_logger(const std::string& source, MidiLog* logger);
    void set_music_pass(bool pass_music) { music_pass_filter = pass_music; }
    void set_channel_reflect(bool reflect) { channel_reflect = reflect; }
    void enable(bool enabled = true);
    void onMessage(const midi::Message& message) override;
};

}
