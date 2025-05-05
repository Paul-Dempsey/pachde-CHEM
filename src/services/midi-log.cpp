#include "midi-log.hpp"
#include <ghc/filesystem.hpp>
#include "misc.hpp"
#include "../plugin.hpp"
#include "../chem-id.hpp"
#include "../em/wrap-HakenMidi.hpp"
using namespace ::rack;

namespace pachde {

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

static std::string unknown{"(unknown)"};
struct Names {

    std::vector<std::string> names;
    Names(const char * source) {
        std::string token;
        const char *scan = source;
        while (*scan) {
            if (' ' == *scan || '\t' == *scan) {
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
        if (index >= names.size()) return unknown;
        return names[index];
    }
};

auto ch1_2cc_names = Names(SSL_ch1_ch2);
auto ch16cc_names = Names(SSL_ch16);
auto task_names = Names(SSL_ccTask);
auto stream_names = Names(SSL_streams);

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
    return asset::user(format_string("%s/midilog-%x.txt", pluginInstance->slug.c_str(), id));
}

static uint32_t midi_log_instance_count(0);

MidiLog::MidiLog() :
    log(nullptr)
{
    id = ++midi_log_instance_count;
}

void MidiLog::ensure_file()
{
    if (log) return;
    auto path = logfile();
    auto dir = system::getDirectory(path);
    system::createDirectories(dir);
    log = std::fopen(path.c_str(), "w");
    log_message("MidiLog", format_string("id = %x", id));
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

std::string tag_prefix(uint8_t tag) {
    switch (as_chem_id(tag)) {
    case ChemId::Unknown:  return "Unknown";
    case ChemId::Haken:    return "Haken";
    case ChemId::Midi1:    return "Midi1";
    case ChemId::Midi2:    return "Midi2";
    case ChemId::Core:     return "Core";
    case ChemId::Macro:    return "Macro";
    case ChemId::Pre:      return "Pre";
    case ChemId::Fx:       return "Fx";
    case ChemId::Post:     return "Post";
    case ChemId::Convo:    return "Convo";
    case ChemId::Jack:     return "Jack";
    case ChemId::Play:     return "Play";
    case ChemId::Settings: return "Settings";
    case ChemId::Preset:   return "Preset";
    case ChemId::Overlay:  return "Overlay";
    case ChemId::XM:       return "XM";
    case ChemId::XMEdit:   return "XMEdit";
    case ChemId::Proto:    return "Proto";
    default:
        return format_string("%d", tag);
    }
}

void MidiLog::logMidi(IO_Direction dir, PackedMidiMessage m)
{
    ensure_file();
    if (!log) return;

    char buffer[256];
    int bytes = 0;
    auto status = midi_status(m);
    auto channel = midi_channel(m);
    char io_glyph = (dir == IO_Direction::In) ? '<' : '>';

    auto pfx = io_glyph + tag_prefix(midi_tag(m));
    const char * prefix = pfx.c_str();

    switch (status) {
        case Haken::ccStat: {
            auto cc = midi_cc(m);
            switch (channel) {
                case Haken::ch1:
                case Haken::ch2:
                    bytes = format_buffer(buffer, 256, "[%s] ch%0-2d cc%s %02d\n", prefix, 1+channel, ch1_2cc_names.Name(cc).c_str(), m.bytes.data2);
                    break;

                case Haken::ch16:
                    switch (cc) {
                    case Haken::ccTask:
                        bytes = format_buffer(buffer, 256, "[%s] ch%-2d cc%s %s\n", prefix, 1+channel, ch16cc_names.Name(cc).c_str(), task_names.Name(m.bytes.data2).c_str());
                        break;
                    case Haken::ccStream:
                        if (127 == m.bytes.data2) {
                            bytes = format_buffer(buffer, 256, "[%s] ch%-2d cc%s [END]\n", prefix, 1+channel, ch16cc_names.Name(cc).c_str());
                        } else {
                            bytes = format_buffer(buffer, 256, "[%s] ch%-2d cc%s %s\n", prefix, 1+channel, ch16cc_names.Name(cc).c_str(), stream_names.Name(m.bytes.data2).c_str());
                        }
                        break;
                    default:
                        bytes = format_buffer(buffer, 256, "[%s] ch%-2d cc%s %02d\n", prefix, 1+channel, ch16cc_names.Name(cc).c_str(), m.bytes.data2);
                        break;
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

void MidiLog::log_message(const char *prefix, const char *info)
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

void MidiLog::log_message(const char *prefix, const std::string& str)
{
    log_message(prefix, printable(str));
}
    


}