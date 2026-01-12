#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "text.hpp"

namespace pachde {

enum Note { C, Cs, D, Eb, E, F, Fs, G, Ab, A, Bb, B };

inline Note fromU8(uint8_t nn) { return static_cast<Note>(nn % 12); }

struct MidiNote {
    uint8_t nn;
 #ifdef UNICODE_NOTE
    const char * display_note_unicode[12] = {
        "C", "C\u{266F}",
        "D",
        "E\u{266D}", "E",
        "F", "F\u{266F}",
        "G",
        "A\u{266D}", "A",
        "B\u{266D}", "B"
    };
#endif
    const char * display_note[12] = {
        "C", "C#","D","Eb","E", "F", "F#", "G", "Ab","A", "Bb", "B"
    };

    MidiNote(uint8_t nn) : nn(nn) {}
    MidiNote(Note note, uint8_t octave) : nn(octave * 12 + uint8_t(note)) {}
    int octave() { return nn / 12; }
    void set_octave(uint8_t octave) { nn = octave * 12 + note(); }
    Note note() { return fromU8(nn); }
    const char * name() { return display_note[note()]; }
    std::string full_name() { return format_string("%s%d", name(), octave()); }

 #ifdef UNICODE_NOTE
    const char * name_unicode() { return display_note_unicode[note()]; }
    std::string full_name_unicode() { return format_string("%s%d", name_unicode(), octave()); }
#endif

    // TODO:
    // MidiNote parse(std::string text);
};

const char * noteName(uint8_t nn) { return MidiNote(nn).name(); }
std::string noteFullName(uint8_t nn) { return MidiNote(nn).full_name(); }

}