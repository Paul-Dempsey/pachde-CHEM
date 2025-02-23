#include "Macro.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;

using namespace svg_theme;
using namespace pachde;

// -- Macro UI -----------------------------------

bool MacroUi::connected()
{
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

MacroUi::~MacroUi()
{
    if (my_module) {
        my_module->ui = nullptr;
    }
}

using MM = MacroModule;
constexpr const float PANEL_WIDTH = 120.f;
constexpr const float CENTER = PANEL_WIDTH * .5f;
constexpr const float KNOB_CX = 25.f;
constexpr const float KNOB_CY = 35.f;
constexpr const float KNOB_TOP = KNOB_CY - 16.f;
constexpr const float MACRO_DY = 40.f;
constexpr const float LABEL_LEFT = 45.f;
constexpr const float LABEL_TOP = KNOB_CY;

constexpr const float INPUT_DX = PANEL_WIDTH*.25f;
constexpr const float INPUT_LEFT = CENTER - (1.5f*INPUT_DX);

constexpr const float ATT_CONNECTOR_X = INPUT_LEFT + 10.f;
constexpr const float ATT_CONNECTOR_Y = S::PORT_TOP + S::PORT_DY*.5f + 10.f;

enum M { M1, M2, M3, M4, M5, M6, K_ATTENUVERTER };

MacroUi::MacroUi(MacroModule *module) :
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

    // knobs
    x = KNOB_CX;
    y = KNOB_CY;
    addChild(knobs[M1] = createChemKnob<BasicKnob>(Vec(x, y), module, MacroModule::P_M1, theme_engine, theme)); y += MACRO_DY;
    addChild(knobs[M2] = createChemKnob<BasicKnob>(Vec(x, y), module, MacroModule::P_M2, theme_engine, theme)); y += MACRO_DY;
    addChild(knobs[M3] = createChemKnob<BasicKnob>(Vec(x, y), module, MacroModule::P_M3, theme_engine, theme)); y += MACRO_DY;
    addChild(knobs[M4] = createChemKnob<BasicKnob>(Vec(x, y), module, MacroModule::P_M4, theme_engine, theme)); y += MACRO_DY;
    addChild(knobs[M5] = createChemKnob<BasicKnob>(Vec(x, y), module, MacroModule::P_M5, theme_engine, theme)); y += MACRO_DY;
    addChild(knobs[M6] = createChemKnob<BasicKnob>(Vec(x, y), module, MacroModule::P_M6, theme_engine, theme)); 
    
    addChild(preset_label = createStaticTextLabel<TipLabel>(
        Vec(box.size.x *.5f, S::PORT_SECTION - 18.f), box.size.x, browsing ? "—preset—" : "", theme_engine, theme,
        LabelStyle{"curpreset", TextAlignment::Center, 12.f, false}));

    // knob labels
    x = LABEL_LEFT;
    y = LABEL_TOP;
    LabelStyle control_style{S::control_label};
    control_style.align = TextAlignment::Left;

    addChild(m1_label = createStaticTextLabel(Vec(x,y), 75.f, "i",   theme_engine, theme, control_style)); y += MACRO_DY;
    addChild(m2_label = createStaticTextLabel(Vec(x,y), 75.f, "ii",  theme_engine, theme, control_style)); y += MACRO_DY;
    addChild(m3_label = createStaticTextLabel(Vec(x,y), 75.f, "iii", theme_engine, theme, control_style)); y += MACRO_DY;
    addChild(m4_label = createStaticTextLabel(Vec(x,y), 75.f, "iv",  theme_engine, theme, control_style)); y += MACRO_DY;
    addChild(m5_label = createStaticTextLabel(Vec(x,y), 75.f, "v",   theme_engine, theme, control_style)); y += MACRO_DY;
    addChild(m6_label = createStaticTextLabel(Vec(x,y), 75.f, "vi",  theme_engine, theme, control_style));

    // knob pedal annotations
    y = KNOB_TOP + 2.f;
    addChild(m1_ped_label = createStaticTextLabel(Vec(x,y),40.f, "", theme_engine, theme, S::pedal_label)); y += MACRO_DY;
    addChild(m2_ped_label = createStaticTextLabel(Vec(x,y),40.f, "", theme_engine, theme, S::pedal_label)); y += MACRO_DY;
    addChild(m3_ped_label = createStaticTextLabel(Vec(x,y),40.f, "", theme_engine, theme, S::pedal_label)); y += MACRO_DY;
    addChild(m4_ped_label = createStaticTextLabel(Vec(x,y),40.f, "", theme_engine, theme, S::pedal_label)); y += MACRO_DY;
    addChild(m5_ped_label = createStaticTextLabel(Vec(x,y),40.f, "", theme_engine, theme, S::pedal_label)); y += MACRO_DY;
    addChild(m6_ped_label = createStaticTextLabel(Vec(x,y),40.f, "", theme_engine, theme, S::pedal_label));

    // inputs
    const NVGcolor co_port = PORT_CORN;
    x = INPUT_LEFT;
    y = S::PORT_TOP + S::PORT_DY*.5f;
    addChild(knobs[K_ATTENUVERTER] = createChemKnob<TrimPot>(Vec(x, y), module, MacroModule::P_ATTENUVERT, theme_engine, theme));
    x += INPUT_DX;
    y = S::PORT_TOP;
    for (int i = 0; i <= M6; ++i) {
        addChild(Center(createThemedColorInput(Vec(x, y), my_module, i, co_port, theme_engine, theme)));
        addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_ATT_DX, y - S::PORT_ATT_DY), my_module, i));
        addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 35.f, format_string("M%d", 1 + i), theme_engine, theme, S::in_port_label));
        x += INPUT_DX;
        if (i == 2) {
            y += S::PORT_DY;
            x = INPUT_LEFT + INPUT_DX;
        }
    }

    // footer

    addChild(warning_label = createStaticTextLabel<TipLabel>(
        Vec(28.f, box.size.y - 22.f), box.size.x, "", theme_engine, theme, S::warning_label));
    warning_label->describe("[warning/status]");
    warning_label->glowing(true);

    addChild(haken_device_label = createStaticTextLabel<TipLabel>(
        Vec(28.f, box.size.y - 13.f), 200.f, S::NotConnected, theme_engine, theme, style::haken_label));

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

    if (!my_module || my_module->glow_knobs) {
        glowing_knobs(true);
    }

    if (my_module) {
        my_module->ui = this;
        my_module->set_chem_ui(this);
        onConnectHost(my_module->chem_host);
    }
}

void MacroUi::glowing_knobs(bool glow) {
    for (int i = 0; i < MacroModule::NUM_KNOBS; ++i) {
        knobs[i]->glowing(glow);
    }
}

void MacroUi::setThemeName(const std::string& name, void * context)
{
//    blip->set_light_color(ColorFromTheme(getThemeName(), "warning", nvgRGB(0xe7, 0xe7, 0x45)));
    Base::setThemeName(name, context);
}

void MacroUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        //onPresetChange();
    } else {
        haken_device_label->text(S::NotConnected);
    }
}

void MacroUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    haken_device_label->text(connection ? connection->info.friendly(TextFormatLength::Short) : S::NotConnected);
    haken_device_label->describe(connection ? connection->info.friendly(TextFormatLength::Long) : S::NotConnected);
}

void MacroUi::onPresetChange()
{
    if (my_module) {
        m1_label->text(my_module->macro_names.macro[M1]);
        m2_label->text(my_module->macro_names.macro[M2]);
        m3_label->text(my_module->macro_names.macro[M3]);
        m4_label->text(my_module->macro_names.macro[M4]);
        m5_label->text(my_module->macro_names.macro[M5]);
        m6_label->text(my_module->macro_names.macro[M6]);
        auto preset = chem_host ? chem_host->host_preset() : nullptr;
        if (preset) {
            preset_label->text(preset->name);
            preset_label->describe(preset->text);
        }
    }
}

void set_pedal_text(StaticTextLabel* label, uint8_t item_cc, uint8_t a1, uint8_t a2)
{
    if (item_cc == a1 || item_cc == a2) {
        if (a1 == a2) {
            label->text("p1,p2");
        } else if (item_cc == a1) {
            label->text("p1");
        } else {
            label->text("p2");
        }
    } else {
        label->text("");
    }

}

void MacroUi::step()
{
    Base::step();
    if (!my_module) return;
    knobs[K_ATTENUVERTER]->enable(my_module->attenuator_target >= 0);

    if (!chem_host) return;

    auto em = chem_host->host_matrix();
    auto a1 = em->get_jack_1_assign();
    auto a2 = em->get_jack_2_assign();
    set_pedal_text(m1_ped_label, Haken::ccI, a1, a2);
    set_pedal_text(m2_ped_label, Haken::ccII, a1, a2);
    set_pedal_text(m3_ped_label, Haken::ccIII, a1, a2);
    set_pedal_text(m4_ped_label, Haken::ccIV, a1, a2);
    set_pedal_text(m5_ped_label, Haken::ccV, a1, a2);
    set_pedal_text(m6_ped_label, Haken::ccVI, a1, a2);
}

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
