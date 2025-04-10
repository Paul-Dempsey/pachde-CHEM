#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../chem-core.hpp"
#include "../../em/EaganMatrix.hpp"
#include "../../services/colors.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/draw-button.hpp"
#include "../../widgets/hamburger.hpp"
#include "../../widgets/theme-button.hpp"
#include "../../widgets/tip-label-widget.hpp"
#include "preset-common.hpp"
#include "preset-entry.hpp"
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
    uint8_t hardware{0};
    uint16_t firmware{0};
    bool track_live{false};
    bool use_cached_user_presets{false};
    int active_tab{0};

    PresetModule();
    ~PresetModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }
    PresetUi* ui() { return reinterpret_cast<PresetUi*>(chem_ui); }

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;

    // IChemClient
    rack::engine::Module* client_module() override { return this; }
    std::string client_claim() override { return device_claim; }
    //IDoMidi* client_do_midi() override { return this; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    void process(const ProcessArgs& args) override;
};

struct PresetMenu;
constexpr const int ROWS = 20;
constexpr const int COLS = 2;
constexpr const int PAGE_CAPACITY = ROWS * COLS;

struct PresetUi : ChemModuleWidget, IChemClient, IHandleEmEvents
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

    struct Tab {
        size_t scroll_top{0};
        PresetList list;
        Tab(const Tab&) = delete;
        Tab(PresetTab id) : list(id) {}
        PresetTab id() { return list.tab; }
        void clear() { list.clear(); }
    };
    PresetTab active_tab_id{PresetTab::System};
    Tab user_tab {PresetTab::User};
    Tab system_tab {PresetTab::System};
    PresetTab gathering{PresetTab::Unset};
    bool other_user_gather{false};
    bool other_system_gather{false};
    std::vector<PresetEntry*> preset_grid;

    PresetUi(PresetModule *module);
    
    Tab& get_tab(PresetTab id) {
       switch (id) {
        case PresetTab::Unset: assert(false); goto sys;
        default:
        case PresetTab::System:
        sys: return system_tab;
        case PresetTab::User: return user_tab;
       }
    }
    Tab& active_tab() { return get_tab(active_tab_id); }
    PresetList& preset_list(PresetTab which) { 
        return get_tab(which).list;
    }
    PresetList& presets() {
        return preset_list(active_tab_id);
    }

    bool host_available();

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-preset.svg"); }
    void createScrews(std::shared_ptr<SvgTheme> theme) override;
    
    PresetId get_live_id() { PresetId dead; return live_preset ? live_preset->id : dead; }
    void request_presets(PresetTab which);
    std::string preset_file(PresetTab which);
    bool load_presets(PresetTab which);
    bool save_presets(PresetTab which);
    void set_track_live(bool track);
    void set_tab(PresetTab tab);
    void scroll_to(ssize_t index);
    void scroll_to_live();
    void clear_filters();
    void page_up(bool control, bool shift);
    void page_down(bool control, bool shift);
    void update_page_controls();

    void previous_preset(bool c, bool s);
    void next_preset(bool c, bool s);

    void on_search_text_changed();
    void on_search_text_enter();

    // IHandleEmEvents
    IHandleEmEvents::EventMask em_event_mask{(EventMask)(UserBegin + SystemComplete + UserBegin + UserComplete)};
    void onSystemBegin() override;
    void onSystemComplete() override;
    void onUserBegin() override;
    void onUserComplete() override;

    // ModuleWidget
    void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

