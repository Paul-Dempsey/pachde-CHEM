#include "EaganMatrix.hpp"

namespace pachde {

EaganMatrix::EaganMatrix()
:   ready(false),
    firmware_version(0),
    hardware(0),
    reverse_surface(false),
    broken_midi(false),
    in_preset(false),
    in_user(false),
    in_system(false),
    pendingEditorReply(false),
    data_stream(-1),
    pedal_fraction(0),
    pedal_fraction_ex(0),
    editorReply(0),
    led(0)
{
}

void EaganMatrix::reset()
{
    preset.clear();
    firmware_version = 0;
    hardware = 0;
    reverse_surface = false;
    broken_midi = false;
    in_preset = false;
    in_user = false;
    in_system = false;
    pendingEditorReply = false;
    data_stream = -1;
    pedal_fraction = 0;
    pedal_fraction_ex = 0;
    editorReply = 0;
    led = 0;
}

void EaganMatrix::subscribeEMEvents(IHandleEmEvents* client)
{
    if (clients.cend() == std::find(clients.cbegin(), clients.cend(), client)) {
        clients.push_back(client);
    }
}
void EaganMatrix::unsubscribeEMEvents(IHandleEmEvents* client)
{
    auto item = std::find(clients.cbegin(), clients.cend(), client);
    if (item != clients.cend())
    {
        clients.erase(item);
    }
}
void EaganMatrix::notifyLoopDetect(uint8_t cc, uint8_t value)
{
    for (auto client : clients) {
        client->onLoopDetect(cc, value);
    }
}
void EaganMatrix::notifyEditorReply()
{
    for (auto client : clients) {
        client->onEditorReply(editorReply);
    }
}
void EaganMatrix::notifyPresetChanged()
{
    for (auto client : clients) {
        client->onPresetChanged();
    }
}
void EaganMatrix::notifyHardwareChanged() {
    for (auto client : clients) {
        client->onHardwareChanged(hardware, firmware_version);
    }
}
void EaganMatrix::notifyUserComplete()
{
    for (auto client : clients) {
        client->onUserComplete();
    }
}
void EaganMatrix::notifySystemComplete()
{
    for (auto client : clients) {
        client->onSystemComplete();
    }
}
void EaganMatrix::notifyTaskMessage(uint8_t code)
{
    for (auto client : clients) {
        client->onTaskMessage(code);
    }
}
void EaganMatrix::notifyLED(uint8_t led)
{
    for (auto client : clients) {
        client->onLED(led);
    }

}

void EaganMatrix::begin_preset()
{
    clear_preset();
    in_preset = true;
}

void EaganMatrix::clear_preset()
{
    in_preset = false;
    preset.clear();
    name_buffer.clear();
    text_buffer.clear();
    notifyPresetChanged();
}

void EaganMatrix::onChannelTwoCC(uint8_t cc, uint8_t value)
{
    switch (cc) {
    case Haken::ccFracPed:
        pedal_fraction = value;
        break;

    case Haken::ccFracPedEx:
        pedal_fraction_ex = value;
        break;
    }
}
void EaganMatrix::onChannelOneCC(uint8_t cc, uint8_t value)
{
    switch (cc) {
    // case Haken::ccMini_LB:
    // case Haken::ccLoopDetect:
    //     notifyLoopDetect(cc, value);
    //     break;

    case Haken::ccFracPed:
        pedal_fraction = value;
        break;

    case Haken::ccFracPedEx:
        pedal_fraction_ex = value;
        break;
    }
}

void EaganMatrix::onChannel16CC(uint8_t cc, uint8_t value)
{
    switch (cc) {
    case Haken::ccBankH:
        if (in_preset) {
            preset.id.raw().bytes.bank_hi = value;
        }
        break;

    case Haken::ccBankL: // category
        if (in_preset) {
            preset.id.raw().bytes.bank_lo = value;
        }
        break;

    case Haken::ccStream: {
        switch (value) {
        case Haken::s_Name: {
            name_buffer.clear();
            if (data_stream != -1) {
                broken_midi = true;
            } else {
                data_stream = value;
                //if (in_sys_names || in_user_names) {
                    begin_preset();
                //}
            }
        } break;

        case Haken::s_ConText: {
            text_buffer.clear();
            if (data_stream != -1) {
                broken_midi = true;
            } else {
                data_stream = value;
            }
        } break;

        case Haken::s_StreamEnd: {
            if (in_preset) {
                switch (data_stream) {
                case Haken::s_Name:
                    preset.name = name_buffer.str();
                    break;
                case Haken::s_ConText:
                    preset.text = text_buffer.str();
                    break;
                }
            }
            data_stream = -1;
        } break;
        }
    } break;

    case Haken::ccMini_LB:
    case Haken::ccLoopDetect:
        notifyLoopDetect(cc, value);
        break;

    case Haken::ccFracPed:
        pedal_fraction = value;
        break;

    case Haken::ccFracPedEx:
        pedal_fraction_ex = value;
        break;

    case Haken::ccVersHi:
        firmware_version = value;
        break;

    case Haken::ccVersLo:
        firmware_version = (firmware_version << 7) | value;
        notifyHardwareChanged();
        break;

    case Haken::ccCVCHigh:
        hardware = (value & 0x7c) >> 2;
        break;

    case Haken::ccTask:
        notifyTaskMessage(value);
        switch (value) {
        case Haken::endUserNames:
            notifyUserComplete();
            break;
        case Haken::endSysNames:
            notifySystemComplete();
            break;
        }
        break;

    case Haken::ccEdState:
        led = value & Haken::sLedBits;
        notifyLED(led);
        break;

    case Haken::ccEditorReply:
        editorReply = value;
        if (pendingEditorReply) {
            notifyEditorReply();
            pendingEditorReply = false;
        }
        break;

    case Haken::ccEditor:
        pendingEditorReply = true;
        break;
    }
}

void EaganMatrix::onMessage(PackedMidiMessage msg)
{
    uint8_t channel = midi_channel(msg);
    uint8_t status = midi_status(msg);

    switch (channel) {
    case Haken::ch1: {
        switch (status) {
        case MidiStatus_CC: {
            onChannelOneCC(msg.bytes.data1, msg.bytes.data2);
        } break;
        }
    } break;

    case Haken::ch2: {
        switch (status) {
        case MidiStatus_CC: {
            onChannelTwoCC(msg.bytes.data1, msg.bytes.data2);
        } break;
        }
    } break;

    case Haken::ch16: {
        switch (status) {
        case MidiStatus_CC:
            onChannel16CC(msg.bytes.data1, msg.bytes.data2);
            break;

        case MidiStatus_ProgramChange:
            if (in_preset) {
                preset.id.raw().bytes.number = msg.bytes.data1;
                in_preset = false;
                ready = true;
                notifyPresetChanged();
            }
            break;

        case MidiStatus_PolyKeyPressure:
            if (in_preset) {
                // FW > 1009
                switch(data_stream) {
                case Haken::s_Name:
                    name_buffer.build(msg.bytes.data1);
                    name_buffer.build(msg.bytes.data2);
                    break;
                case Haken::s_ConText:
                    text_buffer.build(msg.bytes.data1);
                    text_buffer.build(msg.bytes.data2);
                    break;
                }
            }
            break;

        case MidiStatus_ChannelPressure:
            if (in_preset) {
                // FW 1009
                switch (data_stream) {
                case Haken::s_Name:
                    name_buffer.build(msg.bytes.data1);
                    break;
                case Haken::s_ConText:
                    text_buffer.build(msg.bytes.data1);
                    break;
                }
            }
            break;
        }
    } break;

    }
}

}