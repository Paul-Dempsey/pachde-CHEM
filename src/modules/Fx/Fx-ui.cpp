#include "Fx.hpp"
#include "em/em-hardware.h"
#include "services/colors.hpp"
#include "widgets/click-region-widget.hpp"
#include "widgets/logo-widget.hpp"
#include "widgets/uniform-style.hpp"

namespace S = pachde::style;
using fx = FxModule;
using namespace svg_theme;
using namespace pachde;

// -- Fx UI -----------------------------------

enum K { K_R1, K_R2, K_R3, K_R4, K_R5, K_R6, K_MIX, K_MODULATION };

FxUi::FxUi(FxModule *module) :
    my_module(module)
{
    setModule(module);
    auto theme = getSvgTheme();
    auto panel = createThemedPanel(panelFilename(), &module_svgs);
    panelBorder = attachPartnerPanelBorder(panel);
    setPanel(panel);
    ::svg_query::BoundsIndex bounds;
    svg_query::addBounds(panel->svg, "k:", bounds, true);

    if (S::show_screws()) {
        createScrews();
    }
    bool browsing = !module;

    const float PANEL_WIDTH = 135.f;
    const float CENTER = PANEL_WIDTH*.5f;

    addChild(selector = createParam<SelectorWidget>(bounds["k:selector"].pos, my_module, fx::P_EFFECT));
    addChild(effect_label = createLabel(bounds["k:effect-label"], "Short reverb", &control_label_style));

    Vec pos{bounds["k:mix"].getCenter()};
    addChild(knobs[K_MIX] = createChemKnob<BlueKnob>(pos, &module_svgs, my_module, fx::P_MIX));
    addChild(tracks[K_MIX] = createTrackWidget(knobs[K_MIX]));
    addChild(mix_light = createLightCentered<SmallSimpleLight<GreenLight>>(pos.plus(Vec(22.f, -9.f)), my_module, fx::L_MIX));
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, theme);

    addChild(Center(createThemedParamLightButton<MediumRoundParamButton, SmallSimpleLight<RedLight>>(
        bounds["k:effect-off"].getCenter(), &module_svgs, my_module, fx::P_DISABLE, fx::L_DISABLE)));
    addChild(createLabel(bounds["k:off-label"], "Fx Off", &S::control_label));

    add_knob(bounds, "k:r1", "k:r1-label", "R1", fx::P_R1);
    add_knob(bounds, "k:r2", "k:r2-label", "R2", fx::P_R2);
    add_knob(bounds, "k:r3", "k:r3-label", "R3", fx::P_R3);
    add_knob(bounds, "k:r4", "k:r4-label", "R4", fx::P_R4);
    add_knob(bounds, "k:r5", "k:r5-label", "R5", fx::P_R5);
    add_knob(bounds, "k:r6", "k:r6-label", "R6", fx::P_R6);

    // inputs
    addChild(knobs[K_MODULATION] = createChemKnob<TrimPot>(bounds["k:amount"].getCenter(), &module_svgs, module, FxModule::P_MOD_AMOUNT));

    add_input(bounds, "k:mod-mix", "k:m-mix-label", "k:m-mix-click", "MIX", fx::IN_MIX);
    add_input(bounds, "k:mod-r1", "k:m-r1-label", "k:m-r1-click", "R1", fx::IN_R1);
    add_input(bounds, "k:mod-r2", "k:m-r2-label", "k:m-r2-click", "R2", fx::IN_R2);
    add_input(bounds, "k:mod-r3", "k:m-r3-label", "k:m-r3-click", "R3", fx::IN_R3);
    add_input(bounds, "k:mod-r4", "k:m-r4-label", "k:m-r4-click", "R4", fx::IN_R4);
    add_input(bounds, "k:mod-r5", "k:m-r5-label", "k:m-r5-click", "R5", fx::IN_R5);
    add_input(bounds, "k:mod-r6", "k:m-r6-label", "k:m-r6-click", "R6", fx::IN_R6);

    // footer

    addChild(haken_device_label = createLabel<TipLabel>(bounds["k:haken"], S::NotConnected, &S::haken_label));

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y - S::U1), &module_svgs, "Core link");

    if (my_module) {
        link_button->set_handler([=](bool ctrl, bool shift) {
            ModuleBroker::get()->addHostPickerMenu(createMenu(), my_module);
        });
    }
    addChild(link_button);

    // Browsing UI

    if (browsing && S::show_browser_logo()) {
        addChild(createWidgetCentered<OpaqueLogo>(Vec(CENTER, 150.f)));
    }

    module_svgs.changeTheme(theme);
    applyChildrenTheme(this, theme);

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

void FxUi::add_knob(
    ::svg_query::BoundsIndex& bounds,
    const char* knob_key,
    const char * label_key,
    const char * label,
    int index
) {
    addChild(knobs[index] = createChemKnob<BasicKnob>(bounds[knob_key].getCenter(), &module_svgs, my_module, index));
    addChild(tracks[index] = createTrackWidget(knobs[index]));
    addChild(r_labels[index] = createLabel<TipLabel>(bounds[label_key], label, &S::control_label));
}

void FxUi::add_input(::svg_query::BoundsIndex &bounds, const char* port_key, const char* label_key, const char* click_key, const char* label, int index)
{
    Vec pos{bounds[port_key].getCenter()};
    addChild(Center(createThemedColorInput(pos, &module_svgs, my_module, index, S::InputColorKey, PORT_CORN)));
    addChild(createLabel(bounds[label_key], label, &S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(bounds[click_key], index, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(pos.minus(S::light_dx), my_module, index));
}

FxUi::~FxUi() {
    if (my_module) my_module->set_chem_ui(nullptr);
}

void FxUi::glowing_knobs(bool glow) {
    for (int i = 0; i < FxModule::NUM_KNOBS; ++i) {
        knobs[i]->glowing(glow);
    }
}

void FxUi::center_knobs()
{
    if (!my_module) return;
    for (int i = 0; i < 6; ++i) {
        my_module->getParam(i).setValue(5.0f);
    }
}

void FxUi::createScrews()
{
    addChild(createThemedWidget<ThemeScrew>(Vec(2*RACK_GRID_WIDTH, 0), &module_svgs));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2*RACK_GRID_WIDTH, 0), &module_svgs));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2*RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), &module_svgs));
}

void FxUi::setThemeName(const std::string& name, void * context)
{
    Base::setThemeName(name, context);
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, getSvgTheme());
}

void FxUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    onConnectionChange(ChemDevice::Haken, host ? host->host_connection(ChemDevice::Haken) : nullptr);
}

void FxUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    onConnectionChangeUiImpl(this, device, connection);
}

bool FxUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

void FxUi::sync_labels()
{
    // TODO: stop polling start eventing
    auto pq = my_module ? my_module->getParamQuantity(FxModule::P_EFFECT) : nullptr;
    auto index = pq ? getParamIndex(pq) : 0;
    if (index != effect) {
        effect = index;
        effect_label->set_text(pq ? pq->getDisplayValueString() : "Short reverb");

        bool six_param = (Haken::R_shortRev == effect || Haken::R_longRev == effect);
        knobs[K_R5]->enable(six_param);
        knobs[K_R6]->enable(six_param);
        if (!six_param) {
            r_labels[K_R5]->set_text("—");
            r_labels[K_R6]->set_text("—");
        }

        switch (effect) {
        case Haken::R_shortRev:
        case Haken::R_longRev:
            r_labels[K_R1]->set_text("Diffuse");
            r_labels[K_R2]->set_text("LPF");
            r_labels[K_R3]->set_text("Damping");
            r_labels[K_R4]->set_text("Decay");
            r_labels[K_R5]->set_text("PreDelay");
            r_labels[K_R6]->set_text("HPF");
            break;
        case Haken::R_modDel:
        case Haken::R_swepEcho:
            r_labels[K_R1]->set_text("ModDepth");
            r_labels[K_R2]->set_text("ModRate");
            r_labels[K_R3]->set_text("Feedback");
            r_labels[K_R4]->set_text("Time");
            break;
        default:
            switch (effect) {
            case Haken::R_anaEcho:
                r_labels[K_R1]->set_text("Noise");
                break;
            case Haken::R_digEchoLPF:
                r_labels[K_R1]->set_text("LPF");
                break;
            case Haken::R_digEchoHPF:
                r_labels[K_R1]->set_text("HPF");
                break;
            default: r_labels[K_R1]->set_text("R1");
                break;
            }
            r_labels[K_R2]->set_text("Offset");
            r_labels[K_R3]->set_text("Feedback");
            r_labels[K_R4]->set_text("Time");
            break;
        }
    }
}

void FxUi::onHoverKey(const HoverKeyEvent &e)
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

void FxUi::step()
{
    Base::step();
    if (!my_module) return;
    bind_host(my_module);

    if (!chem_host || chem_host->host_busy()) return;
    knobs[K_MODULATION]->enable(my_module->modulation.has_target());

    for (int i = 0; i < K_MODULATION; ++i) {
        tracks[i]->set_value(my_module->modulation.get_port(i).modulated());
        tracks[i]->set_active(my_module->getInput(i).isConnected());
    }

    sync_labels();
}

void FxUi::appendContextMenu(Menu *menu)
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
