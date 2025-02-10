#include "Fx.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/logo-widget.hpp"

using namespace svg_theme;
using namespace pachde;
namespace fs = ghc::filesystem;

// -- Fx UI -----------------------------------

constexpr const float ONEU = 15.f;
constexpr const float HALFU = 7.5f;
constexpr const ssize_t SSIZE_0 = 0;

const char * const NotConnected = "[not connected]";

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
    this->panelBorder = new PartnerPanelBorder();
    replacePanelBorder(panel, this->panelBorder);
    setPanel(panel);
    float x, y;
    bool browsing = !module;

    if (browsing) {
        addChild(createWidgetCentered<Logo>(Vec(box.size.x*.5f, box.size.y*.5)));
    }

    addChild(effect_label = createStaticTextLabel<TipLabel>(Vec(34.f, 28.f), 100.f, "Short reverb", theme_engine, theme, LabelStyle{"ctl-label", TextAlignment::Left, 18.f, true}));

    // knobs with labels
    const float center = 82.5f;
    const float dy_knob = 60.f;
    const float knob_x_offset = 32.f;
    const float knob_top = 114.f;
    const float label_offset = 18.f;
    y = knob_top;
    x = center - knob_x_offset;
    LabelStyle r_label_style {"ctl-label", TextAlignment::Center, 14.f, false};

    for (int i = 0; i < K_MIX; ++i) {
        knobs[i] = createChemKnob<BasicKnob>(Vec(x, y), my_module, i, theme_engine, theme);
        addChild(knobs[i]);

        r_labels[i] = createStaticTextLabel<TipLabel>(Vec(x, y + label_offset), 80.f, format_string("R%d", i), theme_engine, theme, r_label_style);
        addChild(r_labels[i]);

        if (i == 2) {
            x = center + knob_x_offset;
            y = knob_top;
        } else {
            y += dy_knob;
        }
    }

    knobs[K_MIX] = createChemKnob<BlueKnob>(Vec(center, 70.f), my_module, FxModule::P_MIX, theme_engine, theme);
    addChild(knobs[K_MIX]);

    if (!my_module || my_module->glow_knobs) {
        glowing_knobs(true);
    }

    // inputs
    // x = 14.f;
//    addChild(Center(createThemedColorInput(Vec(34.f, 314.f), my_module, PreModule::IN_PRE_LEVEL, PORT_CORN, theme_engine, theme)));
    const float PORT_LEFT = 62.75f;
    const float PORT_DX   = 38.75f;
    const float PORT_TOP  = 305.f;
    const float PORT_DY   = 36.f;

    x = PORT_LEFT;
    y = PORT_TOP;
    for (int i = 0; i <= K_R6; ++i) {
        addChild(Center(createThemedColorInput(Vec(x, y), my_module, i, PORT_CORN, theme_engine, theme)));
        x += PORT_DX;
        if (i == 2) {
            y += PORT_DY;
            x = PORT_LEFT;
        }
    }
    addChild(Center(createThemedColorInput(Vec(23.f, 322.f), my_module, FxModule::IN_MIX, PORT_CORN, theme_engine, theme)));

    // footer

    LabelStyle warn{"warning", TextAlignment::Left, 9.f};
    addChild(warning_label = createStaticTextLabel<TipLabel>(
        Vec(28.f, box.size.y - 22.f), box.size.x, browsing ?"[warning/status]":"", theme_engine, theme, warn));
    warning_label->describe("[warning/status]");
    warning_label->glowing(true);

    LabelStyle haken{"dytext", TextAlignment::Left, 10.f};
    addChild(haken_device_label = createStaticTextLabel<TipLabel>(
        Vec(28.f, box.size.y - 13.f), 200.f, NotConnected, theme_engine, theme, haken));

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y-ONEU), theme_engine, theme, "Core link");

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
        onConnectHost(my_module->chem_host);
    }
}

void FxUi::glowing_knobs(bool glow) {
    for (int i = 0; i < FxModule::NUM_KNOBS; ++i) {
        knobs[i]->glowing(glow);
    }
}

void FxUi::setThemeName(const std::string& name)
{
//    blip->set_light_color(ColorFromTheme(getThemeName(), "warning", nvgRGB(0xe7, 0xe7, 0x45)));
    Base::setThemeName(name);
}

void FxUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        //onPresetChange();
    } else {
        haken_device_label->text(NotConnected);
    }
}

void FxUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    haken_device_label->text(connection ? connection->info.friendly(TextFormatLength::Short) : NotConnected);
    haken_device_label->describe(connection ? connection->info.friendly(TextFormatLength::Long) : NotConnected);
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
