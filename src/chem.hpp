#pragma once
#include <rack.hpp>
#include "plugin.hpp"
#include "services/colors.hpp"
#include "widgets/themed-widgets.hpp"

using namespace rack;
using namespace svg_theme;
using namespace pachde;

#define DEFAULT_THEME "Dark"

struct ChemModule : Module, IThemeHolder {
    std::string theme_name;

    void setThemeName(const std::string& name) override { this->theme_name = name; }
    std::string getThemeName() override { return theme_name; }

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;
};

struct ChemModuleWidget : ModuleWidget, IThemeHolder
{
    bool showGrid = false;
    bool hints = false;

    virtual std::string panelFilename() = 0;

    PartnerPanelBorder * panelBorder = nullptr;

    ChemModule* getChemModule() { return static_cast<ChemModule*>(module); }

    bool isDefaultTheme() {
        if (!module) return true;
        auto theme = getChemModule()->getThemeName();
        return theme.empty() || 0 == theme.compare(DEFAULT_THEME);
    }

    // IThemeHolder used by the context menu helper
    std::string getThemeName() override
    {
        return isDefaultTheme() ? DEFAULT_THEME: getChemModule()->getThemeName(); 
    }

    void setThemeName(const std::string& name) override;

    void step() override;
    void drawCrossLine(NVGcontext *vg, float x, float y);
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;

};

NVGcolor ColorFromTheme(const std::string& theme, const char * color_name, const NVGcolor& fallback);
NVGcolor ColorFromTheme(const std::string& theme, const char * color_name, StockColor fallback);
