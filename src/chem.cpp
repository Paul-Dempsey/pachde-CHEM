#include "chem.hpp"
#include "widgets/logo-widget.hpp"

void ChemModule::setThemeName(const std::string &name, void *)
{
    theme_name = name;
}

std::string ChemModule::getThemeName()
{
    if (follow_rack) {
        return ::rack::settings::preferDarkPanels ? "Dark": "Light";
    }
    if (theme_name.empty()) return DEFAULT_THEME;
    return theme_name;
}

void ChemModule::dataFromJson(json_t* root)
{
    json_t* j = json_object_get(root, "theme");
    if (j) {
        theme_name = json_string_value(j);
    }
    j = json_object_get(root, "follow-rack-theme");
    if (j) {
        follow_rack = json_boolean_value(j);
    }
}

json_t* ChemModule::dataToJson()
{
    json_t* root = json_object();
    json_object_set_new(root, "theme", json_string(theme_name.c_str()));
    json_object_set_new(root, "follow-rack-theme", json_boolean(follow_rack));
    return root;
}

// ---------------------------------------------------------------------------
// UI
//

void ChemModuleWidget::set_extender_theme(LeftRight which, const std::string& name)
{
    auto expander = left(which) ? module->getLeftExpander().module : module->getRightExpander().module;
    if (isPeerModule(module, expander)) {
        auto chem = static_cast<ChemModule*>(expander);
        if (chem->chem_ui) {
            chem->chem_ui->setThemeName(name, nullptr);
            chem->chem_ui->set_extender_theme(which, name);
        }
    }
}

void ChemModuleWidget::setThemeName(const std::string& name, void *context)
{
    if (!module) return;

    auto panel = dynamic_cast<rack::app::SvgPanel*>(getPanel());
    if (!panel) return;

    getChemModule()->setThemeName(name, context);

    auto svg_theme = theme_engine.getTheme(name);

    std::shared_ptr<Svg> newSvg = panel->svg;
    if (theme_engine.applyTheme(svg_theme, panelFilename(), newSvg)) {
        panel->setBackground(newSvg);
    }

    svg_theme::ApplyChildrenTheme(this, theme_engine, svg_theme);
    sendDirty(this);

    if (context == this) {
        set_extender_theme(LeftRight::Left, name);
        set_extender_theme(LeftRight::Right, name);
    }
}

void ChemModuleWidget::step()
{
    ModuleWidget::step();
    setPartnerPanelBorder<ChemModuleWidget>(this);
    if (module) {
        auto chem = getChemModule();
        if (chem->follow_rack) {
            auto needed = ::rack::settings::preferDarkPanels ? "Dark" : "Light";
            if (needed != chem->theme_name) {
                setThemeName(needed, nullptr);
            }
        }
    }
}

void ChemModuleWidget::onHoverKey(const HoverKeyEvent& e)
{
    auto mods = e.mods & RACK_MOD_MASK;
    switch (e.key) {
    case GLFW_KEY_F5: {
        if (e.action == GLFW_RELEASE && (0 == mods)) {
            e.consume(this);
            reloadThemes();
            setThemeName(getThemeName(), nullptr);
        }
    } break;

    case GLFW_KEY_MENU:
        if ((e.action == GLFW_RELEASE) && (0 == mods)){
            e.consume(this);
            createContextMenu();
        }
        break;
    }
    Base::onHoverKey(e);

}

#ifdef LAYOUT_HELP
void ChemModuleWidget::drawCrossLine(NVGcontext *vg, float x, float y)
{
    NVGcolor co = Overlay(GetStockColor(StockColor::Gold), 0.35f);
    Line(vg, 0.f, y, box.size.x, y, co, 0.5f);
    Line(vg, x, 0.f, x, box.size.y, co, 0.5f);
}
#endif

void ChemModuleWidget::draw(const DrawArgs& args)
{
    ModuleWidget::draw(args);
#ifdef LAYOUT_HELP
    if (showGrid) {
        auto vg = args.vg;
        NVGcolor co = Overlay(GetStockColor(StockColor::Coral), 0.35f);
        NVGcolor co2 = Overlay(GetStockColor(StockColor::Gold), 0.35f);

        bool odd = true;
        for (float x = 7.5f; x < box.size.x; x += 7.5f) {
            Line(vg, x, 0.f, x, box.size.y, odd ? co : co2, 0.35f);
            odd = !odd;
        }
        odd = true;
        for (float y = 7.5f; y < box.size.y; y += 7.5f) {
            Line(vg, 0.f, y, box.size.x, y, odd ? co : co2, 0.35f);
            odd = !odd;
        }
    }
    if (hints) {
        auto vg = args.vg;
        NVGcolor co = Overlay(GetStockColor(StockColor::Aquamarine), 0.35f);
        Line(vg, 0, 15, box.size.x, 15, co, .5f);
        Line(vg, 0, box.size.y - 15, box.size.x, box.size.y - 15, co, .5f);
        Line(vg, 15, 0, 15, box.size.y, co, .5f);
        Line(vg, box.size.x - 15, 0, box.size.x - 15, box.size.y, co, .5f);

        co = Overlay(GetStockColor(StockColor::Coral), 0.35f);
        Line(vg, box.size.x*.5f, 0, box.size.x*.5f, box.size.y, co, .5f);
        Line(vg, 0, box.size.y*.5f, box.size.x, box.size.y*.5f, co, .5f);

    }
#endif
}

void ChemModuleWidget::appendContextMenu(Menu *menu)
{
    if (!module) return;
    if (!initThemeEngine()) return;
    if (!theme_engine.isLoaded()) return;
    bool follow = module ? getChemModule()->follow_rack : true;
//    bool sync = module ? getChemModule()->sync_theme : true;

    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel("— themes —"));
    AppendThemeMenu(menu, this, theme_engine, follow, this);

    menu->addChild(new MenuSeparator);
    menu->addChild(createCheckMenuItem("Follow Rack theme", "",
        [=](){ return follow; },
        [=](){
            auto chem = getChemModule();
            chem->follow_rack = !follow;
            this->setThemeName(getThemeName(), nullptr);
        }));

    menu->addChild(createMenuItem("Hot-reload themes", "F5", [this]() {
        reloadThemes();
        this->setThemeName(getThemeName(), this);
    }));

#ifdef LAYOUT_HELP
    menu->addChild(new MenuSeparator);
    menu->addChild(createCheckMenuItem(
        "Grid", "",
        [this]() { return showGrid; },
        [this]() { showGrid = !showGrid; }));
    menu->addChild(createCheckMenuItem(
        "Layout help", "",
        [this]() { return hints; },
        [this]() { hints = !hints; }));
#endif
}

// ---------------------------------------------------------------------------
// utils
//

NVGcolor ColorFromTheme(const std::string& theme, const char * color_name, const NVGcolor& fallback)
{
    assert(color_name);
    if (theme.empty()) return fallback;
    auto co = fromPacked(theme_engine.getFillColor(theme, color_name, true));
    return isColorVisible(co) ? co : fallback;
}

NVGcolor ColorFromTheme(const std::string& theme, const char * color_name, StockColor fallback)
{
    assert(color_name);
    if (theme.empty()) return GetStockColor(fallback);
    auto co = fromPacked(theme_engine.getFillColor(theme, color_name, true));
    return isColorVisible(co) ? co : GetStockColor(fallback);
}
