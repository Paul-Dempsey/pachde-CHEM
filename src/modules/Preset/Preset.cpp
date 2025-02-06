#include "Preset.hpp"

void PresetModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    json_t* j = json_object_get(root, "haken-device");
    if (j) {
        device_claim = json_string_value(j);
    }
}

json_t* PresetModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    return root;
}

// IChemClient
rack::engine::Module* PresetModule::client_module()
{
    return this; 
}
std::string PresetModule::client_claim()
{
    return device_claim;
}
void PresetModule::onConnectHost(IChemHost* host)
{
    chem_host = host;
}
void PresetModule::onPresetChange()
{
}
void PresetModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) 
{
}

Model *modelPreset = createModel<PresetModule, PresetModuleWidget>("chem-preset");
