#include "Play.hpp"
#include <ghc/filesystem.hpp>

namespace fs = ghc::filesystem;
using namespace pachde;

PlayModule::PlayModule()
:   track_live(true)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    configInput(IN_PRESET_PREV, "Previous trigger");
    configInput(IN_PRESET_NEXT, "Next trigger");
}

void PlayModule::update_mru(std::string path)
{
    auto it = std::find(playlist_mru.begin(), playlist_mru.end(), path);
    if (it != playlist_mru.end()) {
        playlist_mru.erase(it);
    }
    playlist_mru.push_front(path);
}

void PlayModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    json_read_string(root, "haken-device", device_claim);
    json_read_string(root, "playlist-folder", playlist_folder);
    json_read_string(root, "playlist-file", playlist_file);
    json_read_bool(root, "track-live", track_live);

    playlist_mru.clear();
    auto jar = json_object_get(root, "history");
    if (jar) {
        json_t* jp;
        size_t index;
        json_array_foreach(jar, index, jp) {
            auto path = json_string_value(jp);
            if (system::exists(path)) {
                playlist_mru.push_back(path);
            }
        }
    }

    ModuleBroker::get()->try_bind_client(this);
}

json_t* PlayModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    json_object_set_new(root, "track-live", json_boolean(track_live));
    json_object_set_new(root, "playlist-folder", json_string(playlist_folder.c_str()));
    if (!playlist_folder.empty()) {
        auto kv = get_plugin_kv_store();
        if (kv && kv->load()) {
            if (kv->lookup("playlist-folder").empty()) {
                kv->update("playlist-folder", playlist_folder);
                kv->save();
            }
        }
    }
    json_object_set_new(root, "playlist-file", json_string(playlist_file.c_str()));

    auto jaru = json_array();
    int count = 0;
    for (auto path: playlist_mru) {
        if (system::exists(path)) {
            json_array_append_new(jaru, json_string(path.c_str()));
            if (++count > 20) break;
        }
    }
    json_object_set_new(root, "history", jaru);

    return root;
}

void PlayModule::onRandomize()
{
    if (chem_ui) {
        ui()->select_random();
    }
}

// IChemClient
::rack::engine::Module* PlayModule::client_module() { return this; }
std::string PlayModule::client_claim() { return device_claim; }

void PlayModule::onConnectHost(IChemHost* host)
{
    onConnectHostModuleImpl(this, host);
}

void PlayModule::onPresetChange()
{
    if (chem_ui) ui()->onPresetChange();
}

void PlayModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

void PlayModule::process(const ProcessArgs& args)
{
    ChemModule::process(args);

    if (!chem_host || chem_host->host_busy() || !ui()) return;

    if (ui()->preset_count()) {
        auto prev_in = getInput(Inputs::IN_PRESET_PREV);
        if (prev_in.isConnected()) {
            auto v = prev_in.getVoltage();
            if (prev_trigger.process(v, 0.1f, 5.f)) {
                prev_trigger.reset();
                ui()->prev_preset();
            }
        }
        auto next_in = getInput(Inputs::IN_PRESET_NEXT);
        if (next_in.isConnected()) {
            auto v = next_in.getVoltage();
            if (next_trigger.process(v, 0.1f, 5.f)) {
                next_trigger.reset();
                ui()->next_preset();
            }
        }
    }
}

Model *modelPlay = createModel<PlayModule, PlayUi>("chem-play");

