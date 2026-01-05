#include "Play.hpp"
#include <ghc/filesystem.hpp>
#include "services/open-file.hpp"

namespace fs = ghc::filesystem;
using namespace pachde;
const char * file_dialog_filter = "Playlists (.json):json;Any (*):*";
bool PlayUi::load_playlist(std::string path, bool set_folder)
{
    close_playlist();
    DEFER({sync_to_presets();});

    FILE* file = std::fopen(path.c_str(), "r");
	if (!file) {
		return false;
    }
	DEFER({std::fclose(file);});
	json_error_t error;
	json_t* root = json_loadf(file, 0, &error);
	if (!root) {
		WARN("Invalid JSON at %d:%d %s in %s", error.line, error.column, error.text, path.c_str());
        sync_to_presets();
        return false;
    }
	DEFER({json_decref(root);});
    presets_from_json(root);

    set_modified(false);
    if (set_folder) {
        my_module->playlist_folder = system::getDirectory(path);
    }
    my_module->playlist_file = path;
    my_module->update_mru(path);

    playlist_name = system::getStem(path);
    playlist_label->set_text(playlist_name);
    std::string tip = my_module->playlist_file;
    if (!playlist_device.empty()) {
        tip.append("\n- for ");
        tip.append(playlist_device);
        playlist_label->describe(tip);
    }
    check_playlist_device();
    return true;
}

void PlayUi::open_playlist()
{
    if (!my_module) return;
    std::string path;
    if (modified && module && !my_module->playlist_file.empty()) {
        save_playlist();
    }
    if (my_module->playlist_folder.empty()) {
        auto kv = get_plugin_kv_store();
        if (kv && kv->load()) {
            my_module->playlist_folder = kv->lookup("playlist-folder");
        }
    }
    bool ok = openFileDialog(my_module->playlist_folder, file_dialog_filter, playlist_name, path);
    if (ok) {
        load_playlist(path, true);
    } else {
        playlist_name = "";
        playlist_device = "";
        my_module->playlist_file = "";
        set_modified(false);
    }
}

void PlayUi::close_playlist()
{
    clear_playlist(true);
    set_modified(false);
}

void PlayUi::save_playlist()
{
    if (!my_module) return;
    if (my_module->playlist_file.empty()) {
        save_as_playlist();
        return;
    }
    auto dir = system::getDirectory(my_module->playlist_file);
    system::createDirectories(dir);

    auto root = json_object();
    if (!root) return;
    DEFER({json_decref(root);});

    if (playlist_device.empty()) {
        if (chem_host) {
            auto em = chem_host->host_matrix();
            if (em) {
                playlist_device = PresetClassName(em->hardware);
            }
        }
    }
    presets_to_json(root);

    std::string tmpPath = system::join(dir, TempName(".tmp.json"));
    FILE* file = std::fopen(tmpPath.c_str(), "w");
    if (!file) {
        system::remove(tmpPath);
        return;
    }
	json_dumpf(root, file, JSON_INDENT(2));
	std::fclose(file);
    system::sleep(0.0005);
	system::remove(my_module->playlist_file);
    system::sleep(0.0005);
	system::rename(tmpPath, my_module->playlist_file);
    set_modified(false);
}

void PlayUi::save_as_playlist()
{
    if (!my_module) return;
    std::string path;
    bool ok = saveFileDialog(my_module->playlist_folder, file_dialog_filter, playlist_name, path);
    if (ok) {
        my_module->playlist_folder = system::getDirectory(path);
        auto ext = system::getExtension(path);
        if (ext.empty()) {
            path.append(".json");
        }
        my_module->playlist_file = path;
        if (playlist_device.empty()) {
            if (chem_host) {
                auto em = chem_host->host_matrix();
                if (em) {
                    playlist_device = PresetClassName(em->hardware);
                }
            }
        }
        playlist_name = system::getStem(path);
        playlist_label->set_text(playlist_name);

        auto tip = my_module->playlist_file;
        if (!playlist_device.empty()) {
            tip.append("\n- for ");
            tip.append(playlist_device);
        }
        playlist_label->describe(tip);

        save_playlist();
        my_module->update_mru(path);
        set_modified(false);
    }
}

void PlayUi::clear_playlist(bool forget_file)
{
    warning_label->set_text("");
    if (!my_module) return;
    for (auto pw : preset_widgets) {
        pw->clear_preset();
    }
    if (forget_file) {
        my_module->playlist_file = "";
        playlist_name = "";
        playlist_label->set_text("");
        playlist_label->describe("");
        playlist_device = "";
        set_modified(false);
    }
    presets.clear();
    clear_selected();
    sync_to_presets();
    if (!forget_file) set_modified(true);
}
