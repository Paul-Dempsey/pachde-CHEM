#include "preset-list.hpp"
#include "../plugin.hpp"
#include "../services/misc.hpp"
#include "em-hardware.h"

using namespace pachde;
namespace eaganmatrix {

void PresetList::add(const PresetDescription* preset)
{
    if (preset->empty()) {
        assert(false);
        return;
    }
    assert(preset->id.key() != 0);
    auto index = index_of_id(preset->id);
    if (-1 == index) {
        auto pi = std::make_shared<PresetInfo>(preset);
        presets.push_back(pi);
        modified = true;
    } else {
        presets[index]->init(preset);
    }
}

ssize_t PresetList::index_of_tag(uint32_t tag)
{
    if (!tag || empty()) return -1;

    auto it = std::find_if(presets.cbegin(), presets.cend(), [tag](const std::shared_ptr<PresetInfo> p){ return tag == p->tag; });
    if (it == presets.cend()) return -1;
    return it - presets.cbegin();
}

ssize_t PresetList::index_of_id(PresetId id)
{
    if (!id.valid() || empty()) return -1;

    auto key = id.key();
    auto it = std::find_if(presets.cbegin(), presets.cend(), [key](const std::shared_ptr<PresetInfo> p){ return key == p->id.key(); });
    if (it == presets.cend()) return -1;
    return it - presets.cbegin();
}

void PresetList::clear()
{
    modified = false;
    presets.clear();
}

bool PresetList::load(const std::string &path)
{
    if (path.empty() || !system::exists(path)) return false;
    FILE* file = std::fopen(path.c_str(), "rb");
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

    order = PresetOrder(get_json_int(root, "order", int(order)));
    auto jar = json_object_get(root, "presets");
    if (jar) {
        json_t* jp;
        size_t index;
        json_array_foreach(jar, index, jp) {
            auto preset = std::make_shared<PresetInfo>();
            preset->fromJson(jp);
            preset->ensure_meta();
            presets.push_back(preset);
        }
    }
    return true;
}

void PresetList::to_json(json_t* root, uint8_t hardware, const std::string& connection_info)
{
    json_object_set_new(root, "connection", json_string(connection_info.c_str()));
    json_object_set_new(root, "haken-device", json_string(PresetClassName(hardware))); // human-readable
    json_object_set_new(root, "order", json_integer(int(order)));
    auto jar = json_array();
    for (auto preset: presets) {
        json_array_append_new(jar, preset->toJson(true, true, true));
    }
    json_object_set_new(root, "presets", jar);
}

void PresetList::sort(PresetOrder new_order)
{
    if (modified || (order != new_order)) {
        order = new_order;
        if (PresetOrder::None != order) {
            auto orderfn = getPresetSort(new_order);
            std::sort(presets.begin(), presets.end(), orderfn);
        }
        modified = true;
    }
}

std::string preset_file_name(eaganmatrix::PresetTab which, uint8_t hardware)
{
    valid_tab(which);
    auto preset_filename = format_string("%s-%s.json", PresetClassName(hardware), (eaganmatrix::PresetTab::User == which) ? "user": "system");
    return user_plugin_asset(preset_filename);
}

}