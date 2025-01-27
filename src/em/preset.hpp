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

    PresetDescription() {}

    void clear() {
        id.clear();
        name.clear();
        text.clear();
    }

    json_t* toJson();
    void fromJson(const json_t* root);
};

}