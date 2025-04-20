#include "Preset.hpp"
#include "../../em/preset-meta.hpp"
#include "../../services/colors.hpp"
#include "../../widgets/click-region-widget.hpp"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"
#include "../../widgets/hamburger.hpp"
#include "../../widgets/spinner.hpp"

namespace S = pachde::style;
using PM = PresetModule;

// PresetMenu
struct PresetMenu : Hamburger
{
    using Base = Hamburger;

    PresetUi* ui;
    PresetMenu() : ui(nullptr) { }

    void setUi(PresetUi* w) { ui = w; }
    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override;
    void appendContextMenu(ui::Menu* menu) override;

    void onHoverKey(const HoverKeyEvent& e) override
    {
        switch (e.key) {
            case GLFW_KEY_ENTER:
            case GLFW_KEY_MENU:
            if (e.action == GLFW_RELEASE) {
                e.consume(this);
                createContextMenu();
                return;
            }
        }
        Base::onHoverKey(e);
    }
};

bool PresetMenu::applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
    return Base::applyTheme(engine, theme);
}

void PresetMenu::appendContextMenu(ui::Menu* menu)
{
    menu->addChild(createMenuLabel("— Preset Actions —"));

    auto item = createMenuItem<ColorDotMenuItem>("Sort alphabetically", "",
        [this](){ ui->sort_presets(PresetOrder::Alpha); }, false);
    item->color = ui->active_tab().list.order == PresetOrder::Alpha ? nvgHSL(360/200, .5, .5) : RampGray(G_45);
    menu->addChild(item);

    item = createMenuItem<ColorDotMenuItem>("Sort by category", "",
        [this](){ ui->sort_presets(PresetOrder::Category); }, false);
    item->color = ui->active_tab().list.order == PresetOrder::Category ? nvgHSL(360/200, .5, .5) : RampGray(G_45);
    menu->addChild(item);

    item = createMenuItem<ColorDotMenuItem>("Sort by Natural (system) order", "",
        [this](){ ui->sort_presets(PresetOrder::Natural); }, false);
    item->color = ui->active_tab().list.order == PresetOrder::Natural ? nvgHSL(360/200, .5, .5) : RampGray(G_45);
    menu->addChild(item);

    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem("Show live preset", "",
        [this](){ ui->scroll_to_live(); },
        !ui->get_live_id().valid()
    ));
    menu->addChild(createMenuItem("Clear filters", "",
        [this](){ ui->clear_filters(); },
        !ui->my_module
    ));

    menu->addChild(new MenuSeparator);
    menu->addChild(createCheckMenuItem("Track live preset", "", 
        [this](){ return ui->my_module->track_live; },
        [this](){ ui->set_track_live(!ui->my_module->track_live); },
        !ui->my_module
    ));
    menu->addChild(createCheckMenuItem("Use cached User presets", "", 
        [this]() { return ui->my_module->use_cached_user_presets; },
        [this]() { ui->my_module->use_cached_user_presets = !ui->my_module->use_cached_user_presets; },
        !ui->my_module
    ));
    menu->addChild(createCheckMenuItem("Keep search filters", "", 
        [this]() { return ui->my_module->keep_search_filters; },
        [this]() { ui->my_module->keep_search_filters = !ui->my_module->keep_search_filters; },
        !ui->my_module
    ));

    menu->addChild(new MenuSeparator);
    bool host = ui->host_available();
    menu->addChild(createMenuItem("Refresh User presets", "", 
        [this](){ ui->request_presets(PresetTab::User); },
        !host
    ));
    menu->addChild(createMenuItem("Refresh System presets", "", 
        [this](){ ui->request_presets(PresetTab::System); },
        !host
    ));
    std::string name = format_string("Build full %s database", ui->active_tab_id == PresetTab::User ? "User" : "System");
    menu->addChild(createMenuItem(name, "",
        [this](){ ui->build_database(PresetTab::System); },
        !host
    ));

}

// PresetUi

constexpr const float PANEL_WIDTH = 360.f;
constexpr const float RCENTER = PANEL_WIDTH - S::U1;

PresetUi::PresetUi(PresetModule *module) :
    my_module(module)
{
    setModule(module);

    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);

    float x, y;
    search_entry = new SearchField();
    search_entry->box.pos = Vec(23.5f, 18.f);
    search_entry->box.size = Vec(114.f, 14.f);
    search_entry->applyTheme(theme_engine, theme);
    search_entry->placeholder = "preset search";
    search_entry->change_handler = [this](){ this->on_search_text_changed(); };
    search_entry->enter_handler = [this](){ this->on_search_text_enter(); };
    addChild(search_entry);

    x = 184.f;
    y = 23.75f;
    addChild(createLightCentered<SmallLight<RedLight>>(Vec(x,y), my_module, PresetModule::L_FILTER));;

    // tab labels
    x = 270.f;
    y = 18.f;
    addChild(user_label = createLabel<TextLabel>(Vec(x,y), 26.f, "User", theme_engine, theme, tab_style));
    addChild(createClickRegion(RECT_ARGS(user_label->box.grow(Vec(6,2))), (int)PresetTab::User, [=](int id, int mods) { set_tab((PresetTab)id, true); }));

    x += 55.f;
    addChild(system_label = createLabel<TextLabel>(Vec(x,y), 40.f, "System", theme_engine, theme, current_tab_style));
    addChild(createClickRegion(RECT_ARGS(system_label->box.grow(Vec(6,2))), (int)PresetTab::System, [=](int id, int mods) { set_tab((PresetTab)id, true); }));

    // right controls

    x = RCENTER;
    y = 28.f;
    LabelStyle style{"dytext", TextAlignment::Center, 9.f};
    addChild(page_label = createLabel<TextLabel>(Vec(x - .75, y), 30.f, "1 of 1", theme_engine, theme, style));

    y = 46.f;
    up_button = createWidgetCentered<UpButton>(Vec(x, y));
    up_button->describe("Page up");
    up_button->applyTheme(theme_engine, theme);
    up_button->setHandler([this](bool c, bool s){ 
        page_up(c,s);
    });
    addChild(up_button);

    y += 15.f;
    down_button = createWidgetCentered<DownButton>(Vec(x, y));
    down_button->describe("Page down");
    down_button->applyTheme(theme_engine, theme);
    down_button->setHandler([this](bool c, bool s){
        page_down(c,s);
    });
    addChild(down_button);

    y = 84.f;
    auto prev = createWidgetCentered<PrevButton>(Vec(x, y));
    prev->describe("Select previous preset");
    prev->applyTheme(theme_engine, theme);
    prev->setHandler([this](bool c, bool s) {
        previous_preset(c,s);
    });
    addChild(prev);

    y += 15.f;
    auto next = createWidgetCentered<NextButton>(Vec(x, y));
    next->describe("Select next preset");
    next->applyTheme(theme_engine, theme);
    next->setHandler([this](bool c, bool s){ 
        next_preset(c,s);
    });
    addChild(next);

    menu = Center(createThemedWidget<PresetMenu>(Vec(x, 132.f), theme_engine, theme));
    menu->setUi(this);
    menu->describe("Preset actions menu");
    addChild(menu);

    const float FILTER_DY = 20.f;
    y = 168.f;
    FilterButton* filter{nullptr};
    
    addChild(Center(filter = makeCatFilter(Vec(x,y), theme_engine, theme, [=](uint64_t state){ on_filter_change(FilterId::Category, state); })));
    filter_buttons.push_back(filter);

    y += FILTER_DY;
    addChild(Center(filter = makeTypeFilter(Vec(x,y), theme_engine, theme, [=](uint64_t state){ on_filter_change(FilterId::Type, state); })));
    filter_buttons.push_back(filter);

    y += FILTER_DY;
    addChild(Center(filter = makeCharacterFilter(Vec(x,y), theme_engine, theme, [=](uint64_t state){ on_filter_change(FilterId::Character, state); })));
    filter_buttons.push_back(filter);

    y += FILTER_DY;
    addChild(Center(filter = makeMatrixFilter(Vec(x,y), theme_engine, theme, [=](uint64_t state){ on_filter_change(FilterId::Matrix, state); })));
    filter_buttons.push_back(filter);

    y += FILTER_DY;
    addChild(Center(filter = makeSettingFilter(Vec(x,y), theme_engine, theme, [=](uint64_t state){ on_filter_change(FilterId::Setting, state); })));
    filter_buttons.push_back(filter);

    if (my_module) {
        user_tab.list.init_filters(my_module->user_filters);
        system_tab.list.init_filters(my_module->system_filters);
        auto filters = my_module->filters();
        for (auto fb: filter_buttons) {
            fb->set_state(*filters++);
        }
    }

    // preset grid
    const float PRESET_TOP = 38.f;
    x = 9.f; y = PRESET_TOP;
    for (int i = 0; i < PAGE_CAPACITY; ++i) {
        auto entry = PresetEntry::create(Vec(x,y), preset_grid, this, theme);
        entry->applyTheme(theme_engine, theme);
        preset_grid.push_back(entry);
        addChild(entry);
        y += 16.f;
        if (i == PAGE_CAPACITY/2 - 1) {
            x = 172;
            y = PRESET_TOP;
        }
    }

    // footer
    link_button = createThemedButton<LinkButton>(Vec(15.f, box.size.y - S::U1), theme_engine, theme, "Core link");
    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ui::Menu* menu = createMenu();
            menu->addChild(createMenuLabel("— Link to Core Module —"));
            auto broker = ModuleBroker::get();
            broker->addHostPickerMenu(menu, my_module);
        });
    }
    addChild(link_button);

    addChild(haken_device_label = createLabel<TipLabel>(
        Vec(32.f, box.size.y - 13.f), 150.f, S::NotConnected, theme_engine, theme, S::haken_label));

    LabelStyle cp_style{"curpreset", TextAlignment::Right, 10.f, true};
    addChild(live_preset_label = createLabel<TipLabel>(
        Vec(box.size.x - 2*RACK_GRID_WIDTH, box.size.y - 13.f), 125.f, "[preset]", theme_engine, theme, cp_style));
    live_preset_label->glowing(true);

    if (S::show_screws()) {
        createScrews(theme);
    }

    if (!module) {
        auto logo = new WatermarkLogo(1.8f);
        logo->box.pos = Vec(84.f, 180.f - logo->box.size.y*.6);
        addChild(logo);
    }
    if (module) {
        my_module->set_chem_ui(this);
        onConnectHost(my_module->chem_host);
        //user_tab.list.set_device_info(my_module->firmware, my_module->hardware);
        //system_tab.list.set_device_info(my_module->firmware, my_module->hardware);
        user_tab.list.order = my_module->user_order;
        system_tab.list.order = my_module->system_order;

        set_tab(PresetTab(my_module->active_tab), false);
    }
}

PresetUi::~PresetUi()
{
    if (user_tab.list.dirty()) {
        save_presets(PresetTab::User);
    }
    if (user_tab.list.dirty()) {
        save_presets(PresetTab::System);
    }
}

void PresetUi::createScrews(std::shared_ptr<SvgTheme> theme)
{
    addChild(createThemedWidget<ThemeScrew>(Vec(5 * RACK_GRID_WIDTH, 0), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 6 * RACK_GRID_WIDTH, 0), theme_engine, theme));
    
    addChild(createThemedWidget<ThemeScrew>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
}

void no_preset(TipLabel* label) {
    label->text("");
    label->describe("(none)");
}

void PresetUi::build_database(PresetTab which)
{
    if (!host_available()) return;
    start_spinner();
    Tab& tab = active_tab();
    db_builder = new DBBuilder();
    PresetId dummy;
    db_builder->init(tab.list.preset_list, live_preset ? live_preset->id : dummy);
    db_builder->next(chem_host->host_haken());
}

void PresetUi::request_presets(PresetTab which)
{
    if (!host_available()) return;

    switch (which) {
    case PresetTab::Unset:
        assert(false);
        break;
    case PresetTab::User:
        user_tab.clear();
        gathering = PresetTab::User;
        chem_host->host_haken()->request_user(ChemId::Preset);
        break;
    case PresetTab::System:
        system_tab.clear();
        gathering = PresetTab::System;
        chem_host->host_haken()->request_system(ChemId::Preset);
        break;
    }
}

std::string PresetUi::preset_file(PresetTab which)
{
    assert(PresetTab::Unset != which);
    const char * s_u = which == PresetTab::System ? "system" : "user";
    auto em = chem_host->host_matrix();
    auto preset_filename = format_string("%s-%s-%d.json", PresetClassName(em->hardware), s_u, em->firmware_version);
    return user_plugin_asset(preset_filename);
}

bool PresetUi::load_presets(PresetTab which)
{
    assert(PresetTab::Unset != which);
    if (!host_available()) return false;

    if ((PresetTab::User == which) && !my_module->use_cached_user_presets) return false;

    auto em = chem_host->host_matrix();
    if (!em || (0 == em->firmware_version) || (0 == em->hardware)) return false;

    auto path = preset_file(which);
    if (!system::exists(path)) return false;

    FILE* file = std::fopen(path.c_str(), "r");
	if (!file) return false;

    DEFER({std::fclose(file);});

    json_error_t error;
	json_t* root = json_loadf(file, 0, &error);
	if (!root) {
		WARN("Invalid JSON at %d:%d %s in %s", error.line, error.column, error.text, path.c_str());
        return false;
    }
    Tab& tab = get_tab(which);
    tab.clear();
    //tab.list.set_device_info(em->firmware_version, em->hardware);
    tab.list.order = which == PresetTab::User ? my_module->user_order : my_module->system_order;
    return tab.list.from_json(root, path);
}

bool PresetUi::save_presets(PresetTab which)
{
    if (!host_available()) return false;

    Tab& tab = get_tab(which);
    if (0 == tab.count()) return false;

    auto root = json_object();
    if (!root) { return false; }
	DEFER({json_decref(root);});

    auto path = preset_file(which);
    if (system::exists(path)) {
        system::remove(path);
    }
    auto dir = system::getDirectory(path);
    system::createDirectories(dir);

    FILE* file = std::fopen(path.c_str(), "wb");
    if (!file) return false;

    uint8_t hardware = chem_host->host_matrix()->hardware;

    tab.list.to_json(root, hardware, my_module->device_claim);
    auto e = json_dumpf(root, file, JSON_INDENT(2));
	std::fclose(file);
    return e >= 0;
}

void PresetUi::sort_presets(PresetOrder order)
{
    if (!my_module) return;
    
    Tab & tab = active_tab();
    if (tab.list.order == order) return;
    my_module->set_order(active_tab_id, order);
    if (0 == tab.count()) return;

    PresetId current_id;
    if ((tab.current_index >= 0) && tab.list.count()) {
        current_id = tab.list.nth(tab.current_index)->id;
    }

    tab.list.sort(order);
    save_presets(active_tab_id);

    if (my_module->track_live) {
        scroll_to_live();
    } else {
        if (current_id.valid()) {
            size_t index = tab.list.index_of_id(current_id);
            tab.current_index = index;
        }
        scroll_to_page_of_index(tab.current_index);
    }
}

void PresetUi::onSystemBegin()
{
    if (PresetTab::Unset == gathering) {
        other_system_gather = true;
    } else {
        start_spinner();
        gather_start = true;
    }
}

void PresetUi::onSystemComplete()
{
    stop_spinner();
    gather_start = false;
    if (other_system_gather) {
        other_system_gather = false;
        return;
    }
    if (gathering == PresetTab::System) {
        gathering = PresetTab::Unset;

        //auto em = chem_host->host_matrix();
        //system_tab.list.set_device_info(em->firmware_version, em->hardware);
        system_tab.list.sort(my_module->system_order);
        save_presets(PresetTab::System);
        system_tab.list.refresh_filter_view();
        if (active_tab_id == PresetTab::System) {
            scroll_to(0);
        }
    }
}

void PresetUi::onUserBegin()
{
    if (PresetTab::Unset == gathering) {
        other_user_gather = true;
    } else {
        start_spinner();
        gather_start = true;
    }
}

void PresetUi::onUserComplete()
{
    stop_spinner();
    gather_start = false;
    if (other_user_gather) {
        other_user_gather = false;
        return;
    }
    if (gathering == PresetTab::User) {
        gathering = PresetTab::Unset;

        //auto em = chem_host->host_matrix();
        //user_tab.list.set_device_info(em->firmware_version, em->hardware);
        user_tab.list.sort(my_module->user_order);
        save_presets(PresetTab::User);
        user_tab.list.refresh_filter_view();

        if (active_tab_id == PresetTab::User) {
            scroll_to(0);
        }
    }
}

void PresetUi::onPresetChange()
{
    // ignore preset changes while some other module is gathering presets
    if (other_user_gather || other_system_gather) return;

    if (chem_host){
        auto em = chem_host->host_matrix();
        if (em->ready) {
            if (my_module){
                //my_module->firmware = em->firmware_version;
                //my_module->hardware = em->hardware;
            }
            //user_tab.list.set_device_info(em->firmware_version, em->hardware);
            //system_tab.list.set_device_info(em->firmware_version, em->hardware);
        }
    }

    if (db_builder) {
        bool done = false;
        if (host_available()) {
            auto preset = chem_host->host_preset();
            if (preset) {
                Tab& tab = active_tab();
                auto n = tab.list.index_of_id(preset->id);
                if (n >= 0) {
                    auto p = tab.list.nth(n);
                    if (p) {
                        p->set_text(preset->text);
                        tab.list.set_dirty();
                    }
                }
                if (!db_builder->next(chem_host->host_haken())) {
                    save_presets(active_tab_id);
                    done = true;
                }
            }
        } else {
            done = true;
        }

        if (done) {
            delete db_builder;
            db_builder = nullptr;
            stop_spinner();
        }
        return;
    }
    
    switch (gathering) {
    case PresetTab::Unset:
        break;
    case PresetTab::User:
    case PresetTab::System: {
        if (!gather_start) return;
        auto preset = chem_host->host_preset();
        if (preset && !preset->empty()) {
            get_tab(gathering).list.add(preset);
        }
        return;
    } break;
    }
    if (chem_host) {
        auto preset = chem_host->host_preset();
        if (preset) {
            live_preset = std::make_shared<PresetDescription>();
            live_preset->init(preset);
            live_preset_label->text(live_preset->name);
            live_preset_label->describe(live_preset->meta_text());
            Tab& tab = active_tab();
            auto n = tab.list.index_of_id(live_preset->id);
            if (n >= 0) {
                auto p = tab.list.nth(n);
                p->set_text(live_preset->text);
                tab.list.set_dirty();
            }
        } else {
            live_preset = nullptr;
            no_preset(live_preset_label);
        }
    } else {
        live_preset = nullptr;
        no_preset(live_preset_label);
    }
    if (live_preset && my_module->track_live) {
        scroll_to_live();
    }
    for (auto pw : preset_grid) {
        if (!pw->valid()) break;
        pw->live = live_preset && live_preset->valid() && (pw->preset_id() == live_preset->id);
    }
}

void PresetUi::set_current_index(size_t index)
{
    active_tab().current_index = index;
}

bool PresetUi::host_available()
{
    if (!chem_host) return false;
    if (chem_host->host_busy()) return false;
    if (!chem_host->host_connection(ChemDevice::Haken)) return false;
    auto em = chem_host->host_matrix();
    if (!em || !em->is_ready() || em->busy()) return false;

    return true;
}

void PresetUi::start_spinner()
{
    if (!spinning) {
        startSpinner(this, Vec(170.f, 190.f));
        spinning = true;
    }
}

void PresetUi::stop_spinner()
{
    stopSpinner(this);
    spinning = false;
}

void PresetUi::onConnectHost(IChemHost *host)
{
    if (chem_host) {
        chem_host->host_matrix()->unsubscribeEMEvents(this);
    }
    chem_host = host;
    if (chem_host) {
        chem_host->host_matrix()->subscribeEMEvents(this);
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        onPresetChange();
    } else {
        onConnectionChange(ChemDevice::Haken, nullptr);
        no_preset(live_preset_label);
    }
}

void PresetUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (ChemDevice::Haken == device) {
        user_tab.clear();
        system_tab.clear();
    }
    onConnectionChangeUiImpl(this, device, connection);
}

void PresetUi::set_track_live(bool track)
{
    if (!module) return;
    if (track == my_module->track_live) return;
    my_module->track_live = track;
    // TODO: track live preset
}

void PresetUi::on_search_text_changed()
{
}

void PresetUi::on_search_text_enter()
{
}

void PresetUi::on_filter_change(FilterId filter, uint64_t state)
{
    if (!my_module) return;
    my_module->filters()[filter] = state;

    Tab& tab{active_tab()};
    PresetId current_id = tab.current_id();
    PresetId track_id;
    if (!current_id.valid() && tab.list.count()) {
        track_id = tab.list.nth(tab.scroll_top)->id;
    }
    tab.list.set_filter(filter, state);
    tab.current_index = tab.list.index_of_id(current_id.valid() ? current_id : track_id);
    scroll_to_page_of_index(tab.current_index);
}

void PresetUi::clear_filters()
{
    Tab& tab{active_tab()};
    PresetId current_id = tab.current_id();
    PresetId track_id;
    if (!current_id.valid() && tab.list.count()) {
        track_id = tab.list.nth(tab.scroll_top)->id;
    }

    for (auto pf: filter_buttons) {
        pf->close_dialog();
        pf->set_state(0);
    }
    if (my_module) {
        my_module->clear_filters(active_tab_id);
    }
    active_tab().list.no_filter();

    tab.current_index = tab.list.index_of_id(current_id.valid() ? current_id : track_id);
    scroll_to_page_of_index(tab.current_index);
}


void PresetUi::set_tab(PresetTab tab_id, bool fetch)
{
    switch (tab_id) {
    case PresetTab::User:
        user_label->style(current_tab_style);
        system_label->style(tab_style);
        active_tab_id = PresetTab::User;
        break;

    case PresetTab::Unset:
        assert(false);
        goto sys;

    case PresetTab::System:
    sys:
        user_label->style(tab_style);
        system_label->style(current_tab_style);
        active_tab_id = PresetTab::System;
        break;
    }

    if (my_module) {
        my_module->active_tab = tab_id;
        auto filters = my_module->filters();
        for (auto fb: filter_buttons) {
            fb->set_state(*filters++);
        }
   }

    Tab& tab = active_tab();
    if (fetch && (0 == tab.count()) && host_available()) {
        if (load_presets(tab_id)) {

        } else {
            request_presets(tab_id);
        }
    }
    
    auto theme = theme_engine.getTheme(getThemeName());
    user_label->applyTheme(theme_engine, theme);
    system_label->applyTheme(theme_engine, theme);

    scroll_to(tab.scroll_top);
}

void PresetUi::scroll_to(ssize_t index)
{
    Tab& tab = active_tab();
    tab.scroll_top = index < 0 ? 0 : size_t(index);
    size_t ip = tab.scroll_top;
    for (auto pw: preset_grid) {
        if (ip < tab.count()) {
            auto preset = tab.list.nth(ip);
            auto live_id = get_live_id();
            bool live = live_id.valid() && (preset->id == live_id);
            pw->set_preset(ip, ssize_t(ip) == tab.current_index, live, preset);
        } else {
            pw->clear_preset();
        }
        ++ip;
    }
    update_page_controls();
}

void PresetUi::scroll_to_page_of_index(ssize_t index)
{
    scroll_to(page_index(index));
}

ssize_t PresetUi::page_index(ssize_t index)
{
    index = std::max(ssize_t(0), index);
    index = std::min(ssize_t(active_tab().count()), index);
    return PAGE_CAPACITY * (index / PAGE_CAPACITY);
}

void PresetUi::scroll_to_live()
{
    auto live_id = get_live_id();
    if (!live_id.valid()) return;
    ssize_t index = active_tab().list.index_of_id(live_id);
    if (index < 0) index = 0;
    scroll_to(page_index(index));
}

void PresetUi::page_up(bool ctrl, bool shift)
{
    Tab& tab = active_tab();
    if (tab.scroll_top < PAGE_CAPACITY) return;
    if (ctrl) {
        scroll_to(0);
    } else {
        auto index = std::max(ssize_t(0), ssize_t(tab.scroll_top) - PAGE_CAPACITY);
        scroll_to(index);
    }
}

void PresetUi::page_down(bool ctrl, bool shift)
{
    Tab& tab = active_tab();
    auto count = tab.list.count();
    if (count < PAGE_CAPACITY) {
        scroll_to(0);
        return;
    }
    if (ctrl) {
        scroll_to(page_index(count));
    } else {
        size_t pos = std::min(count, tab.scroll_top + PAGE_CAPACITY);
        scroll_to(page_index(pos));
    }
}

void PresetUi::update_page_controls()
{
    Tab& tab = active_tab();
    size_t count = tab.list.count();
    size_t page = 1 + (tab.scroll_top / PAGE_CAPACITY);
    size_t pages = 1 + (count / PAGE_CAPACITY);
    auto info = format_string("%d of %d", page, pages);
    page_label->text(info);

    if (0 == tab.scroll_top) {
        up_button->enable(false);
        down_button->enable(count > PAGE_CAPACITY);
        return;
    }

    up_button->enable(true);
    down_button->enable(true);

    if (count <= PAGE_CAPACITY) {
        up_button->enable(false);
        down_button->enable(false);
        return;
    }
    if (tab.scroll_top < PAGE_CAPACITY) {
        up_button->enable(false);
    }
    if (count - tab.scroll_top <= PAGE_CAPACITY) {
        down_button->enable(false);
    }
}

void PresetUi::next_preset(bool ctrl, bool shift)
{
}

void PresetUi::previous_preset(bool ctrl, bool shift)
{
}

void PresetUi::onButton(const ButtonEvent &e)
{
   //e.consume(this);
   Base::onButton(e);
}

void PresetUi::onSelectKey(const SelectKeyEvent &e)
{
    if (APP->event->getSelectedWidget() != this) {
        Base::onSelectKey(e);
        return;
    }
    auto mods = e.mods & RACK_MOD_MASK;
    Tab& tab = active_tab();
    if ((e.action == GLFW_PRESS || e.action == GLFW_REPEAT))
    {
        switch (e.key) {
        case GLFW_KEY_TAB:
            APP->event->setSelectedWidget(this->search_entry);
            e.consume(this);
            return;

        case GLFW_KEY_ESCAPE:
            APP->event->setSelectedWidget(nullptr);
            e.consume(this);
            return;

        case GLFW_KEY_ENTER:
            if (0 == mods)  {
                if ((tab.current_index >= 0) && host_available() && (tab.list.count() > 0)) {
                    chem_host->host_haken()->select_preset(ChemId::Preset, tab.list.nth(tab.current_index)->id);
                }
            }
            e.consume(this);
            return;

        case GLFW_KEY_HOME:
            if (0 == mods)  {
                tab.current_index = tab.scroll_top;
                scroll_to_page_of_index(tab.current_index);
            } else if (mods & RACK_MOD_CTRL) {
                tab.current_index = 0;
                scroll_to_page_of_index(0);
            }
            e.consume(this);
            return;

        case GLFW_KEY_END:
            if (0 == mods)  {
                tab.current_index = tab.scroll_top + PAGE_CAPACITY - 1;
                scroll_to_page_of_index(tab.current_index);
            } else if (mods & RACK_MOD_CTRL) {
                tab.current_index = tab.count() - 1;
                scroll_to_page_of_index(tab.current_index);
            }
            e.consume(this);
            return;

        case GLFW_KEY_UP:
            if (0 == mods)  {
                if (tab.current_index < 0) {
                    tab.current_index = 0;
                    scroll_to_page_of_index(0);
                    e.consume(this);
                    return;
                } else if (tab.current_index > 0) {
                    tab.current_index -= 1;
                    scroll_to_page_of_index(tab.current_index);
                }
            }
            e.consume(this);
            return;

        case GLFW_KEY_PAGE_UP:
            if (mods & RACK_MOD_CTRL) {
                page_up(false, (e.mods & GLFW_MOD_SHIFT));
                e.consume(this);
                return;
            }

            if (tab.current_index < 0) {
                tab.current_index = 0;
                scroll_to_page_of_index(0);
            } else if (tab.current_index > 0) {
                tab.current_index -= PAGE_CAPACITY;
                if (tab.current_index < 0) tab.current_index = 0;
                scroll_to_page_of_index(tab.current_index);
            }
            e.consume(this);
            return;

        case GLFW_KEY_DOWN:
            if (0 == mods) {
                if (tab.current_index < 0) {
                    tab.current_index = 0;
                    scroll_to_page_of_index(0);
                } else if (tab.current_index < ssize_t(tab.count())) {
                    tab.current_index += 1;
                    scroll_to_page_of_index(tab.current_index);
                }
            }
            e.consume(this);
            return;
            
        case GLFW_KEY_PAGE_DOWN:
            if (0 == mods) {
                if (tab.current_index < 0) {
                tab.current_index = 0;
                scroll_to_page_of_index(0);
                } else if (tab.current_index < ssize_t(tab.count())) {
                    tab.current_index = std::min(ssize_t(tab.count()) -1, tab.current_index + PAGE_CAPACITY);
                    scroll_to_page_of_index(tab.current_index);
                }
            }
            e.consume(this);
            return;

        case GLFW_KEY_RIGHT:
            if (0 == mods) {
                if (tab.current_index < 0) {
                    tab.current_index = 0;
                    scroll_to_page_of_index(0);
                } else if (tab.current_index - tab.scroll_top < ROWS) {
                    tab.current_index = std::min(ssize_t(tab.count()) -1, tab.current_index + ROWS);
                    scroll_to_page_of_index(tab.current_index);
                }
            }
            e.consume(this);
            return;

        case GLFW_KEY_LEFT:
            if (0 == mods) {
                if (tab.current_index < 0) {
                    tab.current_index = 0;
                    scroll_to_page_of_index(0);
                } else if (tab.current_index - tab.scroll_top > ROWS) {
                    tab.current_index = std::max(ssize_t(0), tab.current_index - ROWS);
                    scroll_to_page_of_index(tab.current_index);
                }
            }
            e.consume(this);
            return;
        }
    }
    Base::onSelectKey(e);
}

void PresetUi::onHoverScroll(const HoverScrollEvent &e)
{
    if (in_range(e.pos.x, 8.f, 334.f) && in_range(e.pos.y, 38.f, 185.f)) {
        int delta = PAGE_CAPACITY * ((e.scrollDelta.y < 0) ? 1 : (e.scrollDelta.y > 0) ? -1 : 0);
        if (0 != delta) {
            Tab& tab = active_tab();
            auto index = tab.scroll_top + delta;
            if (index <= tab.count()) {
                scroll_to_page_of_index(index);
            }
        }
        e.consume(this);
        return;
    }
    Base::onHoverScroll(e);
}

void PresetUi::step()
{
    Base::step();
    bind_host(my_module);
    if (active_tab().list.empty() && host_available()) {
        set_tab(active_tab_id, true);
    }
}

void PresetUi::draw(const DrawArgs &args)
{
    Base::draw(args);
    #ifdef LAYOUT_HELP
    if (hints) {
        Line(args.vg, RCENTER, 0, RCENTER, 380, nvgTransRGBAf(PORT_VIOLET, .5f), .5f);
    }
    #endif
}

void PresetUi::appendContextMenu(Menu *menu)
{
    // menu->addChild(new MenuSeparator);
    // menu->addChild(createCheckMenuItem("Track live preset", "", 
    //     [this](){ return my_module->track_live; },
    //     [this](){ set_track_live(!my_module->track_live); }
    // ));

    Base::appendContextMenu(menu);
}
