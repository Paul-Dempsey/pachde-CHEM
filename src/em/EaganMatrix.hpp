#pragma once
#include <stdint.h>
#include "wrap-HakenMidi.hpp"
#include "midi-message.h"
#include "em-hardware.h"
#include "preset.hpp"
#include "FixedStringBuffer.hpp"

namespace pachde {

struct IHandleEmEvents {
    virtual void onLoopDetect(uint8_t cc, uint8_t value) = 0;
    virtual void onEditorReply(uint8_t reply) = 0;
    virtual void onHardwareChanged(uint8_t hardware, uint16_t firmware_version) = 0;
    virtual void onPresetChanged() = 0;
    virtual void onUserComplete() = 0;
    virtual void onSystemComplete() = 0;
    virtual void onTaskMessage(uint8_t code) = 0;
    virtual void onLED(uint8_t led) = 0;
};

struct ChannelData
{
    uint8_t cc[128]{0};
    uint8_t pedal_fraction() { return cc[Haken::ccFracPed]; }
    uint8_t pedal_fraction_ex() { return cc[Haken::ccFracPedEx]; }
};

struct EaganMatrix {
    bool ready;
    uint16_t firmware_version;
    //uint16_t cvc_version;
    uint8_t hardware;
    //uint8_t middle_c;
    bool reverse_surface;

    bool broken_midi;
    bool in_preset;
    bool in_user;
    bool in_system;
    bool pendingEditorReply;
    int data_stream;

    uint8_t editorReply;
    uint8_t led;
    ChannelData ch1;
    ChannelData ch2;
    ChannelData ch16;

    FixedStringBuffer<32> name_buffer;
    FixedStringBuffer<256> text_buffer;
    PresetDescription preset;

    void reset();

    std::vector<IHandleEmEvents*> clients;
    void clearClients() { clients.clear(); }
    void subscribeEMEvents(IHandleEmEvents* client);
    void unsubscribeEMEvents(IHandleEmEvents* client);
    void notifyLoopDetect(uint8_t cc, uint8_t value);
    void notifyEditorReply();
    void notifyHardwareChanged();
    void notifyPresetChanged();
    void notifyUserComplete();
    void notifySystemComplete();
    void notifyTaskMessage(uint8_t code);
    void notifyLED(uint8_t led);

    EaganMatrix();

    void begin_preset();
    void clear_preset(bool notify);

    void onChannelOneCC(uint8_t cc, uint8_t value);
    void onChannelTwoCC(uint8_t cc, uint8_t value);
    void onChannel16CC(uint8_t cc, uint8_t value);
    void onMessage(PackedMidiMessage msg);

};

}