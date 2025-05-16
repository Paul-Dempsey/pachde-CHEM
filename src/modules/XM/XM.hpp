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

using namespace pachde;

struct XMUi;

struct XMModule : ChemModule, IChemClient, IDoMidi, IOverlayClient
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
        P_RANGE_MIN,
        P_RANGE_MAX,
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
        L_OVERLAY,
        L_CORE,
        NUM_LIGHTS
    };

    std::string device_claim;
    
    std::string title;
    PackedColor title_bg;
    PackedColor title_fg;
    bool has_mod_knob;

    int mod_target{-1};
    int last_mod_target{-2};
    bool glow_knobs;
    bool in_mat_poke;

    MacroData my_macros;

    IOverlay* overlay{nullptr};

    XMModule();
    virtual ~XMModule();

    XMUi* ui() { return reinterpret_cast<XMUi*>(chem_ui); };

    void set_modulation_target(int id) {
        if (getInput(id).isConnected()) {
            mod_target = id;
        }
    }

    void update_param_info();
    
    // IOverlayClient
    void on_overlay_change(IOverlay* ovr) override;
    IOverlay* get_overlay() override { return overlay; }
    int64_t get_module_id() override { return id; }

    // IDoMidi
    void do_message(PackedMidiMessage msg) override;

    void xm_clear();

    // IChemClient
    rack::engine::Module* client_module() override;
    std::string client_claim() override;
    IDoMidi* client_do_midi() override { return this; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;
    void try_bind_overlay();
    
    //void onExpanderChange(const ExpanderChangeEvent& e) override;
    void onPortChange(const PortChangeEvent& e) override;
    void process_params(const ProcessArgs& args);
    void process(const ProcessArgs& args) override;
};

// -- XMacro UI -----------------------------------

struct EditWireframe;
struct MacroEdit;

struct XMUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost* chem_host{nullptr};
    XMModule* my_module{nullptr};

    // LinkButton* link_button{nullptr};
    // IndicatorWidget* link{nullptr};

    ElementStyle edit_style{"xm-edit", "hsl(60,80%,75%)", "hsl(60,80%,75%)", .85f};
    ElementStyle placeholder_style{"xm-placeholder", "hsl(0,0%,55%)", "hsl(0,0%,55%)", .25f};
    
    Swatch* title_bar{nullptr};
    TextLabel * title{nullptr};
    PackedColor title_bg;
    PackedColor title_fg;
    MacroEdit* edit_macro{nullptr};
    EditWireframe* wire_frame{nullptr};
    bool editing{false};
    bool draw_placeholders{true};
    TrimPot* knobs[9]{nullptr};
    TextLabel* labels[8]{nullptr};

    XMUi(XMModule *module);
    virtual ~XMUi() {
        if (editing) set_edit_mode(false);
    }

    void glowing_knobs(bool glow);
    void center_knobs();

    void update_main_ui(std::shared_ptr<SvgTheme> theme);

    IOverlay* get_overlay() { return my_module ? my_module->get_overlay() : nullptr; }
    Vec knob_center(int index);
    Vec input_center(int index);
    void set_edit_mode(bool edit);
    void set_edit_item(int index);
    void commit_macro();
    MacroDescription* get_edit_macro();
    std::shared_ptr<MacroDescription> get_persistent_macro(int index);
    bool get_modulation();
    void set_modulation(bool mod);

    void xm_clear();
    std::string get_header_text();
    PackedColor get_header_color();
    PackedColor get_header_text_color();
    void set_header_color(PackedColor color);
    void set_header_text_color(PackedColor color);
    void set_header_text(std::string title);

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

