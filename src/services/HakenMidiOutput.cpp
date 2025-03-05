#include "HakenMidiOutput.hpp"
#include "../em/wrap-HakenMidi.hpp"

namespace pachde {

void HakenMidiOutput::dispatch(float sampleTime)
{
    float midi_time = midi_timer.process(sampleTime);
    if (midi_time < MIDI_RATE) return;
    midi_timer.reset();

    output.channel = -1;
    while (!ring.empty()) {
        auto message = ring.shift();
        if (log) {
            log->logMidi(IO_Direction::Out, message);
        }
        ++message_count;
        output.setChannel(-1);
        output.sendMessage(rackFromPacked(message));
    }
}

void HakenMidiOutput::queueMessage(PackedMidiMessage msg)
{
    if (ring.full()) {
        assert(false);
    } else {
        ring.push(msg);
    }
}

void HakenMidiOutput::doMessage(PackedMidiMessage message)
{
    if (MidiTag::Haken == as_midi_tag(message.bytes.tag)) return;
    queueMessage(message);
}

}
