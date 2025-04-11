#include "Preset.hpp"
#include "../../em/preset-meta.hpp"
#include "../../services/colors.hpp"
#include "../../widgets/click-region-widget.hpp"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
using PM = PresetModule;

// Preset Menu

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

bool PresetMenu::applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
    return Base::applyTheme(engine, theme);
}

void PresetMenu::appendContextMenu(ui::Menu* menu)
{
    menu->addChild(createMenuLabel("— Preset Actions —"));
    menu->addChild(createMenuItem("Sort by category", "",    [this](){ /*ui->foo();*/ }, false));
    menu->addChild(createMenuItem("Sort alphabetically", "",    [this](){ /*ui->foo();*/ }, false));
    menu->addChild(createMenuItem("System order", "",    [this](){ /*ui->foo();*/ }, false));

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
}



constexpr const float PANEL_WIDTH = 360.f;
constexpr const float ROW_HEIGHT = 16.f;
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

    // tab labels
    x = 270.f;
    y = 18.f;
    addChild(user_label = createLabel<TextLabel>(Vec(x,y), 26.f, "User", theme_engine, theme, tab_style));
    addChild(createClickRegion(RECT_ARGS(user_label->box.grow(Vec(6,2))), (int)PresetTab::User, [=](int id, int mods) { set_tab((PresetTab)id); }));

    x += 55.f;
    addChild(system_label = createLabel<TextLabel>(Vec(x,y), 40.f, "System", theme_engine, theme, current_tab_style));
    addChild(createClickRegion(RECT_ARGS(system_label->box.grow(Vec(6,2))), (int)PresetTab::System, [=](int id, int mods) { set_tab((PresetTab)id); }));

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
    addChild(Center(createThemedParamButton<CatParamButton>(Vec(x,y), my_module, PM::P_CAT, theme_engine, theme)));
    y += FILTER_DY;
    addChild(Center(createThemedParamButton<TypeParamButton>(Vec(x,y), my_module, PM::P_TYPE, theme_engine, theme)));
    y += FILTER_DY;
    addChild(Center(createThemedParamButton<CharacterParamButton>(Vec(x,y), my_module, PM::P_CHARACTER, theme_engine, theme)));
    y += FILTER_DY;
    addChild(Center(createThemedParamButton<MatrixParamButton>(Vec(x,y), my_module, PM::P_MATRIX, theme_engine, theme)));
    y += FILTER_DY;
    addChild(Center(createThemedParamButton<GearParamButton>(Vec(x,y), my_module, PM::P_GEAR, theme_engine, theme)));

    // preset grid
    const float PRESET_TOP = 38.f;
    x = 9.f; y = PRESET_TOP;
    for (int i = 0; i < PAGE_CAPACITY; ++i) {
        auto entry = PresetEntry::create(Vec(x,y), preset_grid, theme);
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
        user_tab.list.set_device_info(my_module->firmware, my_module->hardware);
        system_tab.list.set_device_info(my_module->firmware, my_module->hardware);
        set_tab(PresetTab(my_module->active_tab));
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
    tab.list.set_device_info(em->firmware_version, em->hardware);
    return tab.list.from_json(root, path);
}

bool PresetUi::save_presets(PresetTab which)
{
    if (!host_available()) return false;

    Tab& tab = get_tab(which);
    if (tab.list.presets.empty()) return false;

    auto root = json_object();
    if (!root) { return false; }
	DEFER({json_decref(root);});

    auto path = preset_file(which);
    auto dir = system::getDirectory(path);
    system::createDirectories(dir);
    
    FILE* file = std::fopen(path.c_str(), "wb");
    if (!file) return false;

    tab.list.to_json(root, my_module->device_claim);
    auto e = json_dumpf(root, file, JSON_INDENT(2));
	std::fclose(file);
    return e >= 0;
}

void PresetUi::onSystemBegin()
{
    if (PresetTab::Unset == gathering) {
        other_system_gather = true;
    }
}

void PresetUi::onSystemComplete()
{
    if (other_system_gather) {
        other_system_gather = false;
        return;
    }
    if (gathering == PresetTab::System) {
        gathering = PresetTab::Unset;

        auto em = chem_host->host_matrix();
        system_tab.list.set_device_info(em->firmware_version, em->hardware);
        save_presets(PresetTab::System);

        if (active_tab_id == PresetTab::System) {
            scroll_to(0);
        }
    }
}

void PresetUi::onUserBegin()
{
    if (PresetTab::Unset == gathering) {
        other_user_gather = true;
    }
}

void PresetUi::onUserComplete()
{
    if (other_user_gather) {
        other_user_gather = false;
        return;
    }
    if (gathering == PresetTab::User) {
        gathering = PresetTab::Unset;

        auto em = chem_host->host_matrix();
        user_tab.list.set_device_info(em->firmware_version, em->hardware);
        save_presets(PresetTab::User);

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
                my_module->firmware = em->firmware_version;
                my_module->hardware = em->hardware;
            }
            user_tab.list.set_device_info(em->firmware_version, em->hardware);
            system_tab.list.set_device_info(em->firmware_version, em->hardware);
        }
    }
    switch (gathering) {
    case PresetTab::Unset:
        break;
    case PresetTab::User:
    case PresetTab::System: {
        auto preset = chem_host->host_preset();
        if (preset && !preset->empty()) {
            auto pd = std::make_shared<PresetDescription>();
            pd->init(preset);
            get_tab(gathering).list.add(pd);
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
            if (preset->text.empty()) {
                live_preset_label->describe(preset->summary());
            } else {
                auto meta = hakenCategoryCode.make_category_mulitline_text(preset->text);
                auto text = format_string("%s\n[%d.%d.%d]\n%s", preset->name.c_str(), preset->id.bank_hi(), preset->id.bank_lo(), preset->id.number(), meta.c_str());
                live_preset_label->describe(text);
            }
            return;
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
        if (!pw->preset_id.valid()) break;
        pw->live = live_preset ? pw->preset_id == live_preset->id : false;
    }
}

bool PresetUi::host_available()
{
    if (!chem_host) return false;
    if (chem_host->host_busy()) return false;
    if (!chem_host->host_connection(ChemDevice::Haken)) return false;
    auto em = chem_host->host_matrix();
    if (!em || !em->is_ready()) return false;

    return true;
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

void PresetUi::set_tab(PresetTab tab_id)
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
        my_module->active_tab = int(active_tab_id);
    }

    Tab& tab = active_tab();
    if (tab.list.presets.empty()) {
        if (!load_presets(tab_id)) {
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
    tab.scroll_top = index;
    size_t ip = tab.scroll_top;
    for (auto pw: preset_grid) {
        if (ip < tab.list.presets.size()) {
            auto preset = tab.list.nth(ip);
            auto live_id = get_live_id();
            bool live = live_id.valid() && (preset->id == live_id);
            pw->set_preset(ip, false, live, preset);
        } else {
            pw->clear_preset();
        }
        ++ip;
    }
    update_page_controls();
}

void PresetUi::scroll_to_live()
{
    auto live_id = get_live_id();
    if (!live_id.valid()) return;
    size_t index = 0;
    for (auto p : active_tab().list.presets) {
        if (live_id == p->id) break;
        index++;
    }

    scroll_to(PAGE_CAPACITY * (index / PAGE_CAPACITY));
}

void PresetUi::clear_filters()
{
    if (!my_module) return;
    for (Param& p : my_module->params) {
        p.setValue(0);
    }
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
        scroll_to((count / PAGE_CAPACITY) * PAGE_CAPACITY);
    } else {
        size_t pos = std::min(count, tab.scroll_top + PAGE_CAPACITY);
        size_t page = pos / PAGE_CAPACITY;
        scroll_to(page * PAGE_CAPACITY);
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

void PresetUi::step()
{
    Base::step();
    bind_host(my_module);
    if (active_tab().list.empty() && host_available()) {
        set_tab(active_tab_id);
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
