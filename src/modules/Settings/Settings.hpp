#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../services/colors.hpp"
#include "../../services/em-midi-port.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/theme-button.hpp"
#include "../../widgets/tip-label-widget.hpp"

using namespace pachde;

struct SettingsUi;

struct SettingsModule : ChemModule, IChemClient, IDoMidi
{
    enum Params {
        P_SURFACE_DIRECTION,
        P_X,
        P_Y,
        P_Z,
        P_NOTE_PROCESSING,
        P_NOTE_PRIORITY,
        P_BASE_POLYPHONY,
        P_EXPAND_POLYPHONY,
        P_DOUBLE_COMPUTATION,

        P_MONO,
        P_MONO_MODE,
        P_MONO_INTERVAL,

        P_OCTAVE_SWITCH,
        P_OCTAVE_TYPE,
        P_OCTAVE_RANGE,

        P_ROUND_TYPE,
        P_ROUND_INITIAL,
        P_ROUND_RATE,
        P_TUNING,

        P_MIDI_DSP,
        P_MIDI_CVC,
        P_MIDI_MIDI,
        P_SURFACE_DSP,
        P_SURFACE_CVC,
        P_SURFACE_MIDI,

        P_KEEP_MIDI,

        P_MOD_AMOUNT,
        NUM_PARAMS
    };

    enum Inputs {
        IN_ROUND_INITIAL,
        IN_ROUND_RATE,
        NUM_INPUTS
    };

    enum Outputs {
        NUM_OUTPUTS
    };

    enum Lights {
        L_MONO,
        L_OCTAVE_SWITCH,

        // rounding
        L_ROUND_Y,
        L_ROUND_INITIAL,
        L_ROUND,
        L_ROUND_RELEASE,

        // routing
        L_MIDI_DSP,
        L_MIDI_CVC,
        L_MIDI_MIDI,
        L_SURFACE_DSP,
        L_SURFACE_CVC,
        L_SURFACE_MIDI,

        NUM_LIGHTS
    };

    Modulation modulation;
    std::string device_claim;
    
    SettingsModule();
    ~SettingsModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }

    SettingsUi* ui() { return reinterpret_cast<SettingsUi*>(chem_ui); }
    bool connected();
    // void set_modulation_target(int id) {
    //     modulation.set_modulation_target(id);
    // }

    // IDoMidi
    void do_message(PackedMidiMessage message) override;
    
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
        modulation.onPortChange(e);
    }
    void update_from_em();
    void process_params(const ProcessArgs& args);
    void process(const ProcessArgs& args) override;
};

// -- Settings UI -----------------------------------

struct SettingsUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost* chem_host{nullptr};
    SettingsModule* my_module{nullptr};

    TextLabel* effect_label;

    LinkButton*   link_button{nullptr};
    TipLabel*     haken_device_label{nullptr};
    TipLabel*     warning_label{nullptr};

    TextLabel* x_value{nullptr};
    TextLabel* y_value{nullptr};
    TextLabel* z_value{nullptr};
    TextLabel* note_processing_value{nullptr};
    TextLabel* note_priority_value{nullptr};
    TextLabel* base_polyphony_value{nullptr};
    TextLabel* expand_polyphony_value{nullptr};
    TextLabel* double_computation_value{nullptr};
    TextLabel* mono_mode_value{nullptr};
    TextLabel* mono_interval_value{nullptr};
    TextLabel* octave_type_value{nullptr};
    TextLabel* octave_range_value{nullptr};
    TextLabel* round_type_value{nullptr};
    TextLabel* tuning_value{nullptr};

    SettingsUi(SettingsModule *module);

    bool connected();
    void create_rounding_leds(float x, float y, float spread);

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override {}
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-settings.svg"); }
    void setThemeName(const std::string& name, void * context) override;

    void sync_labels();
    void onHoverKey(const HoverKeyEvent &e) override;
    void step() override;
    void appendContextMenu(Menu *menu) override;
};

