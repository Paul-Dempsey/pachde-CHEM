#include "chem.hpp"
#include "widgets/logo-widget.hpp"

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
    if (follow_rack) {

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


void ChemModuleWidget::setThemeName(const std::string& name)
{
    if (!module) return;
    auto panel = dynamic_cast<rack::app::SvgPanel*>(getPanel());
    if (!panel) return;

    auto svg_theme = theme_engine.getTheme(name);

    std::shared_ptr<Svg> newSvg = panel->svg;
    if (theme_engine.applyTheme(svg_theme, panelFilename(), newSvg)) {
        panel->setBackground(newSvg);
    }

    svg_theme::ApplyChildrenTheme(this, theme_engine, svg_theme);
    getChemModule()->setThemeName(name);
    sendDirty(this);
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
                setThemeName(needed);
            }
        }
    }
}

// Debug layout
void ChemModuleWidget::drawCrossLine(NVGcontext *vg, float x, float y)
{
    NVGcolor co = Overlay(GetStockColor(StockColor::Gold), 0.35f);
    Line(vg, 0.f, y, box.size.x, y, co, 0.5f);
    Line(vg, x, 0.f, x, box.size.y, co, 0.5f);
}

void ChemModuleWidget::draw(const DrawArgs& args)
{
    ModuleWidget::draw(args);
    if (showGrid) {
        auto vg = args.vg;
        NVGcolor co = Overlay(GetStockColor(StockColor::Gold), 0.35f);

        for (float x = 0.f; x < box.size.x; x += 7.5f) {
            Line(vg, x, 0.f, x, box.size.y, co, 0.5f); 
        }
        for (float y = 0.f; y < box.size.y; y += 7.5f) {
            Line(vg, 0.f, y, box.size.x, y, co, 0.5f);
        }
    }
    if (hints) {
        auto vg = args.vg;
        NVGcolor co = Overlay(GetStockColor(StockColor::Aquamarine), 0.35f);
        Line(vg, 0, 15, box.size.x, 15, co, .5f);
        Line(vg, 0, box.size.y - 15, box.size.x, box.size.y - 15, co, .5f);
        Line(vg, 15, 0, 15, box.size.y, co, .5f);
        Line(vg, box.size.x - 15, 0, box.size.x - 15, box.size.y, co, .5f);
    }
}

void ChemModuleWidget::appendContextMenu(Menu *menu)
{
    if (!module) return;
    if (!initThemeEngine()) return;
    if (!theme_engine.isLoaded()) return;
    bool follow = module ? getChemModule()->follow_rack : true;
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel("— themes —"));
    AppendThemeMenu(menu, this, theme_engine, follow);
    menu->addChild(new MenuSeparator);
    menu->addChild(createCheckMenuItem("Follow Rack theme", "",
        [=](){ return follow; },
        [=](){
            auto chem = getChemModule();
            chem->follow_rack = !follow;
            this->setThemeName(getThemeName());
        }));
    menu->addChild(createMenuItem("Reload themes", "", [this]() {
        reloadThemes();
        this->setThemeName(getThemeName());
    }));
    menu->addChild(new MenuSeparator);

    menu->addChild(createCheckMenuItem(
        "Grid", "",
        [this]() { return showGrid; },
        [this]() { showGrid = !showGrid; }));
    menu->addChild(createCheckMenuItem(
        "Layout help", "",
        [this]() { return hints; },
        [this]() { hints = !hints; }));
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
