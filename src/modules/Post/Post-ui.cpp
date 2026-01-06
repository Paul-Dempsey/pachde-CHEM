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
    ::svg_query::BoundsIndex bounds;
    svg_query::addBounds(panel->svg, "k:", bounds, true);

    addChild(knobs[K_POST_LEVEL] = createChemKnob<YellowKnob>(bounds["k:level"].getCenter(), &module_svgs, module, PostModule::P_POST_LEVEL));
    addChild(tracks[K_POST_LEVEL] = createTrackWidget(knobs[K_POST_LEVEL]));

    addChild(Center(createThemedParamLightButton<LargeRoundParamButton, SmallLight<RedLight>>(
        bounds["k:mute"].getCenter(), &module_svgs, my_module, PostModule::P_MUTE, PostModule::L_MUTE)));
    addChild(createLabel(bounds["k:mute-label"], "Mute", &S::control_label));

    addChild(createLabel(bounds["k:eq-label"], "EQ", &center_label));

    addChild(knobs[K_MIX] = createChemKnob<BlueKnob>(bounds["k:mix"].getCenter(), &module_svgs, module, PostModule::P_MIX));
    addChild(tracks[K_MIX] = createTrackWidget(knobs[K_MIX]));
    addChild(mix_light = createLightCentered<SmallSimpleLight<GreenLight>>(bounds["k:mix-light"].getCenter(), my_module, PostModule::L_MIX));
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, getSvgTheme());

    add_knob(bounds, "k:tilt", "k:tilt-label", "Tilt", PostModule::P_TILT);
    add_knob(bounds, "k:freq", "k:tilt-freq", "Frequency", PostModule::P_FREQUENCY);

    // inputs
    addChild(knobs[K_MODULATION] = createChemKnob<TrimPot>(bounds["k:amount"].getCenter(), &module_svgs, module, PostModule::P_MOD_AMOUNT));

    addChild(Center(createThemedColorInput(bounds["k:in-mute"].getCenter(), &module_svgs, my_module, PostModule::IN_MUTE, S::InputColorKey, PORT_RED)));
    addChild(createLabel(bounds["k:m-mute-label"], "MUTE", &S::in_port_label));

    add_input(bounds, "k:mod-lvl", "k:m-lvl-label", "k:m-lvl-click", "LVL", PostModule::IN_POST_LEVEL);
    add_input(bounds, "k:mod-mix", "k:m-mix-label", "k:m-mix-click", "MIX", PostModule::IN_MIX);
    add_input(bounds, "k:mod-tlt", "k:m-tlt-label", "k:m-tlt-click", "TLT", PostModule::IN_TILT);
    add_input(bounds, "k:mod-frq", "k:m-frq-label", "k:m-frq-click", "FRQ", PostModule::IN_FREQUENCY);

    // footer

    addChild(haken_device_label = createLabel<TipLabel>(bounds["k:haken"], S::NotConnected, &S::haken_label));
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

void PostUi::add_input(::svg_query::BoundsIndex &bounds, const char* port_key, const char* label_key, const char* click_key, const char* label, int index)
{
    Vec pos{bounds[port_key].getCenter()};
    addChild(Center(createThemedColorInput(pos, &module_svgs, my_module, index, S::InputColorKey, PORT_CORN)));
    addChild(createLabel(bounds[label_key], label, &S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(bounds[click_key], index, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(pos.minus(S::light_dx), my_module, index));
}

PostUi::~PostUi() {
    if (my_module) {
        my_module->set_chem_ui(nullptr);
    }
}

void PostUi::glowing_knobs(bool glow) {
    for (int i = 0; i < PostModule::NUM_KNOBS; ++i) {
        knobs[i]->glowing(glow);
    }
}

void PostUi::center_knobs() {
    if (!my_module) return;
    for (int i = 1; i < 4; ++i) {
        my_module->getParam(i).setValue(5.0f);
    }
}

void PostUi::add_knob(::svg_query::BoundsIndex &bounds, const char *knob_key, const char *label_key, const char *label, int index){
    addChild(knobs[index] = createChemKnob<BasicKnob>(bounds[knob_key].getCenter(), &module_svgs, module, index));
    addChild(tracks[index] = createTrackWidget(knobs[index]));
    addChild(createLabel(bounds[label_key], label, &S::control_label));
}

void PostUi::setThemeName(const std::string& name, void * context) {
    Base::setThemeName(name, context);
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, getSvgTheme());
}

void PostUi::onConnectHost(IChemHost* host) {
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
