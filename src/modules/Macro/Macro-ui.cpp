#include "Macro.hpp"
#include "services/colors.hpp"
#include "em/em-hardware.h"
#include "widgets/click-region-widget.hpp"
#include "widgets/logo-widget.hpp"
#include "widgets/uniform-style.hpp"

namespace S = pachde::style;

using namespace svg_theme;
using namespace pachde;
using namespace widgetry;

// -- Macro UI -----------------------------------

bool MacroUi::connected()
{
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

using MM = MacroModule;
constexpr const float PANEL_WIDTH = 120.f;
constexpr const float CENTER = PANEL_WIDTH * .5f;
constexpr const float KNOB_CX = 25.f;
constexpr const float KNOB_CY = 35.f;
constexpr const float KNOB_TOP = KNOB_CY - 16.f;
constexpr const float MACRO_DY = 41.5f;
constexpr const float LABEL_LEFT = 45.f;
constexpr const float LABEL_TOP = KNOB_CY;

constexpr const float INPUT_DX = PANEL_WIDTH*.25f;
constexpr const float INPUT_LEFT = CENTER - (1.5f*INPUT_DX);

enum M { M1, M2, M3, M4, M5, M6, K_MODULATION };

MacroUi::MacroUi(MacroModule *module) :
    my_module(module)
{
    setModule(module);
    auto theme = getSvgTheme();
    auto panel = createThemedPanel(panelFilename(), &module_svgs);
    panelBorder = attachPartnerPanelBorder(panel, theme);
    setPanel(panel);
    float x, y;
    bool browsing = !module;

    // knobs
    x = KNOB_CX;
    y = KNOB_CY;
    addChild(knobs[M1] = createChemKnob<BasicKnob>(Vec(x, y), &module_svgs, module, MacroModule::P_M1, theme)); y += MACRO_DY;
    addChild(knobs[M2] = createChemKnob<BasicKnob>(Vec(x, y), &module_svgs, module, MacroModule::P_M2, theme)); y += MACRO_DY;
    addChild(knobs[M3] = createChemKnob<BasicKnob>(Vec(x, y), &module_svgs, module, MacroModule::P_M3, theme)); y += MACRO_DY;
    addChild(knobs[M4] = createChemKnob<BasicKnob>(Vec(x, y), &module_svgs, module, MacroModule::P_M4, theme)); y += MACRO_DY;
    addChild(knobs[M5] = createChemKnob<BasicKnob>(Vec(x, y), &module_svgs, module, MacroModule::P_M5, theme)); y += MACRO_DY;
    addChild(knobs[M6] = createChemKnob<BasicKnob>(Vec(x, y), &module_svgs, module, MacroModule::P_M6, theme));

    addChild(tracks[M1] = createTrackWidget(knobs[M1], theme));
    addChild(tracks[M2] = createTrackWidget(knobs[M2], theme));
    addChild(tracks[M3] = createTrackWidget(knobs[M3], theme));
    addChild(tracks[M4] = createTrackWidget(knobs[M4], theme));
    addChild(tracks[M5] = createTrackWidget(knobs[M5], theme));
    addChild(tracks[M6] = createTrackWidget(knobs[M6], theme));

    addChild(preset_label = createLabel<TipLabel>(
        Vec(box.size.x *.5f, S::PORT_SECTION - 18.f), box.size.x, browsing ? "—preset—" : "", theme,
        LabelStyle{"curpreset", TextAlignment::Center, 12.f, false}));

    // knob labels
    x = LABEL_LEFT;
    y = LABEL_TOP;
    LabelStyle control_style{S::control_label};
    control_style.align = TextAlignment::Left;

    addChild(m1_label = createLabel(Vec(x,y), 75.f, "i",   theme, control_style)); y += MACRO_DY;
    addChild(m2_label = createLabel(Vec(x,y), 75.f, "ii",  theme, control_style)); y += MACRO_DY;
    addChild(m3_label = createLabel(Vec(x,y), 75.f, "iii", theme, control_style)); y += MACRO_DY;
    addChild(m4_label = createLabel(Vec(x,y), 75.f, "iv",  theme, control_style)); y += MACRO_DY;
    addChild(m5_label = createLabel(Vec(x,y), 75.f, "v",   theme, control_style)); y += MACRO_DY;
    addChild(m6_label = createLabel(Vec(x,y), 75.f, "vi",  theme, control_style));

    // knob pedal annotations
    y = KNOB_TOP + 2.f;
    addChild(m1_ped_label = createLabel(Vec(x,y), 20.f, "", theme, S::pedal_label)); y += MACRO_DY;
    addChild(m2_ped_label = createLabel(Vec(x,y), 20.f, "", theme, S::pedal_label)); y += MACRO_DY;
    addChild(m3_ped_label = createLabel(Vec(x,y), 20.f, "", theme, S::pedal_label)); y += MACRO_DY;
    addChild(m4_ped_label = createLabel(Vec(x,y), 20.f, "", theme, S::pedal_label)); y += MACRO_DY;
    addChild(m5_ped_label = createLabel(Vec(x,y), 20.f, "", theme, S::pedal_label)); y += MACRO_DY;
    addChild(m6_ped_label = createLabel(Vec(x,y), 20.f, "", theme, S::pedal_label));

    // inputs
    const NVGcolor co_port = PORT_CORN;
    x = INPUT_LEFT;
    y = S::PORT_TOP + S::PORT_DY*.5f - 5.f;
    addChild(knobs[K_MODULATION] = createChemKnob<TrimPot>(Vec(x, y), &module_svgs, module, MacroModule::P_MOD_AMOUNT, theme));
    x += INPUT_DX;
    y = S::PORT_TOP;
    for (int i = 0; i <= M6; ++i) {
        addChild(Center(createThemedColorInput(Vec(x, y), &module_svgs, my_module, i, S::InputColorKey, co_port, theme)));
        addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, i));
        addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 18.f, format_string("M%d", 1 + i), theme, S::in_port_label));
        if (my_module) {
            float xoff {0.f};
            float w {29.25f};
            if (i == 0 || i == 3) {
                xoff = -3.f;
                w += 5.f;
            }
            addChild(Center(createClickRegion(x + xoff, y -14.f, w, 21.f, i, [=](int id, int mods) { my_module->set_modulation_target(id); })));
        }
        x += INPUT_DX;
        if (i == 2) {
            y += S::PORT_DY;
            x = INPUT_LEFT + INPUT_DX;
        }
    }

    // footer

    addChild(haken_device_label = createLabel<TipLabel>(
        Vec(28.f, box.size.y - 13.f), 200.f, S::NotConnected, theme, style::haken_label));

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y - S::U1), &module_svgs, "Core link");

    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ModuleBroker::get()->addHostPickerMenu(createMenu(), my_module);
        });
    }
    addChild(link_button);

    // Browsing UI

    if (browsing) {
        m1_label->text("i");
        m2_label->text("ii");
        m3_label->text("iii");
        m4_label->text("iv");
        m5_label->text("v");
        m6_label->text("vi");
        m1_ped_label->text("p1");
        m3_ped_label->text("p2");

        if (S::show_browser_logo()) {
            addChild(createWidgetCentered<OpaqueLogo>(Vec(68, box.size.y*.25)));
        }
    }

    module_svgs.changeTheme(theme);

    // init

    if (!my_module || my_module->glow_knobs) {
        glowing_knobs(true);
    }

    if (my_module) {
        my_module->set_chem_ui(this);
        if (!chem_host) {
            onConnectHost(my_module->chem_host);
        }
    }
}

void MacroUi::glowing_knobs(bool glow) {
    for (int i = 0; i < MacroModule::NUM_KNOBS; ++i) {
        knobs[i]->glowing(glow);
    }
}

void MacroUi::center_knobs()
{
    if (!my_module) return;
    for (int i = 0; i < 6; ++i) {
        my_module->getParam(i).setValue(5.0f);
    }
}

void MacroUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    onConnectionChange(ChemDevice::Haken, host ? host->host_connection(ChemDevice::Haken) : nullptr);
}

void MacroUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    onConnectionChangeUiImpl(this, device, connection);
    if (connection) {
        onPresetChange();
    }
    else {
        unconnected_ui();
    }
}

void MacroUi::unconnected_ui()
{
    center_knobs();
    //if (my_module) { my_module->modulation.zero_modulation(); }
    preset_label->text("");
    preset_label->describe("[no preset]");
    m1_label->text("");
    m2_label->text("");
    m3_label->text("");
    m4_label->text("");
    m5_label->text("");
    m6_label->text("");
}

void MacroUi::onPresetChange()
{
    if (my_module) {
        //if (my_module->batch_busy()) return;
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
        } else {
            preset_label->text("");
            preset_label->describe("");
        }
    }
}

void set_pedal_text(TextLabel* label, uint8_t item_cc, uint8_t a1, uint8_t a2)
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

void MacroUi::onHoverKey(const HoverKeyEvent &e)
{
    if (my_module) {
        if (e.action == GLFW_PRESS && ((e.mods & RACK_MOD_MASK) == 0)) {
            switch (e.key) {
            case GLFW_KEY_0:
                e.consume(this);
                my_module->modulation.zero_modulation();
                return;
            case GLFW_KEY_5:
                center_knobs();
                e.consume(this);
                return;
            }
        }
    }
    Base::onHoverKey(e);
}

void MacroUi::step()
{
    Base::step();
    if (!my_module) return;
    bind_host(my_module);

    if (!chem_host || chem_host->host_busy()) return;

    knobs[K_MODULATION]->enable(my_module->modulation.has_target());

    for (int i = 0; i <= M6; ++i) {
        tracks[i]->set_value(my_module->modulation.get_port(i).modulated());
        tracks[i]->set_active(my_module->getInput(i).isConnected());
    }

    auto em = chem_host->host_matrix();
    if (!em) return;
    auto a1 = em->get_jack_1_assign();
    auto a2 = em->get_jack_2_assign();
    set_pedal_text(m1_ped_label, Haken::ccI, a1, a2);
    set_pedal_text(m2_ped_label, Haken::ccII, a1, a2);
    set_pedal_text(m3_ped_label, Haken::ccIII, a1, a2);
    set_pedal_text(m4_ped_label, Haken::ccIV, a1, a2);
    set_pedal_text(m5_ped_label, Haken::ccV, a1, a2);
    set_pedal_text(m6_ped_label, Haken::ccVI, a1, a2);
}

void MacroUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
    menu->addChild(new MenuSeparator);

    bool unconnected = (my_module->inputs.end() == std::find_if(my_module->inputs.begin(), my_module->inputs.end(), [](Input& in){ return in.isConnected(); }));
    menu->addChild(createMenuItem("Zero modulation", "0", [this](){
        my_module->modulation.zero_modulation();
    }, unconnected));

    menu->addChild(createMenuItem("Center knobs", "5", [this](){ center_knobs(); }));

    menu->addChild(createCheckMenuItem("Glowing knobs", "",
        [this](){ return my_module->glow_knobs; },
        [this](){
            my_module->glow_knobs = !my_module->glow_knobs;
            glowing_knobs(my_module->glow_knobs);
        }
    ));
    Base::appendContextMenu(menu);
}
