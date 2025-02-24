#pragma once
#include <rack.hpp>
#include "PresetId.hpp"
using namespace ::rack;

namespace pachde {

struct PresetDescription
{
    PresetId id;
    std::string name;
    std::string text;

    PresetDescription() {
        id.invalidate();
        name.reserve(32);
        text.reserve(256);
    }

    PresetDescription(PresetId id, std::string name, std::string text)
    : id(id), name(name), text(text)
    {}

    void init(const PresetDescription* source) {
        if (source) {
            id = source->id;
            name = source->name;
            text = source->text;
        } else {
            clear();
        }
    }

    bool empty() const {
        return !id.valid() || name.empty() || name == "-";
    }

    void clear() {
        id.clear();
        name.clear();
        text.clear();
    }

    json_t* toJson();
    void fromJson(const json_t* root);
    std::string summary() const;

};


}