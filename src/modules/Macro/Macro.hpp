#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../services/colors.hpp"
#include "../../services/ModuleBroker.hpp"
// #include "../../widgets/blip-widget.hpp"
#include "../../widgets/themed-widgets.hpp"
// #include "../../widgets/draw-button.hpp"
// #include "../../widgets/hamburger.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/tip-label-widget.hpp"

using namespace pachde;

struct MacroUi;

struct MacroModule : ChemModule, IChemClient
{
    std::string device_claim;
    IChemHost* chem_host;
    MacroUi* ui;
    rack::dsp::Timer poll_host;

    bool glow_knobs;

    MacroModule();
    ~MacroModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }

    // IChemClient
    rack::engine::Module* client_module() override;
    std::string client_claim() override;
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    // ----  Rack  ------------------------------------------

    enum Params {
        P_M1, P_M2, P_M3, P_M4, P_M5, P_M6,
        NUM_PARAMS
    };
    enum Inputs {
        IN_M1,
        IN_M2,
        IN_M3,
        IN_M4,
        IN_M5,
        IN_M6,
        NUM_INPUTS
    };
    enum Outputs {
        NUM_OUTPUTS
    };
    enum Lights {
        NUM_LIGHTS
    };

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;

    void process(const ProcessArgs& args) override;
};

// -- Macro UI -----------------------------------

struct MacroMenu;
constexpr const int PLAYLIST_LENGTH = 15;

struct MacroUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost*    chem_host{nullptr};
    MacroModule*  my_module{nullptr};

    LinkButton*   link_button{nullptr};
    TipLabel*     haken_device_label{nullptr};
    TipLabel*     warning_label{nullptr};

    StaticTextLabel* m1_label;
    StaticTextLabel* m2_label;
    StaticTextLabel* m3_label;
    StaticTextLabel* m4_label;
    StaticTextLabel* m5_label;
    StaticTextLabel* m6_label;
    StaticTextLabel* m1_ped_label;
    StaticTextLabel* m2_ped_label;
    StaticTextLabel* m3_ped_label;
    StaticTextLabel* m4_ped_label;
    StaticTextLabel* m5_ped_label;
    StaticTextLabel* m6_ped_label;

    GlowKnob* knobs[6];

    MacroUi(MacroModule *module);
    ~MacroUi();

    bool connected();
    void glowing_knobs(bool glow);

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/CHEM-macro.svg"); }
    void setThemeName(const std::string& name) override;

    //void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

