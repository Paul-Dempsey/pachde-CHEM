#include "Sustain.hpp"
#include "services/colors.hpp"
#include "widgets/logo-widget.hpp"
#include "widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;
using SM = SusModule;

// -- Sustain UI -----------------------------------

constexpr const float ONEU = 15.f;
constexpr const float CENTER = 15.f;

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
    auto theme = getSvgTheme();
    auto panel = createThemedPanel(panelFilename(), &module_svgs);
    panelBorder = attachPartnerPanelBorder(panel);
    setPanel(panel);
    float x, y;
    x = CENTER;
    const float MIDDLE = 160.f;
    y = MIDDLE - 64.f - 4.f;
    addChild(Center(createThemedParamButton<DotParamButton>(Vec(x,y), &module_svgs, my_module, SM::P_MAX)));
    addChild(slider = Center(createSlider<FillSlider>(Vec(x,MIDDLE), 128.f, my_module, SM::P_VALUE)));
    y = MIDDLE + 64.f + 4.f;
    addChild(Center(createThemedParamButton<DotParamButton>(Vec(x,y), &module_svgs, my_module, SM::P_MIN)));

    auto co_port = PORT_CORN;
    y = S::PORT_TOP;
    addChild(mod_knob = createChemKnob<TrimPot>(Vec(x, y), &module_svgs, module, SM::P_MOD_AMOUNT));

    y += S::PORT_DY;
    addChild(Center(createThemedColorInput(Vec(x , y), &module_svgs, my_module, SM::IN_MOD, S::InputColorKey, co_port)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 20, InputLabel(), S::in_port_label));

    link_button = createThemedButton<LinkButton>(Vec(3.5f, box.size.y-ONEU), &module_svgs, "Core link");
    addChild(link = createIndicatorCentered(22.f,box.size.y-9.f, RampGray(G_50), "[connection]"));
    link->setFill(false);

    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ModuleBroker::get()->addHostPickerMenu(createMenu(), my_module);
        });
    }
    addChild(link_button);

    if (!module && S::show_browser_logo()) {
        auto logo = new OpaqueLogo(0.25f);
        logo->box.pos = Vec(CENTER, 60);
        addChild(Center(logo));
    }

    module_svgs.changeTheme(theme);

    // init

    if (my_module) {
        my_module->set_chem_ui(this);
        if (!chem_host) {
            onConnectHost(my_module->chem_host);
        }
    }
}

void SusUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    onConnectionChange(ChemDevice::Haken, host ? host->host_connection(ChemDevice::Haken) : nullptr);
}

void SusUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    if (connection) {
        link->describe(connection->info.friendly(NameFormat::Long));
        link->setFill(true);
        link->setColor(PORT_BLUE);
    } else {
        link->describe("[not connected]");
        link->setFill(false);
        link->setColor(RampGray(G_50));
    }
}

void SusUi::step()
{
    Base::step();
    if (!my_module) return;
    bind_host(my_module);
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
