#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../chem-core.hpp"
#include "../../services/colors.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/draw-button.hpp"
#include "../../widgets/hamburger.hpp"
#include "../../widgets/theme-button.hpp"
#include "../../widgets/tip-label-widget.hpp"
#include "preset-common.hpp"
#include "preset-list.hpp"
#include "search-widget.hpp"

using namespace pachde;
struct PresetUi;

struct PresetModule : ChemModule, IChemClient, IDoMidi
{
    enum Params {
        P_CAT,
        P_TYPE,
        P_CHARACTER,
        P_MATRIX,
        P_GEAR,
        NUM_PARAMS
    };
    enum Inputs {
        IN_PREV,
        IN_NEXT,
        NUM_INPUTS
    };
    enum Outputs {
        NUM_OUTPUTS
    };
    enum Lights {
        NUM_LIGHTS
    };

    std::string device_claim;
    bool track_live;

    PresetModule();
    ~PresetModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }
    PresetUi* ui() { return reinterpret_cast<PresetUi*>(chem_ui); }

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;

    // IDoMidi
    void do_message(PackedMidiMessage message) override;

    // IChemClient
    rack::engine::Module* client_module() override { return this; }
    std::string client_claim() override { return device_claim; }
    IDoMidi* client_do_midi() override { return this; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    void process(const ProcessArgs& args) override;
};

struct PresetMenu;

struct PresetUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;
    PresetModule* my_module{nullptr};
    IChemHost*  chem_host{nullptr};
    LinkButton* link_button{nullptr};
    TipLabel* haken_device_label{nullptr};

    SearchField* search_entry{nullptr};

    LabelStyle tab_style{"tab-label", TextAlignment::Right, 16.f};
    LabelStyle current_tab_style{"tab-label-hi", TextAlignment::Right, 16.f, true};
    TextLabel* user_label{nullptr};
    TextLabel* system_label{nullptr};
    
    TextLabel* page_label{nullptr};
    UpButton* up_button{nullptr};
    DownButton* down_button{nullptr};
    TipLabel* live_preset_label{nullptr};
    PresetMenu* menu;
    
    std::shared_ptr<PresetDescription> live_preset;

    PresetTab active_tab{PresetTab::System};
    PresetList user_presets{PresetTab::User};
    PresetList system_presets{PresetTab::System};

    PresetUi(PresetModule *module);

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-preset.svg"); }
    void createScrews(std::shared_ptr<SvgTheme> theme) override;
    
    PresetId get_live_id() { PresetId id; return live_preset ? live_preset->id : id; }
    void set_track_live(bool track);
    void on_search_text_changed();
    void on_search_text_enter();
    void set_tab(PresetTab tab);
    void page_up(bool control, bool shift);
    void page_down(bool control, bool shift);
    void previous_preset(bool c, bool s);
    void next_preset(bool c, bool s);

    void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

