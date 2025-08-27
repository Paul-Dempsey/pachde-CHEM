#pragma once
#include <stdint.h>
#include "preset.hpp"
#include "preset-sort.hpp"

namespace eaganmatrix {

enum class PresetTab { Unset = -1, System, User };

inline void valid_tab(PresetTab which) { assert((PresetTab::User == which) || (PresetTab::System == which)); }

struct PresetList
{
    PresetList(const PresetList&) = delete;

    PresetOrder order{PresetOrder::Natural};
    bool modified{false};
    std::vector<std::shared_ptr<PresetInfo>> presets;
    std::string filename;
    uint8_t hardware{0};

    PresetList(){}

    ssize_t size() { return presets.size(); }
    bool empty() { return presets.empty(); }
    bool dirty() { return modified; }
    ssize_t index_of_id(PresetId id);
    ssize_t index_of_tag(uint32_t tag);
    void add(const PresetDescription* preset);
    void clear();
    bool load(const std::string& path);
    bool save();
    bool save(const std::string& path, uint8_t hardware);
    bool from_json(const json_t* root);
    void to_json(json_t* root, uint8_t hardware);
    void sort(PresetOrder order);
};

std::string preset_file_name(PresetTab which, uint8_t hardware, const std::string& device_name);

}