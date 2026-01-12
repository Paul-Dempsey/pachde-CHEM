#pragma once
#include "my-plugin.hpp"
#include "chem.hpp"
#include "chem-core.hpp"
#include "em/EaganMatrix.hpp"
#include "services/colors.hpp"
#include "services/ModuleBroker.hpp"
#include "widgets/widgets.hpp"
#include "preset-common.hpp"
#include "preset-midi.hpp"
#include "preset-tab.hpp"
#include "./widgets/filter-widget.hpp"
#include "./widgets/preset-entry.hpp"

using namespace pachde;
using namespace eaganmatrix;
struct PresetUi;
struct PresetModule;

namespace S = pachde::style;
constexpr const float PANEL_WIDTH = 360.f;
constexpr const float RCENTER = PANEL_WIDTH - S::U1;
constexpr const int ROWS = 20;
constexpr const int COLS = 2;
constexpr const int PAGE_CAPACITY = ROWS * COLS;

inline ssize_t index_from_paged(uint8_t page, uint8_t offset) {
    return (page * PAGE_CAPACITY) + offset;
}

inline ssize_t page_of_index(ssize_t index) {
    if (index < 0) return 0;
    return index / PAGE_CAPACITY;
}

inline ssize_t offset_of_index(ssize_t index) {
    if (index < 0) return 0;
    return index % PAGE_CAPACITY;
}


struct PresetModule : ChemModule, IChemClient, INavigateList
{
    enum Params {
        P_NAV,
        NUM_PARAMS
    };
    enum Inputs {
        NUM_INPUTS
    };
    enum Outputs {
        NUM_OUTPUTS
    };
    enum Lights {
        L_FILTER,
        NUM_LIGHTS
    };

    PresetMidi preset_midi;

    std::string device_claim; // Core
    std::string search_query;
    PresetTab active_tab{PresetTab::System};

    uint64_t user_filters[5]{0};
    uint64_t system_filters[5]{0};
    uint64_t* filters();

    bool track_live{false};
    bool keep_search_filters{true};
    bool search_name{true};
    bool search_meta{true};
    bool search_anchor{false};
    bool search_incremental{true};

    PresetModule();
    ~PresetModule();

    PresetUi* ui() { return reinterpret_cast<PresetUi*>(chem_ui); }
    void set_nav_index(ssize_t index);
    ssize_t get_nav_index();

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;

    void clear_filters(PresetTab tab_id);

    // INavigateList
    NavUnit nav_unit{NavUnit::Index};
    void nav_send() override;
    //NavUnit nav_get_unit() override;
    void nav_set_unit(NavUnit unit) override;
    void nav_previous() override;
    void nav_next() override;
    void nav_item(uint8_t offset) override;
    void nav_absolute(uint16_t offset) override;

    // IChemClient
    rack::engine::Module* client_module() override { return this; }
    std::string client_claim() override { return device_claim; }
    //IDoMidi* client_do_midi() override { return this; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    void onRemove() override;
    void onRandomize() override;
    void process(const ProcessArgs& args) override;
};

struct PresetMenu;

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

constexpr const float PRESET_TOP = 38.f;

struct PresetUi : ChemModuleWidget, IChemClient, IHandleEmEvents, IPresetListClient
{
    using Base = ChemModuleWidget;
    PresetModule* my_module{nullptr};
    IChemHost*  chem_host{nullptr};
    LinkButton* link_button{nullptr};
    TipLabel* haken_device_label{nullptr};
    TextInput* search_entry{nullptr};

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

    LabelStyle tab_style{"tab-label", HAlign::Right, 16.f};
    LabelStyle current_tab_style{"tab-label-hi", HAlign::Right, 16.f, true};
    LabelStyle dytext_style{"dytext", HAlign::Center, 9.f};
    LabelStyle help_style{"curpreset", HAlign::Center, 16.f, true};
    LabelStyle cp_style{"curpreset", HAlign::Right, 10.f, true};

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
    ssize_t get_current_index() { return active_tab().current_index; }
    void set_nav_param(ssize_t index);
    void set_current_index(size_t index);
    bool host_available();
    PresetId get_live_id() { return live_preset ? live_preset->id : PresetId{}; }
    bool load_presets(PresetTab which);
    void sort_presets(PresetOrder order);
    void set_track_live(bool track);
    void set_live_current();
    void scroll_to(ssize_t index);
    void scroll_to_page_of_index(ssize_t index);
    ssize_t page_index(ssize_t index);
    void scroll_to_live();
    void page_up(bool control, bool shift);
    void page_down(bool control, bool shift);
    void update_page_controls();
    void update_help();
    void send_random_preset();
    void send_preset(ssize_t index);
    void previous_preset(bool c, bool s);
    void next_preset(bool c, bool s);
    void select_previous_preset();
    void select_next_preset();

    void on_search_text_changed(const std::string& text);
    void on_search_text_enter();
    bool filtering();
    void clear_filters();
    void on_filter_change(FilterId id, uint64_t state);

    void configure_midi();
    bool ready();

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-preset.svg"); }
    void createScrews() override;
    void hot_reload() override;

    // IPresetListClient
    void on_list_changed(eaganmatrix::PresetTab which) override;

    // IHandleEmEvents
    void onSystemBegin() override;
    void onSystemComplete() override;
    void onUserBegin() override;
    void onUserComplete() override;

    // ModuleWidget
    void onSelectKey(const SelectKeyEvent &e) override;
    void onHoverScroll(const HoverScrollEvent & e) override;

    void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

