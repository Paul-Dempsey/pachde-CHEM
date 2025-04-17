#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../em/em-batch-state.hpp"
#include "../../services/colors.hpp"
#include "../../services/kv-store.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/blip-widget.hpp"
#include "../../widgets/theme-button.hpp"
#include "../../widgets/draw-button.hpp"
#include "../../widgets/hamburger.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/tip-label-widget.hpp"
#include "../../widgets/spinner.hpp"
#include "preset-widget.hpp"

using namespace pachde;

struct PlayUi;

struct PlayModule : ChemModule, IChemClient
{
    std::string device_claim;
    
    rack::dsp::SchmittTrigger prev_trigger;
    rack::dsp::SchmittTrigger next_trigger;
    
    std::string playlist_folder;
    std::string playlist_file;
    std::deque<std::string> playlist_mru;
    bool track_live;
    EmBatchState em_batch;
    
    PlayModule();
    ~PlayModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }
    
    PlayUi* ui() { return reinterpret_cast<PlayUi*>(chem_ui); }

    void update_mru(std::string path);
    void clear_mru() { playlist_mru.clear(); }
    bool batch_busy() { return em_batch.busy(); }

    // IChemClient
    rack::engine::Module* client_module() override;
    std::string client_claim() override;
    IDoMidi* client_do_midi() override { return &em_batch; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    // ----  Rack  ------------------------------------------

    enum Params {
        NUM_PARAMS
    };
    enum Inputs {
        IN_PRESET_PREV,
        IN_PRESET_NEXT,
        NUM_INPUTS
    };
    enum Outputs {
        NUM_OUTPUTS
    };
    enum Lights {
        NUM_LIGHTS
    };

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;

    void process(const ProcessArgs& args) override;
};

// -- Play UI -----------------------------------
enum class FillOptions { None, User, System, All };
struct EmHandler;
struct PlayMenu;
constexpr const int PAGE_CAPACITY = 15;

struct PlayUi : ChemModuleWidget, IChemClient, IPresetAction
{
    using Base = ChemModuleWidget;

    IChemHost*  chem_host{nullptr};
    PlayModule* my_module{nullptr};
    LinkButton* link_button{nullptr};
    UpButton *  up_button{nullptr};
    DownButton* down_button{nullptr};
    PlayMenu*   play_menu{nullptr};
    TipLabel*   haken_device_label{nullptr};
    TipLabel*   playlist_label{nullptr};
    TextLabel*  page_label{nullptr};
    TipLabel*   live_preset_label{nullptr};
    TipLabel*   warning_label{nullptr};
    Blip*       blip{nullptr};

    EmHandler* em_handler{nullptr};
    FillOptions gather{FillOptions::None};
    bool pending_device_check{false};
    bool modified{false};
    void set_modified(bool schmutz) {
        modified = schmutz;
        blip->set_brightness(modified ? 1.f : 0.f);
    }

    std::shared_ptr<PresetDescription> live_preset;
    PresetId get_live_id() { PresetId id; return live_preset ? live_preset->id : id; }
    std::shared_ptr<PresetDescription> current_preset;
    ssize_t current_index{-1};
    
    std::deque<std::shared_ptr<PresetDescription>> presets;
    ssize_t preset_count() { return static_cast<ssize_t>(presets.size()); }
    
    std::vector<PresetWidget*> preset_widgets;
    ssize_t scroll_top{0};  // index of top preset
    
    std::string playlist_name;
    std::string playlist_device;
    
    PlayUi(PlayModule *module);
    ~PlayUi();

    bool connected();
    void select_preset(PresetId id);
    void sync_to_current_index();
    void widgets_clear_current();
    
    std::vector<int> selected;
    int first_selected();
    int last_selected();
    const std::vector<int>& get_selected_indices();
    void clear_selected() { selected.clear(); }
    bool is_selected(int index) { return selected.cend() != std::find(selected.cbegin(), selected.cend(), index); }
    void select_item(int index);
    void unselect_item(int index);
    
    std::vector<std::shared_ptr<PresetDescription>> extract(const std::vector<int>& list);
    void set_track_live(bool track);
    void presets_to_json(json_t* root);
    void presets_from_json(json_t* root);
    void sync_to_presets();
    void update_live();
    void update_up_down();
    void page_up(bool ctrl, bool shift);
    void page_down(bool ctrl, bool shift);
    void make_visible(ssize_t index);
    void scroll_to(ssize_t index);
    void scroll_to_live();
    void scroll_to_page_of_index(ssize_t index);
    ssize_t page_index_of_index(ssize_t index);
    bool is_visible(ssize_t index);
    bool load_playlist(std::string path, bool set_folder);
    void add_live();
    void prev_preset();
    void next_preset();
    void open_playlist();
    void close_playlist();
    void save_playlist();
    void save_as_playlist();
    void clear_playlist(bool forget_file);
    void select_none();
    void clone();
    void remove_selected();
    void to_first();
    void to_last();
    void to_up();
    void to_down();
    void to_n(int dx);
    void check_playlist_device();
    void fill(FillOptions which);
    void on_fill_complete();

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // IPresetAction
    void onClearSelection() override;
    void onSetSelection(PresetWidget* source, bool on) override;
    void onDelete(PresetWidget* source) override;
    void onSetCurrrent(int index) override;
    void onDropFile(const widget::Widget::PathDropEvent& e) override;
    void onChoosePreset(PresetWidget* source) override;
    PresetWidget* getDropTarget(Vec pos) override;
    void onDropPreset(PresetWidget* target) override;
    Widget* widget() override { return this; }

    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-play.svg"); }
    void setThemeName(const std::string& name, void * context) override;

    void onHoverKey(const HoverKeyEvent& e) override;
    void onButton(const ButtonEvent&e) override;
    void onSelectKey(const SelectKeyEvent &e) override;
    void onHoverScroll(const HoverScrollEvent & e) override;

    void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

