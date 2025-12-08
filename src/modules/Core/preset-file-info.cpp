#include "preset-file-info.hpp"
#include "services/misc.hpp"
#include "my-plugin.hpp"

namespace pachde {

void PresetFileInfo::fromJson(const json_t* root)
{
    connection = get_json_string(root, "conn");
    file = get_json_string(root, "file");
    hardware = get_json_int(root, "hw", 0);
}

json_t* PresetFileInfo::toJson()
{
    if (empty()) return nullptr;
    json_t* root = json_object();
    json_object_set_new(root, "hw", json_integer(hardware));
    json_object_set_new(root, "file", json_string(file.c_str()));
    json_object_set_new(root, "conn", json_string(connection.c_str()));
    return root;
}

void load_pfis(std::string path, std::vector<std::shared_ptr<PresetFileInfo>>& data)
{
    data.clear();
    if (path.empty() || !system::exists(path)) return;
    FILE* file = std::fopen(path.c_str(), "rb");
	if (!file) return;

    DEFER({std::fclose(file);});
	json_error_t error;
	json_t* root = json_loadf(file, 0, &error);
	if (!root) {
		WARN("Invalid JSON at %d:%d %s in %s", error.line, error.column, error.text, path.c_str());
        return;
    }
	DEFER({json_decref(root);});
    auto jar = json_object_get(root, "items");
    if (jar) {
        json_t* jp;
        size_t index;
        json_array_foreach(jar, index, jp) {
            auto pfi = std::make_shared<PresetFileInfo>();
            pfi->fromJson(jp);
            if (!pfi->empty()) {
                data.push_back(pfi);
            }
        }
    }
}

bool save_pfis(std::string path, std::vector<std::shared_ptr<PresetFileInfo>> &data)
{
    (void)std::remove_if(data.begin(), data.end(), [](std::shared_ptr<PresetFileInfo>pfi){ return pfi->empty(); });
    if (data.empty()) {
        system::remove(path);
        return true;
    }

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
    auto jar = json_array();
    if (!jar) return false;
    for (auto pfi: data) {
        auto j = pfi->toJson();
        if (j) json_array_append_new(jar, j);
    }
    json_object_set_new(root, "items", jar);
    return json_dumpf(root, file, JSON_INDENT(2)) >= 0;
}

std::string pfis_filename()
{
    return user_plugin_asset("CHEM-device-user-lists.json");
}

}