#pragma once
#include "preset.hpp"
#include "preset-sort.hpp"

namespace eaganmatrix {

struct PresetList
{
    PresetList(const PresetList&) = delete;
    
    PresetOrder order{PresetOrder::Natural};
    std::vector<std::shared_ptr<PresetInfo>> presets;
    bool modified{false};
    
    PresetList(){}

    ssize_t size() { return presets.size(); }
    bool empty() { return presets.empty(); }
    bool dirty() { return modified; }
    ssize_t index_of_id(PresetId id);
    ssize_t index_of_tag(uint32_t tag);
    void add(const PresetDescription* preset);
    void clear();
    bool load(const std::string& path);
    bool save(const std::string& path, uint8_t hardware, const std::string& connection_info);
    bool from_json(const json_t* root, const std::string &path);
    void to_json(json_t* root, uint8_t hardware, const std::string& connection_info);
    void sort(PresetOrder order);
};

std::string preset_file_name(bool user, uint8_t hardware);

}