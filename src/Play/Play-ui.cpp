#include "Play.hpp"
#include <ghc/filesystem.hpp>
#include "../services/colors.hpp"
#include "../services/open-file.hpp"

using namespace svg_theme;
using namespace pachde;
namespace fs = ghc::filesystem;

struct PlayMenu : Hamburger
{
    using Base = Hamburger;

    PlayUi* ui;
    PlayMenu() : ui(nullptr) { }

    void setUi(PlayUi* w) { ui = w; }
    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override;
    void appendContextMenu(ui::Menu* menu) override;
};

bool PlayMenu::applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
    return Base::applyTheme(engine, theme);
}

void PlayMenu::appendContextMenu(ui::Menu* menu)
{
    menu->addChild(createMenuLabel("--- Playlist Actions --"));
    menu->addChild(createMenuItem("Open...", "",    [this](){ ui->openPlaylist(); }, false));
    menu->addChild(createMenuItem("Close", "",      [this](){ ui->closePlaylist(); }, false));
    menu->addChild(createMenuItem("Save", "",       [this](){ ui->savePlaylist(); } , false));
    menu->addChild(createMenuItem("Save as...", "", [this](){ ui->saveAsPlaylist(); } , false));
    if (ui->my_module) {
        menu->addChild(createSubmenuItem("Open recent", "", [=](Menu* menu) {
            if (ui->my_module->playlist_mru.empty()) {
                menu->addChild(createMenuLabel("(empty)"));
            } else {
                for (auto path : ui->my_module->playlist_mru) {
                    auto name = system::getStem(path);
                    menu->addChild(createMenuItem(name, "", [=]() {
                        ui->load_playlist(path);
                    }));
                }
            }
        }));
    }
    // menu->addChild(new MenuSeparator);
    // menu->addChild(createMenuItem("Add", "", [this](){}, false));
    // menu->addChild(createMenuItem("Duplicate", "", [this](){}, false));
    // menu->addChild(createMenuItem("Remove", "", [this](){}, false));
    // menu->addChild(new MenuSeparator);
    // menu->addChild(createMenuItem("Move to first", "", [this](){}, false));
    // menu->addChild(createMenuItem("Move up", "", [this](){}, false));
    // menu->addChild(createMenuItem("Move down", "", [this](){}, false));
    // menu->addChild(createMenuItem("Move to last", "", [this](){}, false));
}

// ----------------------------------------------
constexpr const float ONEU = 15.f;
constexpr const float HALFU = 7.5f;
constexpr const float RIGHT_MARGIN_CENTER = 186.f;
constexpr const size_t SIZE_0 = 0;

PlayUi::PlayUi(PlayModule *module)
{
    my_module = module;
    setModule(module);
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    this->panelBorder = new PartnerPanelBorder();
    replacePanelBorder(panel, this->panelBorder);
    setPanel(panel);

    float y = 20.f;
    for (int i = 0; i < 15; ++i) {
        auto pw = createPresetWidget(&presets, 12.f, y, theme_engine, theme);
        pw->preset_name->text(format_string("%d", i));
        preset_widgets.push_back(pw);
        addChild(pw);
        y += 21;
    }
    LabelStyle style{"dytext", TextAlignment::Left, 10.f};

    addChild(haken_device_label = createStaticTextLabel<StaticTextLabel>(
        Vec(28.f, box.size.y - 13.f), 200.f, "[em device]", theme_engine, theme, style));

    style.height = 16.f;
    addChild(playlist_label = createStaticTextLabel<TipLabel>(
        Vec(ONEU, 3.5), 148.f, "[playlist file]", theme_engine, theme, style));

    style.height = 9.f;
    style.align = TextAlignment::Center;
    addChild(page_label = createStaticTextLabel<StaticTextLabel>(
        Vec(RIGHT_MARGIN_CENTER, 35.f),  35.f, "1 of 1", theme_engine, theme, style));

    style.key = "curpreset";
    style.height = 14.f;
    style.bold = true;
    addChild(live_preset_label = createStaticTextLabel<StaticTextLabel>(
        Vec(87, 340.f), 150.f, "[preset]", theme_engine, theme, style));

    auto heart = createThemedButton<HeartButton>(Vec(ONEU, 342.f), theme_engine, theme, "Add to playlist");
    heart->setHandler([this](bool c, bool s){ 
        addCurrent();
    });
    addChild(heart);

    play_menu = createThemedWidget<PlayMenu>(Vec(150.f, HALFU), theme_engine, theme);
    play_menu->setUi(this);
    play_menu->describe("Play actions menu");
    addChild(play_menu);

    up_button = createWidgetCentered<UpButton>(Vec(RIGHT_MARGIN_CENTER, 52.f));
    up_button->describe("Scroll up");
    up_button->setHandler([this](bool c, bool s){ 
        scroll_up(c, s);
    });
    up_button->applyTheme(theme_engine, theme);
    addChild(up_button);

    down_button = createWidgetCentered<DownButton>(Vec(RIGHT_MARGIN_CENTER, 67.f));
    down_button->describe("Scroll down");
    down_button->setHandler([this](bool c, bool s){
        scroll_down(c, s);
    });
    down_button->applyTheme(theme_engine, theme);
    addChild(down_button);

    auto prev = createWidgetCentered<PrevButton>(Vec(RIGHT_MARGIN_CENTER - 9.5f, 98.f));
    prev->describe("Select previous preset");
    prev->setHandler([this](bool c, bool s){});
    prev->applyTheme(theme_engine, theme);
    addChild(prev);

    auto next = createWidgetCentered<NextButton>(Vec(RIGHT_MARGIN_CENTER + 9.f, 98.f));
    next->describe("Select next preset");
    next->setHandler([this](bool c, bool s){});
    next->applyTheme(theme_engine, theme);
    addChild(next);

    addChild(Center(createThemedColorInput(Vec(RIGHT_MARGIN_CENTER - 9.f, RACK_GRID_HEIGHT - 26.f), my_module, PlayModule::IN_PRESET_PREV, PORT_CORN, theme_engine, theme)));
    addChild(Center(createThemedColorInput(Vec(RIGHT_MARGIN_CENTER + 9.f, RACK_GRID_HEIGHT - 26.f), my_module, PlayModule::IN_PRESET_NEXT, PORT_CORN, theme_engine, theme)));

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y-ONEU), theme_engine, theme, "Core link");

    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ui::Menu* menu = createMenu();
            menu->addChild(createMenuLabel("-- Link to Core --"));
            auto broker = ModuleBroker::get();
            broker->addHostPickerMenu(menu, my_module);
        });
    }
    addChild(link_button);

    if (my_module) {
        my_module->ui = this;
        onConnectHost(my_module->chem_host);
        if (!my_module->playlist_file.empty()) {
            load_playlist(my_module->playlist_file);
        }
    } else {
        update_up_down();
    }
}

void PlayUi::presetsToJson(json_t * root)
{
    auto jaru = json_array();
    for (auto preset: presets) {
        json_array_append_new(jaru, preset->toJson());
    }
    json_object_set_new(root, "playlist", jaru);
}

void PlayUi::presetsFromJson(json_t * root)
{
    auto jar = json_object_get(root, "playlist");
    if (jar) {
        json_t* jp;
        size_t index;
        json_array_foreach(jar, index, jp) {
            auto preset = std::make_shared<PresetDescription>();
            preset->fromJson(jp);
            presets.push_back(preset);
        }
    }
}

void PlayUi::addCurrent()
{
    if (!live_preset) return;
    presets.push_back(live_preset);
    auto live_key = live_preset->id.key();
    bool added = false;
    for (auto pw : preset_widgets) {
        pw->live = pw->preset_id.key() == live_key;
        if (!added && !pw->preset_id.valid()) {
            auto index = presets.size()-1;
            pw->set_preset(index, live_preset);
            added = true;
        }
    }
    scroll_to_live();
    update_up_down();
}

void PlayUi::sync_to_presets()
{
    scroll_top = 0;
    auto pit = presets.cbegin();
    uint32_t live_key = live_preset ? live_preset->id.key() : PresetId::InvalidKey;
    size_t index = 0;
    for (auto pw : preset_widgets) {
        if (pit != presets.cend()) {
            auto preset = *pit++;
            pw->set_preset(index, preset);
            if (live_key != PresetId::InvalidKey) {
                pw->live = live_key == preset->id.key();
            }
        } else {
            pw->set_preset(-1, nullptr);
        }
    }
    update_up_down();
}

bool PlayUi::load_playlist(std::string path)
{
    closePlaylist();
    DEFER({sync_to_presets();});

    FILE* file = std::fopen(path.c_str(), "r");
	if (!file) {
		return false;
    }
	DEFER({std::fclose(file);});
	json_error_t error;
	json_t* root = json_loadf(file, 0, &error);
	if (!root) {
		WARN("Invalid JSON at %d:%d %s in %s", error.line, error.column, error.text, path.c_str());
        sync_to_presets();
        return false;
    }
	DEFER({json_decref(root);});
    presetsFromJson(root);

    my_module->playlist_folder = system::getDirectory(path);
    my_module->playlist_file = path;
    my_module->update_mru(path);

    playlist_name = system::getStem(path);
    playlist_label->text(playlist_name);
    playlist_label->describe(my_module->playlist_file);

    sync_to_presets();
    return true;
}

void PlayUi::openPlaylist()
{
    if (!my_module) return;
    std::string path;
    bool ok = openFileDialog(my_module->playlist_folder, "Playlists (.json):json;Any (*):*", playlist_name, path);
    if (ok) {
        load_playlist(path);
    } else {
        playlist_name = "";
        my_module->playlist_file = "";
    }
}

void PlayUi::closePlaylist()
{
    clearPlaylist(true);
}

void PlayUi::savePlaylist()
{
    if (!my_module) return;
    if (my_module->playlist_file.empty()) {
        saveAsPlaylist();
        return;
    }
    auto dir = system::getDirectory(my_module->playlist_file);
    system::createDirectories(dir);

    auto root = json_object();
    if (!root) return;
    DEFER({json_decref(root);});
    presetsToJson(root);

    std::string tmpPath = system::join(dir, TempName(".tmp.json"));
    FILE* file = std::fopen(tmpPath.c_str(), "w");
    if (!file) {
        system::remove(tmpPath);
        return;
    }
	json_dumpf(root, file, JSON_INDENT(2));
	std::fclose(file);
    system::sleep(0.0005);
	system::remove(my_module->playlist_file);
    system::sleep(0.0005);
	system::rename(tmpPath, my_module->playlist_file);
}

void PlayUi::saveAsPlaylist()
{
    if (!my_module) return;
    std::string path;
    bool ok = saveFileDialog(my_module->playlist_folder, "Playlists (.json):json;Any (*):*", playlist_name, path);
    if (ok) {
        my_module->playlist_file = path;
        my_module->playlist_folder = system::getDirectory(path);

        playlist_name = system::getStem(path);
        playlist_label->text(playlist_name);
        playlist_label->describe(my_module->playlist_file);

        savePlaylist();
        my_module->update_mru(path);
    }    
}

void PlayUi::clearPlaylist(bool forget_file)
{
    if (!my_module) return;
    for (auto pw : preset_widgets) {
        pw->set_preset(-1, nullptr);
    }
    if (forget_file) {
        my_module->playlist_file = "";
        playlist_name = "";
        playlist_label->text("");
        playlist_label->describe("");
    }
    presets.clear();
}

void PlayUi::onConnectHost(IChemHost* host)
{
    this->host = host;
    if (this->host) {
        onConnectionChange(ChemDevice::Haken, this->host->host_connection(ChemDevice::Haken));
        onPresetChange();
    } else {
        haken_device_label->text("");
        live_preset_label->text("");
    }
}

bool PlayUi::is_visible(size_t index)
{
    for (auto pw: preset_widgets) {
        if (static_cast<size_t>(pw->preset_index) == index) return true;
    }
    return false;
}

void PlayUi::update_live()
{
    if (live_preset) {
        auto live_key = live_preset->id.key();
        for (auto pw : preset_widgets) {
            if (!pw->preset_id.valid()) break;
            pw->live = pw->preset_id == live_key;
        }
    } else {
        for (auto pw : preset_widgets) {
            if (!pw->preset_id.valid()) break;
            pw->live = false;
        }
    }
}

void PlayUi::update_up_down()
{
    bool up{false}, down{false};
    if (presets.size() > PLAYLIST_LENGTH) {
        up = scroll_top > 0;
        down = scroll_top < presets.size() - PLAYLIST_LENGTH;
    }
    up_button->enable(up);
    down_button->enable(down);
}

void PlayUi::scroll_up(bool ctrl, bool shift)
{
    if (presets.empty()) return;
    if (ctrl) {
        scroll_to(0);
    } else {
        scroll_to(std::max(SIZE_0, scroll_top - PLAYLIST_LENGTH));
    }
}

void PlayUi::scroll_down(bool ctrl, bool shift)
{
    if (presets.size() <= PLAYLIST_LENGTH) return;
    size_t last = presets.size() - PLAYLIST_LENGTH;
    if (ctrl) {
        scroll_to(last);
    } else {
        scroll_to(std::min(last, scroll_top + PLAYLIST_LENGTH));
    }
}

void PlayUi::scroll_to(size_t top)
{
    //assert(in_range(static_cast<size_t>(top), SIZE_0, presets.size() - PLAYLIST_LENGTH - 1));
    if (scroll_top != top) {
        scroll_top = top;
        for (auto pw : preset_widgets) {
            if (top < presets.size()) {
                pw->set_preset(top, presets[top]);
                ++top;
            } else {
                pw->set_preset(-1, nullptr);
            }
        }
        int total = 1 + (presets.size() / PLAYLIST_LENGTH);
        int page  = 1 + (top / PLAYLIST_LENGTH);
        page_label->text(format_string("%d of %d", page, total));
    }
    update_up_down();
    update_live();
}

void PlayUi::make_visible(size_t index)
{
    scroll_to((index / PLAYLIST_LENGTH) * PLAYLIST_LENGTH);
}

void PlayUi::scroll_to_live()
{
    if (!live_preset) return;
    auto live_key = live_preset->id.key();
    int index = 0;
    for (auto p : presets) {
        if (live_key == p->id.key()) {
            if (is_visible(index)) return;
            scroll_to((index / PLAYLIST_LENGTH) * PLAYLIST_LENGTH);
            break;
        }
        ++index;
    }
}

void PlayUi::onPresetChange()
{
    if (host) {
        auto preset = host->host_preset();
        if (preset) {
            live_preset = std::make_shared<PresetDescription>();
            live_preset->init(preset);
        } else {
            live_preset = nullptr;
        }
        live_preset_label->text(preset ? printable(preset->name) : "");
    } else {
        live_preset_label->text("");
    }
    auto live_key = live_preset ? live_preset->id.key() : PresetId::InvalidKey;
    for (auto pw : preset_widgets) {
        pw->live = (live_key != PresetId::InvalidKey) && (live_key == pw->preset_id.key());
    }
    scroll_to_live();
}

void PlayUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    haken_device_label->text(connection ? connection->info.friendly(TextFormatLength::Short) : "");
}

void PlayUi::draw(const DrawArgs& args)
{
    Base::draw(args);
    if (hints) {
        Line(args.vg, RIGHT_MARGIN_CENTER, 0, RIGHT_MARGIN_CENTER, 380, nvgTransRGBAf(PORT_VIOLET, .5f), .5f);
    }
}

// void PlayUi::step()
// {
//     ChemModuleWidget::step();
// }

void PlayUi::appendContextMenu(Menu *menu)
{
    ChemModuleWidget::appendContextMenu(menu);
}
