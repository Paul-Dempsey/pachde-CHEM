#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../services/colors.hpp"
#include "../../services/em-midi-port.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/widgets.hpp"
#include "midi-pad.hpp"

using namespace pachde;
struct MidiPadUi;

struct MidiPadModule : ChemModule, IChemClient, IDoMidi
{
    enum Params {
        NUM_PARAMS
    };
    enum Inputs {
        IN_PAD_A1, IN_PAD_A2, IN_PAD_A3, IN_PAD_A4,
        IN_PAD_B1, IN_PAD_B2, IN_PAD_B3, IN_PAD_B4,
        IN_PAD_C1, IN_PAD_C2, IN_PAD_C3, IN_PAD_C4,
        IN_PAD_D1, IN_PAD_D2, IN_PAD_D3, IN_PAD_D4,
        NUM_INPUTS
    };
    enum Outputs {
        NUM_OUTPUTS
    };
    enum Lights {
        L_EDITING,
        NUM_LIGHTS
    };

    std::string device_claim;
    std::shared_ptr<MidiPad> pad_defs[16]{0};
    bool editing{false};
    WallTimer ticker;

    MidiPadModule();
    ~MidiPadModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }

    MidiPadUi* ui() { return reinterpret_cast<MidiPadUi*>(chem_ui); };
    PackedColor get_pad_color(int id);
    void set_pad_color(int pad, PackedColor color);
    std::shared_ptr<MidiPad> first_pad();
    void ensure_pad(int id);

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

// -- MidiPad UI -----------------------------------

struct MidiPadMenu;

struct MidiPadUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost*  chem_host{nullptr};
    MidiPadModule*   my_module{nullptr};

    LinkButton* link_button{nullptr};
    TipLabel* haken_device_label{nullptr};
    PadWidget* pads[16]{nullptr};
    EditButton* edit_button{nullptr};
    Palette1Button* palette{nullptr};
    TextInput* name_field{nullptr};
    //TextInput* midi_field{nullptr};
    MultiTextInput* midi_field{nullptr};

    int edit_pad{-1};

    MidiPadUi(MidiPadModule *module);

    bool connected();
    void edit_mode(bool editing);
    void set_edit_pad(int id);
    void on_click_pad(int id);
    void on_name_text_changed(std::string text);
    void on_midi_text_changed(std::string text);
    std::shared_ptr<MidiPad> get_pad(int id) { return my_module ? my_module->pad_defs[id] : nullptr; }

    PackedColor get_pad_color();
    void set_pad_color(PackedColor color);

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override {}
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-4x4.svg"); }
    void setThemeName(const std::string& name, void * context) override;

    void sync_labels();
    void step() override;
    void appendContextMenu(Menu *menu) override;
};

