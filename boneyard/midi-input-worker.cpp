// Copyright (C) Paul Chase Dempsey
#include "midi-input-worker.hpp"

namespace pachde {

void MidiInputWorker::start()
{
    my_thread = std::thread(&MidiInputWorker::run, this);
}

void MidiInputWorker::post_quit()
{
    std::unique_lock<std::mutex> lock(m);
    stop = true;
    cv.notify_one();
}

void MidiInputWorker::pause()
{
    std::unique_lock<std::mutex> lock(m);
    midi_consume.clear();
    pausing = true;
    cv.notify_one();
}

void MidiInputWorker::resume()
{
    std::unique_lock<std::mutex> lock(m);
    pausing = false;
    cv.notify_one();
}

void MidiInputWorker::post_message(PackedMidiMessage msg)
{
    if (stop || pausing) return;
    std::unique_lock<std::mutex> lock(m);
    midi_consume.push(msg);
    cv.notify_one();
}

void MidiInputWorker::run() {
    contextSet(context);
	system::setThreadName("CHEM Midi Input worker");
    while (1)
    {
        {
            std::unique_lock<std::mutex> lock(m);
            cv.wait(lock, [this]{ return stop || !midi_consume.empty(); });
            if (stop) {
                return;
            }
            if (pausing) {
                continue;
            }
        }
        while (!midi_consume.empty()) {
            auto msg = midi_consume.shift();
            for (auto target: targets)
            {
                target->doMessage(msg);
            }
        }
    }
}

}