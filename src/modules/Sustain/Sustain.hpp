#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../services/colors.hpp"
#include "../../services/em-midi-port.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/indicator-widget.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/slider-widget.hpp"
#include "../../widgets/theme-button.hpp"
#include "../../widgets/theme-knob.hpp"
#include "../../widgets/themed-widgets.hpp"
#include "../../widgets/tip-label-widget.hpp"

using namespace pachde;

struct SusUi;

struct SusModule : ChemModule, IChemClient, IDoMidi
{
    enum Params {
        P_VALUE,
        P_MIN,
        P_MAX,
        P_MOD_AMOUNT,
        NUM_PARAMS,
    };
    enum Inputs {
        IN_MOD,
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

    SusModule(ChemId id, uint8_t cc);
    virtual ~SusModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }
    void init();
    virtual const char * param_name() = 0;
    virtual const char * min_param_name() = 0;
    virtual const char * max_param_name() = 0;
    virtual uint8_t from_em() = 0;

    SusUi* ui() { return reinterpret_cast<SusUi*>(chem_ui); };
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

// -- Sus UI ---------------------------------------

struct SusUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost* chem_host{nullptr};
    SusModule* my_module{nullptr};

    LinkButton* link_button{nullptr};
    IndicatorWidget* link{nullptr};
    GlowKnob* mod_knob{nullptr};
    FillSlider* slider{nullptr};

    SusUi(SusModule *module);

    void create_ui();
    bool connected();
 
    virtual const char * InputLabel() = 0;

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override {}
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // Rack
    void step() override;
    void appendContextMenu(Menu *menu) override;
};

struct SustainModule : SusModule
{
    explicit SustainModule() : SusModule(ChemId::Sustain, Haken::ccSus) {
        init();
    }
    const char * param_name() override { return "Sustain"; }
    const char * min_param_name() override { return "No sustain"; }
    const char * max_param_name() override { return "Max sustain"; }
    uint8_t from_em() override { return chem_host->host_matrix()->get_sustain(); }

};
struct SustainUi : SusUi
{
    explicit SustainUi(SustainModule* module) : SusUi(module) {
        create_ui();
    }
    const char * InputLabel() override { return "SU"; }
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-sustain.svg"); }
};

struct SostenutoModule : SusModule
{
    explicit SostenutoModule() : SusModule(ChemId::Sostenuto, Haken::ccSos) {
        init();
    }
    const char * param_name() override { return "Sostenuto"; }
    const char * min_param_name() override { return "No sostenuto"; }
    const char * max_param_name() override { return "Max sostenuto"; }
    uint8_t from_em() override { return chem_host->host_matrix()->get_sos(); }
};

struct SostenutoUi : SusUi
{
    explicit SostenutoUi(SostenutoModule* module) : SusUi(module) {
        create_ui();
    }
    const char * InputLabel() override { return "SO"; }
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-sostenuto.svg"); }
};

struct Sostenuto2Module : SusModule
{
    explicit Sostenuto2Module() : SusModule(ChemId::Sostenuto2, Haken::ccSos2) {
        init();
    }
    const char * param_name() override { return "Sostenuto 2"; }
    const char * min_param_name() override { return "No sostenuto"; }
    const char * max_param_name() override { return "Max sostenuto"; }
    uint8_t from_em() override { return chem_host->host_matrix()->get_sos2(); }
};

struct Sostenuto2Ui : SusUi
{
    explicit Sostenuto2Ui(Sostenuto2Module* module) : SusUi(module) {
        create_ui();
    }
    const char * InputLabel() override { return "S2"; }
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-sostenuto2.svg"); }
};
