#include "Play.hpp"
#include <ghc/filesystem.hpp>
#include "../services/colors.hpp"
#include "../services/open-file.hpp"
#include "../widgets/draw-button.hpp"

using namespace svg_theme;
using namespace pachde;
namespace fs = ghc::filesystem;

struct PlayMenu : Hamburger
{
    PlayModuleWidget* ui;
    PlayMenu() : ui(nullptr) { }
    void setUi(PlayModuleWidget* w) { ui = w; }
    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override;
    void appendContextMenu(ui::Menu* menu) override;
};

bool PlayMenu::applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
    return Hamburger::applyTheme(engine, theme);
}

void PlayMenu::appendContextMenu(ui::Menu* menu)
{
    menu->addChild(createMenuItem("--- Playlist Actions --", "", [](){}, true));
    menu->addChild(createMenuItem("Open...", "",    [this](){ ui->openPlaylist(); }, false));
    menu->addChild(createMenuItem("Close", "",      [this](){ ui->closePlaylist(); }, false));
    menu->addChild(createMenuItem("Save as...", "", [this](){ ui->saveAsPlaylist(); } , false));
    menu->addChild(createMenuItem("Clear", "",      [this](){ ui->clearPlaylist(); }, false));
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem("Add", "", [this](){}, false));
    menu->addChild(createMenuItem("Duplicate", "", [this](){}, false));
    menu->addChild(createMenuItem("Remove", "", [this](){}, false));
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem("Move to first", "", [this](){}, false));
    menu->addChild(createMenuItem("Move up", "", [this](){}, false));
    menu->addChild(createMenuItem("Move down", "", [this](){}, false));
    menu->addChild(createMenuItem("Move to last", "", [this](){}, false));
}

// ----------------------------------------------

PlayModuleWidget::PlayModuleWidget(PlayModule *module)
{
    my_module = module;
    setModule(module);

    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    this->panelBorder = new PartnerPanelBorder();
    replacePanelBorder(panel, this->panelBorder);
    setPanel(panel);

    LabelStyle style{"dytext", TextAlignment::Left, 10.f};

    addChild(haken_device_label = createStaticTextLabel<StaticTextLabel>(
        Vec(28.f, box.size.y - 13.f), 200.f, "<em device>", theme_engine, theme, style));

    style.height = 16.f;
    addChild(playlist_label = createStaticTextLabel<TipLabel>(
        Vec(15.f, 7.5), 148.f, "<playlist-file>", theme_engine, theme, style));

    style.height = 9.f;
    style.align = TextAlignment::Center;
    addChild(page_label = createStaticTextLabel<StaticTextLabel>(
        Vec(187.5f, 35.f),  35.f, "1 of 1", theme_engine, theme, style));

    style.key = "curpreset";
    style.height = 14.f;
    style.align = TextAlignment::Left;
    style.bold = true;
    addChild(temp_preset_label = createStaticTextLabel<StaticTextLabel>(
        Vec( 15.f, 345.f), 150.f, "<preset>", theme_engine, theme, style));

    play_menu = createThemedWidget<PlayMenu>(Vec(150.f, 7.5f), theme_engine, theme);
    play_menu->setUi(this);
    play_menu->describe("Play actions menu");
    addChild(play_menu);

    auto up = createWidgetCentered<UpButton>(Vec(186.f, 51.5f));
    up->describe("Scroll up");
    up->setHandler([this](bool c, bool s){});
    addChild(up);

    auto down = createWidgetCentered<DownButton>(Vec(186.f, 65.f));
    down->describe("Scroll down");
    down->setHandler([this](bool c, bool s){});
    addChild(down);

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y-15.f), theme_engine, theme, "Core link");
    link_button->setHandler([=](bool ctrl, bool shift) {
        ui::Menu* menu = createMenu();
        menu->addChild(createMenuItem("-- Link to Core --", "", [](){}, true));
        //auto broker = ModuleBroker::get();
        menu->addChild(createMenuItem("Continuum 700108", "", [](){}, false));
        menu->addChild(createMenuItem("CMini SN001503", "", [](){}, false));
    });
    addChild(link_button);
}

void PlayModuleWidget::openPlaylist()
{
    std::string path;
    bool ok = openFileDialog(playlist_folder, "Playlists (.json):json;Any (*):*", playlist_name, path);
    if (ok) {
        playlist_folder = system::getDirectory(path);
        playlist_name = system::getStem(path);
        playlist_file = path;

        playlist_label->text(playlist_name);
        playlist_label->describe(playlist_file);
    } else {
        playlist_name = "";
        playlist_file = "";
    }
}

void PlayModuleWidget::closePlaylist()
{
    playlist_name = "";
    playlist_file = "";
    playlist_label->text("");
    playlist_label->describe("");
}

void PlayModuleWidget::savePlaylist()
{

}

void PlayModuleWidget::saveAsPlaylist()
{
    std::string path;
    bool ok = saveFileDialog(playlist_folder, "Playlists (.json):json;Any (*):*", playlist_name, path);
    if (ok) {
        playlist_file = path;
        playlist_folder = system::getDirectory(path);
        playlist_name = system::getStem(path);

        playlist_label->text(playlist_name);
        playlist_label->describe(playlist_file);
    } else {
    }    
}

void PlayModuleWidget::clearPlaylist()
{

}

void PlayModuleWidget::onConnectHost(IChemHost* host)
{
    this->host = host;
    host->register_chem_client(this);
}

void PlayModuleWidget::onPresetChange()
{
    if (!host) return;
    auto preset = host->host_preset();
    temp_preset_label->text(preset ? printable(preset->name): "");
}

void PlayModuleWidget::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    haken_device_label->text(connection->info.friendly(TextFormatLength::Compact));
}


// void PlayModuleWidget::step()
// {
//     ChemModuleWidget::step();
// }

void PlayModuleWidget::appendContextMenu(Menu *menu)
{
    ChemModuleWidget::appendContextMenu(menu);
}
