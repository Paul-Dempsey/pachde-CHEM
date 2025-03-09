#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../services/colors.hpp"
#include "../../services/em-midi-port.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/tip-label-widget.hpp"
#include "../../widgets/theme-button.hpp"
#include "../../widgets/themed-widgets.hpp"

using namespace pachde;

struct SostenutoUi;

struct SostenutoModule : ChemModule, IChemClient, IDoMidi
{
    enum Params {
        P_SOSTENUTO,
        P_MIN_SOS,
        P_MAX_SOS,
        NUM_PARAMS,
    };
    enum Inputs {
        NUM_INPUTS
    };
    enum Outputs {
        //OUT_SOS,
        NUM_OUTPUTS
    };
    enum Lights {
        NUM_LIGHTS
    };
    uint8_t last_value;

    std::string device_claim;
    SostenutoUi* ui() { return reinterpret_cast<SostenutoUi*>(chem_ui); };
    
    SostenutoModule();
    ~SostenutoModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }

    bool connected();

    // IDoMidi
    void doMessage(PackedMidiMessage message) override;

    // IChemClient
    rack::engine::Module* client_module() override;
    std::string client_claim() override;
    IDoMidi* client_do_midi() override { return this; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    // ----  Rack  ------------------------------------------
    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;

    void update_from_em();
    void process_params(const ProcessArgs& args);
    void process(const ProcessArgs& args) override;
};

// -- Sostenuto UI -----------------------------------

struct SostenutoUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost* chem_host{nullptr};
    SostenutoModule* my_module{nullptr};

    LinkButton*   link_button{nullptr};
    SostenutoUi(SostenutoModule *module);

    bool connected();

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override {}
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-sostenuto.svg"); }

    // Rack
    void step() override;
    void appendContextMenu(Menu *menu) override;
};

