#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "text.hpp"

namespace pachde {

enum eNote { C, Cs, D, Eb, E, F, Fs, G, Ab, A, Bb, B };
inline eNote eNoteFromNoteNumber(uint8_t nn) {
    return static_cast<eNote>(nn % 12);
}

struct MidiNote {
    const char * display_note[12] = {
        "C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab","A", "Bb", "B"
    };
    uint8_t nn;

    MidiNote(uint8_t nn) : nn(nn) {}
    MidiNote(eNote note, uint8_t octave) : nn(static_cast<uint8_t>(note) + (octave * 12)) {}
    uint8_t number() { return nn; }
    int octave() { return nn / 12; }
    void set_octave(uint8_t octave) { nn = (nn % 12) + (octave * 12); }
    eNote note() { return eNoteFromNoteNumber(nn); }
    const char * name() { return display_note[nn % 12]; }
    std::string full_name() { return format_string("%s%d", name(), octave()); }

    // TODO:
    // MidiNote parse(std::string text);
};

inline const char * noteName(uint8_t nn) { return MidiNote(nn).name(); }
inline std::string noteFullName(uint8_t nn) { return MidiNote(nn).full_name(); }

}