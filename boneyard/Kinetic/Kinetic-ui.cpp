#include "Kinetic.hpp"
#include "../../services/colors.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;
using KM = KineticModule;

// -- Kinetic UI -----------------------------------

constexpr const float ONEU = 15.f;
constexpr const float CENTER = 7.5f;

bool KineticUi::connected()
{
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

KineticUi::KineticUi(KineticModule *module) :
    my_module(module)
{
    setModule(module);
}

void KineticUi::create_ui()
{
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);
    float x, y;
    x = CENTER;
    const float MIDDLE = 160.f;
    y = MIDDLE - 64.f - 4.f;

    auto co_port = PORT_CORN;
    y = S::PORT_TOP;
    addChild(mod_knob = createChemKnob<TrimPot>(Vec(x, y), module, KM::P_MOD_AMOUNT, theme_engine, theme));

    y += S::PORT_DY;

    // footer
    addChild(warning_label = createStaticTextLabel<TipLabel>(
         Vec(S::CORE_LINK_TEXT, box.size.y - 22.f), box.size.x, "", theme_engine, theme, S::warning_label));
    warning_label->describe("[warning/status]");
    warning_label->glowing(true);

    addChild(haken_device_label = createStaticTextLabel<TipLabel>(
        Vec(S::CORE_LINK_TEXT, box.size.y - S::CORE_LINK_TEXT_DY), 200.f, S::NotConnected, theme_engine, theme, S::haken_label));

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

void KineticUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        //onPresetChange();
    } else {
        //haken_device_label->text(S::NotConnected);
    }
}

void KineticUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    //haken_device_label->text(connection ? connection->info.friendly(TextFormatLength::Short) : S::NotConnected);
    //haken_device_label->describe(connection ? connection->info.friendly(TextFormatLength::Long) : S::NotConnected);
}

void KineticUi::step()
{
    Base::step();
    if (!my_module) return;
    mod_knob->enable(my_module->getInput(0).isConnected());
    if (my_module->getInput(KM::IN_MOD).isConnected()) {
        slider->set_modulation(my_module->modulation.get_port(KM::IN_MOD).modulated());
    } else {
        slider->unmodulate();
    }
}

void KineticUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
    menu->addChild(new MenuSeparator);
    menu->addChild(createCheckMenuItem("Glowing knobs", "", 
        [this](){ return my_module->glow_knobs; },
        [this](){
            my_module->glow_knobs = !my_module->glow_knobs; 
            mod_knob->glowing(my_module->glow_knobs);
        }
    ));
    Base::appendContextMenu(menu);
}
