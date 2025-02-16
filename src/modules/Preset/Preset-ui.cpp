#include "Preset.hpp"
#include "../../services/colors.hpp"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;

PresetModuleWidget::PresetModuleWidget(PresetModule *module) :
    my_module(module)
{
    setModule(module);

    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);

    if (!module) {
        auto logo = new WatermarkLogo(1.8f);
        logo->box.pos = Vec(84.f, 180.f - logo->box.size.y*.6);
        addChild(logo);
    }
    if (module) {
        my_module->set_chem_ui(this);
    }

}

// void PresetModuleWidget::step()
// {
//     ChemModuleWidget::step();
// }

void PresetModuleWidget::draw(const DrawArgs& args)
{
    Base::draw(args);
}

void PresetModuleWidget::appendContextMenu(Menu *menu)
{
    Base::appendContextMenu(menu);
}
