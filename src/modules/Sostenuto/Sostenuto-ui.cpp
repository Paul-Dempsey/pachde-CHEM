#include "Sostenuto.hpp"
#include "../../services/colors.hpp"
#include "../../widgets/uniform-style.hpp"
#include "../../widgets/slider-widget.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;
using SM = SostenutoModule;

// -- Sostenuto UI -----------------------------------

constexpr const float ONEU = 15.f;
constexpr const float HALFU = 7.5f;
constexpr const ssize_t SSIZE_0 = 0;
constexpr const float CENTER = 7.5f;

bool SostenutoUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

SostenutoUi::SostenutoUi(SostenutoModule *module) :
    my_module(module)
{
    setModule(module);
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);
    float x, y;
    x = CENTER;
    y = 200.f;
    addChild(Center(createThemedParamButton<DotParamButton>(Vec(x,y), my_module, SM::P_MAX_SOS, theme_engine, theme)));
    y = 340.f;
    addChild(Center(createThemedParamButton<DotParamButton>(Vec(x,y), my_module, SM::P_MIN_SOS, theme_engine, theme)));
    y = 207.f;
    addChild(createSlider<FillSlider>(Vec(x,y + 64.f), 128.f, my_module, SM::P_SOSTENUTO, theme_engine, theme));

    // footer
    // addChild(warning_label = createStaticTextLabel<TipLabel>(
    //     Vec(S::CORE_LINK_TEXT, box.size.y - 22.f), box.size.x, "", theme_engine, theme, S::warning_label));
    // warning_label->describe("[warning/status]");
    // warning_label->glowing(true);

    // addChild(haken_device_label = createStaticTextLabel<TipLabel>(
    //     Vec(S::CORE_LINK_TEXT, box.size.y - S::CORE_LINK_TEXT_DY), 200.f, S::NotConnected, theme_engine, theme, S::haken_label));

    link_button = createThemedButton<LinkButton>(Vec(1.5f, box.size.y-ONEU), theme_engine, theme, "Core link");

    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ui::Menu* menu = createMenu();
            menu->addChild(createMenuLabel("— Link to Core Module —"));
            auto broker = ModuleBroker::get();
            broker->addHostPickerMenu(menu, my_module);
        });
    }
    addChild(link_button);

    // init

    if (my_module) {
        my_module->set_chem_ui(this);
        onConnectHost(my_module->chem_host);
    }
}

void SostenutoUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        //onPresetChange();
    } else {
        //haken_device_label->text(S::NotConnected);
    }
}

void SostenutoUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    //haken_device_label->text(connection ? connection->info.friendly(TextFormatLength::Short) : S::NotConnected);
    //haken_device_label->describe(connection ? connection->info.friendly(TextFormatLength::Long) : S::NotConnected);
}

void SostenutoUi::step()
{
    Base::step();
    if (!my_module) return;
}

void SostenutoUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
    Base::appendContextMenu(menu);
}
