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

struct FxUi;

struct FxModule : ChemModule, IChemClient
{
    enum Params {
        // Knobs
        P_R1,
        P_R2,
        P_R3,
        P_R4,
        P_R5,
        P_R6,
        P_MIX,
        P_MOD_AMOUNT, NUM_MOD_PARAMS = P_MOD_AMOUNT,

        // Switches
        P_DISABLE, NUM_KNOBS = P_DISABLE,
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
        //IN_ENABLE,
        NUM_INPUTS
    };
    enum Outputs {
        NUM_OUTPUTS
    };
    enum Lights {
        L_R1_MOD,
        L_R2_MOD,
        L_R3_MOD,
        L_R4_MOD,
        L_R5_MOD,
        L_R6_MOD,
        L_MIX_MOD,
        L_DISABLE,
        L_MIX,
        NUM_LIGHTS
    };

    Modulation modulation;
    std::string device_claim;
    int last_disable;
    bool glow_knobs;
    
    FxModule();
    ~FxModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }

    FxUi* ui() { return reinterpret_cast<FxUi*>(chem_ui); };
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
    void process(const ProcessArgs& args) override;
};

// -- Fx UI -----------------------------------

struct FxMenu;
constexpr const int PLAYLIST_LENGTH = 15;

struct FxUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost*  chem_host{nullptr};
    FxModule*   my_module{nullptr};

    LinkButton* link_button{nullptr};
    TipLabel*   haken{nullptr};
    TipLabel*   warn{nullptr};

    SelectorWidget* selector{nullptr};
    StaticTextLabel* effect_label;
    StaticTextLabel* r_labels[6];
    SmallSimpleLight<GreenLight>* mix_light;
    GlowKnob* knobs[FxModule::NUM_KNOBS];
    TrackWidget* tracks[FxModule::NUM_MOD_PARAMS];

#ifdef LAYOUT_HELP
    bool layout_hinting{false};
#endif
    int effect{-1};

    FxUi(FxModule *module);

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

    void sync_labels();
    void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

