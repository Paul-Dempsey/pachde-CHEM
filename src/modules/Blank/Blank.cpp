#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../widgets/symbol-set.hpp"
using namespace pachde;

struct BlankModule : ChemModule
{
    using Base = ChemModule;

    enum Params {
       NUM_PARAMS
    };
    enum Inputs {
       NUM_INPUTS
    };
    enum Outputs {
       NUM_OUTPUTS
    };
    enum Lights {
       NUM_LIGHTS
    };

    BlankModule()
    {
        config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    }

    json_t* dataToJson() override
    {
       auto root = Base::dataToJson();
       return root;
    }
    void dataFromJson(json_t* root) override
    {
        Base::dataFromJson(root);
    }

    IChemHost* get_host() override { return nullptr; }
};

constexpr const float PANEL_WIDTH = 105;
constexpr const float CENTER = PANEL_WIDTH*.5;

struct BlankModuleWidget : ChemModuleWidget
{
    SymbolProvider symbols;
    SymbolSetWidget* pedal_image[8];

    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-blank.svg"); }

    BlankModuleWidget(BlankModule *module)
    {
        setModule(module);
        initThemeEngine();
        auto theme = theme_engine.getTheme(getThemeName());
        auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
        panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
        setPanel(panel);
    
        // Add standard rack screws
        // addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        // addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        symbols.add_source("res/symbols/pedal-none.svg");
        symbols.add_source("res/symbols/pedal-switch.svg");
        symbols.add_source("res/symbols/pedal-expression.svg");
        symbols.add_source("res/symbols/pedal-damper.svg");
        symbols.add_source("res/symbols/pedal-tristate.svg");
        symbols.add_source("res/symbols/pedal-cv.svg");
        symbols.add_source("res/symbols/pedal-pot.svg");
        symbols.add_source("res/symbols/pedal-other.svg");
        symbols.applyTheme(theme_engine, theme);

        float x = PANEL_WIDTH / 3.f;
        float dx = x;
        float y = 156.f;
        float dy = 48.f;
        for (int i = 0; i < 8; ++i) {
            auto p = new SymbolSetWidget(&symbols);
            p->box.pos = Vec(x, y);
            p->set_index(i);
            addChild(Center(p));
            if (i & 1) {
                x = dx;
                y += dy;
            } else {
                x += dx;
            }
        }

    }

    // Add options to your module's menu here
    //void appendContextMenu(Menu *menu) override
    //{
    //}
};

Model *modelBlank = createModel<BlankModule, BlankModuleWidget>("chem-blank");
