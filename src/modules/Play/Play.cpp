#include "Play.hpp"
#include <ghc/filesystem.hpp>
#include "services/json-help.hpp"
namespace fs = ghc::filesystem;
using namespace pachde;

PlayModule::PlayModule()
:   track_live(true)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    configInput(IN_PRESET_PREV, "Previous trigger");
    configInput(IN_PRESET_NEXT, "Next trigger");
    configInput(IN_PRESET_SELECT, "Select preset poly trigger");
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
    device_claim = get_json_string(root, "haken-device");
    playlist_folder = get_json_string(root, "playlist-folder");
    playlist_file = get_json_string(root, "playlist-file");
    track_live = get_json_bool(root, "track-live", true);

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
    set_json(root, "haken-device", device_claim);
    set_json(root, "track-live", track_live);
    set_json(root, "playlist-folder", playlist_folder);
    if (!playlist_folder.empty()) {
        auto kv = get_plugin_kv_store();
        if (kv && kv->load()) {
            if (kv->lookup("playlist-folder").empty()) {
                kv->update("playlist-folder", playlist_folder);
                kv->save();
            }
        }
    }
    set_json(root, "playlist-file", playlist_file);

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

void PlayModule::onConnectHost(IChemHost* host) {
    onConnectHostModuleImpl(this, host);
}

void PlayModule::onPresetChange() {
    if (chem_ui) ui()->onPresetChange();
}

void PlayModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) {
    if (chem_ui && (device == ChemDevice::Haken)) ui()->onConnectionChange(device, connection);
}

void PlayModule::onPortChange(const PortChangeEvent &e) {
    if (e.connecting) {
        if (e.portId == Inputs::IN_PRESET_SELECT) {
            getInput(IN_PRESET_SELECT).setChannels(16);
        }
    }
}

void PlayModule::process(const ProcessArgs& args) {
    ChemModule::process(args);

    if (!host_connected(chem_host) || chem_host->host_busy() || !chem_ui) return;

    if (throttle_preset.finished()) {
        throttle_preset.stop();
    }
    if (!throttle_preset.running()) {
        bool preset_sent{false};
        if (ui()->preset_count()) {
            auto select_in = getInput(Inputs::IN_PRESET_SELECT);
            if (select_in.isConnected()) {
                int lim = std::min(16, int(ui()->preset_count()));
                for (int i = 0; i < lim; ++i) {
                    if (select_triggers[i].process(select_in.getVoltage(i), 0.1f, 5.f)) {
                        ui()->select_index(i);
                        preset_sent = true;
                        throttle_preset.start(4);
                        break;
                    }
                }
            }

            auto next_in = getInput(Inputs::IN_PRESET_NEXT);
            if (!preset_sent && next_in.isConnected()) {
                auto v = next_in.getVoltage();
                if (next_trigger.process(v, 0.1f, 5.f)) {
                    next_trigger.reset();
                    ui()->next_preset();
                    preset_sent = true;
                    throttle_preset.start(4);
                }
            }

            auto prev_in = getInput(Inputs::IN_PRESET_PREV);
            if (!preset_sent && prev_in.isConnected()) {
                auto v = prev_in.getVoltage();
                if (prev_trigger.process(v, 0.1f, 5.f)) {
                    prev_trigger.reset();
                    ui()->prev_preset();
                    preset_sent = true;
                    throttle_preset.start(4);
                }
            }
        }
    }
}

Model *modelPlay = createModel<PlayModule, PlayUi>("chem-play");

