#include "Sustain.hpp"
#include "../../services/colors.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;
using SM = SusModule;

// -- Sustain UI -----------------------------------

constexpr const float ONEU = 15.f;
constexpr const float HALFU = 7.5f;
constexpr const ssize_t SSIZE_0 = 0;
constexpr const float CENTER = 7.5f;

bool SusUi::connected()
{
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

SusUi::SusUi(SusModule *module) :
    my_module(module)
{
    setModule(module);
}

void SusUi::create_ui()
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
    addChild(Center(createThemedParamButton<DotParamButton>(Vec(x,y), my_module, SM::P_MAX, theme_engine, theme)));
    addChild(slider = createSlider<FillSlider>(Vec(x,MIDDLE), 128.f, my_module, SM::P_VALUE, theme_engine, theme));
    y = MIDDLE + 64.f + 4.f;
    addChild(Center(createThemedParamButton<DotParamButton>(Vec(x,y), my_module, SM::P_MIN, theme_engine, theme)));

    auto co_port = PORT_CORN;
    y = S::PORT_TOP;
    addChild(mod_knob = createChemKnob<TrimPot>(Vec(x, y), module, SM::P_MOD_AMOUNT, theme_engine, theme));

    y += S::PORT_DY;
    addChild(Center(createThemedColorInput(Vec(x , y), my_module, SM::IN_MOD, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 20, InputLabel(), theme_engine, theme, S::in_port_label));

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

void SusUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        //onPresetChange();
    } else {
        //haken_device_label->text(S::NotConnected);
    }
}

void SusUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    //haken_device_label->text(connection ? connection->info.friendly(TextFormatLength::Short) : S::NotConnected);
    //haken_device_label->describe(connection ? connection->info.friendly(TextFormatLength::Long) : S::NotConnected);
}

void SusUi::step()
{
    Base::step();
    if (!my_module) return;
    mod_knob->enable(my_module->getInput(0).isConnected());
    if (my_module->getInput(SM::IN_MOD).isConnected()) {
        slider->set_modulation(my_module->modulation.get_port(SM::IN_MOD).modulated());
    } else {
        slider->unmodulate();
    }
}

void SusUi::appendContextMenu(Menu *menu)
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
