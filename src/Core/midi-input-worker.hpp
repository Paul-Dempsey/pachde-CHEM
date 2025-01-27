// Copyright (C) Paul Chase Dempsey
#pragma once
#include <condition_variable>
#include <mutex>
#include <thread>
#include <rack.hpp>
#include "../services/midi-io.hpp"

using namespace ::rack;

namespace pachde {

struct MidiInputWorker
{
    rack::dsp::RingBuffer<PackedMidiMessage, 1024> midi_consume;
    std::mutex m;
    std::condition_variable cv;
    rack::Context* context;
    std::thread my_thread;
    bool stop;
    bool pausing;
    std::vector<IDoMidi*> targets;

    MidiInputWorker(rack::Context* rack)
    :   context(rack),
        stop(false),
        pausing(false) 
    {}

    void addTarget(struct IDoMidi* out) { targets.push_back(out); }
    void start();
    void pause();
    void resume();
    void post_quit();
    void post_message(PackedMidiMessage msg);
    void run();
};

}
