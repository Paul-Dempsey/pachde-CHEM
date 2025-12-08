#include "HakenMidiOutput.hpp"
#include "em/wrap-HakenMidi.hpp"

namespace pachde {

void HakenMidiOutput::enable(bool on)
{
    if (enabled == on) return;
    if (!on) {
        clear();
    }
    enabled = on;
}

void HakenMidiOutput::clear()
{
    message_count = 0;
    ring.clear();
    midi_timer.reset();
    output.reset();
    output.setChannel(-1);
    enabled = true;
}

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
    if (!enabled) return;
    if (ring.full()) {
        assert(false);
    } else {
        ring.push(msg);
    }
}

void HakenMidiOutput::do_message(PackedMidiMessage message)
{
    if (!enabled || ChemId::Haken == as_chem_id(message.bytes.tag)) return;
    queueMessage(message);
}

}
