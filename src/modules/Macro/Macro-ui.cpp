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

bool MacroUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

using MM = MacroModule;
enum M { M1, M2, M3, M4, M5, M6, K_MODULATION };

MacroUi::MacroUi(MacroModule *module) :
    my_module(module)
{
    setModule(module);
    auto theme = getSvgTheme();
    auto panel = createThemedPanel(panelFilename(), &module_svgs);
    panelBorder = attachPartnerPanelBorder(panel);
    setPanel(panel);
    ::svg_query::BoundsIndex bounds;
    svg_query::addBounds(panel->svg, "k:", bounds, true);

    bool browsing = !module;

    // knobs
    addChild(knobs[M1] = createChemKnob<BasicKnob>(bounds["k:m1"].getCenter(), &module_svgs, module, MacroModule::P_M1));
    addChild(knobs[M2] = createChemKnob<BasicKnob>(bounds["k:m2"].getCenter(), &module_svgs, module, MacroModule::P_M2));
    addChild(knobs[M3] = createChemKnob<BasicKnob>(bounds["k:m3"].getCenter(), &module_svgs, module, MacroModule::P_M3));
    addChild(knobs[M4] = createChemKnob<BasicKnob>(bounds["k:m4"].getCenter(), &module_svgs, module, MacroModule::P_M4));
    addChild(knobs[M5] = createChemKnob<BasicKnob>(bounds["k:m5"].getCenter(), &module_svgs, module, MacroModule::P_M5));
    addChild(knobs[M6] = createChemKnob<BasicKnob>(bounds["k:m6"].getCenter(), &module_svgs, module, MacroModule::P_M6));
    addChild(knobs[K_MODULATION] = createChemKnob<TrimPot>(bounds["k:amount"].getCenter(), &module_svgs, my_module, MacroModule::P_MOD_AMOUNT));

    addChild(tracks[M1] = createTrackWidget(knobs[M1]));
    addChild(tracks[M2] = createTrackWidget(knobs[M2]));
    addChild(tracks[M3] = createTrackWidget(knobs[M3]));
    addChild(tracks[M4] = createTrackWidget(knobs[M4]));
    addChild(tracks[M5] = createTrackWidget(knobs[M5]));
    addChild(tracks[M6] = createTrackWidget(knobs[M6]));

    addChild(preset_label = createLabel<TipLabel>(bounds["k:preset"], browsing ? "—preset—" : "", &preset_style));

    // knob labels
    addChild(m1_label = createLabel(bounds["k:m1-label"], "i", &S::control_label_left));
    addChild(m2_label = createLabel(bounds["k:m2-label"], "ii", &S::control_label_left));
    addChild(m3_label = createLabel(bounds["k:m3-label"], "iii", &S::control_label_left));
    addChild(m4_label = createLabel(bounds["k:m4-label"], "iv", &S::control_label_left));
    addChild(m5_label = createLabel(bounds["k:m5-label"], "v", &S::control_label_left));
    addChild(m6_label = createLabel(bounds["k:m6-label"], "vi", &S::control_label_left));

    // knob pedal annotations
    addChild(m1_ped_label = createLabel(bounds["k:m1-ped"], "", &S::pedal_label));
    addChild(m2_ped_label = createLabel(bounds["k:m2-ped"], "", &S::pedal_label));
    addChild(m3_ped_label = createLabel(bounds["k:m3-ped"], "", &S::pedal_label));
    addChild(m4_ped_label = createLabel(bounds["k:m4-ped"], "", &S::pedal_label));
    addChild(m5_ped_label = createLabel(bounds["k:m5-ped"], "", &S::pedal_label));
    addChild(m6_ped_label = createLabel(bounds["k:m6-ped"], "", &S::pedal_label));


    add_input(bounds, "k:mod-m1", "k:m-m1-label", "k:m-m1-click", "M1", MacroModule::IN_M1);
    add_input(bounds, "k:mod-m2", "k:m-m2-label", "k:m-m2-click", "M2", MacroModule::IN_M2);
    add_input(bounds, "k:mod-m3", "k:m-m3-label", "k:m-m3-click", "M3", MacroModule::IN_M3);
    add_input(bounds, "k:mod-m4", "k:m-m4-label", "k:m-m4-click", "M4", MacroModule::IN_M4);
    add_input(bounds, "k:mod-m5", "k:m-m5-label", "k:m-m5-click", "M5", MacroModule::IN_M5);
    add_input(bounds, "k:mod-m6", "k:m-m6-label", "k:m-m6-click", "M6", MacroModule::IN_M6);

    // footer

    addChild(haken_device_label = createLabel<TipLabel>(bounds["k:haken"], S::NotConnected, &S::haken_label));
    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y - S::U1), &module_svgs, "Core link");

    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ModuleBroker::get()->addHostPickerMenu(createMenu(), my_module);
        });
    }
    addChild(link_button);

    // Browsing UI

    if (browsing) {
        m1_label->set_text("i");
        m2_label->set_text("ii");
        m3_label->set_text("iii");
        m4_label->set_text("iv");
        m5_label->set_text("v");
        m6_label->set_text("vi");
        m1_ped_label->set_text("p1");
        m3_ped_label->set_text("p2");

        if (S::show_browser_logo()) {
            addChild(createWidgetCentered<OpaqueLogo>(Vec(68, box.size.y*.25)));
        }
    }

    // init
    if (!my_module || my_module->glow_knobs) {
        glowing_knobs(true);
    }

    module_svgs.changeTheme(theme);
    applyChildrenTheme(this, theme);

    if (my_module) {
        my_module->set_chem_ui(this);
        if (!chem_host) {
            onConnectHost(my_module->chem_host);
        }
    }
}

void MacroUi::add_input(::svg_query::BoundsIndex &bounds, const char *port_key, const char *label_key, const char *click_key, const char *label, int index)
{
    Vec pos{bounds[port_key].getCenter()};
    addChild(Center(createThemedColorInput(pos, &module_svgs, my_module, index, S::InputColorKey, PORT_CORN)));
    addChild(createLabel(bounds[label_key], label, &S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(bounds[click_key], index, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(pos.minus(S::light_dx), my_module, index));
}


MacroUi::~MacroUi() {
    if (my_module) my_module->set_chem_ui(nullptr);
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
    preset_label->set_text("");
    preset_label->describe("[no preset]");
    m1_label->set_text("");
    m2_label->set_text("");
    m3_label->set_text("");
    m4_label->set_text("");
    m5_label->set_text("");
    m6_label->set_text("");
}

void MacroUi::onPresetChange()
{
    if (my_module) {
        //if (my_module->batch_busy()) return;
        m1_label->set_text(my_module->macro_names.macro[M1]);
        m2_label->set_text(my_module->macro_names.macro[M2]);
        m3_label->set_text(my_module->macro_names.macro[M3]);
        m4_label->set_text(my_module->macro_names.macro[M4]);
        m5_label->set_text(my_module->macro_names.macro[M5]);
        m6_label->set_text(my_module->macro_names.macro[M6]);
        auto preset = chem_host ? chem_host->host_preset() : nullptr;
        if (preset) {
            preset_label->set_text(preset->name);
            preset_label->describe(preset->text);
        } else {
            preset_label->set_text("");
            preset_label->describe("");
        }
    }
}

void set_pedal_text(TextLabel* label, uint8_t item_cc, uint8_t a1, uint8_t a2)
{
    if (item_cc == a1 || item_cc == a2) {
        if (a1 == a2) {
            label->set_text("p1,p2");
        } else if (item_cc == a1) {
            label->set_text("p1");
        } else {
            label->set_text("p2");
        }
    } else {
        label->set_text("");
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
