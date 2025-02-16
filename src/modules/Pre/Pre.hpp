#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../services/colors.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/selector-widget.hpp"
#include "../../widgets/themed-widgets.hpp"
#include "../../widgets/tip-label-widget.hpp"

using namespace pachde;

struct PreUi;

struct PreModule : ChemModule, IChemClient
{
    std::string device_claim;
    IChemHost* chem_host;
    PreUi* ui;
    rack::dsp::Timer poll_host;

    bool glow_knobs;

    PreModule();
    ~PreModule() {
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
        P_PRE_LEVEL,
        P_MIX,
        P_THRESHOLD_DRIVE,
        P_ATTACK_,
        P_RATIO_MAKEUP,
        P_ATTENUVERT,
        P_SELECT, NUM_KNOBS = P_SELECT,
        NUM_PARAMS
    };
    enum Inputs {
        IN_PRE_LEVEL,
        IN_MIX,
        IN_THRESHOLD,
        IN_ATTACK,
        IN_RATIO,
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

// -- Pre UI -----------------------------------

struct PreUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost* chem_host{nullptr};
    PreModule* my_module{nullptr};

    StaticTextLabel* effect_label;

    LinkButton*   link_button{nullptr};
    TipLabel*     haken_device_label{nullptr};
    TipLabel*     warning_label{nullptr};

    int comp_type;

    SelectorWidget* selector{nullptr};
    StaticTextLabel* selector_label;
    StaticTextLabel* top_knob_label;
    StaticTextLabel* mid_knob_label;
    StaticTextLabel* bot_knob_label;
    StaticTextLabel* in_thresh_drive;
    StaticTextLabel* in_attack_x;
    StaticTextLabel* in_ratio_makeup;

    GlowKnob* knobs[PreModule::NUM_KNOBS];

    PreUi(PreModule *module);

    bool connected();
    void glowing_knobs(bool glow);

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/CHEM-pre.svg"); }
    void setThemeName(const std::string& name, void * context) override;

    void sync_labels();
    void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

