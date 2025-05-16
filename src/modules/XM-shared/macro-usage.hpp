#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../../services/haken-midi.hpp"
#include "../../services/misc.hpp"
#include "../../em/midi-message.h"

namespace pachde{

enum FormElement { W, X, Y, Z, Blend };

struct MacroFormUsage
{
    uint8_t formula{0xff};
    uint8_t elements{0};

    MacroFormUsage() {}
    MacroFormUsage(uint8_t f) : formula(f) {}
    MacroFormUsage(uint8_t f, uint8_t flags) : formula(f), elements(flags) {}

    void clear() {
        formula = 0xff;
        elements = 0;
    }
    void add_W() { elements |= 1; }
    void add_X() { elements |= 2; }
    void add_Y() { elements |= 4; }
    void add_Z() { elements |= 8; }
    void add_Blend() { elements |= 16; }

    std::string to_string() const;
};

struct MacroUsage
{
    uint8_t macro_number{0};
    std::vector<MacroFormUsage> forms;
    int64_t module_id{-1};

    MacroUsage() {}
    MacroUsage(uint8_t num) : macro_number(num) {}
    MacroUsage(uint8_t num, MacroFormUsage form) : macro_number(num) {
        forms.push_back(form);
    }

    void clear() {
        macro_number = 0;
        forms.clear();
    }

    std::string to_string() const;
};

struct MacroUsageBuilder
{
    ChemId client_id{ChemId::Unknown};
    uint8_t form_id{0xff};
    bool in_archive{false};
    bool in_form_poke{false};
    bool in_name{false};
    bool blend_mac{false};

    HakenMidi* haken{nullptr};
    std::function<void()> on_complete{nullptr};
    std::string preset_name;
    std::vector<MacroUsage>& macros;
    void set_on_complete(std::function<void()> fn) { on_complete = fn; }
    void do_message(PackedMidiMessage msg);
    MacroUsageBuilder(std::vector<MacroUsage>& m) : macros(m) {}
};

}