#include "Preset.hpp"
#include "../../services/colors.hpp"
#include "../../widgets/logo-widget.hpp"

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

    if (!module) {
        addChild(createWidgetCentered<Logo>(Vec(box.size.x*.5f, box.size.y*.5)));
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
