#include "Play.hpp"
using namespace pachde;

void PlayModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    json_t* j = json_object_get(root, "haken-device");
    if (j) {
        device_claim = json_string_value(j);
    }
}

json_t* PlayModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    return root;
}

// IChemClient
::rack::engine::Module* PlayModule::client_module(){ return this; }
void PlayModule::onConnectHost(IChemHost* host) {}
void PlayModule::onPresetChange() { }
void PlayModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
}

Model *modelPlay = createModel<PlayModule, PlayModuleWidget>("chem-play");

