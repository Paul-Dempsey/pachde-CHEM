#include <rack.hpp>
#include "midi-io.hpp"
#include "../plugin.hpp"
#include "../services/misc.hpp"
#include "../em/wrap-HakenMidi.hpp"
using namespace ::rack;

namespace pachde {

midi::Message rackFromPacked(PackedMidiMessage message)
{
    midi::Message msg;
    msg.bytes[0] = message.bytes.status_byte;
    switch (MessageBytes(message.bytes.status_byte)) {
    default:
    case 0:
        msg.setSize(1);
        msg.bytes[0] = message.bytes.status_byte;
        break;
    case 1:
        msg.setSize(2);
        msg.bytes[0] = message.bytes.status_byte;
        msg.bytes[1] = message.bytes.data1;
        break;
    case 2:
        msg.setSize(3);
        msg.bytes[0] = message.bytes.status_byte;
        msg.bytes[1] = message.bytes.data1;
        msg.bytes[2] = message.bytes.data2;
        break;
    }
    return msg;
}

PackedMidiMessage packedFromRack(const midi::Message& source, ChemId tag)
{
    switch (source.bytes.size()) {
    case 1: return Tag(MakeRawBase(source.bytes[0], 0, 0), tag);
    case 2: return Tag(MakeRawBase(source.bytes[0], source.bytes[1], 0), tag);
    case 3: return Tag(MakeRawBase(source.bytes[0], source.bytes[1], source.bytes[2]), tag);
    default:
        assert(false);
        return Tag(MakeRawBase(0, 0, 0), tag);
    }
}
    
inline bool is_note_cc(uint8_t cc)
{
    switch (cc) {
    case Haken::ccMod:
    case Haken::ccBreath:
    case Haken::ccUndef:
    case Haken::ccVol:
    case Haken::ccExpres:
    case Haken::ccBrightness:
    case Haken::ccFrac:
        return true;
    default:
        return false;
    }
}

inline bool is_music_message(PackedMidiMessage msg)
{
    switch (msg.bytes.status_byte) {
    case Haken::ccStat1:
    case Haken::ccStat2:
        return is_note_cc(midi_cc(msg));

    // no program change
    case Haken::progChg1:
    case Haken::progChg2:
    case Haken::progChg16:
    case Haken::ccStat16: // no ch16 cc
        return false;

    default:
        break;
    }
    return true;
}
    
void MidiInput::set_logger(const std::string& source, MidiLog* logger) {
    source_name = source;
    log = logger;
}

void MidiInput::dispatch(float sampleTime)
{
    float midi_time = midi_timer.process(sampleTime);
    if (midi_time < MIDI_RATE) return;
    midi_timer.reset();

    while (!ring.empty()) {
        auto message = ring.shift();
        if (log) {
            log->logMidi(IO_Direction::In, message);
        }
        ++message_count;
        target->do_message(message);
    }
}

void MidiInput::drop(int count)
{
    char buffer[100];
    PackedMidiMessage trash[count];
    if (log) {
        format_buffer(buffer, 100, "!! Dropping %d messages out of %d queued", count, ring.size());
        log->log_message(printable(source_name), buffer);
    }
    ring.shiftBuffer(trash, count);
}

MidiInput::MidiInput(ChemId tag): my_tag(tag), message_count(0), log(nullptr), music_pass_filter(false), mute{false}
{
    this->channel = -1;
    midi_timer.time = (random::uniform() * MIDI_RATE); // jitter
}

void MidiInput::queueMessage(PackedMidiMessage msg)
{
    if (ring.full()) {
        drop(16);
    }
    ring.push(msg);
}

void MidiInput::onMessage(const midi::Message& message)
{
    if (mute) return;
    auto msg = packedFromRack(message, my_tag);
    if (music_pass_filter && !is_music_message(msg)) return;
    queueMessage(msg);
}

}