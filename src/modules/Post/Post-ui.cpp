#include "Post.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/click-region-widget.hpp"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;

// -- Post UI -----------------------------------

constexpr const float ONEU = 15.f;
constexpr const float CENTER = 45.f;

bool PostUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

enum K { 
    K_POST_LEVEL, 
    K_MIX, 
    K_TILT, 
    K_FREQUENCY, 
    K_MODULATION
};

PostUi::PostUi(PostModule *module) :
    my_module(module)
{
    setModule(module);
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);
    float x, y;
    //bool browsing = !module;
    LabelStyle knob_label_style ={"ctl-label", TextAlignment::Center, 14.f, false};

    // knobs
    x = CENTER;
    addChild(knobs[K_POST_LEVEL] = createChemKnob<YellowKnob>(Vec(x, 35.f), module, PostModule::P_POST_LEVEL, theme_engine, theme));
    addChild(tracks[K_POST_LEVEL] = createTrackWidget(knobs[K_POST_LEVEL], theme_engine, theme));

    // MUTE
    y = 68.f;
    addChild(Center(createThemedParamLightButton<LargeRoundParamButton, SmallLight<RedLight>>(
        Vec(x, y), my_module, PostModule::P_MUTE, PostModule::L_MUTE, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x,y + 14.f), 40.f, "Mute", theme_engine, theme, knob_label_style));
    
    y = 104.f;
    addChild(effect_label = createLabel<TextLabel>(Vec(x, y), 100.f, "EQ", theme_engine, theme, LabelStyle{"ctl-label", TextAlignment::Center, 16.f, true}));
    y += 34;
    addChild(knobs[K_MIX] = createChemKnob<BlueKnob>(Vec(x, y), module, PostModule::P_MIX, theme_engine, theme));
    addChild(tracks[K_MIX] = createTrackWidget(knobs[K_MIX], theme_engine, theme));
    addChild(mix_light = createLightCentered<SmallSimpleLight<GreenLight>>(Vec(x + 22.f, y-9.f), my_module, PostModule::L_MIX));
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, theme->name);

    y += 40;
    const float PARAM_DY = 54.f;
    const float label_offset = 18.f;

    addChild(knobs[K_TILT] = createChemKnob<BasicKnob>(Vec(x, y), module, PostModule::P_TILT, theme_engine, theme));
    addChild(tracks[K_TILT] = createTrackWidget(knobs[K_TILT], theme_engine, theme));
    addChild(top_knob_label= createLabel<TextLabel>(Vec(x,y + label_offset), 30.f, "Tilt", theme_engine, theme, knob_label_style));
    y += PARAM_DY;
    addChild(knobs[K_FREQUENCY] = createChemKnob<BasicKnob>(Vec(x, y), module, PostModule::P_FREQUENCY, theme_engine, theme));
    addChild(tracks[K_FREQUENCY] = createTrackWidget(knobs[K_FREQUENCY], theme_engine, theme));
    addChild(mid_knob_label= createLabel<TextLabel>(Vec(x,y + label_offset), 60.f, "Frequency", theme_engine, theme, knob_label_style));
    y += PARAM_DY;

    // if (browsing) {
    //     auto logo = new Logo(0.35f);
    //     logo->box.pos = Vec(CENTER - logo->box.size.x*.5, y - 16.f);
    //     addChild(logo);
    // }

    // inputs
    const NVGcolor co_port = PORT_CORN;
    const float label_width = 20.f;
    y = S::PORT_TOP;
    x = CENTER - S::PORT_DX;
    addChild(knobs[K_MODULATION] = createChemKnob<TrimPot>(Vec(x, y), module, PostModule::P_MOD_AMOUNT, theme_engine, theme));
    
    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PostModule::IN_MUTE, S::InputColorKey, PORT_RED, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), label_width + 5.f, "MUTE", theme_engine, theme, S::in_port_label));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PostModule::IN_POST_LEVEL, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), label_width, "LVL", theme_engine, theme, S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PostModule::IN_POST_LEVEL, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PostModule::L_POST_LEVEL_MOD));

    y += S::PORT_DY;
    x = CENTER - S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PostModule::IN_MIX, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), label_width, "MIX", theme_engine, theme, S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PostModule::IN_MIX, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PostModule::L_MIX_MOD));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PostModule::IN_TILT, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), label_width, "TLT", theme_engine, theme, S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PostModule::IN_TILT, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PostModule::L_TILT_MOD));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PostModule::IN_FREQUENCY, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), label_width, "FRQ", theme_engine, theme, S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PostModule::IN_FREQUENCY, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PostModule::L_FREQUENCY_MOD));

    // footer

    addChild(haken_device_label = createLabel<TipLabel>(
        Vec(S::CORE_LINK_TEXT, box.size.y - S::CORE_LINK_TEXT_DY), 200.f, S::NotConnected, theme_engine, theme, S::haken_label));

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

    // init

    if (!my_module || my_module->glow_knobs) {
        glowing_knobs(true);
    }

    if (my_module) {
        my_module->set_chem_ui(this);
        onConnectHost(my_module->chem_host);
    }
}

void PostUi::glowing_knobs(bool glow) {
    for (int i = 0; i < PostModule::NUM_KNOBS; ++i) {
        knobs[i]->glowing(glow);
    }
}

void PostUi::center_knobs()
{
    if (!my_module) return;
    for (int i = 1; i < 4; ++i) {
        my_module->getParam(i).setValue(5.0f);
    }
}

void PostUi::setThemeName(const std::string& name, void * context)
{
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, name);
    Base::setThemeName(name, context);
}

void PostUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    onConnectionChange(ChemDevice::Haken, host ? host->host_connection(ChemDevice::Haken) : nullptr);
}

void PostUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    onConnectionChangeUiImpl(this, device, connection);
}

void PostUi::onPresetChange()
{
}

void PostUi::step()
{
    Base::step();
    if (!my_module) return;
    bind_host(my_module);

    knobs[K_MODULATION]->enable(my_module->modulation.has_target());

    for (int i = 0; i <= K_FREQUENCY; ++i) {
        tracks[i]->set_value(my_module->modulation.get_port(i).modulated());
        tracks[i]->set_active(my_module->getInput(i).isConnected());
    }

}

void PostUi::onHoverKey(const HoverKeyEvent &e)
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

void PostUi::appendContextMenu(Menu *menu)
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
