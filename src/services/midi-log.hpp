#pragma once
#include <rack.hpp>
#include "../em/midi-message.h"

namespace pachde {

enum class IO_Direction { In, Out };

struct MidiLog
{
    uint32_t id;
    FILE * log;
    std::string logfile();

    MidiLog();
    ~MidiLog();
    
    void ensure_file();
    void close();
    void logMidi(IO_Direction dir, PackedMidiMessage message);
    void logMessage(const char *prefix, const char *info);
    void logMessage(const char *prefix, const std::string& str);
};

}