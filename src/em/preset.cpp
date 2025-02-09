#include "preset.hpp"
#include "../services/misc.hpp"

namespace pachde {

json_t* PresetDescription::toJson()
{
    json_t* root = json_object();
    json_object_set_new(root, "hi", json_integer(id.bank_hi()));
    json_object_set_new(root, "lo", json_integer(id.bank_lo()));
    json_object_set_new(root, "num", json_integer(id.number()));
    json_object_set_new(root, "name", json_stringn(name.c_str(), name.size()));
    if (text.size()) {
        json_object_set_new(root, "text", json_stringn(text.c_str(), text.size()));
    }
    return root;
}

void PresetDescription::fromJson(const json_t* root)
{
    uint8_t hi = 0, lo = 0, number = 0;
    auto j = json_object_get(root, "hi");
    if (j) {
        hi = json_integer_value(j);
    }
    j = json_object_get(root, "lo");
    if (j) {
        lo = json_integer_value(j);
    }
    j = json_object_get(root, "num");
    if (j) {
        number = json_integer_value(j);
    }
    id = {hi, lo, number};
    j = json_object_get(root, "name");
    if (j) {
        name = json_string_value(j);
    }
    j = json_object_get(root, "text");
    if (j) {
        text = json_string_value(j);
    }
}

std::string PresetDescription::summary() const
{
    return format_string("[%d.%d.%d] %s", id.bank_hi(), id.bank_lo(), id.number(), name.c_str());
}

}