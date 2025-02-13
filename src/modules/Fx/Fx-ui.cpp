#include "Fx.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;
namespace fs = ghc::filesystem;

// -- Fx UI -----------------------------------

FxUi::~FxUi()
{
}

enum K { K_R1, K_R2, K_R3, K_R4, K_R5, K_R6, K_MIX, K_EFFECT };

FxUi::FxUi(FxModule *module) :
    my_module(module)
{
    setModule(module);
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);
    
    float x, y;
    bool browsing = !module;

    if (browsing) { addChild(createWidgetCentered<Logo>(Vec(82.5, 70.f))); }

    addChild(selector = createThemedParam<SelectorWidget>(Vec(12.f, 38.f), my_module, FxModule::P_EFFECT, theme_engine, theme));
    addChild(effect_label = createStaticTextLabel<TipLabel>(Vec(34.f, 28.f), 100.f, "Short reverb", theme_engine, theme, LabelStyle{"ctl-label", TextAlignment::Left, 16.f, true}));
    
    // knobs with labels
    const float CENTER = 82.5f;
    const float DY_KNOB = 60.f;
    const float KNOB_DX = 32.f;
    const float KNOB_TOP = 114.f;
    const float LABEL_DY = 18.f;
    y = KNOB_TOP;
    x = CENTER - KNOB_DX;

    for (int i = 0; i < K_MIX; ++i) {
        knobs[i] = createChemKnob<BasicKnob>(Vec(x, y), my_module, i, theme_engine, theme);
        addChild(knobs[i]);

        r_labels[i] = createStaticTextLabel<TipLabel>(Vec(x, y + LABEL_DY), 80.f, format_string("R%d", 1+i), theme_engine, theme, S::control_label);
        addChild(r_labels[i]);

        if (i == 2) {
            x = CENTER + KNOB_DX;
            y = KNOB_TOP;
        } else {
            y += DY_KNOB;
        }
    }

    knobs[K_MIX] = createChemKnob<BlueKnob>(Vec(CENTER, 70.f), my_module, FxModule::P_MIX, theme_engine, theme);
    addChild(knobs[K_MIX]);

    if (!my_module || my_module->glow_knobs) {
        glowing_knobs(true);
    }

    // inputs
    // x = 14.f;
    const float PORT_LEFT = 62.75f;
    const float PORT_DX   = 38.75f;
    const float PORT_TOP  = 305.f;
    const float PORT_DY   = 36.f;
    x = PORT_LEFT;
    y = PORT_TOP;
    for (int i = 0; i <= K_R6; ++i) {
        addChild(Center(createThemedColorInput(Vec(x, y), my_module, i, PORT_CORN, theme_engine, theme)));
        addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 35.f, format_string("R%d", 1+i), theme_engine, theme, S::in_port_label));
        x += PORT_DX;
        if (i == 2) {
            y += PORT_DY;
            x = PORT_LEFT;
        }
    }
    addChild(Center(createThemedColorInput(Vec(23.f, y), my_module, FxModule::IN_MIX, PORT_CORN, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(23.f, y + S::PORT_LABEL_DY), 35.f, "MIX", theme_engine, theme, S::in_port_label));

    // footer

    addChild(warn = createStaticTextLabel<TipLabel>(
        Vec(28.f, box.size.y - 22.f), box.size.x, browsing ?"[warning/status]":"", theme_engine, theme, S::warning_label));
    warn->describe("[warning/status]");
    warn->glowing(true);

    addChild(haken = createStaticTextLabel<TipLabel>(
        Vec(28.f, box.size.y - 13.f), 200.f, S::NotConnected, theme_engine, theme, S::haken_label));

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y - S::U1), theme_engine, theme, "Core link");

    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ui::Menu* menu = createMenu();
            menu->addChild(createMenuLabel("— Link to Core Module —"));
            auto broker = ModuleBroker::get();
            broker->addHostPickerMenu(menu, my_module);
        });
    }
    addChild(link_button);

    // Browsing UI

    if (browsing) {
    }

    // init

    if (my_module) {
        my_module->ui = this;
        my_module->set_chem_ui(this);
        onConnectHost(my_module->chem_host);
    }
}

void FxUi::glowing_knobs(bool glow) {
    for (int i = 0; i < FxModule::NUM_KNOBS; ++i) {
        knobs[i]->glowing(glow);
    }
}

void FxUi::setThemeName(const std::string& name, void * context)
{
//    blip->set_light_color(ColorFromTheme(getThemeName(), "warning", nvgRGB(0xe7, 0xe7, 0x45)));
    Base::setThemeName(name, context);
}

void FxUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        //onPresetChange();
    } else {
        haken->text(S::NotConnected);
    }
}

void FxUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    haken->text(connection ? connection->info.friendly(TextFormatLength::Short) : S::NotConnected);
    haken->describe(connection ? connection->info.friendly(TextFormatLength::Long) : S::NotConnected);
}

void FxUi::onPresetChange()
{
}

bool FxUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

// void FxUi::step()
// {
//     Base::step();
    
// }

void FxUi::draw(const DrawArgs& args)
{
    Base::draw(args);
}

void FxUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
    menu->addChild(new MenuSeparator);
    menu->addChild(createCheckMenuItem("Glowing knobs", "", 
        [this](){ return my_module->glow_knobs; },
        [this](){
            my_module->glow_knobs = !my_module->glow_knobs; 
            glowing_knobs(my_module->glow_knobs);
        }
    ));
    Base::appendContextMenu(menu);
}
