#include "Post.hpp"
#include "services/colors.hpp"
#include "em/em-hardware.h"
#include "widgets/click-region-widget.hpp"
#include "widgets/logo-widget.hpp"
#include "widgets/uniform-style.hpp"

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
    auto theme = getSvgTheme();
    auto panel = createThemedPanel(panelFilename(), &module_svgs);
    panelBorder = attachPartnerPanelBorder(panel);
    setPanel(panel);
    float x, y;
    //bool browsing = !module;
    LabelStyle knob_label_style ={"ctl-label", TextAlignment::Center, 14.f, false};

    // knobs
    x = CENTER;
    addChild(knobs[K_POST_LEVEL] = createChemKnob<YellowKnob>(Vec(x, 35.f), &module_svgs, module, PostModule::P_POST_LEVEL));
    addChild(tracks[K_POST_LEVEL] = createTrackWidget(knobs[K_POST_LEVEL]));

    // MUTE
    y = 68.f;
    addChild(Center(createThemedParamLightButton<LargeRoundParamButton, SmallLight<RedLight>>(
        Vec(x, y), &module_svgs, my_module, PostModule::P_MUTE, PostModule::L_MUTE)));
    addChild(createLabel<TextLabel>(Vec(x,y + 14.f), 40.f, "Mute", knob_label_style));

    y = 104.f;
    addChild(effect_label = createLabel<TextLabel>(Vec(x, y), 100.f, "EQ", LabelStyle{"ctl-label", TextAlignment::Center, 16.f, true}));
    y += 34;
    addChild(knobs[K_MIX] = createChemKnob<BlueKnob>(Vec(x, y), &module_svgs, module, PostModule::P_MIX));
    addChild(tracks[K_MIX] = createTrackWidget(knobs[K_MIX]));
    addChild(mix_light = createLightCentered<SmallSimpleLight<GreenLight>>(Vec(x + 22.f, y-9.f), my_module, PostModule::L_MIX));
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, getSvgTheme());

    y += 40;
    const float PARAM_DY = 54.f;
    const float label_offset = 18.f;

    addChild(knobs[K_TILT] = createChemKnob<BasicKnob>(Vec(x, y), &module_svgs, module, PostModule::P_TILT));
    addChild(tracks[K_TILT] = createTrackWidget(knobs[K_TILT]));
    addChild(top_knob_label= createLabel<TextLabel>(Vec(x,y + label_offset), 30.f, "Tilt", knob_label_style));
    y += PARAM_DY;
    addChild(knobs[K_FREQUENCY] = createChemKnob<BasicKnob>(Vec(x, y), &module_svgs, module, PostModule::P_FREQUENCY));
    addChild(tracks[K_FREQUENCY] = createTrackWidget(knobs[K_FREQUENCY]));
    addChild(mid_knob_label= createLabel<TextLabel>(Vec(x,y + label_offset), 60.f, "Frequency", knob_label_style));
    y += PARAM_DY;

    // inputs
    const NVGcolor co_port = PORT_CORN;
    const float label_width = 20.f;
    y = S::PORT_TOP;
    x = CENTER - S::PORT_DX;
    addChild(knobs[K_MODULATION] = createChemKnob<TrimPot>(Vec(x, y), &module_svgs, module, PostModule::P_MOD_AMOUNT));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), &module_svgs, my_module, PostModule::IN_MUTE, S::InputColorKey, PORT_RED)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), label_width + 5.f, "MUTE", S::in_port_label));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), &module_svgs, my_module, PostModule::IN_POST_LEVEL, S::InputColorKey, co_port)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), label_width, "LVL", S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PostModule::IN_POST_LEVEL, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PostModule::L_POST_LEVEL_MOD));

    y += S::PORT_DY;
    x = CENTER - S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), &module_svgs, my_module, PostModule::IN_MIX, S::InputColorKey, co_port)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), label_width, "MIX", S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PostModule::IN_MIX, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PostModule::L_MIX_MOD));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), &module_svgs, my_module, PostModule::IN_TILT, S::InputColorKey, co_port)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), label_width, "TLT", S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PostModule::IN_TILT, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PostModule::L_TILT_MOD));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), &module_svgs, my_module, PostModule::IN_FREQUENCY, S::InputColorKey, co_port)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), label_width, "FRQ", S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PostModule::IN_FREQUENCY, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PostModule::L_FREQUENCY_MOD));

    // footer

    addChild(haken_device_label = createLabel<TipLabel>(
        Vec(S::CORE_LINK_TEXT, box.size.y - S::CORE_LINK_TEXT_DY), 200.f, S::NotConnected, S::haken_label));

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y-ONEU), &module_svgs, "Core link");

    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ModuleBroker::get()->addHostPickerMenu(createMenu(), my_module);
        });
    }
    addChild(link_button);

    if (!module && S::show_browser_logo()) {
        auto logo = new OpaqueLogo(0.75f);
        logo->box.pos = Vec(CENTER, box.size.y*.5);
        addChild(Center(logo));
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
    Base::setThemeName(name, context);
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, getSvgTheme());
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
