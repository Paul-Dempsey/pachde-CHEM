#pragma once
#include "../plugin.hpp"
#include "../chem.hpp"
#include "../services/ModuleBroker.hpp"
#include "../widgets/themed-widgets.hpp"
#include "../widgets/draw-button.hpp"
#include "../widgets/hamburger.hpp"
#include "../widgets/label-widget.hpp"
#include "../widgets/tip-label-widget.hpp"
#include "../widgets/preset-widget.hpp"

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

struct PlayUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;
    PlayModule* my_module = nullptr;
    IChemHost* host = nullptr;
    LinkButton* link_button = nullptr;
    UpButton * up_button = nullptr;
    DownButton* down_button = nullptr;
    PlayMenu* play_menu = nullptr;
    StaticTextLabel* haken_device_label = nullptr;
    TipLabel* playlist_label = nullptr;
    StaticTextLabel* page_label = nullptr;
    StaticTextLabel* live_preset_label = nullptr;
    std::shared_ptr<PresetDescription> live_preset;

    std::vector<std::shared_ptr<PresetDescription>> presets;
    std::vector<PresetWidget*> preset_widgets;
    size_t scroll_top = 0;  // index of top preset

    std::string playlist_name;

    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/CHEM-play.svg"); }

    PlayUi(PlayModule *module);

    void presetsToJson(json_t* root);
    void presetsFromJson(json_t* root);
    void sync_to_presets();
    void update_live();
    void update_up_down();
    void scroll_up(bool ctrl, bool shift);
    void scroll_down(bool ctrl, bool shift);
    void make_visible(size_t index);
    void scroll_to(size_t index);
    void scroll_to_live();
    bool is_visible(size_t index);
    bool load_playlist(std::string path);
    void addCurrent();
    void openPlaylist();
    void closePlaylist();
    void savePlaylist();
    void saveAsPlaylist();
    void clearPlaylist(bool forget_file);

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    //void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

