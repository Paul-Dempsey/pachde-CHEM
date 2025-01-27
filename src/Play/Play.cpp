#include "../plugin.hpp"
#include "../chem.hpp"
#include "../services/colors.hpp"
#include "../widgets/themed-widgets.hpp"
using namespace pachde;

struct PlayModule : ChemModule
{
    // void dataFromJson(json_t* root) override {
    //     ChemModule::dataFromJson(root);
    // }

    // json_t* dataToJson() override {
    //     json_t* root = ChemModule::dataToJson();
    //     return root;
    // }
};



struct PlayModuleWidget : ChemModuleWidget
{
    PlayModule *my_module = nullptr;

    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/CHEM-play.svg"); }

    PlayModuleWidget(PlayModule *module)
    {
        my_module = module;
        setModule(module);

        initThemeEngine();
        auto theme = theme_engine.getTheme(getThemeName());
        auto panel = createThemedPanel(panelFilename(), theme_engine, theme);

        this->panelBorder = new PartnerPanelBorder();
        replacePanelBorder(panel, this->panelBorder);
        setPanel(panel);

    }

    void step() override
    {
        ChemModuleWidget::step();
    }

    void appendContextMenu(Menu *menu) override
    {
        ChemModuleWidget::appendContextMenu(menu);
    }
};

Model *modelPlay = createModel<PlayModule, PlayModuleWidget>("chem-play");
