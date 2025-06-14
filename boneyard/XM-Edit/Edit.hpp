#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../services/colors.hpp"
#include "../../services/em-midi-port.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/widgets.hpp"
#include "../XM-shared/xm-edit-common.hpp"
#include "../XM-shared/xm-overlay.hpp"
#include "add-remove-button.hpp"

using namespace pachde;

struct XMEditUi;

struct XMEditModule : ChemModule, IChemClient, IDoMidi, IOverlayClient
{
    enum Params {
        P_RANGE_MIN,
        P_RANGE_MAX,
        P_INPUT,
        P_MOD,
        NUM_PARAMS
    };
    enum Inputs {
        NUM_INPUTS
    };
    enum Outputs {
        NUM_OUTPUTS
    };
    enum Lights {
        L_XM,
        L_OVERLAY,
        L_CORE,
        NUM_LIGHTS
    };

    std::string device_claim;
    IOverlay* overlay{nullptr};
    IExtendedMacro* xm_client{nullptr};
    bool is_ready{false};

    XMEditModule();
    virtual ~XMEditModule();

    XMEditUi* ui() { return reinterpret_cast<XMEditUi*>(chem_ui); };
    bool ready() { return is_ready; }
    Module* get_xm_module();
    IExtendedMacro* get_xm_client();

    // IOverlayClient
    void on_overlay_change(IOverlay* host) override { overlay = host; }
    IOverlay* get_overlay() override { return overlay; }
    int64_t get_module_id() override { return id; }

    // IChemClient
    rack::engine::Module* client_module() override { return this; }
    std::string client_claim() override { return device_claim; }
    //IDoMidi* client_do_midi() override { return this; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;
    void onExpanderChange(const ExpanderChangeEvent& e) override;
    void process_params(const ProcessArgs& args);
    void process(const ProcessArgs& args) override;
};

// -- XMEdit UI -----------------------------------

struct XMEditMenu;

struct XMEditUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost*  chem_host{nullptr};
    XMEditModule* my_module{nullptr};
    //IndicatorWidget* link{nullptr};
    //LinkButton* link_button{nullptr};
    TextLabel* status{nullptr};
    TabHeader* tab_header{nullptr};
    PlusMinusButton* add_remove{nullptr};
    TextLabel* macro_number{nullptr};
    TextInput* title_entry{nullptr};
    TextInput* name_entry{nullptr};
    Palette1Button * palette_fg{nullptr};
    Palette2Button * palette_bg{nullptr};

    std::vector<StateIndicatorWidget*> range_options;
    ElementStyle current_style{"xm-current", "hsl(60,80%,75%)", "hsl(60,80%,75%)", .25f};

    int knob_index{0};
    std::string title;
    std::shared_ptr<MacroDescription> current_macro{nullptr};
    PackedColor title_bg_color;
    PackedColor title_fg_color;

    //MacroDescription macro;

    XMEditUi(XMEditModule *module);

    PackedColor get_title_background_color();
    PackedColor get_title_text_color();
    
    Module* get_xm_module();
    bool connected() { return nullptr != get_xm_module(); }
    IExtendedMacro* get_client() { return my_module ? my_module->get_xm_client() : nullptr; }
    IOverlay* get_overlay() { return my_module ? my_module->get_overlay() : nullptr; }

    void on_add_remove();
    void on_client_change();
    void on_item_change(int item);
    void refresh_macro_controls();
    void on_title_change(std::string text);
    void on_name_change(std::string text);

    void clear_xm();
    void set_title_background_color(PackedColor color);
    void set_title_text_color(PackedColor color);
    void set_range(MacroRange range);
    void set_range_ui(MacroRange range);
    void set_macro_number(uint8_t num);

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-xm-edit.svg"); }
    void setThemeName(const std::string& name, void * context) override;

    void step() override;
    void appendContextMenu(Menu *menu) override;
    void onSelectKey(const SelectKeyEvent &e) override;
    void draw(const DrawArgs& args) override;
};

