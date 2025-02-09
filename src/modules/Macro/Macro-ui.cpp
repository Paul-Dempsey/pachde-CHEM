#include "Macro.hpp"
#include <ghc/filesystem.hpp>
#include "../../services/colors.hpp"
#include "../../services/open-file.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/logo-widget.hpp"

using namespace svg_theme;
using namespace pachde;
namespace fs = ghc::filesystem;

// -- Macro UI -----------------------------------

constexpr const float ONEU = 15.f;
constexpr const float HALFU = 7.5f;
constexpr const ssize_t SSIZE_0 = 0;

const char * const NotConnected = "[not connected]";

bool MacroUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

MacroUi::~MacroUi()
{
}

constexpr const float KNOB_CX = 25.f;
constexpr const float KNOB_CY = 48.f;
constexpr const float KNOB_TOP = KNOB_CY - 16.f;
constexpr const float MACRO_DY = 40.f;
constexpr const float LABEL_LEFT = 45.f;
constexpr const float LABEL_TOP = KNOB_CY;
constexpr const float INPUT_LEFT = 31.5f;
constexpr const float INPUT_TOP  = 305.5f;
constexpr const float INPUT_DX = 28.5;
constexpr const float INPUT_DY = 37.5;

enum M { M1, M2, M3, M4, M5, M6 };

MacroUi::MacroUi(MacroModule *module) :
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

    // knobs
    x = KNOB_CX;
    y = KNOB_CY;
    addChild(knobs[M1] = createChemKnob<BasicKnob>(Vec(x, y), module, MacroModule::P_M1, theme_engine, theme)); y += MACRO_DY;
    addChild(knobs[M2] = createChemKnob<BasicKnob>(Vec(x, y), module, MacroModule::P_M2, theme_engine, theme)); y += MACRO_DY;
    addChild(knobs[M3] = createChemKnob<BasicKnob>(Vec(x, y), module, MacroModule::P_M3, theme_engine, theme)); y += MACRO_DY;
    addChild(knobs[M4] = createChemKnob<BasicKnob>(Vec(x, y), module, MacroModule::P_M4, theme_engine, theme)); y += MACRO_DY;
    addChild(knobs[M5] = createChemKnob<BasicKnob>(Vec(x, y), module, MacroModule::P_M5, theme_engine, theme)); y += MACRO_DY;
    addChild(knobs[M6] = createChemKnob<BasicKnob>(Vec(x, y), module, MacroModule::P_M6, theme_engine, theme));
    
    if (!my_module || my_module->glow_knobs) {
        glowing_knobs(true);
    }

    // knob labels
    x = LABEL_LEFT;
    y = LABEL_TOP;
    LabelStyle style{"ctl-label", TextAlignment::Left, 14.f, true};
    addChild(m1_label = createStaticTextLabel(Vec(x,y), 75.f, "i", theme_engine, theme, style)); y += MACRO_DY;
    addChild(m2_label = createStaticTextLabel(Vec(x,y), 75.f, "ii", theme_engine, theme, style)); y += MACRO_DY;
    addChild(m3_label = createStaticTextLabel(Vec(x,y), 75.f, "iii", theme_engine, theme, style)); y += MACRO_DY;
    addChild(m4_label = createStaticTextLabel(Vec(x,y), 75.f, "iv", theme_engine, theme, style)); y += MACRO_DY;
    addChild(m5_label = createStaticTextLabel(Vec(x,y), 75.f, "v", theme_engine, theme, style)); y += MACRO_DY;
    addChild(m6_label = createStaticTextLabel(Vec(x,y), 75.f, "vi", theme_engine, theme, style));

    // knob pedal annotations
    style.key = "macro-ped";
    style.height = 9.f;
    y = KNOB_TOP + 2.f;
    addChild(m1_ped_label = createStaticTextLabel(Vec(x,y),40.f, "", theme_engine, theme, style)); y += MACRO_DY;
    addChild(m2_ped_label = createStaticTextLabel(Vec(x,y),40.f, "", theme_engine, theme, style)); y += MACRO_DY;
    addChild(m3_ped_label = createStaticTextLabel(Vec(x,y),40.f, "", theme_engine, theme, style)); y += MACRO_DY;
    addChild(m4_ped_label = createStaticTextLabel(Vec(x,y),40.f, "", theme_engine, theme, style)); y += MACRO_DY;
    addChild(m5_ped_label = createStaticTextLabel(Vec(x,y),40.f, "", theme_engine, theme, style)); y += MACRO_DY;
    addChild(m6_ped_label = createStaticTextLabel(Vec(x,y),40.f, "", theme_engine, theme, style));

    // inputs

    x = INPUT_LEFT;
    y = INPUT_TOP;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, MacroModule::IN_M1, PORT_CORN, theme_engine, theme))); x += INPUT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, MacroModule::IN_M2, PORT_CORN, theme_engine, theme))); x += INPUT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, MacroModule::IN_M3, PORT_CORN, theme_engine, theme)));
    y += INPUT_DY;
    x = INPUT_LEFT;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, MacroModule::IN_M4, PORT_CORN, theme_engine, theme))); x += INPUT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, MacroModule::IN_M5, PORT_CORN, theme_engine, theme))); x += INPUT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, MacroModule::IN_M6, PORT_CORN, theme_engine, theme)));

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
        m1_label->text("Size");
        m2_label->text("OctBelow");
        m3_label->text("DelayAmt");
        m4_label->text("DblLevel");
        m5_label->text("DelayTime");
        m6_label->text("Motion");

        m1_ped_label->text("p1");
        m3_ped_label->text("p2");
    }

    // init

    if (my_module) {
        my_module->ui = this;
        onConnectHost(my_module->chem_host);
    }
}

void MacroUi::glowing_knobs(bool glow) {
    for (int i = 0; i < 6; ++i) {
        knobs[i]->glowing(glow);
    }
}

void MacroUi::setThemeName(const std::string& name)
{
//    blip->set_light_color(ColorFromTheme(getThemeName(), "warning", nvgRGB(0xe7, 0xe7, 0x45)));
    Base::setThemeName(name);
}

void MacroUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        //onPresetChange();
    } else {
        haken_device_label->text(NotConnected);
    }
}

void MacroUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    haken_device_label->text(connection ? connection->info.friendly(TextFormatLength::Short) : NotConnected);
    haken_device_label->describe(connection ? connection->info.friendly(TextFormatLength::Long) : NotConnected);
}

void MacroUi::onPresetChange()
{
}

// void MacroUi::step()
// {
//     Base::step();
    
// }

void MacroUi::draw(const DrawArgs& args)
{
    Base::draw(args);
}

void MacroUi::appendContextMenu(Menu *menu)
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
