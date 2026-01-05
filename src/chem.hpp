#pragma once
#include <rack.hpp>
#include "my-plugin.hpp"
#include "chem-core.hpp"
#include "services/colors.hpp"
#include "services/theme.hpp"
#include "services/svg-theme.hpp"
#include "widgets/themed-widgets.hpp"
#include "widgets/panel-border.hpp"

using namespace rack;
using namespace svg_theme;
using namespace pachde;
using namespace widgetry;
using namespace theme;

#define DEFAULT_THEME "Dark"
struct ChemModuleWidget;

struct ChemModule : Module, IThemeHolder
{
    private: std::string actual_theme;
    public:
    std::string theme_setting{theme_name::PreferDark};
    bool seek_host{false};
    IChemHost* chem_host{nullptr};
    ChemModuleWidget* chem_ui{nullptr};

    virtual IChemHost* get_host() { return chem_host; };

    void set_chem_host(IChemHost* host) { chem_host = host; }
    void set_chem_ui(ChemModuleWidget* ui) { chem_ui = ui; };

    void setThemeName(const std::string& name, void *) override;
    std::string getThemeName() override;
    std::string get_actual_theme_name();

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;

    IChemHost* find_expander_host();

    void process(const ProcessArgs& args) override;
};


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

    SvgCache module_svgs;
    PartnerPanelBorder * panelBorder {nullptr};

    virtual std::string panelFilename() = 0;
    ChemModule* getChemModule() { return static_cast<ChemModule*>(module); }

    // IThemeHolder used by the context menu helper
    void setThemeName(const std::string& name, void *context) override;
    std::string getThemeName() override;
    std::string getActualThemeName();
    std::shared_ptr<SvgTheme> getSvgTheme();

    virtual void createScrews() {}
    virtual void hot_reload();
    void set_extender_theme(LeftRight which, const std::string& name);

    void onHoverKey(const HoverKeyEvent& e) override;
    void step() override;

    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

NVGcolor ColorFromTheme(std::shared_ptr<SvgTheme> theme, const char * color_name, StockColor fallback);
NVGcolor ColorFromTheme(std::shared_ptr<SvgTheme> theme, const char * color_name, const NVGcolor& fallback);

// helpers/Impls
namespace pachde {
bool host_connected(IChemHost* chem_host);
}

// bind_host use from ModuleWidget::step()
template<typename TClientModule>
void bind_host(TClientModule* client_module)
{
    if (!client_module) return;
    if (client_module->seek_host && !client_module->get_host()) {
        auto host = client_module->find_expander_host();
        if (host) {
            host->register_chem_client(client_module);
        }
        client_module->seek_host = false;
    }
}

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

template <typename TModule>
void onConnectHostModuleImpl_no_ui(TModule* self, IChemHost* host)
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
}

template <typename TModuleWidget>
void onConnectionChangeUiImpl(TModuleWidget* self, ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    self->haken_device_label->set_text(connection ? connection->info.friendly(NameFormat::Short) : "[not connected]");
    self->haken_device_label->describe(connection ? connection->info.friendly(NameFormat::Long) : "[not connected]");
}
