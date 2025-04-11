#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "preset-common.hpp"
#include "../../em/preset.hpp"

namespace pachde {

struct PresetList {
    PresetList(const PresetList&) = delete;

    PresetTab tab;
    uint16_t firmware{0};
    uint8_t hardware{0};
    PresetOrder order{PresetOrder::Natural};
    std::vector<std::shared_ptr<PresetDescription>> presets;

    PresetList(PresetTab id): tab(id) {}

    bool empty() { return presets.empty(); }
    size_t count() { return presets.size(); }

    void set_device_info(uint16_t firmware_version, uint8_t hardware_type) {
        firmware = firmware_version;
        hardware = hardware_type;
    }
    void add(std::shared_ptr<PresetDescription> preset) {
        presets.push_back(preset);
    }
    void clear() {
        firmware = 0;
        hardware = 0;
        presets.clear();
    }
    bool load(const std::string& path);
    bool save(const std::string& path, const std::string& connection_info);
    bool from_json(const json_t* root, const std::string &path);
    void to_json(json_t* root, const std::string& connection_info);

    std::shared_ptr<PresetDescription> nth(ssize_t which) {
        return presets[which];
    }
};

}