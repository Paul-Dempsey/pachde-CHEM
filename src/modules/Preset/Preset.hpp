#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../chem-core.hpp"
#include "../../em/EaganMatrix.hpp"
#include "../../services/colors.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/widgets.hpp"
#include "preset-common.hpp"
#include "preset-tab.hpp"
#include "./widgets/filter-widget.hpp"
#include "./widgets/preset-entry.hpp"

using namespace pachde;
using namespace eaganmatrix;
struct PresetUi;

namespace S = pachde::style;
constexpr const float PANEL_WIDTH = 360.f;
constexpr const float RCENTER = PANEL_WIDTH - S::U1;

struct PresetModule : ChemModule, IChemClient, IDoMidi
{
    enum Params {
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
        L_FILTER,
        NUM_LIGHTS
    };

    std::string device_claim;
    bool track_live{false};
    bool keep_search_filters{true};
    PresetTab active_tab{PresetTab::System};

    uint64_t user_filters[5]{0};
    uint64_t system_filters[5]{0};
    uint64_t* filters();
    std::string search_query;
    bool search_name{true};
    bool search_meta{true};
    bool search_anchor{false};
    bool search_incremental{false};

    PresetModule();
    ~PresetModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }
    PresetUi* ui() { return reinterpret_cast<PresetUi*>(chem_ui); }

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;

    void clear_filters(PresetTab tab_id);

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

struct Tab
{
    size_t scroll_top{0};
    ssize_t current_index{-1};
    PresetTabList list;

    Tab(const Tab&) = delete;
    Tab(PresetTab id) : list(id) {}
    void set_list(std::shared_ptr<PresetList> shlist) {
        scroll_top = 0;
        current_index = -1;
        list.set_list(shlist);
    }
    PresetTab id() { return list.tab; }
    size_t count() { return list.count(); }
    void clear() { list.clear(); scroll_top = 0; current_index = -1; }
    PresetId current_id() {
        PresetId id;
        if (current_index < 0) return id;
        return list.count() ? list.nth(current_index)->id : id;
    }
};

struct PresetUi : ChemModuleWidget, IChemClient, IHandleEmEvents, IPresetListClient
{
    using Base = ChemModuleWidget;
    PresetModule* my_module{nullptr};
    IChemHost*  chem_host{nullptr};
    LinkButton* link_button{nullptr};
    TipLabel* haken_device_label{nullptr};
    TextInput* search_entry{nullptr};
    LabelStyle tab_style{"tab-label", TextAlignment::Right, 16.f};
    LabelStyle current_tab_style{"tab-label-hi", TextAlignment::Right, 16.f, true};
    TextLabel* user_label{nullptr};
    TextLabel* system_label{nullptr};
    TextLabel* page_label{nullptr};
    TextLabel* help_label{nullptr};
    UpButton* up_button{nullptr};
    DownButton* down_button{nullptr};
    TipLabel* live_preset_label{nullptr};
    PresetMenu* menu{nullptr};
    std::vector<FilterButton*> filter_buttons;
    StateButton * filter_off_button{nullptr};

    WallTimer start_delay{3.5};

    std::shared_ptr<PresetDescription> live_preset;

    PresetTab active_tab_id{PresetTab::System};
    Tab user_tab {PresetTab::User};
    Tab system_tab {PresetTab::System};

    bool other_user_gather{false};
    bool other_system_gather{false};

    std::vector<PresetEntry*> preset_grid;

    PresetUi(PresetModule *module);
    virtual ~PresetUi();

    Tab& get_tab(PresetTab id) {
       switch (id) {
        case PresetTab::Unset: assert(false); goto sys;
        default:
        case PresetTab::System:
        sys: return system_tab;
        case PresetTab::User: return user_tab;
       }
    }
    bool is_osmose() { 
        return chem_host 
            && chem_host->host_matrix()
            && chem_host->host_matrix()->is_osmose();
    }
    Tab& active_tab() { return get_tab(active_tab_id); }
    void set_tab(PresetTab tab, bool fetch);
    PresetTabList& preset_list(PresetTab which) { return get_tab(which).list; }
    PresetTabList& presets() { return preset_list(active_tab_id); }
    void set_current_index(size_t index);
    bool host_available();
    PresetId get_live_id() { return live_preset ? live_preset->id : PresetId{}; }
    bool load_presets(PresetTab which);
    void sort_presets(PresetOrder order);
    void set_track_live(bool track);
    void scroll_to(ssize_t index);
    void scroll_to_page_of_index(ssize_t index);
    ssize_t page_index(ssize_t index);
    void scroll_to_live();
    void page_up(bool control, bool shift);
    void page_down(bool control, bool shift);
    void update_page_controls();
    void update_help();
    
    void send_preset(ssize_t index);
    void previous_preset(bool c, bool s);
    void next_preset(bool c, bool s);

    void on_search_text_changed(std::string text);
    void on_search_text_enter();
    bool filtering();
    void clear_filters();
    void on_filter_change(FilterId id, uint64_t state);

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-preset.svg"); }
    void createScrews(std::shared_ptr<SvgTheme> theme) override;

    // IPresetListClient
    void on_list_changed(eaganmatrix::PresetTab which) override;

    // IHandleEmEvents
    void onSystemBegin() override;
    void onSystemComplete() override;
    void onUserBegin() override;
    void onUserComplete() override;

    // ModuleWidget
    void onButton(const ButtonEvent&e) override;
    void onSelectKey(const SelectKeyEvent &e) override;
    void onHoverScroll(const HoverScrollEvent & e) override;

    void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

