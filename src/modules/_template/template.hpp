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
using namespace eaganmatrix;

struct Template_Ui;

struct Template_Module : ChemModule, IChemClient, IDoMidi
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
    
    Template_Module();
    ~Template_Module() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }

    Template_Ui* ui() { return reinterpret_cast<Template_Ui*>(chem_ui); };
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

// -- Template_ UI -----------------------------------

struct Template_Menu;

struct Template_Ui : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost*  chem_host{nullptr};
    Template_Module*   my_module{nullptr};

    LinkButton* link_button{nullptr};
    TipLabel*   haken_device_label{nullptr};
    //TipLabel*   warn{nullptr};

    //GlowKnob* knobs[Template_Module::NUM_KNOBS];
    //TrackWidget* tracks[Template_Module::NUM_MOD_PARAMS];

    Template_Ui(Template_Module *module);

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

    void sync_labels();
    void onHoverKey(const HoverKeyEvent &e) override;
    void step() override;
    void appendContextMenu(Menu *menu) override;
};

