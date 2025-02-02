#include "Play.hpp"
#include <ghc/filesystem.hpp>
using namespace pachde;
namespace fs = ghc::filesystem;

PlayModule::PlayModule()
:   chem_host(nullptr),
    ui(nullptr)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    configInput(IN_PRESET_PREV, "Previous preset trigger");
    configInput(IN_PRESET_NEXT, "Next preset trigger");
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
    json_t* j = json_object_get(root, "haken-device");
    if (j) {
        device_claim = json_string_value(j);
    }

    j = json_object_get(root, "playlist-folder");
    if (j) {
        playlist_folder = json_string_value(j);
    }

    j = json_object_get(root, "playlist-file");
    if (j) {
        playlist_file = json_string_value(j);
    }

    playlist_mru.clear();
    auto jar = json_object_get(root, "history");
    if (jar) {
        json_t* jp;
        size_t index;
        json_array_foreach(jar, index, jp) {
            auto path = json_string_value(j);
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
    json_object_set_new(root, "playlist-folder", json_string(playlist_folder.c_str()));
    json_object_set_new(root, "playlist-file", json_string(playlist_file.c_str()));
    
    auto jaru = json_array();
    int count = 0;
    for (std::string& f: playlist_mru) {
        if (++count > 20) break;
        json_array_append_new(jaru, json_string(f.c_str()));
    }
    json_object_set_new(root, "history", jaru);

    return root;
}

// IChemClient
::rack::engine::Module* PlayModule::client_module() { return this; }
std::string PlayModule::client_claim() { return device_claim; }

void PlayModule::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (!host) {
        device_claim = "";
        if (ui) ui->onConnectHost(nullptr);
        return;
    }
    auto conn = chem_host->host_connection(ChemDevice::Haken);
    if (conn) {
        device_claim = conn->info.claim();
    }
    if (ui) ui->onConnectHost(host);
}

void PlayModule::onPresetChange()
{
    if (ui) ui->onPresetChange();
}

void PlayModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (ui) ui->onConnectionChange(device, connection);
}

void PlayModule::process(const ProcessArgs& args)
{
    if (!chem_host && !device_claim.empty()) {
        if (poll_host.process(args.sampleTime) > 2.f) {
            auto broker = ModuleBroker::get();
            broker->try_bind_client(this);
        }
    }
}

Model *modelPlay = createModel<PlayModule, PlayUi>("chem-play");

