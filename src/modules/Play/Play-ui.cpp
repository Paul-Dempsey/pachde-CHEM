#include "Play.hpp"
#include <ghc/filesystem.hpp>
#include "../../services/colors.hpp"
#include "../../services/open-file.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
namespace fs = ghc::filesystem;
using namespace svg_theme;
using namespace pachde;


// -- Play Menu ---------------------------------

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
        menu->addChild(createMenuItem("Clear recents", "", [this](){ ui->my_module->clear_mru(); }, false));
    }
    menu->addChild(new MenuSeparator);
    menu->addChild(createSubmenuItem("Append", "", [=](Menu* menu) {
        menu->addChild(createMenuItem("User presets", "", [this](){ ui->fill(FillOptions::User); }));
        menu->addChild(createMenuItem("System presets", "", [this](){ ui->fill(FillOptions::System); }));
        menu->addChild(createMenuItem("All presets", "", [this](){ ui->fill(FillOptions::All); }));
    }));

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

// -- Play UI -----------------------------------

constexpr const float ONEU = 15.f;
constexpr const float RIGHT_MARGIN_CENTER = 186.f;
constexpr const ssize_t SSIZE_0 = 0;
constexpr const float PRESETS_TOP = 32.f;
constexpr const float PRESETS_LEFT = 12.f;

bool PlayUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

PlayUi::~PlayUi()
{
    if (modified && module && !my_module->playlist_file.empty()) {
        save_playlist();
    }
}

PlayUi::PlayUi(PlayModule *module) :
    my_module(module)
{
    setModule(module);
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);
    if (S::show_screws()) {
        addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
    }

    float y = PRESETS_TOP;
    for (int i = 0; i < PAGE_CAPACITY; ++i) {
        auto pw = createPresetWidget(this, &presets, PRESETS_LEFT, y, theme_engine, theme);
        if (!module) {
            pw->set_text(in_range(i, 4, 14) ? "..." : format_string("[preset #%d]", 1 + i));
        }
        preset_widgets.push_back(pw);
        addChild(pw);
        y += 20;
    }
    addChild(haken_device_label = createLabel<TipLabel>(
        Vec(28.f, box.size.y - 13.f), 200.f, S::NotConnected, theme_engine, theme, S::haken_label));

    addChild(warning_label = createLabel<TipLabel>(
        Vec(28.f, box.size.y - 22.f), box.size.x, "", theme_engine, theme, S::warning_label));
    warning_label->describe("[warning/status]");
    warning_label->glowing(true);

    LabelStyle style{"dytext", TextAlignment::Left, 14.f};
    addChild(playlist_label = createLabel<TipLabel>(
        Vec(ONEU, 16), 148.f, "My Favorites", theme_engine, theme, style));
    playlist_label->glowing(true);

    addChild(blip = createBlipCentered(7.5f, 24.f, "Playlist unsaved when lit"));
    blip->set_radius(2.5f);
    blip->set_rim_color(nvgTransRGBAf(RampGray(G_65), .4f));
    blip->set_light_color(ColorFromTheme(getThemeName(), "warning", nvgRGB(0xe7, 0xe7, 0x45)));
    blip->set_brightness(0.f);

    style.height = 9.f;
    style.align = TextAlignment::Center;
    addChild(page_label = createLabel<TextLabel>(
        Vec(RIGHT_MARGIN_CENTER, 35.f),  35.f, "1 of 1", theme_engine, theme, style));

    auto heart = createThemedButton<HeartButton>(Vec(ONEU, 342.f), theme_engine, theme, "Add to playlist");
    heart->setHandler([this](bool c, bool s){ 
        add_live();
    });
    addChild(heart);

    style.key = "curpreset";
    style.height = 14.f;
    style.bold = true;
    addChild(live_preset_label = createLabel<TipLabel>(
        Vec(87, 340.f), 150.f, "[current device preset]", theme_engine, theme, style));
    live_preset_label->glowing(true);

    play_menu = createThemedWidget<PlayMenu>(Vec(150.f, PRESETS_TOP - 12.f), theme_engine, theme);
    play_menu->setUi(this);
    play_menu->describe("Play actions menu");
    addChild(play_menu);

    up_button = createWidgetCentered<UpButton>(Vec(RIGHT_MARGIN_CENTER, 52.f));
    up_button->describe("Page up");
    up_button->setHandler([this](bool c, bool s){ 
        page_up(c, s);
    });
    up_button->applyTheme(theme_engine, theme);
    addChild(up_button);

    down_button = createWidgetCentered<DownButton>(Vec(RIGHT_MARGIN_CENTER, 67.f));
    down_button->describe("Page down");
    down_button->setHandler([this](bool c, bool s){
        page_down(c, s);
    });
    down_button->applyTheme(theme_engine, theme);
    addChild(down_button);

    auto prev = createWidgetCentered<PrevButton>(Vec(RIGHT_MARGIN_CENTER - 9.5f, 98.f));
    prev->describe("Select previous preset");
    prev->setHandler([this](bool c, bool s){ prev_preset(); });
    prev->applyTheme(theme_engine, theme);
    addChild(prev);

    auto next = createWidgetCentered<NextButton>(Vec(RIGHT_MARGIN_CENTER + 9.f, 98.f));
    next->describe("Select next preset");
    next->setHandler([this](bool c, bool s){ next_preset(); });
    next->applyTheme(theme_engine, theme);
    addChild(next);

    addChild(Center(createThemedColorInput(Vec(RIGHT_MARGIN_CENTER - 9.f, RACK_GRID_HEIGHT - 26.f), my_module, PlayModule::IN_PRESET_PREV, S::InputColorKey, PORT_CORN, theme_engine, theme)));
    addChild(Center(createThemedColorInput(Vec(RIGHT_MARGIN_CENTER + 9.f, RACK_GRID_HEIGHT - 26.f), my_module, PlayModule::IN_PRESET_NEXT, S::InputColorKey, PORT_CORN, theme_engine, theme)));

    if (!module) {
        auto logo = new Logo(1.25f);
        logo->box.pos = Vec(90.f, box.size.y*.5);
        addChild(Center(logo));
    }

    // Footer

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
        my_module->set_chem_ui(this);
        if (!chem_host) {
            onConnectHost(my_module->chem_host);
        }
        if (!my_module->playlist_file.empty()) {
            load_playlist(my_module->playlist_file, false);
        }
    } else {
        update_up_down();
    }
    set_modified(false);
}

void PlayUi::setThemeName(const std::string& name, void * context)
{
    blip->set_light_color(ColorFromTheme(getThemeName(), "warning", nvgRGB(0xe7, 0xe7, 0x45)));
    Base::setThemeName(name, context);
}

void PlayUi::set_track_live(bool track)
{
    if (!module) return;
    if (track == my_module->track_live) return;
    my_module->track_live = track;
    if (live_preset) {
        scroll_to_live();
        auto wit = std::find_if(preset_widgets.begin(), preset_widgets.end(), [](PresetWidget* pw) {
            return pw->is_live();            
        });
        if (wit != preset_widgets.end()) {
            widgets_clear_current();
            current_index = (*wit)->get_index();
            current_preset = presets[current_index];
            (*wit)->set_current(true);
        }
    }
}

void PlayUi::presets_to_json(json_t * root)
{
    json_object_set_new(root, "haken-device", json_string(playlist_device.c_str()));

    auto jaru = json_array();
    for (auto preset: presets) {
        json_array_append_new(jaru, preset->toJson(true, true, false));
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

void PlayUi::add_live()
{
    if (!live_preset) return;
    auto it = std::find_if(presets.cbegin(), presets.cend(), [this](std::shared_ptr<pachde::PresetDescription> p){
        return live_preset->id == p->id;
    });
    if (it == presets.cend()) {
        presets.push_back(live_preset);
        set_modified(true);
        page_down(true, false);
    } else {
        auto cit = std::find_if(preset_widgets.cbegin(), preset_widgets.cend(), [this](PresetWidget* pw) {
            return pw->get_preset_id() == live_preset->id;
        });
        if (cit != preset_widgets.cend()) {
            make_visible((*cit)->get_index());
        }
    }
}

void PlayUi::select_preset(PresetId id)
{
    if (!id.valid()) return;

    auto haken = chem_host->host_haken();
    if (haken) {
        if (haken->log) {
            haken->log->log_message("play", format_string("Selecting preset [%d:%d:%d]]", id.bank_hi(), id.bank_lo(), id.number()));
        }
        haken->select_preset(ChemId::Play, id);
    }
}

void PlayUi::sync_to_current_index()
{
    current_preset = presets[current_index];
    auto wit = std::find_if(preset_widgets.begin(), preset_widgets.end(), [this](PresetWidget* pw){
        return pw->get_index() == current_index;
    });
    if (wit != preset_widgets.end()) {
        onChoosePreset(*wit); 
        return;
    }
    make_visible(current_index);
    select_preset(current_preset->id);
}

void PlayUi::widgets_clear_current()
{
    for(auto wit = preset_widgets.begin(); wit != preset_widgets.end(); wit++) {
        (*wit)->set_current(false);
    }
}

void PlayUi::prev_preset()
{
    if (--current_index < 0) {
        current_index = preset_count() - 1;
    }
    sync_to_current_index();
}

void PlayUi::next_preset()
{
    if (++current_index >= preset_count()) current_index = 0;
    sync_to_current_index();
}

void PlayUi::sync_to_presets()
{
    scroll_top = 0;
    clear_selected();
    auto live_id = get_live_id();
    auto pit = presets.cbegin();
    size_t index = 0;
    for (auto pw : preset_widgets) {
        pw->clear_states();
        if (pit != presets.cend()) {
            auto preset = *pit++;
            pw->set_preset(index, preset == current_preset, preset->id == live_id, preset);
            ++index;
        } else {
            pw->clear_preset();
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

    set_modified(false);
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
    }
    check_playlist_device();
    return true;
}

void PlayUi::open_playlist()
{
    if (!my_module) return;
    std::string path;
    if (modified && module && !my_module->playlist_file.empty()) {
        save_playlist();
    }
    if (my_module->playlist_folder.empty()) {
        auto kv = get_plugin_kv_store();
        if (kv && kv->load()) {
            my_module->playlist_folder = kv->lookup("playlist-folder");
        }
    }
    bool ok = openFileDialog(my_module->playlist_folder, "Playlists (.json):json;Any (*):*", playlist_name, path);
    if (ok) {
        load_playlist(path, true);
    } else {
        playlist_name = "";
        playlist_device = "";
        my_module->playlist_file = "";
        set_modified(false);
    }
}

void PlayUi::close_playlist()
{
    clear_playlist(true);
    set_modified(false);
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
                playlist_device = PresetClassName(em->hardware);
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
    set_modified(false);
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
                    playlist_device = PresetClassName(em->hardware);
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
        set_modified(false);
    }    
}

void PlayUi::clear_playlist(bool forget_file)
{
    warning_label->text("");
    if (!my_module) return;
    for (auto pw : preset_widgets) {
        pw->clear_preset();
    }
    if (forget_file) {
        my_module->playlist_file = "";
        playlist_name = "";
        playlist_label->text("");
        playlist_label->describe("");
        playlist_device = "";
        set_modified(false);
    }
    presets.clear();
    clear_selected();
    sync_to_presets();
    set_modified(true);
}

void PlayUi::select_none()
{
    if (selected.empty()) return;
    clear_selected();
    scroll_to(scroll_top);
}

void PlayUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onPresetChange();
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
    } else {
        onConnectionChange(ChemDevice::Haken, nullptr);
        live_preset_label->text("");
        live_preset_label->describe("none");
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
        for (auto pw : preset_widgets) {
            if (pw->empty()) break;
            pw->set_live(pw->get_preset_id() == live_preset->id);
        }
    } else {
        for (auto pw : preset_widgets) {
            pw->set_live(false);
        }
    }
}

void PlayUi::update_up_down()
{
    bool up{false}, down{false};
    if (presets.size() > PAGE_CAPACITY) {
        up = scroll_top > 0;
        down = scroll_top < preset_count() - PAGE_CAPACITY;
    }
    up_button->enable(up);
    down_button->enable(down);

    int page  = 1 + (scroll_top / PAGE_CAPACITY);
    int total = 1 + (preset_count() / PAGE_CAPACITY);
    page_label->text(format_string("%d of %d", page, total));
}

void PlayUi::page_up(bool ctrl, bool shift)
{
    if (ctrl || presets.empty()) {
        scroll_to(0);
    } else {
        scroll_to(std::max(SSIZE_0, scroll_top - PAGE_CAPACITY));
    }
}

void PlayUi::page_down(bool ctrl, bool shift)
{
    ssize_t last_page = (preset_count() / PAGE_CAPACITY) * PAGE_CAPACITY;

    if (ctrl) {
        scroll_to(last_page);
    } else {
        ssize_t next_page = 1 + (scroll_top / PAGE_CAPACITY);
        scroll_to(std::min(last_page, next_page) * PAGE_CAPACITY);
    }
}

void PlayUi::scroll_to(ssize_t pos)
{
    // if (pos < 0) {
    //     pos = 0;
    //     refresh = true;
    // } else if (pos > preset_count() - PAGE_CAPACITY) {
    //     pos = preset_count() - PAGE_CAPACITY;
    //     refresh = true;
    // }
    assert(pos >= SSIZE_0);

    scroll_top = pos;
    auto live_id = get_live_id();
    auto sit = get_selected_indices().cbegin();

    auto pit = presets.cbegin() + pos;
    for (auto pw : preset_widgets) {
        pw->clear_states();
        if (pit != presets.cend()) {
            pw->set_preset(pos, current_index == pos, live_id == (*pit)->id, *pit);

            if (sit != selected.cend()) {
                bool is_selected = (pos == *sit);
                pw->set_selected(is_selected);
                if (is_selected) sit++;
            } else {
                pw->set_selected(false);
            }

            ++pos;
            pit++;
        } else {
            pw->clear_preset();
        }
    }
    update_up_down();
}

void PlayUi::make_visible(ssize_t index)
{
    scroll_to_page_of_index(index);
}

void PlayUi::scroll_to_live()
{
    if (!live_preset) return;
    auto id = live_preset->id;
    auto it = std::find_if(presets.cbegin(), presets.cend(), [id](const std::shared_ptr<pachde::PresetDescription>& p){
        return p->id == id;
    });
    if (it != presets.cend()) {
        make_visible(it - presets.cbegin());
    }
}

void PlayUi::scroll_to_page_of_index(ssize_t index)
{
    scroll_to(page_index_of_index(index));
}

ssize_t PlayUi::page_index_of_index(ssize_t index)
{
    index = std::max(ssize_t(0), index);
    index = std::min(ssize_t(presets.size() - 1), index);
    return PAGE_CAPACITY * (index / PAGE_CAPACITY);
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
    auto index = preset_count()-1;
    for (size_t i = 0; i < count; ++i) {
        selected.push_back(index--);
    }
    std::sort(selected.begin(), selected.end());
    page_down(true, false);
    set_modified(true);
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
    set_modified(true);
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
    page_up(true, false);
    set_modified(true);
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
    auto index = preset_count() - 1;
    for (size_t i = 0; i < count; ++i) {
        selected.push_back(index--);
    }
    std::sort(selected.begin(), selected.end());
    page_down(true, false);
    set_modified(true);
}

void PlayUi::to_n(int dx)
{
    if (selected.empty() || 0 == dx) return;
    if ((first_selected() + dx) < 0) return;
    if ((last_selected() + dx) > (preset_count()-1)) return;

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
    scroll_to((first / PAGE_CAPACITY) * PAGE_CAPACITY);
    set_modified(true);
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
    if (gather != FillOptions::None) {
        auto preset = chem_host->host_preset();
        if (preset) {
            if (preset->empty()) return;
            auto it = std::find_if(presets.cbegin(), presets.cend(), [preset](std::shared_ptr<pachde::PresetDescription> p){
                return preset->id.key() == p->id.key();
            });
            if (it == presets.cend()) {
                auto new_preset = std::make_shared<PresetDescription>();
                new_preset->init(preset);
                presets.push_back(new_preset);
                set_modified(true);
            }
        }
        return;
    }

    if (my_module && my_module->batch_busy()) return;

    if (chem_host) {
        auto preset = chem_host->host_preset();
        if (preset) {
            live_preset = std::make_shared<PresetDescription>();
            live_preset->init(preset);
        } else {
            live_preset = nullptr;
        }
        live_preset_label->text(preset ? preset->name : "");
        live_preset_label->describe(preset ? preset->text : "");
    } else {
        live_preset_label->text("");
        live_preset_label->describe("(none)");
    }

    if (live_preset && my_module->track_live) {
        scroll_to_live();
    }
    auto live_id = get_live_id();
    for (auto pw : preset_widgets) {
        if (pw->empty()) break;
        pw->set_live(live_id.valid() && (live_id == pw->get_preset_id()));
        if (pw->is_live() && my_module->track_live) {
            current_index = pw->get_index();
            current_preset = presets[current_index];
        }
    }
}

void PlayUi::check_playlist_device()
{
    if (!connected() || playlist_device.empty()) {
        warning_label->text("");
        pending_device_check = false;
        return;
    }
    if (chem_host) {
        auto em = chem_host->host_matrix();
        if (em && em->hardware != 0) {
            const char * current_device = PresetClassName(em->hardware);
            if (0 != strcmp(current_device, playlist_device.c_str())) {
                warning_label->text(format_string("[WARNING] Playlist for %s", playlist_device.c_str()));
            } else {
                warning_label->text("");
            }
            pending_device_check = false;
        } else {
            warning_label->text("");
        }
    }
}

struct EmHandler: IHandleEmEvents
{
    PlayUi* ui;
    EmHandler(PlayUi* host) : ui(host) {
        em_event_mask = EventMask::UserComplete 
            + EventMask::SystemComplete;
    }
    virtual ~EmHandler() {};

    void onUserComplete() override {
        ui->on_fill_complete();
    }
    void onSystemComplete() override {
        ui->on_fill_complete();
    }
};

void PlayUi::fill(FillOptions which)
{
    if (!chem_host) return;
    auto haken = chem_host->host_haken();
    auto matrix = chem_host->host_matrix();
    em_handler = new EmHandler(this);
    matrix->subscribeEMEvents(em_handler);
    gather = which;

    startSpinner(this, Vec(box.size.x*.5f, box.size.y * .5f));

    switch (which) {
    case FillOptions::None: break;
    case FillOptions::User: haken->request_user(ChemId::Play); break;
    case FillOptions::System: haken->request_system(ChemId::Play); break;
    case FillOptions::All: haken->request_user(ChemId::Play); break;
    }
}

void PlayUi::on_fill_complete()
{
    if (!chem_host) return;
    switch (gather) {
    case FillOptions::None:
        assert(false);
        return;
    case FillOptions::User:
    case FillOptions::System:
        break;
    case FillOptions::All:
        gather = FillOptions::System;
        chem_host->host_haken()->request_system(ChemId::Play);
        return;
    }

    stopSpinner(this);

    gather = FillOptions::None;
    auto matrix = chem_host->host_matrix();
    matrix->unsubscribeEMEvents(em_handler);
    delete em_handler;
    em_handler = nullptr;
    sync_to_presets();
}

void PlayUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    onConnectionChangeUiImpl(this, device, connection);
    check_playlist_device();
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
    scroll_to(scroll_top);
}

void PlayUi::onDelete(PresetWidget* source)
{
    clear_selected();
    auto index = source->get_index();
    selected.push_back(index);
    std::sort(selected.begin(), selected.end());

    remove_selected();
    make_visible(--index);
    set_modified(true);
}

void PlayUi::onSetCurrrent(int index)
{
    current_index = index;
    if (-1 == index) {
        current_preset = nullptr;
        widgets_clear_current();
    } else {
        current_preset = presets[index];
        for (auto pw : preset_widgets) {
            if (pw->empty()) break;
            pw->set_current(pw->get_index() == index);
        }
    }
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
    auto id = source->get_preset_id();
    assert(id.valid());
    current_index = source->get_index();
    current_preset = presets[current_index];

    auto haken = chem_host->host_haken();
    if (haken) {
        if (haken->log) {
            haken->log->log_message("play", format_string("Selecting %s", current_preset->summary().c_str()));
        }
        haken->select_preset(ChemId::Play, id);
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
        if (target->is_selected()) {
            bool next_free = false;
            for (auto pw : preset_widgets) {
                if (next_free) {
                    if (!pw->empty()) {
                        if (!pw->is_selected()) {
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
        //selected.clear();

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
                    //selected.push_back(index);
                    presets.push_back(p);
                    index++;
                }
            }
            presets.push_back(p);
            index++;
        }
        make_visible(target_index);
    }
    selected.clear();
    set_modified(true);
}

void PlayUi::onHoverKey(const HoverKeyEvent& e)
{
    Base::onHoverKey(e);
}

void PlayUi::onButton(const ButtonEvent &e)
{
    Base::onButton(e);
}

void PlayUi::onSelectKey(const SelectKeyEvent &e)
{
    if (APP->event->getSelectedWidget() != this) {
        Base::onSelectKey(e);
        return;
    }
    auto mods = e.mods & RACK_MOD_MASK;
    if ((e.action == GLFW_PRESS || e.action == GLFW_REPEAT)) {
        switch (e.key) {
        case GLFW_KEY_ESCAPE:
            if (0 == mods) {
                select_none();
            }
            e.consume(this);
            return;

        case GLFW_KEY_ENTER:
            if (current_index < 0) {
            } else {
                current_preset = presets[current_index];
                auto haken = chem_host->host_haken();
                if (haken) {
                    if (haken->log) {
                        haken->log->log_message("play", format_string("Selecting %s", current_preset->summary().c_str()));
                    }
                    haken->select_preset(ChemId::Play, current_preset->id);
                } 
            }
            e.consume(this);
            return;

        case GLFW_KEY_UP:
            if (current_index < 0) {
                current_index = 0;
                scroll_to(0);
            } else if ((0 == mods) && current_index > 0) {
                current_index--;
                scroll_to_page_of_index(current_index);
            }
            e.consume(this);
            return;

        case GLFW_KEY_DOWN:
            if (current_index < 0) {
                current_index = 0;
                scroll_to(0);
            } else if ((0 == mods) && (current_index < ssize_t(presets.size())-1)) {
                current_index++;
                scroll_to_page_of_index(current_index);
            }
            e.consume(this);
            return;

        case GLFW_KEY_HOME:
            if (0 == mods) {
                current_index = scroll_top;
            } else if (mods & RACK_MOD_CTRL) {
                current_index = 0;
            }
            scroll_to(current_index);
            e.consume(this);
            return;

        case GLFW_KEY_END:
            if (0 == mods) {
                current_index = std::min(ssize_t(presets.size()) - 1, scroll_top + PAGE_CAPACITY - 1);
            } else if (mods & RACK_MOD_CTRL) {
                current_index = presets.size() -1;
            }
            scroll_to_page_of_index(current_index);
            e.consume(this);
            return;

        case GLFW_KEY_PAGE_UP: 
            if (current_index < 0) {
                page_up(false, false);
            } else {
                current_index = std::max(ssize_t(0), current_index - PAGE_CAPACITY);
            }
            scroll_to_page_of_index(current_index);
            e.consume(this);
            return;

        case GLFW_KEY_PAGE_DOWN: 
            if (current_index < 0) {
                page_down(false, false);
            } else {
                current_index = std::min(ssize_t(presets.size()) - 1, current_index + PAGE_CAPACITY);
            }
            scroll_to_page_of_index(current_index);
            e.consume(this);
            return;

        case GLFW_KEY_MENU:
            if (0 == mods) {
                e.consume(this);
                createContextMenu();
                return;
            }
            break;
        }
    }
    Base::onSelectKey(e);
}

void PlayUi::onHoverScroll(const HoverScrollEvent &e)
{
    if (in_range(e.pos.x, PRESETS_LEFT, PRESETS_LEFT + 150.f) && in_range(e.pos.y, PRESETS_TOP, 340.f)) {
        int delta = 5 * ((e.scrollDelta.y < 0) ? 1 : (e.scrollDelta.y > 0) ? -1 : 0);
        auto mods = APP->window->getMods();
        if (mods & GLFW_MOD_CONTROL) {
            delta *= 3.f;
        }
        ssize_t index = scroll_top + delta;
        if (index > ssize_t(presets.size()) - PAGE_CAPACITY) index = presets.size() - PAGE_CAPACITY;
        if (index < 0) index = 0;
        scroll_to(index);
        e.consume(this);
        return;
    }
    Base::onHoverScroll(e);
}

void PlayUi::step()
{
    Base::step();
    bind_host(my_module);

    if (pending_device_check) {
        check_playlist_device();
    }
}

void PlayUi::draw(const DrawArgs& args)
{
    Base::draw(args);
// #ifdef LAYOUT_HELP
//     if (hints) {
//         Line(args.vg, RIGHT_MARGIN_CENTER, 0, RIGHT_MARGIN_CENTER, 380, nvgTransRGBAf(PORT_VIOLET, .5f), .5f);
//     }
// #endif
}

void PlayUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
    menu->addChild(new MenuSeparator);
    menu->addChild(createCheckMenuItem("Track live preset", "", 
        [this](){ return my_module->track_live; },
        [this](){ set_track_live(!my_module->track_live); }
    ));
    Base::appendContextMenu(menu);
}
