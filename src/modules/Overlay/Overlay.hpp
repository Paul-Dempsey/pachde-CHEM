#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../services/colors.hpp"
#include "../../services/color-help.hpp"
#include "../../services/em-midi-port.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/color-picker.hpp"
#include "../../widgets/indicator-widget.hpp"
#include "../../widgets/knob-track-widget.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/selector-widget.hpp"
#include "../../widgets/theme-button.hpp"
#include "../../widgets/theme-knob.hpp"
#include "../../widgets/tip-label-widget.hpp"
#include "v-text.hpp"

using namespace pachde;

struct OverlayUi;

struct OverlayModule : ChemModule, IChemClient, IDoMidi
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
        L_CONNECTED,
        NUM_LIGHTS
    };

    bool in_mat_poke{false};
    bool preset_connected{false};
    std::string device_claim;
    std::string overlay_preset;
    std::string title;
    PackedColor bg_color{0};
    PackedColor fg_color;

    OverlayModule();
    ~OverlayModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }

    OverlayUi* ui() { return reinterpret_cast<OverlayUi*>(chem_ui); };

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
    // void onPortChange(const PortChangeEvent& e) override {
    //     //modulation.onPortChange(e);
    // }
    void process_params(const ProcessArgs& args);
    void process(const ProcessArgs& args) override;
};

// -- Overlay_ UI -----------------------------------

struct Overlay_Menu;

struct OverlayUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost*  chem_host{nullptr};
    OverlayModule*   my_module{nullptr};

    LinkButton* link_button{nullptr};
    IndicatorWidget* link{nullptr};
    Swatch* bg_widget{nullptr};
    VText* title_widget{nullptr};

    OverlayUi(OverlayModule *module);

    bool connected();
    void set_title(std::string text);
    void set_preset(std::string preset);
    void set_bg_color(PackedColor color);
    void set_fg_color(PackedColor color);
    PackedColor get_bg_color() { return my_module ? my_module->bg_color : 0; }
    PackedColor get_fg_color() { return my_module ? my_module->fg_color : parse_color("hsl(42,60%,50%)"); }
    
    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override {}
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-xovr.svg"); }
    void setThemeName(const std::string& name, void * context) override;

    void onHoverKey(const HoverKeyEvent &e) override;
    void step() override;
    void appendContextMenu(Menu *menu) override;
};

