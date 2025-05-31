#include "midi-pad.hpp"
#include "../../services/misc.hpp"
#include "../../services/colors.hpp"
#include "hcl.hpp"

namespace pachde {

// See ../../../doc/hcl.md

static const char * default_pad_name[] = {
    "A1", "A2", "A3", "A4",
    "B1", "B2", "B3", "B4",
    "C1", "C2", "C3", "C4",
    "D1", "D2", "D3", "D4"
};

MidiPad::MidiPad(int id) : id(id), color(0xff8c8c8c), text_color(0xff000000)
{
    name = default_pad_name[id];
}

MidiPad::MidiPad(json_t *j)
{
    from_json(j);
}

bool MidiPad::compile()
{
    HclCompiler hc;
    if (hc.compile(this->def, &this->midi)) {
        ok = true;
        error_message = "";
        error_pos = 0;
        return true;
    }
    ok = false;
    error_message = hc.error_message;
    error_pos = hc.error_pos;
    return false;
}

json_t * MidiPad::to_json()
{
    json_t * root = json_object();
    json_object_set_new(root, "pad", json_integer(id));
    json_object_set_new(root, "name", json_string(name.c_str()));
    json_object_set_new(root, "midi", json_string(def.c_str()));
    json_object_set_new(root, "color", json_string(hex_string(color).c_str()));
    json_object_set_new(root, "text-color", json_string(hex_string(text_color).c_str()));
    return root;
}

void MidiPad::from_json(json_t* root)
{
    id = get_json_int(root, "pad", -1);
    def = get_json_string(root, "midi");
    name = get_json_string(root, "name");
    color = parse_color(get_json_string(root, "color"), RARE_COLOR);
    if (color == RARE_COLOR) {
        color = 0xff8c8c8c;
    }
    text_color = parse_color(get_json_string(root, "text-color"), RARE_COLOR);
    if (text_color == RARE_COLOR) {
        text_color = 0xff000000;
    }

    //compile();
}



}