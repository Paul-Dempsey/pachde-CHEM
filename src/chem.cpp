#include "chem.hpp"
#include "widgets/hamburger.hpp"
#include "widgets/logo-widget.hpp"
#include "widgets/layout-help.hpp"
#include "widgets/uniform-style.hpp"
#include "services/kv-store.hpp"
#include "services/json-help.hpp"
using namespace widgetry;

void ChemModule::setThemeName(const std::string &name, void *) {
    theme_setting = name;
    actual_theme = theme::get_actual_theme(theme_setting);
}

std::string ChemModule::getThemeName() {
    return theme_setting;
}

std::string ChemModule::get_actual_theme_name() {
    if (actual_theme.empty()) {
        actual_theme = theme::get_actual_theme(theme_setting);
    }
    return actual_theme;
}

void ChemModule::dataFromJson(json_t* root)
{
    auto j = json_object_get(root, "follow-rack-theme"); // backwards compat
    if (j && json_boolean_value(j)) {
        theme_setting = theme::theme_name::Ui;
    } else {
        theme_setting = get_json_string(root, "theme", theme_setting);
    }
    actual_theme = theme::get_actual_theme(theme_setting);
}

json_t* ChemModule::dataToJson()
{
    json_t* root = json_object();
    set_json(root, "theme", theme_setting);
    return root;
}

IChemHost *ChemModule::find_expander_host()
{
    Module* scan = this;
    for (scan = leftExpander.module; nullptr != scan; scan = scan->leftExpander.module) {
        if (!isChemModule(scan)) break;
        auto host = static_cast<ChemModule*>(scan)->get_host();
        if (host) return host;
    }
    for (scan = rightExpander.module; nullptr != scan; scan = scan->rightExpander.module) {
        if (!isChemModule(scan)) return nullptr;
        auto host = static_cast<ChemModule*>(scan)->get_host();
        if (host) return host;
    }
    return nullptr;
}

void ChemModule::process(const ProcessArgs &args)
{
    if (!seek_host && !get_host() && ((args.frame + id) % 80) == 0) {
        seek_host = true;
    }
}

// ---------------------------------------------------------------------------
// UI
//
void add_screws()
{
    auto engine = APP->engine;
    auto mods = engine->getModuleIds();
    for (auto id: mods) {
        auto module = engine->getModule(id);
        if (isChemModule(module)) {
            auto chem = dynamic_cast<ChemModule*>(module);
            assert(chem);
            auto ui = chem->chem_ui;
            if (ui) {
                ui->createScrews();
            }
        }
    }
}

void remove_screws()
{
    auto engine = APP->engine;
    auto mods = engine->getModuleIds();
    for (auto id: mods) {
        auto module = engine->getModule(id);
        if (isChemModule(module)) {
            auto chem = dynamic_cast<ChemModule*>(module);
            assert(chem);
            if (chem->chem_ui) {
                std::vector<Widget*> screws;
                for (auto child: chem->chem_ui->children) {
                    auto screw = dynamic_cast<ThemeScrew*>(child);
                    if (screw) screws.push_back(screw);
                }
                for (auto screw: screws) {
                    chem->chem_ui->removeChild(screw);
                }
            }
        }
    }
}

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

    // auto panel = dynamic_cast<rack::app::SvgPanel*>(getPanel());
    // if (!panel) return;

    getChemModule()->setThemeName(name, context);
    auto svg_theme = getSvgTheme();
    module_svgs.changeTheme(svg_theme);
    // std::shared_ptr<Svg> newSvg = module_svgs.loadSvg(panelFilename());
    // if (applySvgTheme(newSvg, svg_theme)) {
    //     panel->setBackground(newSvg);
    // }
    svg_theme::applyChildrenTheme(this, svg_theme);

    if (context == this) {
        set_extender_theme(LeftRight::Left, name);
        set_extender_theme(LeftRight::Right, name);
    }
}

std::string ChemModuleWidget::getThemeName() {
    return module
        ? getChemModule()->getThemeName()
        : theme::get_default_theme();
}

std::string ChemModuleWidget::getActualThemeName() {
    return module
        ? getChemModule()->get_actual_theme_name()
        : get_default_theme();
}

std::shared_ptr<SvgTheme> ChemModuleWidget::getSvgTheme() {
    return getThemeCache().getTheme(getActualThemeName());
}

void ChemModuleWidget::step()
{
    ModuleWidget::step();
    setPartnerPanelBorder<ChemModuleWidget>(this);
    if (module) {
        auto chem = getChemModule();
        auto theme = getThemeName();
        if (theme::is_special_theme(theme)) {
            auto actual = theme::get_actual_theme(theme);
            if (actual != chem->get_actual_theme_name()) {
                setThemeName(theme, nullptr);
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
            reloadThemeCache();
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

// Hide or show screw widgets in all loaded modules.
// void set_screws(Widget* widget, bool screwed) {
//     for (Widget* child : widget->children) {
//         set_screws(child, screwed);
//     }

//     auto screw = dynamic_cast<SvgScrew*>(widget);
//     if (screw) {
//         screw->setVisible(!screwed);
//         screw->fb->setDirty(true);
//     }
// }

void ChemModuleWidget::appendContextMenu(Menu *menu)
{
    if (!module) return;
    initThemeCache();

    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel<HamburgerTitle>("Themes"));
    add_theme_items(menu, this, this);

    menu->addChild(new MenuSeparator);
    bool screws = style::show_screws();
    menu->addChild(createMenuItem(screws ? "Banish screws" : "Show screws", "",
        [=](){
            auto kv = get_plugin_kv_store();
            if (kv && kv->load()) {
                auto key = "show-screws";
                bool screws = !KVStore::bool_value(kv->lookup(key), true);
                if (screws) {
                    add_screws();
                } else {
                    remove_screws();
                }
                kv->update(key, KVStore::bool_text(screws));
                kv->save();
            }
        }));


    menu->addChild(createMenuItem("Hot-reload themes", "F5", [=]() {
        reloadThemeCache();
        setThemeName(getThemeName(), this);
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
        [this]() {
            hints = !hints;
            layout_help::enable_children(this, hints);
        }));
#endif
}

// ---------------------------------------------------------------------------
// utils
//

NVGcolor ColorFromTheme(std::shared_ptr<SvgTheme> theme, const char * color_name, const NVGcolor& fallback)
{
    assert(color_name);
    if (!theme) return fallback;
    PackedColor co{colors::NoColor};
    if (theme->getFillColor(co, color_name, true)) {
        return fromPacked(co);
    }
    return fallback;
}

NVGcolor ColorFromTheme(std::shared_ptr<SvgTheme> theme, const char * color_name, StockColor fallback)
{
    assert(color_name);
    if (!theme) return GetStockColor(fallback);
    PackedColor co{colors::NoColor};
    if (theme->getFillColor(co, color_name, true)) {
        return fromPacked(co);
    }
    return GetStockColor(fallback);
}

namespace pachde {

bool host_connected(IChemHost* chem_host)
{
    if (!chem_host) return false;
    auto conn = chem_host->host_connection(ChemDevice::Haken);
    return conn && conn->identified();
}

}