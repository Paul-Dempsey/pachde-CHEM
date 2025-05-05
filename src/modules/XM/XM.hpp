#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../services/colors.hpp"
#include "../../services/color-help.hpp"
#include "../../services/em-midi-port.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/widgets.hpp"
#include "../XM-shared/xm-edit-common.hpp"
#include "../XM-shared/xm-overlay.hpp"
#include "macro-data.hpp"

using namespace pachde;

struct MacroUi {
    ssize_t index;
    GlowKnob* knob{nullptr};
    TrackWidget* track{nullptr};
    TextLabel * label{nullptr};
};

struct XMUi;

struct XMModule : ChemModule, IChemClient, IDoMidi, IExtendedMacro, IOverlayClient
{
    enum Params {
        P_1,
        P_2,
        P_3,
        P_4,
        P_5,
        P_6,
        P_7,
        P_8,
        P_MODULATION,
        NUM_PARAMS
    };
    enum Inputs {
        IN_1,
        IN_2,
        IN_3,
        IN_4,
        IN_5,
        IN_6,
        IN_7,
        IN_8,
        NUM_INPUTS
    };
    enum Outputs {
        NUM_OUTPUTS
    };
    enum Lights {
        L_IN_1,
        L_IN_2,
        L_IN_3,
        L_IN_4,
        L_IN_5,
        L_IN_6,
        L_IN_7,
        L_IN_8,
        NUM_LIGHTS
    };

    std::string device_claim;
    
    std::string title;
    PackedColor title_bg;
    PackedColor title_fg;
    MacroData macros;
    int mod_target{-1};
    int last_mod_target{-2};
    bool glow_knobs;
    bool in_mat_poke;

    IOverlay* overlay{nullptr};

    XMModule();
    ~XMModule() {
        if (overlay) {
            overlay->overlay_unregister_client(this);
        }
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }

    XMUi* ui() { return reinterpret_cast<XMUi*>(chem_ui); };

    void set_modulation_target(int id) {
        if (getInput(id).isConnected()) {
            mod_target = id;
        }
    }

    // IOverlayClient
    void on_overlay_change(IOverlay* host) override { overlay = host; }
    IOverlay* get_overlay() override { return overlay; }

    // IDoMidi
    void do_message(PackedMidiMessage msg) override;

    // IExtendedMacro
    //virtual bool has_modulation() = 0;
    std::string get_title() override;
    PackedColor get_header_color() override;
    PackedColor get_header_text_color() override;
    void set_header_color(PackedColor color) override;
    void set_header_text_color(PackedColor color) override;
    void set_header_text(std::string title) override;
    void set_macro_edit(int index) override;
    std::shared_ptr<MacroDescription> get_macro(int index) override;
    void add_macro(int index) override;
    void remove_macro(int index) override;
    void on_macro_change(int index) override;

    // IChemClient
    rack::engine::Module* client_module() override;
    std::string client_claim() override;
    IDoMidi* client_do_midi() override { return this; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;
    //void onExpanderChange(const ExpanderChangeEvent& e) override;
    void onPortChange(const PortChangeEvent& e) override;
    void process_params(const ProcessArgs& args);
    void process(const ProcessArgs& args) override;
};

// -- XMacro UI -----------------------------------

//struct XMacroMenu;

struct XMUi : ChemModuleWidget, IChemClient, IExtendedMacro
{
    using Base = ChemModuleWidget;

    IChemHost* chem_host{nullptr};
    XMModule* my_module{nullptr};

    LinkButton* link_button{nullptr};
    IndicatorWidget* link{nullptr};
    ElementStyle current_style{"xm-current", "hsl(60,80%,75%)", "hsl(60,80%,75%)", .85f};

    int edit_item{0};
    Swatch* title_bar{nullptr};
    TextLabel * title{nullptr};
    PackedColor title_bg;
    PackedColor title_fg;
    MacroData* macro_data{nullptr};
    MacroUi macro_ui[8];

    XMUi(XMModule *module);

    bool host_connected();
    void glowing_knobs(bool glow);
    void center_knobs();

    bool editing() { return (edit_item >= 0) && (nullptr != get_edit_module()); }
    Module * get_edit_module();

    Vec knob_center(int index);

    // IExtendedMacro
    //virtual bool has_modulation() = 0;
    std::string get_title() override;
    PackedColor get_header_color() override;
    PackedColor get_header_text_color() override;
    void set_header_color(PackedColor color) override;
    void set_header_text_color(PackedColor color) override;
    void set_header_text(std::string title) override;
    void set_macro_edit(int index) override;
    std::shared_ptr<MacroDescription> get_macro(int index) override;
    void add_macro(int index) override;
    void remove_macro(int index) override;
    void on_macro_change(int index) override;

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override {}
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-xm.svg"); }
    void setThemeName(const std::string& name, void * context) override;

    void sync_labels();
    void onHoverKey(const HoverKeyEvent &e) override;
    void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

