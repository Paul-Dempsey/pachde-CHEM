#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "preset-common.hpp"
#include "../../em/preset.hpp"

namespace pachde {

struct PresetList {
    PresetTab tab;
    uint16_t firmware{0};
    uint8_t hardware{0};
    PresetOrder order{PresetOrder::Natural};
    std::vector<std::shared_ptr<PresetDescription>> presets;
    std::vector<uint16_t> index;

    PresetList(PresetTab id) : tab(id) {}

    void set_device_info(std::string id, uint16_t firmware_version) {
        firmware = firmware_version;
    }
    void add(std::shared_ptr<PresetDescription> preset) {
        presets.push_back(preset);
    }

    bool load(const std::string& path);
    bool save(const std::string& path, const std::string& connection_info);
    bool from_json(const json_t* root, const std::string &path);
    void to_json(json_t* root, const std::string& connection_info);

    std::shared_ptr<PresetDescription> nth(size_t which) {
        return index.empty() ? presets[which] : presets[index[which]];
    }
    void make_index(PresetOrder order) {
        switch (order) {
        case PresetOrder::Natural: index.clear(); break;
        case PresetOrder::Alpha:
            break;
        case PresetOrder::Category:
           break;
        }
    }
};

}