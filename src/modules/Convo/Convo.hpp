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
#include "convolution.hpp"

using namespace pachde;

struct ConvoUi;

struct ConvoModule : ChemModule, IChemClient, IDoMidi
{
    enum Params {
        P_PRE_MIX,
        P_PRE_INDEX,
        P_POST_MIX,
        P_POST_INDEX,

        P_TYPE,
        P_LENGTH,
        P_TUNING,
        P_WIDTH,
        P_LEFT, 
        P_RIGHT,

        P_MOD_AMOUNT, 

        P_SELECT, 
        P_EXTEND,
        NUM_PARAMS,
        
        NUM_MOD_PARAMS = P_TYPE,
        NUM_KNOBS = P_SELECT
    };
    enum Inputs {
        IN_PRE_MIX,
        IN_PRE_INDEX,
        IN_POST_MIX,
        IN_POST_INDEX,
        NUM_INPUTS
    };
    enum Outputs {
        NUM_OUTPUTS
    };
    enum Lights {
        L_PRE_MIX_MOD,
        L_PRE_INDEX_MOD,
        L_POST_MIX_MOD,
        L_POST_INDEX_MOD,
        L_EXTEND,
        NUM_LIGHTS
    };

    std::string device_claim;
    Modulation modulation;
    bool glow_knobs;

    ConvolutionParams conv;

    int conv_number;
    int last_conv;
    bool extend;

    ConvoModule();
    ~ConvoModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }
    ConvoUi* ui() { return reinterpret_cast<ConvoUi*>(chem_ui); };

    void set_modulation_target(int id) {
        modulation.set_modulation_target(id);
    }
    void params_from_internal();
    void update_from_em();

    // IDoMidi
    void do_message(PackedMidiMessage message) override;

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
    void sync_from_params();
    void process_params(const ProcessArgs& args);
    void process(const ProcessArgs& args) override;
};

// -- Convo UI -----------------------------------

struct ConvoUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost* chem_host{nullptr};
    ConvoModule* my_module{nullptr};

    TextLabel* conv_number_label;

    LinkButton*   link_button{nullptr};
    TipLabel*     haken_device_label{nullptr};
    TipLabel*     warning_label{nullptr};
    
    SmallRoundParamButton* extend_button;
    SelectorWidget* selector{nullptr};

    TextLabel* selector_label;
    TextLabel* type_label;
    int last_convo;
    float last_type;
    float last_extend;

    GlowKnob* knobs[ConvoModule::NUM_KNOBS];
    TrackWidget* tracks[ConvoModule::NUM_MOD_PARAMS];
#ifdef LAYOUT_HELP
    bool layout_hinting{false};
#endif

    ConvoUi(ConvoModule *module);

    bool connected();
    void glowing_knobs(bool glow);

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override {}
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-convo.svg"); }
    void setThemeName(const std::string& name, void * context) override;

    void sync_select_label();
    void sync_type_label();

    void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

