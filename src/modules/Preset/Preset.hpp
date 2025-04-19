#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../chem-core.hpp"
#include "../../em/EaganMatrix.hpp"
#include "../../services/colors.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/bits-widget.hpp"
#include "../../widgets/draw-button.hpp"
#include "../../widgets/theme-button.hpp"
#include "../../widgets/tip-label-widget.hpp"
#include "preset-common.hpp"
#include "preset-entry.hpp"
#include "preset-list.hpp"
#include "filter-widget.hpp"
#include "search-widget.hpp"

using namespace pachde;
struct PresetUi;

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
    //uint8_t hardware{0};
    //uint16_t firmware{0};
    bool track_live{false};
    bool use_cached_user_presets{false};
    bool keep_search_filters{true};
    PresetTab active_tab{PresetTab::System};

    PresetOrder user_order{PresetOrder::Natural};
    PresetOrder system_order{PresetOrder::Alpha};

    uint64_t user_filters[5];
    uint64_t system_filters[5];
    uint64_t* filters();

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
    void set_order(PresetTab tab_id, PresetOrder order);
    //void set_filter(PresetTab tab_id, FilterId which, uint64_t mask);

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

struct Tab {
    size_t scroll_top{0};
    ssize_t current_index{-1};
    PresetList list;

    Tab(const Tab&) = delete;
    Tab(PresetTab id) : list(id) {}

    PresetTab id() { return list.tab; }
    size_t count() { return list.count(); }
    void clear() { list.clear(); scroll_top = 0; current_index = -1; }
    PresetId current_id() {
        PresetId id;
        if (current_index < 0) return id;
        return list.nth(current_index)->id;
    }
};

struct DBBuilder {
    std::deque<PresetId> presets;
    void init(const std::vector<std::shared_ptr<PresetInfo>>& source, PresetId current) {
        for (auto p : source) {
            presets.push_back(p->id);
        }
        if (current.valid()) {
            presets.push_back(current);
        }
    }
    bool next(HakenMidi* haken) {
        if (presets.empty()) return false;
        PresetId id = presets.front();
        presets.pop_front();
        haken->select_preset(ChemId::Preset, id);
        return true;
    }
};

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
    PresetMenu* menu{nullptr};
    
    CatFilter* cat_filter{nullptr};
    TypeFilter* type_filter{nullptr};
    CharacterFilter* character_filter{nullptr};
    MatrixFilter* matrix_filter{nullptr};
    GearFilter* setting_filter{nullptr};

    std::shared_ptr<PresetDescription> live_preset;

    PresetTab active_tab_id{PresetTab::System};
    Tab user_tab {PresetTab::User};
    Tab system_tab {PresetTab::System};
    PresetTab gathering{PresetTab::Unset};
    bool gather_start{false};
    bool other_user_gather{false};
    bool other_system_gather{false};
    bool spinning{false};
    std::vector<PresetEntry*> preset_grid;

    DBBuilder* db_builder{nullptr};

    PresetUi(PresetModule *module);
    ~PresetUi();
    
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
    PresetList& preset_list(PresetTab which) { return get_tab(which).list; }
    PresetList& presets() { return preset_list(active_tab_id); }
    void set_current_index(size_t index);
    bool host_available();
    void start_spinner();
    void stop_spinner();
    PresetId get_live_id() { PresetId dead; return live_preset ? live_preset->id : dead; }
    void build_database(PresetTab which);
    void request_presets(PresetTab which);
    std::string preset_file(PresetTab which);
    bool load_presets(PresetTab which);
    bool save_presets(PresetTab which);
    void sort_presets(PresetOrder order);
    void set_track_live(bool track);
    void set_tab(PresetTab tab, bool fetch);
    void scroll_to(ssize_t index);
    void scroll_to_page_of_index(ssize_t index);
    ssize_t page_index(ssize_t index);
    void scroll_to_live();
    void page_up(bool control, bool shift);
    void page_down(bool control, bool shift);
    void update_page_controls();
    
    void previous_preset(bool c, bool s);
    void next_preset(bool c, bool s);
    
    void on_search_text_changed();
    void on_search_text_enter();
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
    
    // IHandleEmEvents
    IHandleEmEvents::EventMask em_event_mask{(EventMask)(UserBegin + SystemComplete + UserBegin + UserComplete)};
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

