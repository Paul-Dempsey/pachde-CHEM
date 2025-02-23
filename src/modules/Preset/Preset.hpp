#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../chem-core.hpp"
#include "../../services/colors.hpp"
#include "../../widgets/themed-widgets.hpp"
using namespace pachde;

struct PresetModule : ChemModule, IChemClient
{
    std::string device_claim;

    PresetModule() : chem_host(nullptr) {}
    ~PresetModule() {
        if (chem_host) chem_host->unregister_chem_client(this);
    }

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;

    // IChemClient
    IChemHost* chem_host;
    IChemHost* get_host() override {
        return chem_host;
    }
    rack::engine::Module* client_module() override;
    std::string client_claim() override;
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
};


struct PresetModuleWidget : ChemModuleWidget
{
    using Base = ChemModuleWidget;
    PresetModule *my_module = nullptr;

    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/CHEM-preset.svg"); }

    PresetModuleWidget(PresetModule *module);

    //void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

