#pragma once
#include <stdint.h>
namespace pachde {

enum class MidiTag : uint8_t {
    Unknown,
    Haken,
    Midi1,
    Midi2,
    Core,
    Macro,
    Pre,
    Fx,
    Post,
    Convo,
    Jack,
    Play,
    Preset
};

inline uint8_t as_u8(MidiTag tag) { return static_cast<uint8_t>(tag); }
inline MidiTag as_midi_tag(uint8_t byte) { return static_cast<MidiTag>(byte); }

}