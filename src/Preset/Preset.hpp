#pragma once
#include "../plugin.hpp"
#include "../chem.hpp"
#include "../chem-core.hpp"
#include "../services/colors.hpp"
#include "../widgets/themed-widgets.hpp"
using namespace pachde;

struct PresetModule : ChemModule, IChemClient
{
    std::string device_claim;

    void dataFromJson(json_t* root) override;

    json_t* dataToJson() override;

    // IChemClient
    rack::engine::Module* client_module() override;
    std::string client_claim() override;
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
};


struct PresetModuleWidget : ChemModuleWidget
{
    PresetModule *my_module = nullptr;

    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/CHEM-preset.svg"); }

    PresetModuleWidget(PresetModule *module);

    void step() override;
    void appendContextMenu(Menu *menu) override;
};

