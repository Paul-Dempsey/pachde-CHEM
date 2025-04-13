#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "preset-common.hpp"
#include "../../em/preset.hpp"
#include "../../em/preset-sort.hpp"

namespace pachde {

struct PresetList {
    PresetList(const PresetList&) = delete;

    PresetTab tab;
    PresetOrder order{PresetOrder::Natural};
    uint16_t firmware{0};
    uint8_t hardware{0};
    std::vector<std::shared_ptr<PresetInfo>> presets;

    PresetList(PresetTab id):
        tab(id),
        order(PresetTab::User == id ? PresetOrder::Natural : PresetOrder::Alpha) 
    {}

    bool empty() { return presets.empty(); }
    size_t count() { return presets.size(); }

    void set_device_info(uint16_t firmware_version, uint8_t hardware_type) {
        firmware = firmware_version;
        hardware = hardware_type;
    }
    void add(std::shared_ptr<PresetDescription> preset) {
        auto pi = std::make_shared<PresetInfo>(preset);
        presets.push_back(pi);
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
    void sort(PresetOrder order);
    std::shared_ptr<PresetInfo> nth(ssize_t which) {
        return presets[which];
    }
};

}