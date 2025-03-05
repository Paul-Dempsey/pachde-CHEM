#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../services/colors.hpp"
#include "../../services/em-midi-port.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/selector-widget.hpp"
#include "../../widgets/theme-button.hpp"
#include "../../widgets/theme-knob.hpp"
#include "../../widgets/tip-label-widget.hpp"
#include "../../widgets/knob-track-widget.hpp"

using namespace pachde;

struct PreUi;

struct PreModule : ChemModule, IChemClient
{
    enum Params {
        P_PRE_LEVEL,
        P_MIX,
        P_THRESHOLD_DRIVE,
        P_ATTACK,
        P_RATIO_MAKEUP,
        P_MOD_AMOUNT, NUM_MOD_PARAMS = P_MOD_AMOUNT,
        P_SELECT, NUM_KNOBS = P_SELECT,
        NUM_PARAMS
    };
    enum Inputs {
        IN_INVALID = -1,
        IN_PRE_LEVEL,
        IN_MIX,
        IN_THRESHOLD_DRIVE,
        IN_ATTACK,
        IN_RATIO_MAKEUP,
        NUM_INPUTS
    };
    enum Outputs {
        NUM_OUTPUTS
    };
    enum Lights {
        L_PRE_LEVEL_MOD,
        L_MIX_MOD,
        L_THRESHOLD_DRIVE_MOD,
        L_ATTACK_MOD,
        L_RATIO_MAKEUP_MOD,
        L_MIX,
        NUM_LIGHTS
    };

    Modulation modulation;
    std::string device_claim;
    int last_select;
    bool glow_knobs;
    
    PreModule();
    ~PreModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }

    PreUi* ui() { return reinterpret_cast<PreUi*>(chem_ui); }
    bool connected();
    void set_modulation_target(int id) {
        modulation.set_modulation_target(id);
    }
    
    // IChemClient
    rack::engine::Module* client_module() override;
    std::string client_claim() override;
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;
    void onPortChange(const PortChangeEvent& e) override {
        modulation.onPortChange(e);
    }
    void update_from_em(bool with_knobs);
    void process_params(const ProcessArgs& args);
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
    SmallSimpleLight<GreenLight>* mix_light;
    GlowKnob* knobs[PreModule::NUM_KNOBS];
    TrackWidget* tracks[PreModule::NUM_MOD_PARAMS];

    #ifdef LAYOUT_HELP
    bool layout_hinting{false};
#endif

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
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-pre.svg"); }
    void setThemeName(const std::string& name, void * context) override;

    void sync_labels();
    void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

