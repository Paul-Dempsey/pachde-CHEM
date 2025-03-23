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
    IChemHost* chem_host{nullptr};
    ChemModuleWidget* chem_ui{nullptr};

    virtual IChemHost* get_host() { return chem_host; };

    void set_chem_host(IChemHost* host) { chem_host = host; }
    void set_chem_ui(ChemModuleWidget* ui) { chem_ui = ui; };

    void setThemeName(const std::string& name, void *) override;
    std::string getThemeName() override;

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;

    IChemHost* find_expander_host();
};

template<typename TClientModule>
void find_and_bind_host(TClientModule* self, const ::rack::engine::Module::ProcessArgs& args)
{
    if (!self->get_host() && ((args.frame + self->id) % 80) == 0) {
        auto host = self->find_expander_host();
        if (host) {
            host->register_chem_client(self);
        }
    }    
}

enum class LeftRight: uint8_t { Left, Right };
inline bool left(LeftRight lr) { return LeftRight::Left == lr; }
inline bool right(LeftRight lr) { return LeftRight::Right == lr; }

struct ChemModuleWidget : ModuleWidget, IThemeHolder
{
    using Base = ModuleWidget;

#ifdef LAYOUT_HELP
    bool showGrid{false};
    bool hints{false};
#endif

    virtual std::string panelFilename() = 0;

    PartnerPanelBorder * panelBorder {nullptr};

    ChemModule* getChemModule() { return static_cast<ChemModule*>(module); }

    // IThemeHolder used by the context menu helper
    std::string getThemeName() override
    {
        return module ? getChemModule()->getThemeName() : ::rack::settings::preferDarkPanels ? "Dark": "Light";
    }

    void set_extender_theme(LeftRight which, const std::string& name);

    void setThemeName(const std::string& name, void *context) override;

    void onHoverKey(const HoverKeyEvent& e) override;
    void step() override;

    #ifdef LAYOUT_HELP
    void drawCrossLine(NVGcontext *vg, float x, float y);
    #endif
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

NVGcolor ColorFromTheme(const std::string& theme, const char * color_name, const NVGcolor& fallback);
NVGcolor ColorFromTheme(const std::string& theme, const char * color_name, StockColor fallback);

template <typename TModule>
void onConnectHostModuleImpl(TModule* self, IChemHost* host)
{
    self->chem_host = host;
    if (host) {
        auto conn = host->host_connection(ChemDevice::Haken);
        if (conn) {
            self->device_claim = conn->info.claim();
        }
    } else {
        self->device_claim = "";
    }
    if (self->chem_ui) self->ui()->onConnectHost(host);
}

template <typename TModuleWidget>
void onConnectionChangeUiImpl(TModuleWidget* self, ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    self->haken_device_label->text(connection ? connection->info.friendly(TextFormatLength::Short) : "[not connected]");
    self->haken_device_label->describe(connection ? connection->info.friendly(TextFormatLength::Long) : "[not connected]");
}
