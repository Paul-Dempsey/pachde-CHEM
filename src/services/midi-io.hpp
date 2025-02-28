// Copyright (C) Paul Chase Dempsey
#pragma once
#include <rack.hpp>
#include <ghc/filesystem.hpp>
#include "../em/midi-message.h"
#include "../em/PresetId.hpp"
#include "../em/wrap-HakenMidi.hpp"

using namespace ::rack;
namespace pachde {

midi::Message rackFromPacked(PackedMidiMessage source);
PackedMidiMessage packedFromRack(const midi::Message& source);
midi::Message makeMidiMessage(uint8_t status, uint8_t channel, uint8_t value);
midi::Message makeMidiMessage(uint8_t status, uint8_t channel, uint8_t d1, uint8_t d2);

struct IDoMidi {
    virtual void doMessage(PackedMidiMessage message) = 0;
};

constexpr const float MIDI_RATE = 0.05f;
constexpr const float DISPATCH_NOW = MIDI_RATE;

struct MidiLog
{
    uint32_t id;
    FILE * log;
    std::string logfile();

    MidiLog();
    ~MidiLog();
    
    void ensure_file();
    void close();
    void logMessage(const char *prefix, PackedMidiMessage message);
    void logMessage(const char *prefix, const char *info);
    void logMessage(const char *prefix, const std::string& str);
};


struct MidiInput : midi::Input
{
    std::vector<IDoMidi*> targets;
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
    MidiInput() : message_count(0), log(nullptr), music_pass_filter(false) { this->channel = -1; }

    uint64_t count() { return message_count; }
    void clear()
    {
        targets.clear(); 
        reset();
        this->setDeviceId(-1);
        this->channel = -1;
    }
    void addTarget(IDoMidi* out) { targets.push_back(out); }
    void removeTarget(IDoMidi* out);
    void setLogger(const std::string& source, MidiLog* logger);
    void setMusicPassFilter(bool pass_music) { music_pass_filter = pass_music; }
    void onMessage(const midi::Message& message) override;
};

}
