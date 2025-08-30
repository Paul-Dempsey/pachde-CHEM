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
    filename.clear();
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
    bool ok = from_json(root);
    if (ok) {
        modified = false;
        filename = path;
    }
    return ok;
}

bool PresetList::save()
{
    std::string path = filename;
    return save(filename, hardware);
}

bool PresetList::save(const std::string &path, uint8_t hardware)
{
    filename.clear();
    hardware = 0;
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
    to_json(root, hardware);
    bool ok = json_dumpf(root, file, JSON_INDENT(2)) < 0 ? false : true;
    if (ok) {
        modified = false;
        filename = path;
        this->hardware = hardware;
    }
    return ok;
}

bool PresetList::from_json(const json_t* root)
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

void PresetList::to_json(json_t* root, uint8_t hardware)
{
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

std::string EMFileId(const std::string& device_name)
{
    std::string result = device_name;
    if (0 == result.compare(0, 12, "ContinuuMini", 0, 12)) {
        result.erase(0, 8); // "Mini ..."
    } else if (0 == result.compare(0, 17, "EaganMatrix Micro")) {
        result.erase(0, 12); // "Micro ..."
    } else if (0 == result.compare(0, 18, "EaganMatrix Module")) {
        result.replace(0, 18, "EMM ...");
    } else if (std::string::npos != result.find("(Osmose)")) {
        return "Osmose";
    }
    std::replace(result.begin(), result.end(), ' ', '-');
    return result;
}

std::string preset_file_name(eaganmatrix::PresetTab which, uint8_t hardware, const std::string& device_name)
{
    //valid_tab(which);
    std::string preset_filename;
    switch (which) {
    case PresetTab::User:
        preset_filename = format_string("%s-user.json", EMFileId(device_name).c_str());
        break;
    case PresetTab::System:
        preset_filename = format_string("%s-system.json", PresetClassName(hardware));
        break;
    default:
        assert(false);
        break;
    }
    return user_plugin_asset(preset_filename);
}

}