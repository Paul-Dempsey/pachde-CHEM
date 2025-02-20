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
    std::string device_claim;
    IChemHost* chem_host;
    ConvoUi* ui;

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

    // IChemClient
    rack::engine::Module* client_module() override;
    std::string client_claim() override;
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    // ----  Rack  ------------------------------------------

    enum Params {
        P_TYPE,
        P_LENGTH,
        P_TUNING,
        P_WIDTH,
        P_LEFT, 
        P_RIGHT,
        P_PRE_MIX,
        P_PRE_INDEX,
        P_POST_MIX,
        P_POST_INDEX,
        P_ATTENUVERT, NUM_KNOBS = 1 + P_ATTENUVERT,
        P_SELECT, 
        P_EXTEND,
        NUM_PARAMS
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
        L_IN_PRE_MIX,
        L_IN_PRE_INDEX,
        L_IN_POST_MIX,
        L_IN_POST_INDEX,
        L_EXTEND,
        NUM_LIGHTS
    };

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;

    void process_params(const ProcessArgs& args);
    void process(const ProcessArgs& args) override;
};

// -- Convo UI -----------------------------------

struct ConvoUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost* chem_host{nullptr};
    ConvoModule* my_module{nullptr};

    StaticTextLabel* conv_number_label;

    LinkButton*   link_button{nullptr};
    TipLabel*     haken_device_label{nullptr};
    TipLabel*     warning_label{nullptr};

    SmallRoundButton* extend_button;
    SelectorWidget* selector{nullptr};

    //MiniVUWidget* l_vu;
    //MiniVUWidget* r_vu;

    // dynamic labels
    StaticTextLabel* selector_label;
    StaticTextLabel* type_label;
    int last_convo;
    float last_type;
    float last_extend;

    GlowKnob* knobs[ConvoModule::NUM_KNOBS];

    ConvoUi(ConvoModule *module);

    bool connected();
    void glowing_knobs(bool glow);

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/CHEM-convo.svg"); }
    void setThemeName(const std::string& name, void * context) override;

    void sync_select_label();
    void sync_type_label();
    void sync_extend();

    void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

