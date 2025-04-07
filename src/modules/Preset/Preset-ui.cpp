#include "Preset.hpp"
#include "../../services/colors.hpp"
#include "../../widgets/click-region-widget.hpp"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
using PM = PresetModule;

constexpr const int ROWS = 20;
constexpr const float PANEL_WIDTH = 360.f;
constexpr const float ROW_HEIGHT = 16.f;
constexpr const float RCENTER = PANEL_WIDTH - S::U1;

PresetModuleWidget::PresetModuleWidget(PresetModule *module) :
    my_module(module)
{
    setModule(module);

    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);
    if (S::show_screws()) {
        createScrews(theme);
    }

    float x, y;
    search_entry = new SearchField();
    search_entry->box.pos = Vec(23.5f, 10.f);
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
    addChild(haken_device_label = createLabel<TipLabel>(
        Vec(28.f, box.size.y - 13.f), 200.f, S::NotConnected, theme_engine, theme, S::haken_label));

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y - S::U1), theme_engine, theme, "Core link");

    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ui::Menu* menu = createMenu();
            menu->addChild(createMenuLabel("— Link to Core Module —"));
            auto broker = ModuleBroker::get();
            broker->addHostPickerMenu(menu, my_module);
        });
    }
    addChild(link_button);

    if (!module) {
        auto logo = new WatermarkLogo(1.8f);
        logo->box.pos = Vec(84.f, 180.f - logo->box.size.y*.6);
        addChild(logo);
    }
    if (module) {
        my_module->set_chem_ui(this);
    }
}

void PresetModuleWidget::createScrews(std::shared_ptr<SvgTheme> theme)
{
    addChild(createThemedWidget<ThemeScrew>(Vec(5 * RACK_GRID_WIDTH, 0), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 6 * RACK_GRID_WIDTH, 0), theme_engine, theme));
    
    addChild(createThemedWidget<ThemeScrew>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2*RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
    //addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
}

void PresetModuleWidget::on_search_text_changed()
{
}

void PresetModuleWidget::on_search_text_enter()
{
}

void PresetModuleWidget::set_tab(PresetTab tab)
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

void PresetModuleWidget::page_up(bool ctrl, bool shift)
{
}

void PresetModuleWidget::page_down(bool ctrl, bool shift)
{
}

void PresetModuleWidget::next_preset(bool ctrl, bool shift)
{
}

void PresetModuleWidget::previous_preset(bool ctrl, bool shift)
{
}

// void PresetModuleWidget::step()
// {
//     ChemModuleWidget::step();
// }

void PresetModuleWidget::draw(const DrawArgs& args)
{
    Base::draw(args);
    #ifdef LAYOUT_HELP
    if (hints) {
        Line(args.vg, RCENTER, 0, RCENTER, 380, nvgTransRGBAf(PORT_VIOLET, .5f), .5f);
    }
    #endif
}

void PresetModuleWidget::appendContextMenu(Menu *menu)
{
    Base::appendContextMenu(menu);
}
