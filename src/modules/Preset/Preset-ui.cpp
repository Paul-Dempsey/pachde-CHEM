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
    menu->addChild(createMenuItem("Sort by category", "",    [this](){ /*ui->open_playlist();*/ }, false));
    menu->addChild(createMenuItem("Sort alphabetically", "",    [this](){ /*ui->open_playlist();*/ }, false));
    menu->addChild(createMenuItem("System order", "",    [this](){ /*ui->open_playlist();*/ }, false));

    // menu->addChild(createMenuItem("Open...", "",    [this](){ ui->open_playlist(); }, false));
    // menu->addChild(createMenuItem("Close", "",      [this](){ ui->close_playlist(); }, false));
    // menu->addChild(createMenuItem("Save", "",       [this](){ ui->save_playlist(); } , false));
    // menu->addChild(createMenuItem("Save as...", "", [this](){ ui->save_as_playlist(); } , false));
    // menu->addChild(createMenuItem("Clear", "", [this](){ ui->clear_playlist(false); }, ui->presets.empty()));

    // if (ui->my_module) {
    //     menu->addChild(new MenuSeparator);
    //     menu->addChild(createSubmenuItem("Open recent", "", [=](Menu* menu) {
    //         if (ui->my_module->playlist_mru.empty()) {
    //             menu->addChild(createMenuLabel("(empty)"));
    //         } else {
    //             for (auto path : ui->my_module->playlist_mru) {
    //                 auto name = system::getStem(path);
    //                 menu->addChild(createMenuItem(name, "", [=]() {
    //                     ui->load_playlist(path, true);
    //                 }));
    //             }
    //         }
    //     }));
    //     menu->addChild(createMenuItem("Clear recents", "", [this](){ ui->my_module->clear_mru(); }, false));
    // }
    // menu->addChild(new MenuSeparator);
    // menu->addChild(createSubmenuItem("Append", "", [=](Menu* menu) {
    //     menu->addChild(createMenuItem("User presets", "", [this](){ ui->fill(FillOptions::User); }));
    //     menu->addChild(createMenuItem("System presets", "", [this](){ ui->fill(FillOptions::System); }));
    //     menu->addChild(createMenuItem("All presets", "", [this](){ ui->fill(FillOptions::All); }));
    // }));

    // menu->addChild(new MenuSeparator);
    // bool no_selection =  ui->selected.empty();
    // bool first = (0 == ui->first_selected());
    // bool last = (ui->last_selected() == static_cast<int>(ui->presets.size())-1);
    // menu->addChild(createMenuLabel("— Selected —"));
    // menu->addChild(createMenuItem("Duplicate", "",     [this](){ ui->clone(); }, no_selection));
    // menu->addChild(createMenuItem("Remove", "",        [this](){ ui->remove_selected(); }, no_selection));
    // menu->addChild(new MenuSeparator);
    // menu->addChild(createMenuItem("Move to first", "", [this](){ ui->to_first(); }, first || no_selection));
    // menu->addChild(createMenuItem("Move up", "",       [this](){ ui->to_up(); }, first || no_selection));
    // menu->addChild(createMenuItem("Move down", "",     [this](){ ui->to_down(); }, last || no_selection));
    // menu->addChild(createMenuItem("Move to last", "",  [this](){ ui->to_last(); }, last || no_selection));
    // menu->addChild(new MenuSeparator);
    // menu->addChild(createMenuItem("Select none", "Esc",  [this](){ ui->select_none(); }, no_selection));
}



constexpr const int ROWS = 20;
constexpr const int COLS = 2;
constexpr const int PAGE_CAPACITY = ROWS * COLS;
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
    search_entry->box.pos = Vec(23.5f, 11.f);
    search_entry->box.size = Vec(114.f, 14.f);
    search_entry->applyTheme(theme_engine, theme);
    search_entry->placeholder = "preset search";
    search_entry->change_handler = [this](){ this->on_search_text_changed(); };
    search_entry->enter_handler = [this](){ this->on_search_text_enter(); };
    addChild(search_entry);

    // tab labels
    x = 270.f;
    y = 10.f;
    addChild(user_label = createLabel<TextLabel>(Vec(x,y), 26.f, "User", theme_engine, theme, tab_style));
    addChild(createClickRegion(RECT_ARGS(user_label->box.grow(Vec(6,2))), (int)PresetTab::User, [=](int id, int mods) { set_tab((PresetTab)id); }));

    x += 55.f;
    addChild(system_label = createLabel<TextLabel>(Vec(x,y), 40.f, "System", theme_engine, theme, current_tab_style));
    addChild(createClickRegion(RECT_ARGS(system_label->box.grow(Vec(6,2))), (int)PresetTab::System, [=](int id, int mods) { set_tab((PresetTab)id); }));

    // right controls

    x = RCENTER;
    LabelStyle style{"dytext", TextAlignment::Center, 9.f};
    addChild(page_label = createLabel<TextLabel>(Vec(x, 16.f), 20.f, "1 of 1", theme_engine, theme, style));

    up_button = createWidgetCentered<UpButton>(Vec(x, 36.f));
    up_button->describe("Page up");
    up_button->applyTheme(theme_engine, theme);
    up_button->setHandler([this](bool c, bool s){ 
        page_up(c,s);
    });
    addChild(up_button);

    down_button = createWidgetCentered<DownButton>(Vec(x, 51.f));
    down_button->describe("Page down");
    down_button->applyTheme(theme_engine, theme);
    down_button->setHandler([this](bool c, bool s){
        page_down(c,s);
    });
    addChild(down_button);

    auto prev = createWidgetCentered<PrevButton>(Vec(x, 74.f));
    prev->describe("Select previous preset");
    prev->applyTheme(theme_engine, theme);
    prev->setHandler([this](bool c, bool s) {
        previous_preset(c,s);
    });
    addChild(prev);

    auto next = createWidgetCentered<NextButton>(Vec(x, 88.f));
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
    }
}

void no_preset(TipLabel* label) {
    label->text("");
    label->describe("(none)");
}

void PresetUi::onConnectHost(IChemHost *host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        onPresetChange();
    } else {
        onConnectionChange(ChemDevice::Haken, nullptr);
        no_preset(live_preset_label);
    }
}

void PresetUi::onPresetChange()
{
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
        //scroll_to_live();
    }
}

void PresetUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    onConnectionChangeUiImpl(this, device, connection);
}

void PresetUi::createScrews(std::shared_ptr<SvgTheme> theme)
{
    addChild(createThemedWidget<ThemeScrew>(Vec(5 * RACK_GRID_WIDTH, 0), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 6 * RACK_GRID_WIDTH, 0), theme_engine, theme));
    
    addChild(createThemedWidget<ThemeScrew>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
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

void PresetUi::set_tab(PresetTab tab)
{
    if (tab == PresetTab::User) {
        user_label->style(current_tab_style);
        system_label->style(tab_style);
        active_tab = PresetTab::User;
    } else {
        user_label->style(tab_style);
        system_label->style(current_tab_style);
        active_tab = PresetTab::System;
    }
    auto theme = theme_engine.getTheme(getThemeName());
    user_label->applyTheme(theme_engine, theme);
    system_label->applyTheme(theme_engine, theme);
}

void PresetUi::page_up(bool ctrl, bool shift)
{
}

void PresetUi::page_down(bool ctrl, bool shift)
{
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
    menu->addChild(new MenuSeparator);
    menu->addChild(createCheckMenuItem("Track live preset", "", 
        [this](){ return my_module->track_live; },
        [this](){ set_track_live(!my_module->track_live); }
    ));
    Base::appendContextMenu(menu);
}
