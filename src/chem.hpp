#pragma once
#include <rack.hpp>
#include "plugin.hpp"
#include "chem-core.hpp"
#include "services/colors.hpp"
#include "widgets/themed-widgets.hpp"

using namespace rack;
using namespace svg_theme;
using namespace pachde;

#define DEFAULT_THEME "Dark"
struct ChemModuleWidget;

struct ChemModule : Module, IThemeHolder
{
    std::string theme_name;
    bool follow_rack{false};
    ChemModuleWidget* chem_ui{nullptr};

    virtual IChemHost* get_host() = 0;
    void set_chem_ui(ChemModuleWidget* ui) { chem_ui = ui; };

    void setThemeName(const std::string& name, void *) override;
    std::string getThemeName() override;

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;
};

struct ChemModuleWidget : ModuleWidget, IThemeHolder
{
    using Base = ModuleWidget;

    bool showGrid{false};
    bool hints{false};

    virtual std::string panelFilename() = 0;

    PartnerPanelBorder * panelBorder = nullptr;

    ChemModule* getChemModule() { return static_cast<ChemModule*>(module); }

    // IThemeHolder used by the context menu helper
    std::string getThemeName() override
    {
        return module ? getChemModule()->getThemeName() : ::rack::settings::preferDarkPanels ? "Dark": "Light";
    }

    void set_extender_theme(bool left, const std::string& name);

    void setThemeName(const std::string& name, void *context) override;

    void onHoverKey(const HoverKeyEvent& e) override;
    void step() override;
    void drawCrossLine(NVGcontext *vg, float x, float y);
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

NVGcolor ColorFromTheme(const std::string& theme, const char * color_name, const NVGcolor& fallback);
NVGcolor ColorFromTheme(const std::string& theme, const char * color_name, StockColor fallback);
