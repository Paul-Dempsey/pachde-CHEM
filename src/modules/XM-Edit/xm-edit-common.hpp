#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../../em/wrap-HakenMidi.hpp"
#include "../../services/colors.hpp"
#include "../../services/svt_rack.hpp"

using namespace pachde;
enum MacroRange{ Bipolar, Unipolar, Custom };
const uint8_t INVALID_MACRO{0};

struct MacroDescription
{
    ssize_t index{-1};
    uint8_t macro_number{INVALID_MACRO};
    std::string name;
    MacroRange range{Bipolar};
    uint16_t min{0};
    uint16_t max{Haken::max14};
    bool modulated{false};

    void clear() { 
        index = -1;
        macro_number = INVALID_MACRO;
        name.clear();
        min = 0;
        max = Haken::max14;
        modulated = false;
    }
    bool active() { return in_range(macro_number, U8(7), U8(90)); }
    float rack_min() { return rescale(min, 0, Haken::max14, -5.f, 5.f); }
    float rack_max() { return rescale(max, 0, Haken::max14, -5.f, 5.f); }

    void from_json(json_t* root) {
        index = get_json_int(root, "index", -1);
        macro_number = get_json_int(root, "macro", INVALID_MACRO);
        name = get_json_string(root, "name");
        min = get_json_int(root,"min", 0);
        max = get_json_int(root,"max", Haken::max14);
        if ((min == 0) && max == (Haken::max14)) {
            range = MacroRange::Bipolar;
        } else if ((min == Haken::zero14) && (max == Haken::max14)) {
            range = MacroRange::Unipolar;
        } else {
            range = MacroRange::Custom;
        }
        modulated = get_json_bool(root, "modulated", false);
    }

    json_t * to_json() {
        json_t * root = json_object();
        json_object_set_new(root, "index", json_integer(index));
        json_object_set_new(root, "macro", json_integer(macro_number));
        json_object_set_new(root, "name", json_string(name.c_str()));
        json_object_set_new(root, "min", json_integer(min));
        json_object_set_new(root, "max", json_integer(max));
        json_object_set_new(root, "modulated", json_boolean(modulated));
        return root;
    }
};

struct IExtendedMacro
{
//    virtual bool has_modulation() = 0;
    virtual std::string get_title() = 0;
    virtual PackedColor get_header_color() = 0;
    virtual PackedColor get_header_text_color() = 0;
    virtual void set_header_color(PackedColor color) = 0;
    virtual void set_header_text_color(PackedColor color) = 0;
    virtual void set_header_text(std::string title) = 0;
    virtual void set_macro_edit(int index) = 0;
    virtual std::shared_ptr<MacroDescription> get_macro(int index) = 0;
    virtual void add_macro(int index) = 0;
    virtual void remove_macro(int index) = 0;
    virtual void on_macro_change(int index) = 0;
};