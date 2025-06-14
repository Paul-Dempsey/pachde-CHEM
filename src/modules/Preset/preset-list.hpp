#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "preset-common.hpp"
#include "../../em/preset.hpp"
#include "../../em/preset-sort.hpp"

namespace pachde {

struct PresetList
{
    PresetList(const PresetList&) = delete;

    PresetTab tab;
    PresetOrder order{PresetOrder::Natural};
    std::vector<std::shared_ptr<PresetInfo>> preset_list;
    std::vector<std::shared_ptr<PresetInfo>> preset_view;

    uint64_t filter_masks[5]{0};
    std::string search_query;
    bool search_name{false};
    bool search_meta{false};
    bool search_anchor{false};
    bool modified{false};
    bool filtering{false};
    bool mask_filtering{false};

    bool filtered() { return filtering; }

    void set_search_query(std::string query, bool name, bool meta, bool anchor);
    uint64_t get_filter(FilterId index) { return filter_masks[index]; }
    void set_filter(FilterId index, uint64_t mask);
    void init_filters( uint64_t* filters);
    void no_filter();

    PresetList(PresetTab id):
        tab(id),
        order(PresetTab::User == id ? PresetOrder::Natural : PresetOrder::Alpha) 
    {}

    bool empty() { return preset_list.empty(); }
    bool dirty() { return modified; }
    void set_dirty() { modified = true; }
    size_t count() { return filtering ? preset_view.size() : preset_list.size(); }
    ssize_t index_of_id(PresetId id);
    ssize_t index_of_id_unfiltered(PresetId id);
    std::vector<std::shared_ptr<PresetInfo>>* presets() { return filtering ? &preset_view : &preset_list; }

    void refresh_filter_view();

    void add(const PresetDescription* preset);

    void clear() {
        modified = false;
        preset_view.clear();
        preset_list.clear();
    }

    bool load(const std::string& path);
    bool save(const std::string& path, uint8_t hardware, const std::string& connection_info);
    bool from_json(const json_t* root, const std::string &path);
    void to_json(json_t* root, uint8_t hardware, const std::string& connection_info);
    void sort(PresetOrder order);
    std::shared_ptr<PresetInfo> nth(ssize_t which) {
        if (which < 0) which = 0;
        if (filtering) {
            return preset_view.empty() ? nullptr : preset_view[which];
        } else {
            return preset_list.empty() ? nullptr : preset_list[which];
        }
    }
};

}