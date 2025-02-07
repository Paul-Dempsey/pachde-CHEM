#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/themed-widgets.hpp"
#include "../../widgets/draw-button.hpp"
#include "../../widgets/hamburger.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/tip-label-widget.hpp"
#include "../../widgets/preset-widget.hpp"

using namespace pachde;

struct PlayUi;

struct PlayModule : ChemModule, IChemClient
{
    std::string device_claim;
    IChemHost* chem_host;
    PlayUi* ui;
    rack::dsp::Timer poll_host;

    std::string playlist_folder;
    std::string playlist_file;
    std::deque<std::string> playlist_mru;

    PlayModule();
    ~PlayModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }

    void update_mru(std::string path);
    void clear_mru() { playlist_mru.clear(); }

    // IChemClient
    rack::engine::Module* client_module() override;
    std::string client_claim() override;
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

struct PlayMenu;

constexpr const int PLAYLIST_LENGTH = 15;

struct PlayUi : ChemModuleWidget, IChemClient, IPresetAction
{
    using Base = ChemModuleWidget;

    IChemHost* chem_host = nullptr;
    PlayModule* my_module = nullptr;
    LinkButton* link_button = nullptr;
    UpButton * up_button = nullptr;
    DownButton* down_button = nullptr;
    PlayMenu* play_menu = nullptr;
    TipLabel* haken_device_label = nullptr;
    TipLabel* playlist_label = nullptr;
    StaticTextLabel* page_label = nullptr;
    StaticTextLabel* live_preset_label = nullptr;
    TipLabel* warning_label = nullptr;

    std::shared_ptr<PresetDescription> live_preset;

    std::deque<std::shared_ptr<PresetDescription>> presets;
    std::vector<PresetWidget*> preset_widgets;
    ssize_t scroll_top = 0;  // index of top preset

    std::string playlist_name;
    std::string playlist_device;

    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/CHEM-play.svg"); }

    PlayUi(PlayModule *module);
    bool connected();

    std::vector<int> selected;
    int first_selected();
    int last_selected();
    const std::vector<int>& get_selected_indices();
    void clear_selected() { selected.clear(); }
    bool is_selected(int index) { return selected.cend() != std::find(selected.cbegin(), selected.cend(), index); }
    void select_item(int index);
    void unselect_item(int index);

    std::vector<std::shared_ptr<PresetDescription>> extract(const std::vector<int>& list);

    void presets_to_json(json_t* root);
    void presets_from_json(json_t* root);
    void sync_to_presets();
    void update_live();
    void update_up_down();
    void page_up(bool ctrl, bool shift, bool refresh);
    void page_down(bool ctrl, bool shift, bool refresh);
    void make_visible(ssize_t index);
    void scroll_to(ssize_t index, bool refresh);
    void scroll_to_live();
    bool is_visible(ssize_t index);
    bool load_playlist(std::string path, bool set_folder);
    void add_current();
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
    void onDropFile(const widget::Widget::PathDropEvent& e) override;
    void onChoosePreset(PresetWidget* source) override;
    PresetWidget* getDropTarget(Vec pos) override;
    void onDropPreset(PresetWidget* target) override;

    void onHoverKey(const HoverKeyEvent& e) override;
    //void step() override;
    void draw(const DrawArgs& args) override;
    //void appendContextMenu(Menu *menu) override;
};

