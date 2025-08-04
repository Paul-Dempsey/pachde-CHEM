#include "Preset.hpp"
#include "../../em/preset-meta.hpp"
#include "../../services/colors.hpp"
#include "../../widgets/click-region-widget.hpp"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/hamburger.hpp"
#include "../../widgets/spinner.hpp"

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
    menu->addChild(createMenuLabel<HamburgerTitle>("Preset Actions"));

    NVGcolor co_dot{nvgHSL(200.f/360.f, .5, .5)};

    Tab & tab = ui->active_tab();
    PresetOrder order = tab.list.preset_list ? tab.list.preset_list->order : PresetOrder::Alpha;
    auto item = createMenuItem<ColorDotMenuItem>("Sort alphabetically", "",
        [this](){ ui->sort_presets(PresetOrder::Alpha); }, false);
    item->color = (PresetOrder::Alpha == order) ? co_dot : RampGray(G_45);
    menu->addChild(item);

    item = createMenuItem<ColorDotMenuItem>("Sort by category", "",
        [this](){ ui->sort_presets(PresetOrder::Category); }, false);
    item->color = (PresetOrder::Category == order) ? co_dot : RampGray(G_45);
    menu->addChild(item);

    item = createMenuItem<ColorDotMenuItem>("Sort by preset number", "",
        [this](){ ui->sort_presets(PresetOrder::Natural); }, false);
    item->color = (PresetOrder::Natural == order) ? co_dot : RampGray(G_45);
    menu->addChild(item);

    menu->addChild(new MenuSeparator);

    menu->addChild(createMenuItem("Clear filters", "",
        [this](){ ui->clear_filters(); },
        !ui->filtering()
    ));

    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem("Show live preset", "",
        [this](){ ui->scroll_to_live(); },
        !ui->get_live_id().valid()
    ));
    menu->addChild(createCheckMenuItem("Track live preset", "",
        [this](){ return ui->my_module->track_live; },
        [this](){ ui->set_track_live(!ui->my_module->track_live); },
        !ui->my_module
    ));

    //bool osmose = ui->is_osmose();
    // menu->addChild(createCheckMenuItem("Use cached User presets", "",
    //     [this, osmose]() { return osmose || ui->my_module->use_cached_user_presets; },
    //     [this]() { ui->my_module->use_cached_user_presets = !ui->my_module->use_cached_user_presets; },
    //     osmose || !ui->my_module
    // ));
    menu->addChild(createCheckMenuItem("Keep search filters", "",
        [this]() { return ui->my_module->keep_search_filters; },
        [this]() { ui->my_module->keep_search_filters = !ui->my_module->keep_search_filters; },
        !ui->my_module
    ));
}

struct SearchMenu : PresetMenu
{
    void appendContextMenu(ui::Menu* menu) override {
        menu->addChild(createMenuLabel<HamburgerTitle>("Search Options"));

        menu->addChild(createCheckMenuItem("Search preset Name", "",
            [this](){ return ui->my_module->search_name; },
            [this](){
                ui->my_module->search_name = !ui->my_module->search_name;
                if (!ui->my_module->search_name) {
                    ui->my_module->search_meta = true;
                }
                ui->on_search_text_enter();
            },
            !ui->my_module
        ));
        menu->addChild(createCheckMenuItem("Search preset Metadata", "",
            [this](){ return ui->my_module->search_meta; },
            [this](){
                ui->my_module->search_meta = !ui->my_module->search_meta;
                if (!ui->my_module->search_meta) {
                    ui->my_module->search_name = true;
                }
                ui->on_search_text_enter();
            },
            !ui->my_module
        ));
        menu->addChild(createCheckMenuItem("Match at Word start", "",
            [this](){ return ui->my_module->search_anchor; },
            [this](){
                ui->my_module->search_anchor = !ui->my_module->search_anchor;
                ui->on_search_text_enter();
            },
            !ui->my_module
        ));

        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Clear filters", "",
            [this](){ ui->clear_filters(); },
            !ui->filtering()
        ));

        menu->addChild(new MenuSeparator);
        NVGcolor co_dot{nvgHSL(200.f/360.f, .5, .5)};
        auto item = createMenuItem<ColorDotMenuItem>("Search on ENTER", "",
            [this](){ ui->my_module->search_incremental = false; }, !ui->my_module);
        if (ui->my_module) {
            item->color = (!ui->my_module->search_incremental) ? co_dot : RampGray(G_45);
        } else {
            item->color = RampGray(G_45);
        }
        menu->addChild(item);

        item = createMenuItem<ColorDotMenuItem>("Search as you type", "",
            [this](){ ui->my_module->search_incremental = true; },  !ui->my_module);
        if (ui->my_module) {
            item->color = ui->my_module->search_incremental ? co_dot : RampGray(G_45);
        } else {
            item->color =  nvgHSL(360/200, .5, .5);
        }
        menu->addChild(item);
    }
};


// PresetUi

PresetUi::PresetUi(PresetModule *module) :
    my_module(module)
{
    setModule(module);
    IHandleEmEvents::module_id = ChemId::Preset;
    IHandleEmEvents::em_event_mask = (IHandleEmEvents::EventMask)(
        SystemBegin +
        SystemComplete +
        UserBegin +
        UserComplete
    );
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);

    float x, y;

    x = 23.5f;
    y = 18.f;
    search_entry = createThemedTextInput(x, y, 114.f, 14.f, theme_engine, theme,         "",
        [=](std::string text){ on_search_text_changed(text); },
        [=](std::string text){ on_search_text_enter(); },
        "preset search");
    addChild(search_entry);

    menu = Center(createThemedWidget<SearchMenu>(Vec(x + 122.f, y + 7.f), theme_engine, theme));
    menu->setUi(this);
    menu->describe("Search options");
    addChild(menu);

    x = 170.f;
    y = 25.f;
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

    menu = Center(createThemedWidget<PresetMenu>(Vec(x, 148.f), theme_engine, theme));
    menu->setUi(this);
    menu->describe("Preset actions menu");
    addChild(menu);

    // Filter buttons

    const float FILTER_DY = 20.f;
    y = 205.f;
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

    y += FILTER_DY + 6.f;
    addChild(filter_off_button = Center(makeFilterStateButton(Vec(x,y), theme_engine, theme, [=]() {
        if (filtering()) {
            clear_filters();
            filter_off_button->button_down = false;
        }
    })));

    if (my_module) {
        user_tab.list.init_filters(my_module->user_filters);
        system_tab.list.init_filters(my_module->system_filters);
        auto filters = my_module->filters();
        for (auto fb: filter_buttons) {
            fb->set_state(*filters++);
        }
    }

    // preset grid
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

    LabelStyle help_style{"curpreset", TextAlignment::Center, 16.f, true};
    addChild(help_label = createLabel<TextLabel>(Vec(172,180), 280.f, "", theme_engine, theme, help_style));

    // footer
    link_button = createThemedButton<LinkButton>(Vec(15.f, box.size.y - S::U1), theme_engine, theme, "Core link");
    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ModuleBroker::get()->addHostPickerMenu(createMenu(), my_module);
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
        set_tab(PresetTab(my_module->active_tab), false);
    }
}

PresetUi::~PresetUi()
{
    if (chem_host) {
        auto ipl = chem_host->host_preset_list();
        if (ipl) {
            ipl->unregister_preset_list_client(this);
        }
    }
}

void PresetUi::createScrews(std::shared_ptr<SvgTheme> theme)
{
    addChild(createThemedWidget<ThemeScrew>(Vec(5 * RACK_GRID_WIDTH, 0), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 6 * RACK_GRID_WIDTH, 0), theme_engine, theme));

    addChild(createThemedWidget<ThemeScrew>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
}

