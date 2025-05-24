#include "midi-pad.hpp"
#include "../../services/misc.hpp"
#include "../../services/colors.hpp"

namespace pachde {


struct HclCompiler {
    bool ok{false};
    std::vector<PackedMidiMessage> midi;
    std::string error_message;

    bool compile(const std::string& program);
};

bool HclCompiler::compile(const std::string& program)
{
    if (program.empty()) {
        error_message = "Midi definition is empty.";
        return false;
    }
    return true;
}


static const char * default_pad_name[] = {
    "A1", "A2", "A3", "A4",
    "B1", "B2", "B3", "B4",
    "C1", "C2", "C3", "C4",
    "D1", "D2", "D3", "D4"
};

MidiPad::MidiPad(int id) : id(id), color(0xff8c8c8c)
{
    name = default_pad_name[id];
}

MidiPad::MidiPad(json_t *j)
{
    from_json(j);
}

bool MidiPad::compile()
{
    return true;
}

json_t * MidiPad::to_json()
{
    json_t * root = json_object();
    json_object_set_new(root, "pad", json_integer(id));
    json_object_set_new(root, "name", json_string(name.c_str()));
    json_object_set_new(root, "midi", json_string(def.c_str()));
    json_object_set_new(root, "color", json_string(hex_string(color).c_str()));
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

    //compile();
}



}