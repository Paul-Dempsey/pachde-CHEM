#include "macro-usage.hpp"
#include "em/wrap-HakenMidi.hpp"
#include "chem-id.hpp"

namespace pachde {

std::string MacroFormUsage::to_string() const
{
    std::string result;
    if (formula < 45) {
        char f = 'A' + ((formula < 23) ? formula-1 : formula-23);
        result.push_back(f);
        if (formula > 22) result.push_back('\'');
        if (elements) {
            result.append(" (");
            if (elements & 1) result.push_back('W');
            if (elements & 2) result.push_back('X');
            if (elements & 4) result.push_back('Y');
            if (elements & 8) result.push_back('Y');
            if (elements & 16) result.append("blend");
            result.push_back(')');
            }
    } else return "?";
    return result;
}

std::string MacroUsage::to_string() const
{
    std::string result = format_string("Macro %d", macro_number);
    if (!forms.empty()) {
        result.append(": ");
        bool prev = false;
        for (const MacroFormUsage& m: forms)
        {
            if (prev) {
                result.append(", ");
            }
            result.append(m.to_string());
            prev = true;
        }
    }
    return result;
}

bool mu_order(const MacroUsage& a, const MacroUsage& b)
{
    return a.macro_number < b.macro_number;
}

void MacroUsageBuilder::request_macros(HakenMidi* haken)
{
    assert(haken);
    in_request = true;
    macros.clear();
    haken->request_archive_0(client_id);
}

void MacroUsageBuilder::do_message(PackedMidiMessage msg)
{
    if (as_u8(ChemId::Haken) != msg.bytes.tag) return;

    switch (msg.bytes.status_byte) {
    case Haken::ccStat1:
        in_form_poke = false;
        form_id = 0xff;
        break;

    case PKP16:
        if (in_name) {
            preset_name.push_back(msg.bytes.data1);
            preset_name.push_back(msg.bytes.data2);
        } else if (in_form_poke) {
            MacroFormUsage form{form_id, 0};

            switch (msg.bytes.data1) {
            case Haken::idSWMacr: form.add_W(); break;
            case Haken::idXMacr: form.add_X(); break;
            case Haken::idYMacr: form.add_Y(); break;
            case Haken::idZMacr: form.add_Z(); break;
            case Haken::idBlend:
                blend_mac = msg.bytes.data2 == Haken::psMacro;
                break;
            case Haken::idBlMacForm:
                if (blend_mac) {
                    form.add_Blend();
                }
                break;
            }
            if (form.elements) {
                auto number = msg.bytes.data2;
                auto it = std::find_if(macros.begin(), macros.end(), [number](MacroUsage& mu){
                    return mu.macro_number == number;
                });
                if (it == macros.end()) {
                    macros.push_back(MacroUsage{number, form});
                } else {
                    bool found{false};
                    for (MacroFormUsage& fu : it->forms) {
                        if (form.formula == fu.formula) {
                            form.elements |= form.elements;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        it->forms.push_back(form);
                    }
                }
            }
        }
        break;

    case Haken::ccStat16:
        switch (midi_cc(msg)) {
        case Haken::ccDInfo:
            switch (midi_cc_value(msg)) {
            case Haken::edRecordArchive:
                macros.clear();
                in_archive = true;
                break;

            case Haken::archiveEof:
                break;

            case Haken::archiveToFile:
                in_archive = false;
                in_request = false;
                std::sort(macros.begin(), macros.end(), mu_order);
                if (on_complete) on_complete();
                break;
            }
            break;

        case Haken::ccStream:
            switch (midi_cc_value(msg)) {
                case Haken::s_Name:
                    preset_name.clear();
                    in_name = true;
                    break;
                case Haken::s_Form_Poke:
                    in_form_poke = true;
                    blend_mac = false;
                    break;
                case Haken::s_StreamEnd:
                    in_form_poke = false;
                    in_name = false;
                    break;
                default:
                    in_name = false;
                    in_form_poke = false;
                    break;
            }
            break;

        case Haken::ccFormSel:
            form_id = midi_cc_value(msg);
            break;
        }
        break;

    default: break;
    }

}

}
