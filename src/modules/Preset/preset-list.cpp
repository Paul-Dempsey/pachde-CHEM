#include "preset-list.hpp"
#include "../../services/misc.hpp"
#include "../../em/em-hardware.h"

namespace pachde {

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

    // uint16_t ver = get_json_int(root, "firmware", 0);
    // auto hw = get_json_int(root, "hardware", 0);
    // if ((ver != firmware) || (hw != hardware)) {
	// 	WARN("Mismatched hardware/firmware in %s L(%d:%d) /J(%d:%d)", system::getStem(path).c_str(), hardware, firmware, hw, ver);
    //     return false;
    // }

    auto jar = json_object_get(root, "presets");
    if (jar) {
        json_t* jp;
        size_t index;
        json_array_foreach(jar, index, jp) {
            auto preset = std::make_shared<PresetDescription>();
            preset->fromJson(jp);
            preset_list.push_back(std::make_shared<PresetInfo>(preset));
        }
    }
    if (order != PresetOrder(get_json_int(root, "order", int(order)))) {
        auto orderfn = getPresetSort(order);
        std::sort(preset_list.begin(), preset_list.end(), orderfn);
    }
    if (any_filter(filter_masks)) {
        filtering = true;
        refresh_filter_view();
    }
    return true;
}

void PresetList::to_json(json_t* root, uint8_t hardware, const std::string& connection_info)
{
    // json_object_set_new(root, "hardware", json_integer(hardware));
    // json_object_set_new(root, "firmware", json_integer(firmware));
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
            filtering = true;
        } else {
            filtering = any_filter(filter_masks);
        }
        refresh_filter_view();
    }
}

void PresetList::init_filters(uint64_t *filters)
{
    no_filter();
    std::memcpy(filter_masks, filters, sizeof(filter_masks));
    filtering = any_filter(filter_masks);
}

void PresetList::no_filter()
{
    if (filtering) {
        filtering = false;
        std::memset(filter_masks, 0, sizeof(filter_masks));
        preset_view.clear();
    }
}

inline bool zip_any_filter(uint64_t* a, uint64_t* b)
{
    if (*a++ & *b++) return true;
    if (*a++ & *b++) return true;
    if (*a++ & *b++) return true;
    if (*a++ & *b++) return true;
    if (*a++ & *b++) return true;
    return false;
}

void PresetList::refresh_filter_view()
{
    preset_view.clear(); 
    if (filtering) {
        auto inserter = std::back_inserter(preset_view);
        for (auto p: preset_list) {
            uint64_t preset_masks[5];
            FillMetaCodeMasks(p->meta, preset_masks);
            if (zip_any_filter(filter_masks, preset_masks)) {
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
    auto it = std::find_if(list->cbegin(), list->cend(), [key](const std::shared_ptr<pachde::PresetInfo> p){ return key == p->id.key(); });
    if (it == list->cend()) return -1;
    return it - list->cbegin();
}

ssize_t PresetList::index_of_id_unfiltered(PresetId id)
{
    if (empty()) return -1;
    auto key = id.key();
    auto it = std::find_if(preset_list.cbegin(), preset_list.cend(), [key](const std::shared_ptr<pachde::PresetInfo> p){ return key == p->id.key(); });
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