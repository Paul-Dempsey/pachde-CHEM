#include "Fx.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/click-region-widget.hpp"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;

// -- Fx UI -----------------------------------

enum K { K_R1, K_R2, K_R3, K_R4, K_R5, K_R6, K_MIX, K_MODULATION };

FxUi::FxUi(FxModule *module) :
    my_module(module)
{
    setModule(module);
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);
    if (S::show_screws()) {
        createScrews(theme);
    }
    float x, y;
    bool browsing = !module;

    const float PANEL_WIDTH = 135.f;
    const float CENTER = PANEL_WIDTH*.5f;

    addChild(selector = createThemedParam<SelectorWidget>(Vec(3.5f, 28.f), my_module, FxModule::P_EFFECT, theme_engine, theme));
    addChild(effect_label = createLabel<TextLabel>(Vec(CENTER, 20.f), 100.f, "Short reverb", theme_engine, theme, LabelStyle{"ctl-label", TextAlignment::Center, 16.f, true}));

    // knobs with labels
    const float DY_KNOB = 62.f;
    const float KNOB_DX = 28.f;
    const float KNOB_TOP = 114.f;
    const float LABEL_DY = 18.5f;
    const float KNOB_AXIS = CENTER + 4.f;

    x = KNOB_AXIS + KNOB_DX;
    y = 60.f;
    addChild(knobs[K_MIX] = createChemKnob<BlueKnob>(Vec(x, y), my_module, FxModule::P_MIX, theme_engine, theme));
    addChild(tracks[K_MIX] = createTrackWidget(knobs[K_MIX], theme_engine, theme));
    addChild(mix_light = createLightCentered<SmallSimpleLight<GreenLight>>(Vec(x + 22.f, y-9.f), my_module, FxModule::L_MIX));
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, theme->name);

    x = KNOB_AXIS - KNOB_DX;
    y -= 8.f;
    addChild(Center(createThemedParamLightButton<SmallRoundParamButton, SmallSimpleLight<RedLight>>(
        Vec(x, y), my_module, FxModule::P_DISABLE, FxModule::L_DISABLE, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + 12.f), 80.f, "Fx Off", theme_engine, theme, S::control_label));

    y = KNOB_TOP;
    x = KNOB_AXIS - KNOB_DX;
    for (int i = 0; i < K_MIX; ++i) {
        addChild(knobs[i] = createChemKnob<BasicKnob>(Vec(x, y), my_module, i, theme_engine, theme));
        addChild(tracks[i] = createTrackWidget(knobs[i], theme_engine, theme));

        r_labels[i] = createLabel<TipLabel>(Vec(x, y + LABEL_DY), 80.f, format_string("R%d", 1+i), theme_engine, theme, S::control_label);
        addChild(r_labels[i]);

        if (i & 1) {
            y += DY_KNOB;
            x = KNOB_AXIS - KNOB_DX;
        } else {
            x = KNOB_AXIS + KNOB_DX;
        }
    }

    // inputs
    const NVGcolor co_port = PORT_CORN;
    const float PORT_LEFT = CENTER - (1.5f*S::PORT_DX);
    const float label_width = 20.f;
    y = S::PORT_TOP;
    x = PORT_LEFT;
    addChild(knobs[K_MODULATION] = createChemKnob<TrimPot>(Vec(x, y), module, FxModule::P_MOD_AMOUNT, theme_engine, theme));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, FxModule::IN_MIX, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), label_width, "MIX", theme_engine, theme, S::in_port_label));
    if (my_module){ addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, FxModule::IN_MIX, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, FxModule::L_MIX_MOD));

    x += S::PORT_DX;
    for (int i = 0; i <= K_R6; ++i) {
        addChild(Center(createThemedColorInput(Vec(x, y), my_module, i, S::InputColorKey, co_port, theme_engine, theme)));
        addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), label_width, format_string("R%d", 1+i), theme_engine, theme, S::in_port_label));
        if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, i, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
        addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, i));

        if (i == 1) {
            y += S::PORT_DY;
            x = PORT_LEFT;
        } else {
            x += S::PORT_DX;
        }
    }

    // footer

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
        addChild(createWidgetCentered<Logo>(Vec(CENTER, 150.f)));
    }

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

void FxUi::createScrews(std::shared_ptr<SvgTheme> theme)
{
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2*RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
}

void FxUi::setThemeName(const std::string& name, void * context)
{
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, name);
    Base::setThemeName(name, context);
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
        effect_label->text(pq ? pq->getDisplayValueString() : "Short reverb");

        bool six_param = (Haken::R_shortRev == effect || Haken::R_longRev == effect);
        knobs[K_R5]->enable(six_param);
        knobs[K_R6]->enable(six_param);
        if (!six_param) {
            r_labels[K_R5]->text("—");
            r_labels[K_R6]->text("—");
        }

        switch (effect) {
        case Haken::R_shortRev:
        case Haken::R_longRev:
            r_labels[K_R1]->text("Diffuse");
            r_labels[K_R2]->text("LPF");
            r_labels[K_R3]->text("Damping");
            r_labels[K_R4]->text("Decay");
            r_labels[K_R5]->text("PreDelay");
            r_labels[K_R6]->text("HPF");
            break;
        case Haken::R_modDel:
        case Haken::R_swepEcho:
            r_labels[K_R1]->text("ModDepth");
            r_labels[K_R2]->text("ModRate");
            r_labels[K_R3]->text("Feedback");
            r_labels[K_R4]->text("Time");
            break;
        default:
            switch (effect) {
            case Haken::R_anaEcho:
                r_labels[K_R1]->text("Noise");
                break;
            case Haken::R_digEchoLPF:
                r_labels[K_R1]->text("LPF");
                break;
            case Haken::R_digEchoHPF:
                r_labels[K_R1]->text("HPF");
                break;
            default: r_labels[K_R1]->text("R1");
                break;
            }
            r_labels[K_R2]->text("Offset");
            r_labels[K_R3]->text("Feedback");
            r_labels[K_R4]->text("Time");
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
