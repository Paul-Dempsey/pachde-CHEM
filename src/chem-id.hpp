#pragma once
#include <stdint.h>
namespace pachde {

enum class ChemId : uint8_t {
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
    Sustain,
    Sostenuto,
    Sostenuto2,
    Settings,
    Preset,
    Blank
};

inline uint8_t as_u8(ChemId tag) { return static_cast<uint8_t>(tag); }
inline ChemId as_chem_id(uint8_t byte) { return static_cast<ChemId>(byte); }

enum class ChemDevice : uint8_t { Unknown, Haken, Midi1, Midi2 };

}