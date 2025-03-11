// Copyright (C) Paul Chase Dempsey
#pragma once
#ifndef MIDI_MESSAGE_H_INCLUDED
#define MIDI_MESSAGE_H_INCLUDED
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MidiStatus_NoteOff         = 0x80,
    MidiStatus_NoteOn          = 0x90,
    MidiStatus_PolyKeyPressure = 0xa0,
    MidiStatus_CC              = 0xb0,
    MidiStatus_ProgramChange   = 0xc0,
    MidiStatus_ChannelPressure = 0xd0,
    MidiStatus_PitchBend       = 0xe0,
    MidiStatus_SysEx           = 0xf0,
} MidiStatus;

int MessageBytes(uint8_t status_byte);

#define STATUS_MASK   0xF0
#define CHANNEL_MASK  0x0F

/// @brief A MIDI 1.0 message packed into a 32-bit unsigned integer
/// Note that PackedMidiMessage is not able to represent general SysEx messages.
typedef union PackedMidiMessage {
    uint32_t data;
    struct {
        uint8_t status_byte;
        uint8_t data1;
        uint8_t data2;
        uint8_t tag;
    } bytes;
} PackedMidiMessage;

struct IDoMidi {
    virtual void do_message(PackedMidiMessage message) {}
};

inline uint8_t midi_status(PackedMidiMessage msg) { return msg.bytes.status_byte & STATUS_MASK; }
inline uint8_t midi_channel(PackedMidiMessage msg) { return msg.bytes.status_byte & CHANNEL_MASK; }
inline uint8_t midi_cc(PackedMidiMessage msg) { return msg.bytes.data1; }
inline uint8_t midi_cc_value(PackedMidiMessage msg) { return msg.bytes.data2; }
inline uint8_t midi_tag(PackedMidiMessage msg) { return msg.bytes.tag; }

inline PackedMidiMessage Tag(PackedMidiMessage msg, uint8_t tag) {
    msg.bytes.tag = tag;
    return msg;
}

PackedMidiMessage MakeRawBase(uint8_t status_byte, uint8_t value, uint8_t value2);
PackedMidiMessage MakeRaw(uint8_t status, uint8_t channel, uint8_t value, uint8_t value2);

inline PackedMidiMessage MakeNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    return MakeRaw(MidiStatus_NoteOff, channel, note, velocity);
}
inline PackedMidiMessage MakeNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    return MakeRaw(MidiStatus_NoteOn, channel, note, velocity); 
}
inline PackedMidiMessage MakePolyKeyPressure(uint8_t channel, uint8_t note, uint8_t pressure) {
    return MakeRaw(MidiStatus_PolyKeyPressure, channel, note, pressure); 
}
inline PackedMidiMessage MakeCC(uint8_t channel, uint8_t cc, uint8_t value) {
    return MakeRaw(MidiStatus_CC, channel, cc, value); 
}
inline PackedMidiMessage MakeProgramChange(uint8_t channel, uint8_t program) {
    return MakeRaw(MidiStatus_ProgramChange, channel, program, 0);
}
inline PackedMidiMessage MakeChannelPressure(uint8_t channel, uint8_t pressure) {
    return MakeRaw(MidiStatus_ChannelPressure, channel, pressure, 0);
}
inline PackedMidiMessage MakePitchBend(uint8_t channel, uint8_t loBend, uint8_t hiBend) {
    return MakeRaw(MidiStatus_PitchBend, channel, loBend, hiBend);
}

/// @brief Callback to handle a packed MIDI message
/// @param context optional data to be passed to handler function
/// @param message Packed MIDI message to handle
/// @return 0 to continue processing, nonzero to halt Midi processing (error or other stop signal)
typedef uint8_t (*MidiHandler)(void *context, PackedMidiMessage message);

#ifdef __cplusplus
} // extern "C"
#endif
#endif