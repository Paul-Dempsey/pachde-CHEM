#include "preset-tab.hpp"
#include "../../services/misc.hpp"
#include "../../em/em-hardware.h"
#include "../../em/preset-meta.hpp"

namespace pachde {

inline bool is_break_char(char c)
{
    if ('_' == c) return true;
    if ('.' == c) return true;
    if ('=' == c) return true;
    return std::isspace(c);
}

std::size_t common_prefix_length_insensitive(
    std::string::const_iterator a, std::string::const_iterator end_a,
    std::string::const_iterator b, std::string::const_iterator end_b
)
{
    int common = 0;
    for (; ((a != end_a) && (b != end_b)) && ((*a == *b) || (std::tolower(*a) == std::tolower(*b)));
        ++a, ++b, ++common) {
        // nothing
    }
    return common;
}

bool search_match(const std::string &query, const std::string &text, bool anchor)
{
    if (text.empty()) return query.empty() ? true : false;
    auto scan = text.cbegin();
    auto terminus = text.cend();
    auto q = query.cbegin();
    if (!anchor) {
        while (scan != terminus) {
            if ((*scan == *q) || (std::tolower(*scan) == std::tolower(*q))) {
                if (query.size() == common_prefix_length_insensitive(q, query.cend(), scan, text.cend())) {
                    return true;
                }
            }
            scan++;
        }
        return false;
    }
    while (scan != terminus) {
        if (scan == text.cbegin() || is_break_char(*(scan - 1))) {
            if ((*scan == *q) || (std::tolower(*scan) == std::tolower(*q))) {
                if (query.size() == common_prefix_length_insensitive(q, query.cend(), scan, text.cend())) {
                    return true;
                }
            }
        }
        scan++;
    }
    return false;

}

bool PresetTabList::save()
{
    return preset_list->save();
}

void PresetTabList::set_filter(FilterId index, uint64_t mask)
{
    if (filter_masks[index] != mask) {
        filter_masks[index] = mask;
        if (mask) {
            mask_filtering = filtering = true;
        } else {
            mask_filtering = any_filter(filter_masks);
            filtering = mask_filtering || !search_query.empty();
        }
        refresh_filter_view();
    }
}

void PresetTabList::init_filters(uint64_t *filters)
{
    std::memcpy(filter_masks, filters, sizeof(filter_masks));
    mask_filtering = any_filter(filter_masks);
    filtering = mask_filtering || !search_query.empty();
}

void PresetTabList::no_filter()
{
    if (filtering) {
        filtering = false;
        std::memset(filter_masks, 0, sizeof(filter_masks));
        search_query = "";
        preset_view.clear();
    }
}

void PresetTabList::set_search_query(std::string query, bool name, bool meta, bool anchor)
{
    search_query = query;
    search_name = name;
    search_meta = meta;
    search_anchor = anchor;
    filtering = mask_filtering || !search_query.empty();
    if (!filtering) {
        preset_view.clear();
    } else {
        refresh_filter_view();
    }
}

inline bool zip_any_filter(uint64_t* a, uint64_t* b)
{
    if (bool(*a) && !bool(*a & *b)) return false;

    a++; b++;
    if (bool(*a) && !bool(*a & *b)) return false;

    a++; b++;
    if (bool(*a) && !bool(*a & *b)) return false;

    a++; b++;
    if (bool(*a) && !bool(*a & *b)) return false;

    a++; b++;
    if (bool(*a) && !bool(*a & *b)) return false;

    return true;
}

void PresetTabList::refresh_filter_view()
{
    preset_view.clear();
    if (filtering && preset_list) {
        auto inserter = std::back_inserter(preset_view);
        for (auto p: preset_list->presets) {
            bool match{true};
            if (mask_filtering) {
                uint64_t preset_masks[5];
                FillMetaCodeMasks(p->meta, preset_masks);
                match = zip_any_filter(filter_masks, preset_masks);
            }
            if (match && !search_query.empty()) {
                if (search_name) {
                    match = search_match(search_query, p->name, search_anchor);
                }
                if (!match && search_meta) {
                    match = search_match(search_query, p->text, search_anchor);
                }
            }
            if (match) {
                *inserter++ = p;
            }
        }
    }
}

void PresetTabList::set_list(std::shared_ptr<PresetList> list)
{
    if (list != preset_list) {
        preset_list = list;
        refresh_filter_view();
    }
}

ssize_t PresetTabList::index_of_id(PresetId id)
{
    if (!id.valid() || empty()) return -1;

    auto key = id.key();
    auto list{presets()};
    if (!list) return -1;
    auto it = std::find_if(list->cbegin(), list->cend(), [key](const std::shared_ptr<PresetInfo> p){ return key == p->id.key(); });
    if (it == list->cend()) return -1;
    return it - list->cbegin();
}

ssize_t PresetTabList::index_of_id_unfiltered(PresetId id)
{
    if (!preset_list) return -1;
    return preset_list->index_of_id(id);
}

void PresetTabList::add(const PresetDescription* preset)
{
    if (!preset_list) return;
    preset_list->add(preset);
}

void PresetTabList::sort(PresetOrder order)
{
    if (!preset_list) return;
    preset_list->sort(order);
    if (filtered()) {
        auto orderfn = getPresetSort(order);
        std::sort(preset_view.begin(), preset_view.end(), orderfn);
    }
}



}