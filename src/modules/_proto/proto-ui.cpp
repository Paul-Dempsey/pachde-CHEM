#include "proto.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/click-region-widget.hpp"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;

// -- UI --------------------------------------

struct RawQuantity : Quantity {
    float value;
    void setValue(float v) override { value = v; }
    float getValue() override { return value; }
    float getMinValue() override { return -INFINITY; }
    float getMaxValue() override { return INFINITY; }
};

ProtoUi::ProtoUi(ProtoModule *module) :
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

    //const float PANEL_WIDTH = 300.f;
    //const float CENTER = PANEL_WIDTH*.5f;
    auto label_style = LabelStyle{"ctl-label", TextAlignment::Right, 12.f};
    auto value_style = LabelStyle{"setting", TextAlignment::Left, 12.f};

    x = 100.f;
    y = 18.f;
    for (int i = 0; i < 5; ++i) {
        addChild(label[i] = createLabel<TextLabel>(Vec(x,y), 100.f, "", theme_engine, theme, label_style));
        addChild(value_text[i] = createLabel<TextLabel>(Vec(x + 5.f,y), 100.f, "", theme_engine, theme, value_style));
        y += 14.f;
    }
    label[0]->text("ccOctShift");
    label[1]->text("ccJack1");
    label[2]->text("ccJack2");
    label[3]->text("idJackShift");
    y += 6.f;
    text_entry = new TextField();
    text_entry->box.pos = Vec(15.f, y);
    text_entry->box.size.x = 85.f;
    addChild(text_entry);

    auto btn = Center(createThemedButton<SmallRoundButton>(Vec(x + 12.f,y + 9.f), theme_engine, theme, "poke"));
    btn->setHandler([=](bool ctl, bool shift){
        if (!chem_host) return;
        if (text_entry->text.empty()) return;
        int value = clamp(std::atoi(text_entry->text.c_str()), 0, 127);
        chem_host->host_haken()->begin_stream(ChemId::Proto, Haken::s_Mat_Poke);
        chem_host->host_haken()->key_pressure(ChemId::Proto, Haken::ch16, Haken::idJackShift, value);
    });
    addChild(btn);

    y += 22.f;
    expr_entry = new TextField();
    expr_entry->box.pos = Vec(15.f, y);
    expr_entry->box.size.x = 85.f;
    addChild(expr_entry);

    addChild(value_text[4] = createLabel<TextLabel>(Vec(x + 22.f,y + 5.f), 100.f, "", theme_engine, theme, value_style));

    btn = Center(createThemedButton<SmallRoundButton>(Vec(x + 12.f,y + 9.f), theme_engine, theme, "expr"));
    btn->setHandler([=](bool ctl, bool shift){
        if (expr_entry->text.empty()) {
            value_text[4]->text("");
            return;
        }
        RawQuantity q;
        q.setDisplayValueString(expr_entry->text);
        value_text[4]->text(q.getDisplayValueString());
    });
    addChild(btn);

    // inputs

    // footer

    addChild(warn = createLabel<TipLabel>(
        Vec(28.f, box.size.y - 22.f), box.size.x, "", theme_engine, theme, S::warning_label));
    warn->describe("[warning/status]");
    warn->glowing(true);

    addChild(haken_device_label = createLabel<TipLabel>(
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
    if (!my_module || my_module->glow_knobs) {
        glowing_knobs(true);
    }

    if (my_module) {
        my_module->set_chem_ui(this);
        onConnectHost(my_module->chem_host);
    }
}

void ProtoUi::glowing_knobs(bool glow) {
    // for (int i = 0; i < ProtoModule::NUM_KNOBS; ++i) {
    //     knobs[i]->glowing(glow);
    // }
}

void ProtoUi::center_knobs()
{
    if (!my_module) return;
}

void ProtoUi::setThemeName(const std::string& name, void * context)
{
    //applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, name);
    Base::setThemeName(name, context);
}

void ProtoUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    onConnectionChange(ChemDevice::Haken, host ? host->host_connection(ChemDevice::Haken) : nullptr);
}

void ProtoUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    onConnectionChangeUiImpl(this, device, connection);
}

bool ProtoUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

void ProtoUi::onHoverKey(const HoverKeyEvent &e)
{
    if (my_module) {
        if (e.action == GLFW_PRESS && ((e.mods & RACK_MOD_MASK) == 0)) {
            switch (e.key) {
            case GLFW_KEY_0:
                e.consume(this);
                //my_module->modulation.zero_modulation();
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

void ProtoUi::step()
{
    Base::step();
    if (!my_module) return;
    bind_host(my_module);

    if (!chem_host) return;
    auto em = chem_host->host_matrix();
    value_text[0]->text(format_string("%d", em->get_octave_shift())); //"ccOctShift"
    auto js = em->get_jack_1();
    value_text[1]->text(format_string("%d (%d:%d)", js, js >> 7, js & 0x7f)); //"ccJack1"
    js = em->get_jack_2();
    value_text[2]->text(format_string("%d (%d:%d)", js, js >> 7, js & 0x7f)); //"ccJack2"
    value_text[3]->text(format_string("%d", em->get_jack_shift())); //"idJackShift"

    //knobs[K_MODULATION]->enable(my_module->modulation.has_target());

    // for (int i = 0; i < K_MODULATION; ++i) {
    //     tracks[i]->set_value(my_module->modulation.get_port(i).modulated());
    //     tracks[i]->set_active(my_module->getInput(i).isConnected());
    // }

}

void ProtoUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
    menu->addChild(new MenuSeparator);

    // bool unconnected = (my_module->inputs.end() == std::find_if(my_module->inputs.begin(), my_module->inputs.end(), [](Input& in){ return in.isConnected(); }));
    // menu->addChild(createMenuItem("Zero modulation", "0", [this](){
    //     my_module->modulation.zero_modulation();
    // }, unconnected));
    
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
