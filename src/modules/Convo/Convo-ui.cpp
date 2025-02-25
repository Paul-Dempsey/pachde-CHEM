#include "Convo.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace pachde;

bool ConvoUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

enum K { 
    K_TYPE,
    K_LENGTH,
    K_TUNING,
    K_WIDTH,
    K_L, 
    K_R,
    K_PRE_MIX,
    K_PRE_INDEX,
    K_POST_MIX,
    K_POST_INDEX,
    K_ATTENUVERT
};

constexpr const float PANEL_WIDTH = 165.f;
constexpr const float CENTER = PANEL_WIDTH*.5f;
constexpr const float KNOB_LABEL_DX = 20.f;
constexpr const float KNOB_LABEL_DY = -7.f;
constexpr const float PORT_DX = 30.f;
using CM = ConvoModule;

ConvoUi::ConvoUi(ConvoModule *module) :
    my_module(module),
    last_convo(-1),
    last_type(-1.f),
    last_extend(-1.f)
{
    setModule(module);
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);

    float x, y;
    bool browsing = !module;

    LabelStyle heading_style = LabelStyle{"ctl-label-hi", TextAlignment::Center, 16.f, true};

    x = 16.f;
    y = 140.f;
    addChild(selector = createThemedParam<SelectorWidget>(Vec(x, y), my_module, CM::P_SELECT, theme_engine, theme));
    addChild(selector_label = createStaticTextLabel<StaticTextLabel>(Vec(x + 5.f, y - 20.5f), 20.f, "", theme_engine, theme, heading_style));

    heading_style.height = 14.f;
    x = CENTER - PANEL_WIDTH *.25;
    y = 16.f;
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y), 25.f, "Pre", theme_engine, theme,  heading_style));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x + CENTER, y), 25.f, "Post", theme_engine, theme, heading_style));

    // knobs
    y = 52.f;
    const float knob_dx = 34.f;
    const float center_dx = 22.f;
    addChild(knobs[K_PRE_MIX]    = createChemKnob<BlueKnob>(Vec(CENTER-center_dx-knob_dx, y), my_module, CM::P_PRE_MIX, theme_engine, theme));
    addChild(knobs[K_PRE_INDEX]  = createChemKnob<BasicKnob>(Vec(CENTER-center_dx, y), my_module, CM::P_PRE_INDEX, theme_engine, theme));
    addChild(knobs[K_POST_MIX]   = createChemKnob<BlueKnob>(Vec(CENTER+center_dx, y), my_module, CM::P_POST_MIX, theme_engine, theme));
    addChild(knobs[K_POST_INDEX] = createChemKnob<BasicKnob>(Vec(CENTER+center_dx+knob_dx, y), my_module, CM::P_POST_INDEX, theme_engine, theme));

    x = 56.5f;
    y = 95.f;
    const float knob_dy = 40.f;
    addChild(knobs[K_TYPE] = createChemKnob<BasicKnob>(Vec(x, y), my_module, CM::P_TYPE, theme_engine, theme));
    addChild(type_label = createStaticTextLabel<StaticTextLabel>(Vec(x + KNOB_LABEL_DX, y+KNOB_LABEL_DY), 80.f, "",
        theme_engine, theme, LabelStyle {"dytext", TextAlignment::Left, 14.f}));

    extend_button = Center(createThemedParamLightButton<SmallRoundParamButton, SmallSimpleLight<RedLight>>(
        Vec(22.f, y + 6.f), my_module, CM::P_EXTEND, CM::L_EXTEND, theme_engine, theme));
    addChild(extend_button);

    x = CENTER;
    y += knob_dy;
    addChild(knobs[K_LENGTH] = createChemKnob<BasicKnob>(Vec(x, y), my_module, CM::P_LENGTH, theme_engine, theme));
    addChild( createStaticTextLabel<StaticTextLabel>(Vec(x + KNOB_LABEL_DX, y+KNOB_LABEL_DY), 80.f, "Length", theme_engine, theme, S::control_label_left));
    y += knob_dy;
    addChild(knobs[K_TUNING] = createChemKnob<BasicKnob>(Vec(x, y), my_module, CM::P_TUNING, theme_engine, theme));
    addChild( createStaticTextLabel<StaticTextLabel>(Vec(x + KNOB_LABEL_DX, y+KNOB_LABEL_DY), 80.f, "Tuning", theme_engine, theme, S::control_label_left));

    y += knob_dy;
    addChild(knobs[K_WIDTH]  = createChemKnob<BasicKnob>(Vec(x, y), my_module, CM::P_WIDTH, theme_engine, theme));
    addChild( createStaticTextLabel<StaticTextLabel>(Vec(x + KNOB_LABEL_DX, y+KNOB_LABEL_DY), 80.f, "Width", theme_engine, theme, S::control_label_left));

    y += knob_dy;
    const float lr_knob_dx = 24.f;
    addChild(knobs[K_L] = createChemKnob<BasicKnob>(Vec(CENTER - lr_knob_dx, y), my_module, CM::P_LEFT, theme_engine, theme));
    addChild( createStaticTextLabel<StaticTextLabel>(Vec(CENTER - lr_knob_dx - 24, y), 80.f, "L", theme_engine, theme, S::control_label_left));
    addChild(knobs[K_R] = createChemKnob<BasicKnob>(Vec(CENTER + lr_knob_dx, y), my_module, CM::P_RIGHT, theme_engine, theme));
    addChild( createStaticTextLabel<StaticTextLabel>(Vec(CENTER + lr_knob_dx + 20, y), 80.f, "R", theme_engine, theme, S::control_label_left));

    addChild(knobs[K_ATTENUVERT] = createChemKnob<TrimPot>(Vec(CENTER,S::PORT_TOP), my_module, CM::P_ATTENUVERT, theme_engine, theme));

    // inputs
    auto co_port = PORT_CORN;
    y = S::PORT_TOP;
    x = CENTER - (PORT_DX + PORT_DX);
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, CM::IN_PRE_MIX, co_port, theme_engine, theme)));
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_ATT_DX, y - S::PORT_ATT_DY), my_module, CM::L_IN_PRE_MIX));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 35.f, "MIX", theme_engine, theme, S::in_port_label));
    x += PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, CM::IN_PRE_INDEX, co_port, theme_engine, theme)));
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_ATT_DX, y - S::PORT_ATT_DY), my_module, CM::L_IN_PRE_INDEX));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 35.f, "IDX", theme_engine, theme, S::in_port_label));

    x = CENTER + PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, CM::IN_POST_MIX, co_port, theme_engine, theme)));
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_ATT_DX, y - S::PORT_ATT_DY), my_module, CM::L_IN_POST_MIX));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 35.f, "MIX", theme_engine, theme, S::in_port_label));
    x += PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, CM::IN_POST_INDEX, co_port, theme_engine, theme)));
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_ATT_DX, y - S::PORT_ATT_DY), my_module, CM::L_IN_POST_INDEX));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 35.f, "IDX", theme_engine, theme, S::in_port_label));

    // footer
    addChild(warning_label = createStaticTextLabel<TipLabel>(
        Vec(28.f, box.size.y - 22.f), box.size.x, "", theme_engine, theme, S::warning_label));
    warning_label->describe("[warning/status]");
    warning_label->glowing(true);

    addChild(haken_device_label = createStaticTextLabel<TipLabel>(
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
        selector_label->text("#1");
        type_label->text("Wood");

        auto logo = new Logo(0.35f);
        logo->box.pos = Vec(CENTER - logo->box.size.x*.5, S::PORT_TOP + S::PORT_DY *.5f);
        addChild(logo);
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

void ConvoUi::glowing_knobs(bool glow) {
    for (int i = 0; i < ConvoModule::NUM_KNOBS; ++i) {
        knobs[i]->glowing(glow);
    }
    selector->bright = glow;
}

void ConvoUi::setThemeName(const std::string& name, void * context)
{
//    blip->set_light_color(ColorFromTheme(getThemeName(), "warning", nvgRGB(0xe7, 0xe7, 0x45)));
    Base::setThemeName(name, context);
}

void ConvoUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        //onPresetChange();
    } else {
        haken_device_label->text(S::NotConnected);
    }
}

void ConvoUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    haken_device_label->text(connection ? connection->info.friendly(TextFormatLength::Short) : S::NotConnected);
    haken_device_label->describe(connection ? connection->info.friendly(TextFormatLength::Long) : S::NotConnected);
}

void ConvoUi::onPresetChange()
{
}

void ConvoUi::sync_select_label()
{
    if (!my_module) return;
    auto current = getParamInt(my_module->getParam(CM::P_SELECT));
    if (last_convo != current) {
        last_convo = current;
        selector_label->text({ '#', (char)(1 + current + '0') });
    }
}

void ConvoUi::sync_type_label()
{
    if (!my_module) return;
    auto sq = dynamic_cast<SwitchQuantity*>(my_module->getParamQuantity(CM::P_TYPE));
    if (sq) {
        auto current = sq->getValue();
        if (last_type != current) {
            type_label->text(sq->getDisplayValueString());
            last_type = current;
        }
    }
}

void ConvoUi::step()
{
    Base::step();
    sync_select_label();
    sync_type_label();
}

void ConvoUi::draw(const DrawArgs& args)
{
    Base::draw(args);
}

void ConvoUi::appendContextMenu(Menu *menu)
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
