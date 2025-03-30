#include "EaganMatrix.hpp"
#include "../services/misc.hpp"

namespace pachde {

EaganMatrix::EaganMatrix()
:   ready(false),
    firmware_version(0),
    hardware(0),
    broken_midi(false),
    in_preset(false),
    in_user(false),
    in_system(false),
    in_mahling(false),
    pending_EditorReply(false),
    pending_config(false),
    frac_hi(false),
    frac_lsb(0),
    data_stream(-1),
    jack_1(0),
    jack_2(0),
    post(0)
{
}

void EaganMatrix::reset()
{
    firmware_version = 0;
    hardware = 0;
    broken_midi = false;
    in_preset = false;
    in_user = false;
    in_system = false;
    pending_EditorReply = false;
    pending_config = false;
    frac_hi = false;
    frac_lsb = 0,
    data_stream = -1;
    jack_1 = jack_2 = 0;
    preset.clear();
    ch1.clear();
    ch2.clear();
    ch16.clear();
    std::memset(macro, 0, sizeof(macro));
    std::memset(mat, 0, sizeof(mat)); // $TODO: init default? (e.g. mat[idFrontBack] = ccBrightness)
    notifyHardwareChanged();
    notifyPresetChanged();
}

float EaganMatrix::get_macro_voltage(int id)
{
    assert(in_range(id, 0, 90));
    uint16_t raw = macro[id];
    assert(in_range(raw, static_cast<uint16_t>(0), static_cast<uint16_t>(Haken::max14)));
    if (id < 7) {
        // unipolar 0 to 10
        auto r = raw * inv_max14;
        return 10.0 * r;
    } else {
        auto r = ((double)raw - (double)Haken::zero14) * inv_zero14;
        return 5.0 * r;
    }
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
        if (client->em_event_mask & IHandleEmEvents::LoopDetect) {
            client->onLoopDetect(cc, value);
        }
    }
}
void EaganMatrix::notifyEditorReply(uint8_t editor_reply)
{
    for (auto client : clients) {
        if (client->em_event_mask & IHandleEmEvents::EditorReply) {
            client->onEditorReply(editor_reply);
        }
    }
}
void EaganMatrix::notifyPresetChanged()
{
    for (auto client : clients) {
        if (client->em_event_mask & IHandleEmEvents::PresetChanged) {
            client->onPresetChanged();
        }
    }
}
void EaganMatrix::notifyHardwareChanged()
{
    for (auto client : clients) {
        if (client->em_event_mask & IHandleEmEvents::HardwareChanged) {
            client->onHardwareChanged(hardware, firmware_version);
        }
    }
}

void EaganMatrix::notifyUserBegin()
{
    for (auto client : clients) {
        if (client->em_event_mask & IHandleEmEvents::UserBegin) {
            client->onUserBegin();
        }
    }
}

void EaganMatrix::notifyUserComplete()
{
    for (auto client : clients) {
        if (client->em_event_mask & IHandleEmEvents::UserComplete) {
            client->onUserComplete();
        }
    }
}

void EaganMatrix::notifySystemBegin()
{
    for (auto client : clients) {
        if (client->em_event_mask & IHandleEmEvents::SystemBegin) {
            client->onSystemBegin();
        }
    }
}

void EaganMatrix::notifySystemComplete()
{
    for (auto client : clients) {
        if (client->em_event_mask & IHandleEmEvents::SystemComplete) {
            client->onSystemComplete();
        }
    }
}

void EaganMatrix::notifyMahlingBegin()
{
    for (auto client : clients) {
        if (client->em_event_mask & IHandleEmEvents::MahlingBegin) {
            client->onMahlingBegin();
        }
    }
}

void EaganMatrix::notifyMahlingComplete()
{
    for (auto client : clients) {
        if (client->em_event_mask & IHandleEmEvents::MahlingComplete) {
            client->onMahlingComplete();
        }
    }
}

void EaganMatrix::notifyTaskMessage(uint8_t code)
{
    for (auto client : clients) {
        if (client->em_event_mask & IHandleEmEvents::TaskMessage) {
            client->onTaskMessage(code);
        }
    }
}
void EaganMatrix::notifyLED(uint8_t led)
{
    for (auto client : clients) {
        if (client->em_event_mask & IHandleEmEvents::LED) {
            client->onLED(led);
        }
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

bool EaganMatrix::set_checked_data_stream(uint8_t stream_id) {
    if (data_stream != -1) {
        broken_midi = true;
        return false;
    } else {
        data_stream = static_cast<int>(stream_id);
        return true;
    }
}

bool EaganMatrix::handle_macro_cc(uint8_t cc, uint8_t value)
{
    // 12-17
    // 40-63
    // 102-119
    if (cc < Haken::ccI || cc > Haken::ccM90) return false;

    uint16_t macro_value = (value << 7) + frac_lsb;

    if (cc <= Haken::ccVI) {
        macro[cc - Haken::ccI] = macro_value;
    } else if (cc < Haken::ccM7) {
        return false;
    } else {
        int offset = frac_hi*48;

        if (cc <= Haken::ccM30) {
            macro[Haken::idM7 + (cc - Haken::ccM7) + offset] = macro_value;
        } else if (cc >= Haken::ccM31) {
            macro[Haken::idM31 + (cc - Haken::ccM31) + offset] = macro_value;
        } else {
            return false;
        }
    }
    frac_lsb = 0;
    return true;
}

void EaganMatrix::onChannelOneCC(uint8_t cc, uint8_t value)
{
    if (Haken::ccFracIM48 == cc) {
        frac_hi = false;
        frac_lsb = value;
        return;
    }
    if (Haken::ccFracM49M90 == cc) {
        frac_hi = true;
        frac_lsb = value;
        return;
    }
    if (get_jack_1_assign() == cc) {
        assert(0 == frac_lsb);
        jack_1 = (value << 7) + frac_lsb;
    }
    if (get_jack_2_assign() == cc) {
        jack_2 = (value << 7) + frac_lsb;
    }
    if (handle_macro_cc(cc, value)) {
        return;
    }
    switch (cc) {
    // not received - case Haken:: ccJack1:
    // not received - case Haken:: ccJack2:
    case Haken::ccPost:
        post = (value << 7) + frac_lsb;
        frac_lsb = 0; 
        break;
    }
}

void EaganMatrix::onChannel16CC(uint8_t cc, uint8_t value)
{
    // The ccs ccMod, ccBreath, ccUndef, ccFoot, ccVol, ccExpres
    // can be used for pedal assignment. 
    // When assigned, the cc comes to channel 16. 
    // Other assigned ccs come to channel 1.

    if (get_jack_1_assign() == cc) {
        jack_1 = (value << 7) + ch16.macro_fraction_lo();
    }
    if (get_jack_2_assign() == cc) {
        jack_2 = (value << 7) + ch16.macro_fraction_lo();
    }

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
            if (set_checked_data_stream(value)) {
                if (!in_preset) {
                    begin_preset();
                }
            }
        } break;

        case Haken::s_ConText: {
            text_buffer.clear();
            set_checked_data_stream(value);
        } break;

        case Haken::s_Mat_Poke: {
            set_checked_data_stream(value);
            // zero mat?
        } break;

        case Haken::s_Conv_Poke: {
            set_checked_data_stream(value);
        } break;

        case Haken::s_StreamEnd: {
            if (in_preset) {
                switch (data_stream) {
                case Haken::s_Name: {
                    preset.name = name_buffer.str();
                } break;

                case Haken::s_ConText: {
                    preset.text = text_buffer.str();
                } break;

                // case Haken::s_Mat_Poke: {
                // } break;

                // case Haken::s_Conv_Poke: {
                // } break;
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
        begin_preset();
        break;

    case Haken::ccTask:
        notifyTaskMessage(value);
        switch (value) {
        case Haken::configToMidi:
            pending_config = true;
            break;
        case Haken::beginUserNames:
            in_user = true;
            notifyUserBegin();
            break;
        case Haken::endUserNames:
            in_user = false;
            notifyUserComplete();
            break;
        case Haken::endSysNames:
            in_system = false;
            notifySystemComplete();
            break;
        case Haken::beginSysNames:
            in_system = true;
            notifySystemBegin();
            break;
        case Haken::remakeSRMahl:
            if (in_mahling) { // pre-10.50
                in_mahling = false;
                notifyMahlingComplete();
            } else {
                in_mahling = true;
                notifyMahlingBegin();
            }
            break;
        case Haken::doneSRMahl:
            in_mahling = false;
            notifyMahlingComplete();
            break;
        }
        break;

    case Haken::ccEdState:
        notifyLED(value & Haken::sLedBits);
        break;

    case Haken::ccEditorReply:
        if (pending_EditorReply) {
            notifyEditorReply(value);
            pending_EditorReply = false;
        }
        break;

    case Haken::ccEditor:
        pending_EditorReply = true;
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
                    // 129 because 1-based numbering where 0 == raw/from file in edit slot
                    if (pn < 129) {
                        preset.id.set_bank_hi(Haken::catUser);
                        --pn;
                    } else {
                        pn -= 129;
                        preset.id.set_bank_hi(Haken::catSSlot);
                    }
                    preset.id.set_bank_lo((pn & 0xff80) >> 7);
                    preset.id.set_number(pn & 0x7f);
                }
                in_preset = false;
                pending_config = false;
                ready = true;
                notifyPresetChanged();
            }
            break;

        case MidiStatus_PolyKeyPressure: {
            switch (data_stream) {
            case Haken::s_Name:
                if (in_preset) {
                    name_buffer.build(msg.bytes.data1);
                    name_buffer.build(msg.bytes.data2);
                }
                break;
            case Haken::s_ConText:
                if (in_preset) {
                    text_buffer.build(msg.bytes.data1);
                    text_buffer.build(msg.bytes.data2);
                }
                break;

            case Haken::s_Mat_Poke:
                mat[msg.bytes.data1] = msg.bytes.data2;
                break;

            case Haken:: s_Conv_Poke:
                conv[msg.bytes.data1] = msg.bytes.data2;
                break;

            case -1:
            default: break;
            }
            break;
        } break;

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