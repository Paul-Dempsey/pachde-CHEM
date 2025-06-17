#include "preset-list.hpp"
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


bool PresetList::load(const std::string &path)
{
    if (path.empty() || !system::exists(path)) return false;
    FILE* file = std::fopen(path.c_str(), "wb");
	if (!file) {
		return false;
    }
	DEFER({std::fclose(file);});
	json_error_t error;
	json_t* root = json_loadf(file, 0, &error);
	if (!root) {
		WARN("Invalid JSON at %d:%d %s in %s", error.line, error.column, error.text, path.c_str());
        return false;
    }
	DEFER({json_decref(root);});
    bool ok = from_json(root, path);
    if (ok) {
        modified = false;
    }
    return ok;
}

bool PresetList::save(const std::string &path, uint8_t hardware, const std::string& connection_info)
{
    if (path.empty()) return false;
    auto dir = system::getDirectory(path);
    system::createDirectories(dir);

    FILE* file = std::fopen(path.c_str(), "wb");
	if (!file) {
		return false;
    }
	DEFER({std::fclose(file);});
    auto root = json_object();
    if (!root) { return false; }
	DEFER({json_decref(root);});
    to_json(root, hardware, connection_info);
    bool ok = json_dumpf(root, file, JSON_INDENT(2)) < 0 ? false : true;
    if (ok) {
        modified = false;
    }
    return ok;
}

bool PresetList::from_json(const json_t* root,const std::string &path)
{
    clear();

    auto jar = json_object_get(root, "presets");
    if (jar) {
        json_t* jp;
        size_t index;
        json_array_foreach(jar, index, jp) {
            auto preset = std::make_shared<PresetInfo>();
            preset->fromJson(jp);
            preset->ensure_meta();
            preset_list.push_back(preset);
        }
    }
    if (order != PresetOrder(get_json_int(root, "order", int(order)))) {
        auto orderfn = getPresetSort(order);
        std::sort(preset_list.begin(), preset_list.end(), orderfn);
    }
    mask_filtering = any_filter(filter_masks);
    if (mask_filtering || !search_query.empty()) {
        filtering = true;
        refresh_filter_view();
    }
    return true;
}

void PresetList::to_json(json_t* root, uint8_t hardware, const std::string& connection_info)
{
    json_object_set_new(root, "connection", json_string(connection_info.c_str()));
    json_object_set_new(root, "preset-class", json_string(PresetClassName(hardware))); // human-readable
    json_object_set_new(root, "order", json_integer(int(order)));
    auto jar = json_array();
    for (auto preset: preset_list) {
        json_array_append_new(jar, preset->toJson(true, true, true));
    }
    json_object_set_new(root, "presets", jar);
}

void PresetList::set_filter(FilterId index, uint64_t mask)
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

void PresetList::init_filters(uint64_t *filters)
{
    std::memcpy(filter_masks, filters, sizeof(filter_masks));
    mask_filtering = any_filter(filter_masks);
    filtering = mask_filtering || !search_query.empty();
}

void PresetList::no_filter()
{
    if (filtering) {
        filtering = false;
        std::memset(filter_masks, 0, sizeof(filter_masks));
        search_query = "";
        preset_view.clear();
    }
}

void PresetList::set_search_query(std::string query, bool name, bool meta, bool anchor)
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

void PresetList::refresh_filter_view()
{
    preset_view.clear(); 
    if (filtering) {
        auto inserter = std::back_inserter(preset_view);
        for (auto p: preset_list) {
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

ssize_t PresetList::index_of_id(PresetId id)
{
    if (!id.valid() || empty()) return -1;

    auto key = id.key();
    auto list{presets()};
    auto it = std::find_if(list->cbegin(), list->cend(), [key](const std::shared_ptr<PresetInfo> p){ return key == p->id.key(); });
    if (it == list->cend()) return -1;
    return it - list->cbegin();
}

ssize_t PresetList::index_of_id_unfiltered(PresetId id)
{
    if (empty()) return -1;
    auto key = id.key();
    auto it = std::find_if(preset_list.cbegin(), preset_list.cend(), [key](const std::shared_ptr<PresetInfo> p){ return key == p->id.key(); });
    if (it == preset_list.cend()) return -1;
    return it - preset_list.cbegin();
}

void PresetList::add(const PresetDescription* preset)
{
    auto index = index_of_id(preset->id);
    if (-1 == index) {
        auto pi = std::make_shared<PresetInfo>(preset);
        preset_list.push_back(pi);
        modified = true;
    }
}

void PresetList::sort(PresetOrder order)
{
    if (this->order != order) {
        this->order = order;
        auto orderfn = getPresetSort(order);
        std::sort(preset_list.begin(), preset_list.end(), orderfn);
        if (filtered()) {
            std::sort(preset_view.begin(), preset_view.end(), orderfn);
        }
        modified = true;
    }
}



}