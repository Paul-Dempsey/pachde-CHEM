#include "Preset.hpp"
#include "../../services/colors.hpp"

PresetModuleWidget::PresetModuleWidget(PresetModule *module) :
    my_module(module)
{
    setModule(module);

    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);

    this->panelBorder = new PartnerPanelBorder();
    replacePanelBorder(panel, this->panelBorder);
    setPanel(panel);

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
