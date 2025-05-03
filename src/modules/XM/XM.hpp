#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../services/colors.hpp"
#include "../../services/color-help.hpp"
#include "../../services/em-midi-port.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/color-picker.hpp"
#include "../../widgets/element-style.hpp"
#include "../../widgets/indicator-widget.hpp"
#include "../../widgets/knob-track-widget.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/selector-widget.hpp"
#include "../../widgets/theme-button.hpp"
#include "../../widgets/theme-knob.hpp"
#include "../../widgets/tip-label-widget.hpp"
#include "../XM-Edit/xm-edit-common.hpp"

using namespace pachde;

struct MacroUi {
    ssize_t index;
    GlowKnob* knob{nullptr};
    TrackWidget* track{nullptr};
    TextLabel * label{nullptr};
};

struct MacroData
{
    std::vector<std::shared_ptr<MacroDescription>> data;
    MacroData() {}
    bool empty() { return data.empty(); }
    size_t size() { return data.size(); }

    void init(ssize_t count) {
        data.clear();
        for (ssize_t i = 0; i < count; ++i) {
            auto p = std::make_shared<MacroDescription>();
            p->index = i;
            add(p);
        }
    }
    void add(std::shared_ptr<MacroDescription> macro) {
        if (-1 == macro->index) {
            macro->index = ssize_t(data.size());
        }
        data.push_back(macro);
    }

    std::shared_ptr<MacroDescription> get_macro(ssize_t index) {
        return in_range(index, ssize_t(0), ssize_t(data.size()))
            ? data[index] : nullptr;
    }

    void from_json(json_t* root) {
        data.clear();
        auto jar = json_object_get(root, "macros");
        if (jar) {
            json_t* jp;
            size_t index;
            json_array_foreach(jar, index, jp) {
                auto macro = std::make_shared<MacroDescription>();
                macro->from_json(jp);
                data.push_back(macro);
            }
        }
    }

    void to_json(json_t* root) {
        auto jar = json_array();
        for (auto macro: data) {
            json_array_append_new(jar, macro->to_json());
        }
        json_object_set_new(root, "macros", jar);
    }
};


struct XMUi;

struct XMModule : ChemModule, IChemClient, IDoMidi, IExtendedMacro
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

    //Modulation modulation;
    std::string device_claim;
    bool glow_knobs;
    bool in_mat_poke;

    std::string title;
    PackedColor title_bg;
    PackedColor title_fg;
    MacroData macros;

    XMModule();
    ~XMModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }

    XMUi* ui() { return reinterpret_cast<XMUi*>(chem_ui); };
    // void set_modulation_target(int id) {
    //     modulation.set_modulation_target(id);
    // }

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
    void onPortChange(const PortChangeEvent& e) override {
        //modulation.onPortChange(e);
    }
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
    ElementStyle current_style{"xm-current", "hsl(60,80%,75%)", "hsl(60,80%,75%)", .25f};

    int edit_item = -1;
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

    bool editing() { return nullptr != get_edit_module(); }
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

