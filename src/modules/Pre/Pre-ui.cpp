#include "Pre.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/logo-widget.hpp"

using namespace svg_theme;
using namespace pachde;
namespace fs = ghc::filesystem;

// -- Pre UI -----------------------------------

constexpr const float ONEU = 15.f;
constexpr const float HALFU = 7.5f;
constexpr const ssize_t SSIZE_0 = 0;
constexpr const float CENTER = 52.5f;

const char * const NotConnected = "[not connected]";

bool PreUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

PreUi::~PreUi()
{
}

enum K { 
    K_PRE_LEVEL, 
    K_MIX, 
    K_THRESHOLD, 
    K_ATTACK, 
    K_RATIO
};

PreUi::PreUi(PreModule *module) :
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
        addChild(createWidgetCentered<Logo>(Vec(CENTER, 380.f)));
    }

    addChild(effect_label = createStaticTextLabel<TipLabel>(Vec(CENTER, 65.f), 100.f, "Compressor", theme_engine, theme, LabelStyle{"ctl-label", TextAlignment::Center, 18.f, true}));

    // knobs
    x = CENTER;
    addChild(knobs[K_MIX]       = createChemKnob<BlueKnob>(Vec(x, 108.f), module, PreModule::P_MIX, theme_engine, theme));
    addChild(knobs[K_PRE_LEVEL] = createChemKnob<YellowKnob>(Vec(34.f, 42.f), module, PreModule::P_PRE_LEVEL, theme_engine, theme));
    const float PARAM_TOP = 148.f;
    const float PARAM_DY = 57.f;
    const float label_offset = 18.f;
    LabelStyle knob_label_style ={"ctl-label", TextAlignment::Center, 14.f, false};

    y = PARAM_TOP;
    addChild(knobs[K_THRESHOLD] = createChemKnob<BasicKnob>(Vec(x, y), module, PreModule::P_THRESHOLD, theme_engine, theme));
    addChild(top_knob_label= createStaticTextLabel<StaticTextLabel>(Vec(x,y + label_offset), 100.f, "Threshhold", theme_engine, theme, knob_label_style));

    y += PARAM_DY;
    addChild(knobs[K_ATTACK] = createChemKnob<BasicKnob>(Vec(x, y), module, PreModule::P_ATTACK, theme_engine, theme));
    addChild(mid_knob_label= createStaticTextLabel<StaticTextLabel>(Vec(x,y + label_offset), 100.f, "Attack", theme_engine, theme, knob_label_style));

    y += PARAM_DY;
    addChild(knobs[K_RATIO] = createChemKnob<BasicKnob>(Vec(x, y), module, PreModule::P_RATIO, theme_engine, theme));
    addChild(bot_knob_label= createStaticTextLabel<StaticTextLabel>(Vec(x,y + label_offset), 100.f, "Ratio", theme_engine, theme, knob_label_style));
    
    if (!my_module || my_module->glow_knobs) {
        glowing_knobs(true);
    }

    // inputs
    addChild(Center(createThemedColorInput(Vec(34.f, 314.f), my_module, PreModule::IN_PRE_LEVEL, PORT_CORN, theme_engine, theme)));
    const float PORT_Y = 347.f;
    const float PORT_DX = 26.f;
    x = 14.f;
    addChild(Center(createThemedColorInput(Vec(x, PORT_Y), my_module, PreModule::IN_MIX, PORT_CORN, theme_engine, theme))); x += PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, PORT_Y), my_module, PreModule::IN_THRESHOLD, PORT_CORN, theme_engine, theme))); x += PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, PORT_Y), my_module, PreModule::IN_ATTACK, PORT_CORN, theme_engine, theme))); x += PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, PORT_Y), my_module, PreModule::IN_RATIO, PORT_CORN, theme_engine, theme)));

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

void PreUi::glowing_knobs(bool glow) {
    for (int i = 0; i < PreModule::NUM_PARAMS; ++i) {
        knobs[i]->glowing(glow);
    }
}

void PreUi::setThemeName(const std::string& name)
{
//    blip->set_light_color(ColorFromTheme(getThemeName(), "warning", nvgRGB(0xe7, 0xe7, 0x45)));
    Base::setThemeName(name);
}

void PreUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        //onPresetChange();
    } else {
        haken_device_label->text(NotConnected);
    }
}

void PreUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    haken_device_label->text(connection ? connection->info.friendly(TextFormatLength::Short) : NotConnected);
    haken_device_label->describe(connection ? connection->info.friendly(TextFormatLength::Long) : NotConnected);
}

void PreUi::onPresetChange()
{
}

// void PreUi::step()
// {
//     Base::step();
    
// }

void PreUi::draw(const DrawArgs& args)
{
    Base::draw(args);
}

void PreUi::appendContextMenu(Menu *menu)
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
