#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../services/colors.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/themed-widgets.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/tip-label-widget.hpp"

using namespace pachde;

struct FxUi;

struct FxModule : ChemModule, IChemClient
{
    std::string device_claim;
    IChemHost* chem_host;
    FxUi* ui;
    rack::dsp::Timer poll_host;

    bool glow_knobs;

    FxModule();
    ~FxModule() {
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
        // Knobs
        P_R1,
        P_R2,
        P_R3,
        P_R4,
        P_R5,
        P_R6,
        P_MIX,
        NUM_KNOBS,
        // Switches
        P_ENABLE = NUM_KNOBS,
        P_EFFECT,

        NUM_PARAMS
    };
    enum Inputs {
        IN_R1,
        IN_R2,
        IN_R3,
        IN_R4,
        IN_R5,
        IN_R6,
        IN_MIX,
        // gate/trigger
        IN_ENABLE,
        NUM_INPUTS
    };
    enum Outputs {
        NUM_OUTPUTS
    };
    enum Lights {
        L_ENABLE,
        NUM_LIGHTS
    };

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;

    void process(const ProcessArgs& args) override;
};

// -- Fx UI -----------------------------------

struct FxMenu;
constexpr const int PLAYLIST_LENGTH = 15;

struct FxUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost*    chem_host{nullptr};
    FxModule*  my_module{nullptr};

    LinkButton*   link_button{nullptr};
    TipLabel*     haken_device_label{nullptr};
    TipLabel*     warning_label{nullptr};

    StaticTextLabel* effect_label;
    StaticTextLabel* r_labels[6];
    GlowKnob* knobs[FxModule::NUM_KNOBS];

    FxUi(FxModule *module);
    ~FxUi();

    bool connected();
    void glowing_knobs(bool glow);

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/CHEM-fx.svg"); }
    void setThemeName(const std::string& name, void * context) override;

    //void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

