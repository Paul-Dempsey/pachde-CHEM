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
struct WidgetInfo { const char * name; int param; int input; int light; };
extern WidgetInfo widget_info[];
const int W_IDX_PP = 0;
const int W_IDX_KR = 4;

struct ConvoModule : ChemModule, IChemClient, IDoMidi
{
    enum Params {
        P_PRE_MIX, P_PRE_INDEX,
        P_POST_MIX, P_POST_INDEX,

        P_1_TYPE,   P_2_TYPE,   P_3_TYPE,   P_4_TYPE,
        P_1_LENGTH, P_2_LENGTH, P_3_LENGTH, P_4_LENGTH,
        P_1_TUNING, P_2_TUNING, P_3_TUNING, P_4_TUNING,
        P_1_WIDTH,  P_2_WIDTH,  P_3_WIDTH,  P_4_WIDTH,
        P_1_LEFT,   P_2_LEFT,   P_3_LEFT,   P_4_LEFT, 
        P_1_RIGHT,  P_2_RIGHT,  P_3_RIGHT,  P_4_RIGHT,

        P_MOD_AMOUNT,

        P_EXTEND,
        NUM_PARAMS,
        
        NUM_KNOBS = P_EXTEND
    };
    enum Inputs {
        IN_PRE_MIX,  IN_PRE_INDEX,
        IN_POST_MIX, IN_POST_INDEX,
        //IN_1_TYPE,   IN_2_TYPE,   IN_3_TYPE,   IN_4_TYPE,
        IN_1_LENGTH, IN_2_LENGTH, IN_3_LENGTH, IN_4_LENGTH,
        IN_1_TUNING, IN_2_TUNING, IN_3_TUNING, IN_4_TUNING,
        IN_1_WIDTH,  IN_2_WIDTH,  IN_3_WIDTH,  IN_4_WIDTH,
        IN_1_LEFT,   IN_2_LEFT,   IN_3_LEFT,   IN_4_LEFT, 
        IN_1_RIGHT,  IN_2_RIGHT,  IN_3_RIGHT,  IN_4_RIGHT,

        NUM_INPUTS
    };
    enum Outputs {
        NUM_OUTPUTS
    };
    enum Lights {
        L_PRE_MIX, L_PRE_INDEX,
        L_POST_MIX, L_POST_INDEX,
        //L_1_TYPE,   L_2_TYPE,   L_3_TYPE,   L_4_TYPE,
        L_1_LENGTH, L_2_LENGTH, L_3_LENGTH, L_4_LENGTH,
        L_1_TUNING, L_2_TUNING, L_3_TUNING, L_4_TUNING,
        L_1_WIDTH,  L_2_WIDTH,  L_3_WIDTH,  L_4_WIDTH,
        L_1_LEFT,   L_2_LEFT,   L_3_LEFT,   L_4_LEFT,
        L_1_RIGHT,  L_2_RIGHT,  L_3_RIGHT,  L_4_RIGHT,

        L_EXTEND,

        NUM_LIGHTS
    };

    std::string device_claim;
    Modulation modulation;
    bool glow_knobs;

    ConvolutionParams conv;

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
    void process_params(const ProcessArgs& args);
    void process(const ProcessArgs& args) override;
};

// -- Convo UI -----------------------------------
using CM = ConvoModule;

struct ConvoUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost* chem_host{nullptr};
    ConvoModule* my_module{nullptr};

    TextLabel* conv_number_label;

    LinkButton* link_button{nullptr};
    TipLabel* haken_device_label{nullptr};
    TipLabel* warning_label{nullptr};
    
    SmallRoundParamButton* extend_button;

    int last_convo;
    float last_type;
    float last_extend;

    GlowKnob* knobs[ConvoModule::NUM_KNOBS];
    TrackWidget* tracks[ConvoModule::NUM_INPUTS];

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

    void step() override;
    void onHoverKey(const HoverKeyEvent &e) override;
    void appendContextMenu(Menu *menu) override;
};

const WidgetInfo& param_info(int param_id);