#pragma once
#include "../plugin.hpp"
#include "../chem.hpp"
#include "../services/ModuleBroker.hpp"
#include "../widgets/themed-widgets.hpp"
#include "../widgets/hamburger.hpp"
#include "../widgets/label-widget.hpp"
#include "../widgets/tip-label-widget.hpp"
using namespace pachde;

struct PlayModule : ChemModule, IChemClient
{
    std::string device_claim;

    void dataFromJson(json_t* root) override;

    json_t* dataToJson() override;

    // IChemClient
    rack::engine::Module* client_module() override;
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

};

struct PlayMenu;

struct PlayModuleWidget : ChemModuleWidget, IChemClient
{
    PlayModule* my_module = nullptr;
    IChemHost* host = nullptr;
    LinkButton* link_button = nullptr;
    PlayMenu* play_menu = nullptr;
    StaticTextLabel* haken_device_label = nullptr;
    TipLabel* playlist_label = nullptr;
    StaticTextLabel* page_label = nullptr;

    StaticTextLabel* temp_preset_label = nullptr; // todo: special control for adding

    std::string playlist_folder;
    std::string playlist_file;
    std::string playlist_name;

    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/CHEM-play.svg"); }

    PlayModuleWidget(PlayModule *module);

    void openPlaylist();
    void closePlaylist();
    void savePlaylist();
    void saveAsPlaylist();
    void clearPlaylist();

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    //void step() override;
    void appendContextMenu(Menu *menu) override;
};

