#pragma once
#include "PresetId.hpp"

namespace pachde {

struct PresetDescription
{
    PresetId id;
    std::string name;
    std::string text;

    PresetDescription(const PresetDescription & preset) = delete; // no copy constructor

    json_t* toJson();
    void fromJson(const json_t* root);
};

}