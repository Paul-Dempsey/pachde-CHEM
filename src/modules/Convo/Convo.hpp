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

struct ConvolutionParams {
    int id;
    uint8_t type;
    uint8_t length;
    uint8_t tuning;
    uint8_t width;
    uint8_t left;
    uint8_t right;

    ConvolutionParams(int id, uint8_t type) : 
        id(id), type(type), length(127), tuning(64), width(64), left(127), right(127)
    {}
};

struct ConvoUi;

struct ConvoModule : ChemModule, IChemClient
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

    int conv_number;
    ConvolutionParams convs[4] = {
        { 0, Haken::cd_Wood },
        { 1, Haken::cd_MetalBright },
        { 2, Haken::cd_Fiber },
        { 3, Haken::cd_Wood },
    };
    int last_conv;
    bool extend;

    int current_conv() { return conv_number; }
    uint8_t current_type() { return convs[conv_number].type; }
    uint8_t current_length() { return convs[conv_number].length; }
    uint8_t current_tuning() { return convs[conv_number].tuning; }
    uint8_t current_width() { return convs[conv_number].width; }
    uint8_t current_left() { return convs[conv_number].left; }
    uint8_t current_right() { return convs[conv_number].right; }

    ConvoModule();
    ~ConvoModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }
    ConvoUi* ui() { return reinterpret_cast<ConvoUi*>(chem_ui); };

    void set_modulation_target(int id) {
        //modulation.set_modulation_target(id);
    }
    void update_from_em();

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

