#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../services/colors.hpp"
#include "../../services/em-midi-port.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/theme-button.hpp"
#include "../../widgets/theme-knob.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/tip-label-widget.hpp"
#include "../../widgets/knob-track-widget.hpp"

using namespace pachde;

struct PostUi;

struct PostModule : ChemModule, IChemClient, IDoMidi
{
    enum Params {
        P_INVALID = -1,
        P_POST_LEVEL,
        P_MIX,
        P_TILT,
        P_FREQUENCY,
        P_MOD_AMOUNT, NUM_MOD_PARAMS = P_MOD_AMOUNT,
        P_MUTE,
        NUM_PARAMS,
        NUM_KNOBS = 1 + P_MOD_AMOUNT
    };
    enum Inputs {
        IN_INVALID = -1,
        IN_POST_LEVEL,
        IN_MIX,
        IN_TILT,
        IN_FREQUENCY,
        NUM_MOD_INPUTS = IN_FREQUENCY,
        IN_MUTE,
        NUM_INPUTS
    };
    enum Outputs {
        NUM_OUTPUTS
    };
    enum Lights {
        L_POST_LEVEL_MOD,
        L_MIX_MOD,
        L_TILT_MOD,
        L_FREQUENCY_MOD,
        L_MIX,
        L_MUTE,
        NUM_LIGHTS
    };

    std::string device_claim;
    PostUi* ui() { return reinterpret_cast<PostUi*>(chem_ui); };
    
    Modulation modulation;
    uint8_t cc_lsb;
    bool glow_knobs;
    bool muted;
    rack::dsp::SchmittTrigger mute_trigger;
    
    PostModule();
    ~PostModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }

    bool connected();

    void set_modulation_target(int id) {
        modulation.set_modulation_target(id);
    }
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

    void update_from_em();
    void sync_mute();
    void onPortChange(const PortChangeEvent& e) override {
        modulation.onPortChange(e);
    }
    void process_params(const ProcessArgs& args);
    void process(const ProcessArgs& args) override;
};

// -- Post UI -----------------------------------

struct PostUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost* chem_host{nullptr};
    PostModule* my_module{nullptr};

    TextLabel* effect_label;
    TextLabel* top_knob_label;
    TextLabel* mid_knob_label;

    LinkButton*   link_button{nullptr};
    TipLabel*     haken_device_label{nullptr};
    TipLabel*     warning_label{nullptr};
    SmallSimpleLight<GreenLight>* mix_light;
    GlowKnob* knobs[PostModule::NUM_PARAMS];
    TrackWidget* tracks[PostModule::NUM_PARAMS];
#ifdef LAYOUT_HELP
    bool layout_hinting{false};
#endif
    PostUi(PostModule *module);

    bool connected();
    void glowing_knobs(bool glow);
    void center_knobs();

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-post.svg"); }
    void setThemeName(const std::string& name, void * context) override;

    void step() override;
    void onHoverKey(const HoverKeyEvent &e) override;
    void appendContextMenu(Menu *menu) override;
};

