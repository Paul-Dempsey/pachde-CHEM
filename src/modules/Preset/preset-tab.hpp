#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "preset-common.hpp"
#include "../../em/preset.hpp"
#include "../../em/preset-sort.hpp"
#include "../../em/preset-list.hpp"
using namespace eaganmatrix;

namespace pachde {

struct PresetTabList
{
    PresetTabList(const PresetTabList&) = delete;

    PresetTab tab;
    std::shared_ptr<PresetList> preset_list{nullptr};
    std::vector<std::shared_ptr<PresetInfo>> preset_view;

    uint64_t filter_masks[5]{0};
    std::string search_query;
    bool search_name{false};
    bool search_meta{false};
    bool search_anchor{false};
    bool filtering{false};
    bool mask_filtering{false};

    bool filtered() { return filtering; }

    void set_search_query(std::string query, bool name, bool meta, bool anchor);
    uint64_t get_filter(FilterId index) { return filter_masks[index]; }
    void set_filter(FilterId index, uint64_t mask);
    void init_filters( uint64_t* filters);
    void no_filter();

    PresetTabList(PresetTab id):
        tab(id)
    {
    }
    void set_list(std::shared_ptr<PresetList> list);
    bool empty() { return preset_list ? preset_list->empty() : true; }
    bool dirty() { return preset_list ? preset_list->modified : false; }
    void set_dirty() { assert(preset_list); preset_list->modified = true; }
    size_t count() { return filtering ? preset_view.size() : (preset_list ? preset_list->size() : 0); }
    ssize_t index_of_id(PresetId id);
    ssize_t index_of_id_unfiltered(PresetId id);
    std::vector<std::shared_ptr<PresetInfo>>* presets() { return filtering ? &preset_view : (preset_list ? &preset_list->presets : nullptr); }

    void refresh_filter_view();

    void add(const PresetDescription* preset);

    void clear() {
        preset_view.clear();
        preset_list = nullptr;
    }

    bool save();
    void sort(PresetOrder order);
    std::shared_ptr<PresetInfo> nth(ssize_t which) {
        if (which < 0) which = 0;
        if (filtering) {
            return preset_view.empty() ? nullptr : preset_view[which];
        } else {
            return !preset_list || preset_list->empty() ? nullptr : preset_list->presets[which];
        }
    }
};

}