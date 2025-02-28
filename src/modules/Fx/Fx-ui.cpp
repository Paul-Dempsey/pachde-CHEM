#include "Fx.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;
namespace fs = ghc::filesystem;

// -- Fx UI -----------------------------------

enum K { K_R1, K_R2, K_R3, K_R4, K_R5, K_R6, K_MIX, K_ATTENUVERTER };

FxUi::FxUi(FxModule *module) :
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

    const float PANEL_WIDTH = 165.f;
    const float CENTER = PANEL_WIDTH*.5f;

    addChild(selector = createThemedParam<SelectorWidget>(Vec(9.f, 30.f), my_module, FxModule::P_EFFECT, theme_engine, theme));
    addChild(effect_label = createStaticTextLabel<TipLabel>(Vec(CENTER, 22.f), 100.f, "Short reverb", theme_engine, theme, LabelStyle{"ctl-label", TextAlignment::Center, 16.f, true}));

    // knobs with labels
    const float DY_KNOB = 60.f;
    const float KNOB_DX = 30.f;
    const float KNOB_TOP = 114.f;
    const float LABEL_DY = 18.f;

    y = KNOB_TOP;
    x = CENTER - KNOB_DX;
    for (int i = 0; i < K_MIX; ++i) {
        knobs[i] = createChemKnob<BasicKnob>(Vec(x, y), my_module, i, theme_engine, theme);
        addChild(knobs[i]);

        r_labels[i] = createStaticTextLabel<TipLabel>(Vec(x, y + LABEL_DY), 80.f, format_string("R%d", 1+i), theme_engine, theme, S::control_label);
        addChild(r_labels[i]);

        if (i & 1) {
            y += DY_KNOB;
            x = CENTER - KNOB_DX;
        } else {
            x = CENTER + KNOB_DX;
        }
    }

    x = CENTER + KNOB_DX;
    y = 70.f;
    knobs[K_MIX] = createChemKnob<BlueKnob>(Vec(x, y), my_module, FxModule::P_MIX, theme_engine, theme);
    addChild(knobs[K_MIX]);
    addChild(mix_light = createLightCentered<SmallSimpleLight<GreenLight>>(Vec(x + 22.f, y-9.f), my_module, FxModule::L_MIX));
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, theme->name);

    x = CENTER - KNOB_DX;
    y -= 8.f;
    addChild(Center(createThemedParamLightButton<SmallRoundParamButton, SmallSimpleLight<RedLight>>(
        Vec(x, y), my_module, FxModule::P_DISABLE, FxModule::L_DISABLE, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + 12.f), 80.f, "Fx Off", theme_engine, theme, S::control_label));

    // inputs
    const NVGcolor co_port = PORT_CORN;
    const float PORT_LEFT = CENTER - (1.5f*S::PORT_DX);

    x = PORT_LEFT + S::PORT_DX;
    y = S::PORT_TOP;
    for (int i = 0; i <= K_R6; ++i) {
        addChild(Center(createThemedColorInput(Vec(x, y), my_module, i, S::InputColorKey, co_port, theme_engine, theme)));
        addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 35.f, format_string("R%d", 1+i), theme_engine, theme, S::in_port_label));
        x += S::PORT_DX;
        if (i == 2) {
            y += S::PORT_DY;
            x = PORT_LEFT + S::PORT_DX;
        }
    }
    y = S::PORT_TOP;
    x = PORT_LEFT;
    addChild(knobs[K_ATTENUVERTER] = createChemKnob<TrimPot>(Vec(x, y), module, FxModule::P_ATTENUVERT, theme_engine, theme));
    y += S::PORT_DY;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, FxModule::IN_MIX, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 35.f, "MIX", theme_engine, theme, S::in_port_label));

    // footer

    addChild(warn = createStaticTextLabel<TipLabel>(
        Vec(28.f, box.size.y - 22.f), box.size.x, "", theme_engine, theme, S::warning_label));
    warn->describe("[warning/status]");
    warn->glowing(true);

    addChild(haken = createStaticTextLabel<TipLabel>(
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

void FxUi::glowing_knobs(bool glow) {
    for (int i = 0; i < FxModule::NUM_KNOBS; ++i) {
        knobs[i]->glowing(glow);
    }
}

void FxUi::setThemeName(const std::string& name, void * context)
{
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, name);
    Base::setThemeName(name, context);
}

void FxUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        //onPresetChange();
    } else {
        haken->text(S::NotConnected);
    }
}

void FxUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    haken->text(connection ? connection->info.friendly(TextFormatLength::Short) : S::NotConnected);
    haken->describe(connection ? connection->info.friendly(TextFormatLength::Long) : S::NotConnected);
}

void FxUi::onPresetChange()
{
}

bool FxUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

void FxUi::step()
{
    Base::step();

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

void FxUi::draw(const DrawArgs& args)
{
    Base::draw(args);
}

void FxUi::appendContextMenu(Menu *menu)
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
