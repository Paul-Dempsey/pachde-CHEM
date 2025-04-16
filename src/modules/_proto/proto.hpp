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
#include "../../widgets/bits-widget.hpp"

using namespace pachde;

struct ProtoUi;

// ccOctShift = oct shift buttons on device
// ccJack1 hi-res
// ccJack2 hi-res
// idJackShift shift amt when pedal is ccOctShift

struct ProtoModule : ChemModule, IChemClient, IDoMidi
{
    enum Params {
        NUM_PARAMS
    };
    enum Inputs {
        NUM_INPUTS
    };
    enum Outputs {
        NUM_OUTPUTS
    };
    enum Lights {
        NUM_LIGHTS
    };

    //Modulation modulation;
    std::string device_claim;
    bool glow_knobs;
    bool in_mat_poke;
    
    ProtoModule();
    ~ProtoModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }

    ProtoUi* ui() { return reinterpret_cast<ProtoUi*>(chem_ui); };
    // void set_modulation_target(int id) {
    //     modulation.set_modulation_target(id);
    // }

    // IDoMidi
    void do_message(PackedMidiMessage msg) override;

    // IChemClient
    rack::engine::Module* client_module() override;
    std::string client_claim() override;
    IDoMidi* client_do_midi() override { return this; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;
    void onPortChange(const PortChangeEvent& e) override {
        //modulation.onPortChange(e);
    }
    void process_params(const ProcessArgs& args);
    void process(const ProcessArgs& args) override;
};

// -- Proto UI -----------------------------------

struct ProtoMenu;

struct ProtoUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost*  chem_host{nullptr};
    ProtoModule*   my_module{nullptr};

    LinkButton* link_button{nullptr};
    TipLabel*   haken_device_label{nullptr};

    TextLabel* label{nullptr};
    BitsWidget* bits{nullptr};
    //GlowKnob* knobs[ProtoModule::NUM_KNOBS];
    //TrackWidget* tracks[ProtoModule::NUM_MOD_PARAMS];

    ProtoUi(ProtoModule *module);

    bool connected();
    void glowing_knobs(bool glow);
    void center_knobs();

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override {}
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-proto.svg"); }
    void setThemeName(const std::string& name, void * context) override;

    void onHoverKey(const HoverKeyEvent &e) override;
    void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

