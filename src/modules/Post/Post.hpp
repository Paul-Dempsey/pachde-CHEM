#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../services/colors.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/theme-button.hpp"
#include "../../widgets/theme-knob.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/tip-label-widget.hpp"

using namespace pachde;

struct PostUi;

struct PostModule : ChemModule, IChemClient
{
    enum Params {
        P_INVALID = -1,
        P_POST_LEVEL,
        P_MIX,
        P_TILT,
        P_FREQUENCY,
        P_ATTENUVERT,
        P_MUTE,
        NUM_PARAMS,
        NUM_KNOBS = 1 + P_ATTENUVERT
    };
    enum Inputs {
        IN_INVALID = -1,
        IN_POST_LEVEL,
        IN_MIX,
        IN_TILT,
        IN_FREQUENCY,
        ATTENUATED_INPUTS = IN_FREQUENCY,
        IN_MUTE,
        NUM_INPUTS
    };
    enum Outputs {
        NUM_OUTPUTS
    };
    enum Lights {
        L_EQ,
        L_MUTE,
        NUM_LIGHTS
    };

    std::string device_claim;
    IChemHost* chem_host;
    PostUi* ui;

    bool glow_knobs;
    int attenuator_target;
    float attenuation[ATTENUATED_INPUTS];
    
    PostModule();
    ~PostModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }

    IChemHost* get_host() override {
        return chem_host;
    }
    // IChemClient
    rack::engine::Module* client_module() override;
    std::string client_claim() override;
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    // ----  Rack  ------------------------------------------
    void onPortChange(const PortChangeEvent& e) override;
    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;

    void process_params(const ProcessArgs& args);
    void process(const ProcessArgs& args) override;
};

// -- Post UI -----------------------------------

struct PostUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost* chem_host{nullptr};
    PostModule* my_module{nullptr};

    StaticTextLabel* effect_label;
    StaticTextLabel* top_knob_label;
    StaticTextLabel* mid_knob_label;

    LinkButton*   link_button{nullptr};
    TipLabel*     haken_device_label{nullptr};
    TipLabel*     warning_label{nullptr};

    LargeRoundButton* mute_button;
    GlowKnob* knobs[PostModule::NUM_PARAMS];

    float last_mute;

    PostUi(PostModule *module);

    bool connected();
    void glowing_knobs(bool glow);

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/CHEM-post.svg"); }
    void setThemeName(const std::string& name, void * context) override;

    void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

