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
    return from_json(root, path);
}

bool PresetList::save(const std::string &path, const std::string& connection_info)
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
    to_json(root, connection_info);
    return json_dumpf(root, file, JSON_INDENT(2)) < 0 ? false : true;
}

bool PresetList::from_json(const json_t* root,const std::string &path)
{
    presets.clear();

    uint16_t ver = get_json_int(root, "firmware", 0);
    auto hw = get_json_int(root, "hardware", 0);
    if ((ver != firmware) || (hw != hardware)) {
		WARN("Mismatched hardware/firmware in %s", path.c_str());
        return false;
    }

    auto jar = json_object_get(root, "presets");
    if (jar) {
        json_t* jp;
        size_t index;
        json_array_foreach(jar, index, jp) {
            auto preset = std::make_shared<PresetDescription>();
            preset->fromJson(jp);
            presets.push_back(std::make_shared<PresetInfo>(preset));
        }
    }
    if (order != PresetOrder(get_json_int(root, "order", int(order)))) {
        auto orderfn = getPresetSort(order);
        std::sort(presets.begin(), presets.end(), orderfn);
    }
    return true;
}

void PresetList::to_json(json_t* root, const std::string& connection_info)
{
    json_object_set_new(root, "hardware", json_integer(hardware));
    json_object_set_new(root, "firmware", json_integer(firmware));
    json_object_set_new(root, "connection", json_string(connection_info.c_str()));
    json_object_set_new(root, "preset-class", json_string(PresetClassName(hardware))); // human-readable
    json_object_set_new(root, "order", json_integer(int(order)));
    auto jar = json_array();
    for (auto preset: presets) {
        json_array_append_new(jar, preset->toJson(true, true, true));
    }
    json_object_set_new(root, "presets", jar);
}

ssize_t PresetList::index_of_id(PresetId id)
{
    if (presets.empty()) return -1;
    auto key = id.key();
    auto it = std::find_if(presets.cbegin(), presets.cend(), [key](const std::shared_ptr<pachde::PresetInfo> p){ return key == p->id.key(); });
    if (it == presets.cend()) return -1;
    return it - presets.cbegin();
}

void PresetList::add(const PresetDescription* preset)
{
    auto index = index_of_id(preset->id);
    if (-1 == index) {
        auto pi = std::make_shared<PresetInfo>(preset);
        presets.push_back(pi);
    }
}

void PresetList::sort(PresetOrder order)
{
    if (this->order != order) {
        this->order = order;
        auto orderfn = getPresetSort(order);
        std::sort(presets.begin(), presets.end(), orderfn);
    }
}

}