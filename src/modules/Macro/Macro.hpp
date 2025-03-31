#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../em/preset-macro.hpp"
#include "../../services/colors.hpp"
#include "../../services/em-midi-port.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/theme-button.hpp"
#include "../../widgets/theme-knob.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/tip-label-widget.hpp"
#include "../../widgets/knob-track-widget.hpp"

using namespace pachde;

struct MacroUi;

struct MacroModule : ChemModule, IChemClient, IDoMidi
{
    MacroUi* ui() { return reinterpret_cast<MacroUi*>(chem_ui); }

    enum Params {
        P_M1,
        P_M2,
        P_M3,
        P_M4,
        P_M5,
        P_M6,
        P_MOD_AMOUNT, 
        NUM_PARAMS, 
        NUM_MOD_PARAMS = P_MOD_AMOUNT,
        NUM_KNOBS = NUM_PARAMS
    };
    enum Inputs {
        IN_INVALID = -1,
        IN_M1,
        IN_M2,
        IN_M3,
        IN_M4,
        IN_M5,
        IN_M6,
        NUM_INPUTS
    };
    enum Outputs { NUM_OUTPUTS };
    enum Lights {
        L_M1,
        L_M2,
        L_M3,
        L_M4,
        L_M5,
        L_M6,
        NUM_LIGHTS };

    std::string device_claim;
    PresetMacro macro_names;
    Modulation modulation;
    uint8_t macro_lsb;
    bool glow_knobs;

    MacroModule();
    ~MacroModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }

    void set_modulation_target(int id) {
        modulation.set_modulation_target(id);
    }

    // IDoMidi
    void do_message(PackedMidiMessage message) override;
    void update_from_em();

    bool connected();

    // IChemClient
    rack::engine::Module* client_module() override;
    std::string client_claim() override;
    IDoMidi* client_do_midi() override { return this; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    // ----  Rack  ------------------------------------------

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;
    void onPortChange(const PortChangeEvent& e) override {
        modulation.onPortChange(e);
    }
    void process_params(const ProcessArgs& args);
    void process(const ProcessArgs& args) override;
};

// -- Macro UI -----------------------------------

struct MacroMenu;

struct MacroUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost*    chem_host{nullptr};
    MacroModule*  my_module{nullptr};

    LinkButton*   link_button{nullptr};
    TipLabel*     haken_device_label{nullptr};

    TipLabel* preset_label;
    TextLabel* m1_label;
    TextLabel* m2_label;
    TextLabel* m3_label;
    TextLabel* m4_label;
    TextLabel* m5_label;
    TextLabel* m6_label;
    TextLabel* m1_ped_label;
    TextLabel* m2_ped_label;
    TextLabel* m3_ped_label;
    TextLabel* m4_ped_label;
    TextLabel* m5_ped_label;
    TextLabel* m6_ped_label;

    GlowKnob* knobs[MacroModule::NUM_KNOBS];
    TrackWidget* tracks[MacroModule::NUM_KNOBS];

    MacroUi(MacroModule *module);

    bool connected();
    void glowing_knobs(bool glow);
    void center_knobs();

    void unconnected_ui();
    
    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-macro.svg"); }
    void setThemeName(const std::string& name, void * context) override;

    void onHoverKey(const HoverKeyEvent& e) override;
    void step() override;
    void appendContextMenu(Menu *menu) override;
};

