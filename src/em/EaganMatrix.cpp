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
    editorReply = 0;
    led = 0;
    notifyHardwareChanged();
    notifyPresetChanged();
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
    clear_preset(false);
    in_preset = true;
}

void EaganMatrix::clear_preset(bool notify)
{
    in_preset = false;
    preset.clear();
    name_buffer.clear();
    text_buffer.clear();
    if (notify) notifyPresetChanged();
}

void EaganMatrix::onChannelTwoCC(uint8_t cc, uint8_t value)
{
}

void EaganMatrix::onChannelOneCC(uint8_t cc, uint8_t value)
{
}

void EaganMatrix::onChannel16CC(uint8_t cc, uint8_t value)
{
    switch (cc) {
    case Haken::ccBankH:
        if (in_system) {
            value = 127;
        }
        preset.id.set_bank_hi(value);
        break;

    case Haken::ccBankL: // category
        preset.id.set_bank_lo(value);
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
        case Haken::beginUserNames:
            in_user = true;
            break;
        case Haken::endUserNames:
            notifyUserComplete();
            break;
        case Haken::endSysNames:
            notifySystemComplete();
            break;
        case Haken::beginSysNames:
            in_system = true;
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
            ch1.cc[msg.bytes.data1] = msg.bytes.data2;
            onChannelOneCC(msg.bytes.data1, msg.bytes.data2);
        } break;
        }
    } break;

    case Haken::ch2: {
        switch (status) {
        case MidiStatus_CC: {
            ch2.cc[msg.bytes.data1] = msg.bytes.data2;
            onChannelTwoCC(msg.bytes.data1, msg.bytes.data2);
        } break;

        case MidiStatus_ProgramChange:
            break;        
        }
    } break;

    case Haken::ch16: {
        switch (status) {
        case MidiStatus_CC:
            ch16.cc[msg.bytes.data1] = msg.bytes.data2;
            onChannel16CC(msg.bytes.data1, msg.bytes.data2);
            break;

        case MidiStatus_ProgramChange:
            if (in_preset) {
                preset.id.set_number(msg.bytes.data1);
                if (Haken::catEdBuf == preset.id.bank_hi()) {
                    uint16_t pn = (static_cast<uint16_t>(preset.id.bank_lo()) << 7) + preset.id.number();
                    if (pn < 129) {
                        preset.id.set_bank_hi(Haken::catUser);
                        --pn;
                    } else {
                        pn -= 129;
                        preset.id.set_bank_hi(Haken::catSSlot);
                    }
                    preset.id.set_bank_lo((pn & 0xff00) >> 7);
                    preset.id.set_number(pn & 0xff);
                }
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