#include "../plugin.hpp"
#include "../chem.hpp"
#include "../chem-core.hpp"
#include "../services/colors.hpp"
#include "../widgets/themed_widgets.hpp"
using namespace pachde;

struct PresetModule : ChemModule, IChemClient
{
    std::string core_claim;

    // IChemClient
    rack::engine::Module* client_module() override { return this; }

    void releaseHost() override {}
    void onPresetChange() override
    {
    }
    void onConnectionChange() override
    {
    }

    void dataFromJson(json_t* root) override {
        ChemModule::dataFromJson(root);
        
    }

    json_t* dataToJson() override {
        json_t* root = ChemModule::dataToJson();
        return root;
    }
};


struct PresetModuleWidget : ChemModuleWidget
{
    PresetModule *my_module = nullptr;

    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/CHEM-preset.svg"); }

    PresetModuleWidget(PresetModule *module)
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

Model *modelPreset = createModel<PresetModule, PresetModuleWidget>("chem-preset");
