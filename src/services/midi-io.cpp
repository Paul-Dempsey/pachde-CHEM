#include <rack.hpp>
#include "midi-io.hpp"
#include "../plugin.hpp"
#include "../services/misc.hpp"
#include "../em/wrap-haken-midi.hpp"
using namespace ::rack;

namespace pachde {

void MidiInput::setLogger(const std::string& source, MidiLog* logger) {
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
            log->logMessage(printable(source_name), message);
        }
        ++message_count;
        for (auto target : targets)
        {
            target->doMessage(message);
        }
    }
}

void MidiInput::queueMessage(PackedMidiMessage msg)
{
    if (ring.full()) {
        assert(false);
    } else {
        ring.push(msg);
    }
}

void MidiInput::onMessage(const midi::Message& message)
{
    queueMessage(packedFromRack(message));
}

// ---------------------------------------------------------------------------

midi::Message rackFromPacked(PackedMidiMessage message)
{
    midi::Message msg;
    msg.bytes[0] = message.bytes.status_byte;
    switch (MessageBytes(message.bytes.status_byte)) {
    default:
    case 0:
        msg.setSize(1);
        break;
    case 1:
        msg.setSize(2);
        msg.bytes[1] = message.bytes.data1;
        break;
    case 2:
        msg.setSize(3);
        msg.bytes[1] = message.bytes.data1;
        msg.bytes[2] = message.bytes.data2;
        break;
    }
    return msg;
}

PackedMidiMessage packedFromRack(const midi::Message& source)
{
    switch (source.bytes.size()) {
    case 1: return MakeRawBase(source.bytes[0], 0, 0);
    case 2: return MakeRawBase(source.bytes[0], source.bytes[1], 0);
    case 3: return MakeRawBase(source.bytes[0], source.bytes[1], source.bytes[2]);
    default:
        assert(false);
        return MakeRawBase(0, 0, 0);
    }
}

midi::Message makeMidiMessage(uint8_t status, uint8_t channel, uint8_t value)
{
    midi::Message msg;
    msg.setSize(2);
    msg.bytes[0] = status | channel;
    msg.bytes[1] = value;
    return msg;
}

midi::Message makeMidiMessage(uint8_t status, uint8_t channel, uint8_t d1, uint8_t d2)
{
    midi::Message msg;
    msg.setSize(3);
    msg.bytes[0] = status | channel;
    msg.bytes[1] = d1;
    msg.bytes[2] = d2;
    return msg;
}

const char * StatusName(uint8_t status) {
    switch (status) {
        case MidiStatus_NoteOff        : return "Off";
        case MidiStatus_NoteOn         : return "On";
        case MidiStatus_PolyKeyPressure: return "KP";
        case MidiStatus_CC             : return "CC";
        case MidiStatus_ProgramChange  : return "PC";
        case MidiStatus_ChannelPressure: return "ChP";
        case MidiStatus_PitchBend      : return "PB";
        case MidiStatus_SysEx          : return "Sys";
        default: return "--";
    }
}

struct Names {
    std::vector<std::string> names;
    Names(const char * source) {
        std::string token;
        const char *scan = source;
        while (*scan) {
            if (' ' == *scan) {
                assert(!token.empty());
                names.push_back(token);
                token.clear();
            } else {
                token.push_back(*scan);
            }
            ++scan;
        }
        if (!token.empty()) {
            names.push_back(token);
        }
    }
    const std::string& Name(uint8_t index) {
        return names[index];
    }
};

auto ch1_2cc_names = Names(SSL_ch1_ch2);
auto ch16cc_names = Names(SSL_ch16);
auto task_names = Names(SSL_ccTask);

const std::string& channelCCName(uint8_t channel, uint8_t cc) {
    static std::string undef = "";
    switch (channel) {
        case Haken::ch1:
        case Haken::ch2: return ch1_2cc_names.Name(cc);
        case Haken::ch16: return ch16cc_names.Name(cc);
        default: return undef;
    }
}

// ---------------------------------------------------------------------------
// MidiLog
//

std::string MidiLog::logfile()
{
    return asset::user(format_string("%s/midilog.txt", pluginInstance->slug.c_str()));
}

MidiLog::MidiLog() :
    //logging(false), 
    log(nullptr)
{
}

void MidiLog::ensure_file()
{
    if (log) return;
    auto path = logfile();
    auto dir = system::getDirectory(path);
    system::createDirectories(dir);
    log = std::fopen(path.c_str(), "w");
}

void MidiLog::close() {
    if (log) {
        std::fclose(log);
        log = nullptr;
    }
}

MidiLog::~MidiLog() {
    close();
}
// void MidiLog::on() { logging = true; }
// void MidiLog::off() { logging = false; }

void MidiLog::logMessage(const char *prefix, PackedMidiMessage m)
{
    ensure_file();
    if (!log) return;

    char buffer[256];
    int bytes = 0;
    auto status = midi_status(m);
    auto channel = midi_channel(m);

    switch (status) {
        case Haken::ccStat: {
            auto cc = midi_cc(m);
            switch (channel) {
                case Haken::ch1:
                case Haken::ch2:
                    bytes = format_buffer(buffer, 256, "[%s] ch%0-2d cc%s %02d\n", prefix, 1+channel, ch1_2cc_names.Name(cc).c_str(), m.bytes.data2);
                    break;

                case Haken::ch16:
                    if (Haken::ccTask == cc) {
                        bytes = format_buffer(buffer, 256, "[%s] ch%-2d cc%s %s\n", prefix, 1+channel, ch16cc_names.Name(cc).c_str(), task_names.Name(m.bytes.data2).c_str());
                    } else {
                        bytes = format_buffer(buffer, 256, "[%s] ch%-2d cc%s %02d\n", prefix, 1+channel, ch16cc_names.Name(cc).c_str(), m.bytes.data2);
                    }
                    break;

                default:
                    break;
            }} break;

        case Haken::progChg: {
            bytes = format_buffer(buffer, 256, "[%s] ch%-2d %s %d\n", prefix, 1+midi_channel(m), StatusName(midi_status(m)), m.bytes.data1);
        } break;

        case Haken::polyKeyPres: {
            switch (channel) {
                case Haken::ch1:
                case Haken::ch2:
                case Haken::ch16:
                    bytes = format_buffer(buffer, 256, "[%s] ch%-2d %s %d %d (%c%c)\n", prefix, 1+midi_channel(m), StatusName(midi_status(m)), m.bytes.data1, m.bytes.data2, printable(m.bytes.data1), printable(m.bytes.data2));
                    break;
                default:
                    break;
            } break;
        } break;

        default:
            bytes = format_buffer(buffer, 256, "[%s] [%08x] ch%-2d %s %d %d\n", prefix, m.data, 1+midi_channel(m), StatusName(midi_status(m)), m.bytes.data1, m.bytes.data2);
            break;
    }
    
    if (bytes) {
        std::fwrite(buffer, 1, bytes, log);
        std::fflush(log);
    }
}

void MidiLog::logMessage(const char *prefix, const char *info)
{
    ensure_file();
    if (!log) return;

    char buffer[256];
    int bytes = 0;
    bytes = format_buffer(buffer, 256, "[%s] %s\n", prefix, info);
    if (bytes) {
        auto written = std::fwrite(buffer, 1, bytes, log);
        if (written > 0) {
            std::fflush(log);
        }
    }
}

}