#include "chem.hpp"

void ChemModule::dataFromJson(json_t* root)
{
    json_t* j = json_object_get(root, "theme");
    if (j) {
        theme_name = json_string_value(j);
    }

}

json_t* ChemModule::dataToJson()
{
    json_t* root = json_object();
    json_object_set_new(root, "theme", json_stringn(theme_name.c_str(), theme_name.size()));

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
}

void ChemModuleWidget::appendContextMenu(Menu *menu)
{
    if (!module) return;
    if (!initThemeEngine()) return;
    if (!theme_engine.isLoaded()) return;

    menu->addChild(new MenuSeparator); 
    AppendThemeMenu(menu, this, theme_engine);
    menu->addChild(new MenuSeparator); 
    menu->addChild(createMenuItem("Reload themes", "", [this]() {
        reloadThemes();
        this->setThemeName(getThemeName());
    }));

    menu->addChild(createCheckMenuItem(
        "Grid", "",
        [this]() { return showGrid; },
        [this]() { showGrid = !showGrid; }));

}

// ---------------------------------------------------------------------------
// utils
//

NVGcolor ColorFromTheme(const std::string& theme, const char * color_name, const NVGcolor& fallback)
{
    assert(color_name);
    if (theme.empty()) return fallback;
    auto co = fromPacked(theme_engine.getFillColor(theme, color_name));
    return isColorVisible(co) ? co : fallback;
}

NVGcolor ColorFromTheme(const std::string& theme, const char * color_name, StockColor fallback)
{
    assert(color_name);
    if (theme.empty()) return GetStockColor(fallback);
    auto co = fromPacked(theme_engine.getFillColor(theme, color_name));
    return isColorVisible(co) ? co : GetStockColor(fallback);
}
