#include "Play.hpp"
#include <ghc/filesystem.hpp>
#include "../../services/colors.hpp"
#include "../../services/open-file.hpp"
#include "../../em/em-hardware.h"
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

    void onHoverKey(const HoverKeyEvent& e) override
    {
        Base::onHoverKey(e);
        switch (e.key) {
        case GLFW_KEY_ENTER:
        case GLFW_KEY_MENU:
            if (e.action == GLFW_RELEASE) {
                e.stopPropagating();
                createContextMenu();
            }
        }
    }

};

bool PlayMenu::applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
    return Base::applyTheme(engine, theme);
}

void PlayMenu::appendContextMenu(ui::Menu* menu)
{
    menu->addChild(createMenuLabel("— Playlist Actions —"));
    menu->addChild(createMenuItem("Open...", "",    [this](){ ui->open_playlist(); }, false));
    menu->addChild(createMenuItem("Close", "",      [this](){ ui->close_playlist(); }, false));
    menu->addChild(createMenuItem("Save", "",       [this](){ ui->save_playlist(); } , false));
    menu->addChild(createMenuItem("Save as...", "", [this](){ ui->save_as_playlist(); } , false));
    menu->addChild(createMenuItem("Clear", "", [this](){ ui->clear_playlist(false); }, ui->presets.empty()));

    if (ui->my_module) {
        menu->addChild(new MenuSeparator);
        menu->addChild(createSubmenuItem("Open recent", "", [=](Menu* menu) {
            if (ui->my_module->playlist_mru.empty()) {
                menu->addChild(createMenuLabel("(empty)"));
            } else {
                for (auto path : ui->my_module->playlist_mru) {
                    auto name = system::getStem(path);
                    menu->addChild(createMenuItem(name, "", [=]() {
                        ui->load_playlist(path, true);
                    }));
                }
            }
        }));
        menu->addChild(createMenuItem("Clear recents", "", [this](){ ui->my_module->clear_mru(); } , false));
    }

    menu->addChild(new MenuSeparator);
    bool no_selection =  ui->selected.empty();
    bool first = (0 == ui->first_selected());
    bool last = (ui->last_selected() == static_cast<int>(ui->presets.size())-1);
    menu->addChild(createMenuLabel("— Selected —"));
    menu->addChild(createMenuItem("Duplicate", "",     [this](){ ui->clone(); }, no_selection));
    menu->addChild(createMenuItem("Remove", "",        [this](){ ui->remove_selected(); }, no_selection));
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem("Move to first", "", [this](){ ui->to_first(); }, first || no_selection));
    menu->addChild(createMenuItem("Move up", "",       [this](){ ui->to_up(); }, first || no_selection));
    menu->addChild(createMenuItem("Move down", "",     [this](){ ui->to_down(); }, last || no_selection));
    menu->addChild(createMenuItem("Move to last", "",  [this](){ ui->to_last(); }, last || no_selection));
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem("Select none", "Esc",  [this](){ ui->select_none(); }, no_selection));
}

// ----------------------------------------------
constexpr const float ONEU = 15.f;
constexpr const float HALFU = 7.5f;
constexpr const float RIGHT_MARGIN_CENTER = 186.f;
constexpr const ssize_t SSIZE_0 = 0;
constexpr const float PRESETS_TOP = 20.f;
constexpr const float PRESETS_LEFT = 12.f;

const char * const NotConnected = "[not connected]";

bool PlayUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

PlayUi::PlayUi(PlayModule *module) :
    my_module(module)
{
    setModule(module);
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    this->panelBorder = new PartnerPanelBorder();
    replacePanelBorder(panel, this->panelBorder);
    setPanel(panel);

    float y = PRESETS_TOP;
    for (int i = 0; i < 15; ++i) {
        auto pw = createPresetWidget(this, &presets, PRESETS_LEFT, y, theme_engine, theme);
        pw->get_label().text(format_string("[preset %d]", i));
        preset_widgets.push_back(pw);
        addChild(pw);
        y += 21;
    }
    LabelStyle style{"dytext", TextAlignment::Left, 10.f};
    addChild(haken_device_label = createStaticTextLabel<TipLabel>(
        Vec(28.f, box.size.y - 13.f), 200.f, NotConnected, theme_engine, theme, style));

    LabelStyle warn{"warning", TextAlignment::Left, 9.f};
    addChild(warning_label = createStaticTextLabel<TipLabel>(
        Vec(28.f, box.size.y - 22.f), box.size.x, "", theme_engine, theme, warn));
    warning_label->describe("[warning/status]");

    style.height = 16.f;
    addChild(playlist_label = createStaticTextLabel<TipLabel>(
        Vec(ONEU, 3.5), 148.f, "[playlist file]", theme_engine, theme, style));

    style.height = 9.f;
    style.align = TextAlignment::Center;
    addChild(page_label = createStaticTextLabel<StaticTextLabel>(
        Vec(RIGHT_MARGIN_CENTER, 35.f),  35.f, "1 of 1", theme_engine, theme, style));

    auto heart = createThemedButton<HeartButton>(Vec(ONEU, 342.f), theme_engine, theme, "Add to playlist");
    heart->setHandler([this](bool c, bool s){ 
        add_current();
    });
    addChild(heart);

    style.key = "curpreset";
    style.height = 14.f;
    style.bold = true;
    addChild(live_preset_label = createStaticTextLabel<StaticTextLabel>(
        Vec(87, 340.f), 150.f, "[preset]", theme_engine, theme, style));

    play_menu = createThemedWidget<PlayMenu>(Vec(150.f, HALFU), theme_engine, theme);
    play_menu->setUi(this);
    play_menu->describe("Play actions menu");
    addChild(play_menu);

    up_button = createWidgetCentered<UpButton>(Vec(RIGHT_MARGIN_CENTER, 52.f));
    up_button->describe("Scroll up");
    up_button->setHandler([this](bool c, bool s){ 
        page_up(c, s, false);
    });
    up_button->applyTheme(theme_engine, theme);
    addChild(up_button);

    down_button = createWidgetCentered<DownButton>(Vec(RIGHT_MARGIN_CENTER, 67.f));
    down_button->describe("Scroll down");
    down_button->setHandler([this](bool c, bool s){
        page_down(c, s, false);
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
            menu->addChild(createMenuLabel("— Link to Core Module —"));
            auto broker = ModuleBroker::get();
            broker->addHostPickerMenu(menu, my_module);
        });
    }
    addChild(link_button);

    if (my_module) {
        my_module->ui = this;
        onConnectHost(my_module->chem_host);
        if (!my_module->playlist_file.empty()) {
            load_playlist(my_module->playlist_file, false);
        }
    } else {
        update_up_down();
    }
}

void PlayUi::presets_to_json(json_t * root)
{
    json_object_set_new(root, "haken-device", json_string(playlist_device.c_str()));

    auto jaru = json_array();
    for (auto preset: presets) {
        json_array_append_new(jaru, preset->toJson());
    }
    json_object_set_new(root, "playlist", jaru);
}

void PlayUi::presets_from_json(json_t * root)
{
    auto j = json_object_get(root, "haken-device");
    if (j) {
        playlist_device = json_string_value(j);
    }
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

void PlayUi::add_current()
{
    if (!live_preset) return;
    auto it = std::find_if(presets.cbegin(), presets.cend(), [this](std::shared_ptr<pachde::PresetDescription> p){
        return live_preset->id == p->id;
    });
    if (it == presets.cend()) {
        presets.push_back(live_preset);
        page_down(true, false, true);
    } else {
        auto cit = std::find_if(preset_widgets.cbegin(), preset_widgets.cend(), [this](PresetWidget* pw) {
            return pw->get_preset_id() == live_preset->id;
        });
        if (cit != preset_widgets.cend()) {
            make_visible((*cit)->get_index());
        }
    }
}

void PlayUi::sync_to_presets()
{
    scroll_top = 0;
    clear_selected();
    auto pit = presets.cbegin();
    uint32_t live_key = live_preset ? live_preset->id.key() : PresetId::InvalidKey;
    size_t index = 0;
    for (auto pw : preset_widgets) {
        pw->clear_states();
        if (pit != presets.cend()) {
            auto preset = *pit++;
            pw->set_preset(index++, preset);
            if (live_key != PresetId::InvalidKey) {
                pw->set_live(live_key == preset->id.key());
            }
        } else {
            pw->set_preset(-1, nullptr);
        }
    }
    update_up_down();
}

bool PlayUi::load_playlist(std::string path, bool set_folder)
{
    close_playlist();
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
    presets_from_json(root);

    if (set_folder) {
        my_module->playlist_folder = system::getDirectory(path);
    }
    my_module->playlist_file = path;
    my_module->update_mru(path);

    playlist_name = system::getStem(path);
    playlist_label->text(playlist_name);
    std::string tip = my_module->playlist_file;
    if (!playlist_device.empty()) {
        tip.append("\n- for ");
        tip.append(playlist_device);
        playlist_label->describe(tip);

        if (chem_host) {
            auto em = chem_host->host_matrix();
            if (em) {
                const char * current_device = HardwarePresetClass(em->hardware);
                if (0 != strcmp(current_device, playlist_device.c_str())) {
                    warning_label->text(format_string("[WARNING] Playlist for %s", playlist_device.c_str()));
                }
            }
        }
    
    }
    return true;
}

void PlayUi::open_playlist()
{
    if (!my_module) return;
    std::string path;
    bool ok = openFileDialog(my_module->playlist_folder, "Playlists (.json):json;Any (*):*", playlist_name, path);
    if (ok) {
        load_playlist(path, true);
    } else {
        playlist_name = "";
        playlist_device = "";
        my_module->playlist_file = "";
    }
}

void PlayUi::close_playlist()
{
    clear_playlist(true);
}

void PlayUi::save_playlist()
{
    if (!my_module) return;
    if (my_module->playlist_file.empty()) {
        save_as_playlist();
        return;
    }
    auto dir = system::getDirectory(my_module->playlist_file);
    system::createDirectories(dir);

    auto root = json_object();
    if (!root) return;
    DEFER({json_decref(root);});

    if (playlist_device.empty()) {
        if (chem_host) {
            auto em = chem_host->host_matrix();
            if (em) {
                playlist_device = HardwarePresetClass(em->hardware);
            }
        }
    }
    presets_to_json(root);

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

void PlayUi::save_as_playlist()
{
    if (!my_module) return;
    std::string path;
    bool ok = saveFileDialog(my_module->playlist_folder, "Playlists (.json):json;Any (*):*", playlist_name, path);
    if (ok) {
        my_module->playlist_folder = system::getDirectory(path);
        auto ext = system::getExtension(path);
        if (ext.empty()) {
            path.append(".json");
        }
        my_module->playlist_file = path;
        if (playlist_device.empty()) {
            if (chem_host) {
                auto em = chem_host->host_matrix();
                if (em) {
                    playlist_device = HardwarePresetClass(em->hardware);
                }
            }
        }
        playlist_name = system::getStem(path);
        playlist_label->text(playlist_name);

        auto tip = my_module->playlist_file;
        if (!playlist_device.empty()) {
            tip.append("\n- for ");
            tip.append(playlist_device);
        }
        playlist_label->describe(tip);

        save_playlist();
        my_module->update_mru(path);
    }    
}

void PlayUi::clear_playlist(bool forget_file)
{
    warning_label->text("");
    if (!my_module) return;
    for (auto pw : preset_widgets) {
        pw->set_preset(-1, nullptr);
    }
    if (forget_file) {
        my_module->playlist_file = "";
        playlist_name = "";
        playlist_label->text("");
        playlist_label->describe("");
        playlist_device = "";
    }
    presets.clear();
    clear_selected();
    sync_to_presets();
}

void PlayUi::select_none()
{
    if (selected.empty()) return;
    clear_selected();
    scroll_to(scroll_top, true);
}

void PlayUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        onPresetChange();
    } else {
        haken_device_label->text(NotConnected);
        live_preset_label->text("");
    }
}

bool PlayUi::is_visible(ssize_t index)
{
    auto it = std::find_if(preset_widgets.begin(), preset_widgets.end(), [index](PresetWidget* pw) {
        return static_cast<ssize_t>(pw->get_index()) == index;
    });
    return it != preset_widgets.cend();
}

void PlayUi::update_live()
{
    if (live_preset) {
        auto live_key = live_preset->id.key();
        for (auto pw : preset_widgets) {
            if (!pw->get_preset_id().valid()) break;
            pw->set_live(pw->get_preset_id() == live_key);
        }
    } else {
        for (auto pw : preset_widgets) {
            if (!pw->get_preset_id().valid()) break;
            pw->set_live(false);
        }
    }
}

void PlayUi::update_up_down()
{
    bool up{false}, down{false};
    if (presets.size() > PLAYLIST_LENGTH) {
        up = scroll_top > 0;
        down = scroll_top < static_cast<ssize_t>(presets.size()) - PLAYLIST_LENGTH;
    }
    up_button->enable(up);
    down_button->enable(down);

    int page  = 1 + (scroll_top / PLAYLIST_LENGTH);
    int total = 1 + (presets.size() / PLAYLIST_LENGTH);
    page_label->text(format_string("%d of %d", page, total));
}

void PlayUi::page_up(bool ctrl, bool shift, bool refresh)
{
    if (ctrl || presets.empty()) {
        scroll_to(0, refresh);
    } else {
        scroll_to(std::max(SSIZE_0, scroll_top - PLAYLIST_LENGTH), refresh);
    }
}

void PlayUi::page_down(bool ctrl, bool shift, bool refresh)
{
    ssize_t last_page = presets.size() / PLAYLIST_LENGTH;

    if (ctrl) {
        scroll_to(last_page, refresh);
    } else {
        ssize_t next_page = 1 + (scroll_top / PLAYLIST_LENGTH);
        scroll_to(std::min(last_page, next_page) * PLAYLIST_LENGTH, refresh);
    }
}

void PlayUi::scroll_to(ssize_t pos, bool refresh)
{
    // if (pos < 0) {
    //     pos = 0;
    //     refresh = true;
    // } else if (pos > static_cast<ssize_t>(presets.size()) - PLAYLIST_LENGTH) {
    //     pos = presets.size() - PLAYLIST_LENGTH;
    //     refresh = true;
    // }
    assert(pos >= SSIZE_0);

    if (scroll_top != pos || refresh) {
        scroll_top = pos;
        auto live_key = live_preset ? live_preset->id.key() : PresetId::InvalidKey;
        auto sit = get_selected_indices().cbegin();

        auto pit = presets.cbegin() + pos;
        for (auto pw : preset_widgets) {
            pw->clear_states();
            if (pit != presets.cend()) {
                pw->set_preset(pos, *pit);

                if (sit != selected.cend()) {
                    bool is_selected = (pos == *sit);
                    pw->set_selected(is_selected);
                    if (is_selected) sit++;
                } else {
                    pw->set_selected(false);
                }

                if (live_key != PresetId::InvalidKey) {
                    pw->set_live(live_key == pit->get()->id.key());
                }

                ++pos;
                pit++;
            } else {
                pw->set_preset(-1, nullptr);
            }
        }
    }
    update_up_down();
}

void PlayUi::make_visible(ssize_t index)
{
    scroll_to((index / PLAYLIST_LENGTH) * PLAYLIST_LENGTH, true);
}

void PlayUi::scroll_to_live()
{
    if (!live_preset) return;
    auto live_key = live_preset->id.key();
    int index = 0;
    for (auto p : presets) {
        if (live_key == p->id.key()) {
            make_visible(index);
            break;
        }
        ++index;
    }
}

int PlayUi::first_selected()
{
    if (selected.empty()) return -1;
    return *selected.cbegin();
}
int PlayUi::last_selected()
{
    if (selected.empty()) return -1;
    return *(selected.cend()-1);
}
void PlayUi::select_item(int index)
{
    if (selected.cend() == std::find(selected.cbegin(), selected.cend(), index)) {
        selected.push_back(index);
        std::sort(selected.begin(), selected.end());
    }
}
void PlayUi::unselect_item(int index)
{
    auto it = std::find(selected.cbegin(), selected.cend(), index);
    if (it != selected.cend()) selected.erase(it);
}
const std::vector<int>& PlayUi::get_selected_indices()
{
    return selected;
}

std::vector<std::shared_ptr<PresetDescription>> PlayUi::extract(const std::vector<int>& list)
{
    std::vector<std::shared_ptr<PresetDescription>> extracted;
    for (int index : list) {
        extracted.push_back(*(presets.begin() + index));
    }
    return extracted;
}

void PlayUi::clone()
{
    auto list = get_selected_indices();
    auto count = list.size();
    auto extracted = extract(list);
    clear_selected();
    for (auto item : extracted) {
        presets.push_back(item);
    }
    auto index = static_cast<int>(presets.size()-1);
    for (size_t i = 0; i < count; ++i) {
        selected.push_back(index--);
    }
    std::sort(selected.begin(), selected.end());
    page_down(true, false, true);
}

void remove_items(std::vector<int>& indices, std::deque<std::shared_ptr<PresetDescription>>& items)
{
    auto rit = indices.crbegin();
    while (rit != indices.crend()) {
        auto it = items.begin() + *rit++;
        items.erase(it);
    }
}

void PlayUi::remove_selected()
{
    if (selected.empty()) return;
    auto list = get_selected_indices();
    auto new_top = std::max(0, *list.cbegin()-1);
    remove_items(list, presets);
    clear_selected();
    make_visible(new_top);
}

void PlayUi::to_first()
{
    if (selected.empty()) return;
    auto list = get_selected_indices();
    auto count = list.size();
    auto extracted = extract(list);
    remove_items(list, presets);
    clear_selected();
    for (auto it = extracted.crbegin(); it != extracted.crend(); it++)
    {
        presets.push_front(*it);
    }
    for (size_t i = 0; i < count; ++i) {
        selected.push_back(static_cast<int>(i));
    }
    std::sort(selected.begin(), selected.end());
    page_up(true, false, true);
}

void PlayUi::to_last()
{
    if (selected.empty()) return;
    auto list = get_selected_indices();
    auto count = list.size();
    auto extracted = extract(list);
    remove_items(list, presets);
    clear_selected();
    for (auto it = extracted.cbegin(); it != extracted.cend(); it++)
    {
        presets.push_back(*it);
    }
    auto index = static_cast<int>(presets.size()-1);
    for (size_t i = 0; i < count; ++i) {
        selected.push_back(index--);
    }
    std::sort(selected.begin(), selected.end());
    page_down(true, false, true);
}

void PlayUi::to_n(int dx)
{
    if (selected.empty() || 0 == dx) return;
    if ((first_selected() + dx) < 0) return;
    if ((last_selected() + dx) > static_cast<int>(presets.size())-1) return;

    // extract the ones we're going to reorder
    auto extracted = extract(selected);
    remove_items(selected, presets);

    // assign new order
    for (int& i : selected) { i += dx; }

    // move presets to temp
    std::vector<std::shared_ptr<PresetDescription>> new_presets;
    new_presets.reserve(presets.size());
    for (auto p : presets) {
        new_presets.push_back(p);
    }
    presets.clear();

    auto pit = new_presets.cbegin();
    auto iit = selected.cbegin();
    auto xit = extracted.cbegin();
    int index = 0;
    while (pit != new_presets.cend() && xit != extracted.cend()) {
        if (*iit == index) {
            presets.push_back(*xit++);
            iit++;
        } else {
            presets.push_back(*pit++);
        }
        ++index;
    }
    while (pit != new_presets.cend()) {
        presets.push_back(*pit++);
    }
    assert(xit == extracted.cend());
    int first = dx < 0 ? first_selected() : last_selected();
    scroll_to((first / PLAYLIST_LENGTH) * PLAYLIST_LENGTH, true);
}

void PlayUi::to_up()
{
    to_n(-1);
}

void PlayUi::to_down()
{
    to_n(1);
}

void PlayUi::onPresetChange()
{
    if (chem_host) {
        auto preset = chem_host->host_preset();
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
        pw->set_live((live_key != PresetId::InvalidKey) 
            && (live_key == pw->get_preset_id().key()));
    }
    scroll_to_live();
}

void PlayUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    haken_device_label->text(connection ? connection->info.friendly(TextFormatLength::Short) : NotConnected);
    haken_device_label->describe(connection ? connection->info.friendly(TextFormatLength::Long) : NotConnected);
    if (!playlist_device.empty()) {
        if (chem_host) {
            auto em = chem_host->host_matrix();
            if (em) {
                const char * current_device = HardwarePresetClass(em->hardware);
                if (0 != strcmp(current_device, playlist_device.c_str())) {
                    warning_label->text(format_string("[WARNING] Playlist for %s", playlist_device.c_str()));
                } else {
                    warning_label->text("");
                }
            }
        }
    }

}


// IPresetAction
void PlayUi::onClearSelection()
{
    select_none();
}

void PlayUi::onSetSelection(PresetWidget* source, bool on)
{
    auto index = source->get_index();
    if (on) {
        select_item(index);
    } else {
        unselect_item(index);
    }
    scroll_to(scroll_top, true);
}

void PlayUi::onDelete(PresetWidget* source)
{
    clear_selected();
    auto index = source->get_index();
    selected.push_back(index);
    std::sort(selected.begin(), selected.end());

    remove_selected();
    make_visible(--index);
}

void PlayUi::onDropFile(const widget::Widget::PathDropEvent& e)
{
    if (!my_module || e.paths.empty()) return;
    std::string path;
    for (auto p : e.paths)
    {
        my_module->update_mru(p);
        path = p;
    }
    load_playlist(path, true);
}

void PlayUi::onChoosePreset(PresetWidget* source)
{
    if (!connected()) return;

    auto haken = chem_host->host_haken();
    if (haken) {
        auto id = source->get_preset_id();
        if (id.valid()) {
            haken->log->logMessage("play", format_string("Selecting %s", presets[source->get_index()]->summary().c_str()));
            haken->select_preset(id);
        }
    }
}

PresetWidget* PlayUi::getDropTarget(Vec pos)
{
    if ((pos.y < PRESETS_TOP) || (pos.y > PRESETS_TOP + 314.f)) return nullptr;
    if ((pos.x < PRESETS_LEFT) || (pos.x > PRESETS_LEFT + 150.f)) return nullptr;
    for (auto pw : preset_widgets) {
        if (pw->box.contains(pos)) {
            return pw;
        }
        if (!pw->get_preset_id().valid()) break;
    }
    return nullptr;
}

void PlayUi::onDropPreset(PresetWidget* target)
{
    if (!target || selected.empty()) return;
    int target_index = target->get_index();
    if (-1 == target_index) {
        to_last();
    } else if (0 == target_index) {
        to_first();
    } else {
        if (target->get_selected()) {
            bool next_free = false;
            for (auto pw : preset_widgets) {
                if (next_free) {
                    if (pw->get_preset_id().valid()) {
                        if (!pw->get_selected()) {
                            target_index = pw->get_index();
                            break;
                        }
                    } else {
                        to_last();
                        return;
                    }
                } else if (pw == target) {
                    next_free = true;
                }
            }
        }
        for (auto it = selected.crbegin(); it != selected.crend(); it++) {
            if (*it < target_index) {
                --target_index;
            }
        }

        // extract the ones we're going to reorder
        auto extracted = extract(selected);
        remove_items(selected, presets);
        selected.clear();

        std::vector<std::shared_ptr<PresetDescription>> new_presets;
        new_presets.reserve(presets.size());
        for (auto p : presets) {
            new_presets.push_back(p);
        }
        presets.clear();

        int index = 0;
        for (auto p : new_presets) {
            if (index == target_index) {
                for (auto p : extracted) {
                    selected.push_back(index);
                    presets.push_back(p);
                    index++;
                }
            }
            presets.push_back(p);
            index++;
        }
        make_visible(target_index);
    }
}

void PlayUi::onHoverKey(const HoverKeyEvent& e)
{
    auto mods = e.mods & RACK_MOD_MASK;
    switch (e.key) {
    case GLFW_KEY_ESCAPE: {
        if (e.action == GLFW_RELEASE) {
            e.consume(this);
            select_none();
        }
    } break;

    case GLFW_KEY_PAGE_UP: 
        if ((e.action == GLFW_RELEASE) && (mods == GLFW_MOD_ALT)) {
            e.consume(this);
            page_up(false, false, true);
        }
        break;

    case GLFW_KEY_PAGE_DOWN: 
        if ((e.action == GLFW_RELEASE) && (mods == GLFW_MOD_ALT)) {
            e.consume(this);
            page_down(false, false, true);
        }
        break;

    case GLFW_KEY_MENU:
        if (e.action == GLFW_RELEASE) {
            e.consume(this);
            createContextMenu();
        }
        break;
    }
    Base::onHoverKey(e);
}

// void PlayUi::step()
// {
//     Base::step();
// }

void PlayUi::draw(const DrawArgs& args)
{
    Base::draw(args);
    if (hints) {
        Line(args.vg, RIGHT_MARGIN_CENTER, 0, RIGHT_MARGIN_CENTER, 380, nvgTransRGBAf(PORT_VIOLET, .5f), .5f);
    }
}

// void PlayUi::appendContextMenu(Menu *menu)
// {
//     Base::appendContextMenu(menu);
// }
