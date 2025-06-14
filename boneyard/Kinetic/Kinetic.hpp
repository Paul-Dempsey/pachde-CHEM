#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../services/colors.hpp"
#include "../../services/em-midi-port.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/slider-widget.hpp"
#include "../../widgets/theme-button.hpp"
#include "../../widgets/theme-knob.hpp"
#include "../../widgets/themed-widgets.hpp"
#include "../../widgets/tip-label-widget.hpp"

using namespace pachde;

struct KineticUi;

struct KineticModule : ChemModule, IChemClient, IDoMidi
{
    enum Params {
        P_ANCHOR,
        P_SPRING,
        P_MASS,
        P_GRAVITY,
        P_HEIGHT,
        P_VISCOCITY,
        P_ELASTICITY,
        P_PADDING,
        P_BOW_SPEED,
        P_SLIP,
        P_STICK,
        P_QUIT,
        P_SPRING_NL,
        P_VISCOCITY_NL,
        P_NOTE_TRIM,
        P_CENT_TRIM,
        P_MOD_AMOUNT,
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
    ChemId chem_id;
    uint8_t my_cc;
    Modulation modulation;

    std::string device_claim;
    bool glow_knobs;

    KineticModule(ChemId id, uint8_t cc);
    virtual ~KineticModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }
    void init();

    KineticUi* ui() { return reinterpret_cast<KineticUi*>(chem_ui); };
    bool connected();

    // IDoMidi
    void do_message(PackedMidiMessage message) override;

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

    void onPortChange(const PortChangeEvent& e) override;
    void update_from_em();
    void process_params(const ProcessArgs& args);
    void process(const ProcessArgs& args) override;
};

// -- Kinetic UI ---------------------------------------

struct KineticUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost* chem_host{nullptr};
    KineticModule* my_module{nullptr};

    LinkButton* link_button{nullptr};
    TipLabel* haken_device_label{nullptr};
    GlowKnob* mod_knob{nullptr};

    KineticUi(KineticModule *module);

    void create_ui();
    bool connected();
 
    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // Rack
    void step() override;
    void appendContextMenu(Menu *menu) override;
};

struct KineticAModule : KineticModule
{
    explicit KineticAModule() : KineticModule(ChemId::Kinetic_A, 0xff) {
        init();
    }
};
struct KineticAUi : KineticUi
{
    explicit KineticAUi(KineticAModule* module) : KineticUi(module) {
        create_ui();
    }
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-kinetic-a.svg"); }
};

struct KineticBModule : KineticModule
{
    explicit KineticBModule() : KineticModule(ChemId::Kinetic_B, 0xff) {
        init();
    }
};

struct KineticBUi : KineticUi
{
    explicit KineticBUi(KineticBModule* module) : KineticUi(module) {
        create_ui();
    }
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-kinetic-b.svg"); }
};
