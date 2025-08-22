#include "Jack.hpp"
#include "../../widgets/uniform-style.hpp"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/flip-switch.hpp"
#include "../../widgets/theme-knob.hpp"

namespace S = pachde::style;
using namespace pachde;

// -- Jack UI -----------------------------------

bool JackUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

constexpr const float CENTER = 45.f*.5f;
constexpr const float LABEL_DY = 10.f;
constexpr const float KNOBS_DY = 20.f;
constexpr const float PEDAL_DY = 21.f;

JackUi::JackUi(JackModule *module) :
    my_module(module),
    last_1(-1.f),
    last_2(-1.f),
    last_p1(-1),
    last_p2(-1)
{
    setModule(module);
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);

    symbols.add_source("res/symbols/pedal-none.svg");
    symbols.add_source("res/symbols/pedal-switch.svg");
    symbols.add_source("res/symbols/pedal-expression.svg");
    symbols.add_source("res/symbols/pedal-damper.svg");
    symbols.add_source("res/symbols/pedal-tristate.svg");
    symbols.add_source("res/symbols/pedal-cv.svg");
    symbols.add_source("res/symbols/pedal-pot.svg");
    symbols.add_source("res/symbols/pedal-other.svg");
    symbols.applyTheme(theme_engine, theme);

    float x, y;

    auto assign_style = S::pedal_label;
    assign_style.align = TextAlignment::Center;
    assign_style.height = 10.f;

    LabelStyle knob_label_style ={"ctl-label", TextAlignment::Center, 10.f, false};

    x = CENTER;
    y = 34.f;
    addChild(Center(createThemedParam<JackMenu>(Vec(x, y), my_module, JackModule::P_ASSIGN_JACK_1, theme_engine, theme)));
    y += 8.f;
    addChild(assign_1_label = createLabel<TextLabel>(Vec(x, y), box.size.x, "", theme_engine, theme, assign_style));

    y += KNOBS_DY;
    addChild(createChemKnob<TrimPot>(Vec(12.f,y), my_module, JackModule::P_MIN_JACK_1, theme_engine, theme));
    addChild(createChemKnob<TrimPot>(Vec(32.f,y), my_module, JackModule::P_MAX_JACK_1, theme_engine, theme));
    y += LABEL_DY;
    addChild(createLabel<TextLabel>(Vec(12, y), 20.f, "min", theme_engine, theme, knob_label_style));
    addChild(createLabel<TextLabel>(Vec(32, y), 20.f, "max", theme_engine, theme, knob_label_style));

    y += PEDAL_DY;
    pedal_image_1 = new SymbolSetWidget(&symbols);
    pedal_image_1->box.pos = Vec(CENTER, y);
    pedal_image_1->set_index(0);
    addChild(Center(pedal_image_1));

    y = 130.f;
    addChild(Center(createThemedParam<JackMenu>(Vec(x, y), my_module, JackModule::P_ASSIGN_JACK_2, theme_engine, theme)));
    y += 8.f;
    addChild(assign_2_label = createLabel<TextLabel>(Vec(x, y), box.size.x, "", theme_engine, theme, assign_style));

    y += KNOBS_DY;
    addChild(createChemKnob<TrimPot>(Vec(12.f, y), my_module, JackModule::P_MIN_JACK_2, theme_engine, theme));
    addChild(createChemKnob<TrimPot>(Vec(32.f, y), my_module, JackModule::P_MAX_JACK_2, theme_engine, theme));
    y += LABEL_DY;
    addChild(createLabel<TextLabel>(Vec(12, y), 15.f, "min", theme_engine, theme, knob_label_style));
    addChild(createLabel<TextLabel>(Vec(32, y), 15.f, "max", theme_engine, theme, knob_label_style));

    y += PEDAL_DY;
    if (module) {
        pedal_image_2 = new SymbolSetWidget(&symbols);
        pedal_image_2->box.pos = Vec(CENTER, y);
        pedal_image_2->set_index(0);
        addChild(Center(pedal_image_2));
    } else if (S::show_browser_logo()) {
        auto logo = new OpaqueLogo(.35f);
        logo->box.pos = Vec(CENTER, y);
        addChild(Center(logo));
    }

    y = 212.f;
    addChild(Center(createThemedParam<ShiftMenu>(Vec(x, y), my_module, JackModule::P_SHIFT, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + 6.f), 40.f, "Shift", theme_engine, theme, S::med_control_label));

    y += 27.f;
    addChild(Center(createThemedParam<HamParam>(Vec(x,y), my_module, JackModule::P_SHIFT_ACTION, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + 6.f), 40.f, "Action", theme_engine, theme, S::med_control_label));

    y += 27.f;
    addChild(Center(createThemedParamButton<CheckParamButton>(Vec(x,y), my_module, JackModule::P_KEEP, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + 6.f), 40.f, "Keep", theme_engine, theme, S::med_control_label));

    // outputs
    auto co_port = PORT_ORANGE;

    x = CENTER;
    y = S::PORT_TOP;
    addChild(level_1 = createWidgetCentered<LevelWidget>(Vec(CENTER - 14.25f, y - 5.f)));
    addChild(level_2 = createWidgetCentered<LevelWidget>(Vec(CENTER + 14.25f, y - 5.f + S::PORT_DY)));

    addChild(Center(createThemedColorOutput(Vec(x , y), my_module, JackModule::OUT_JACK_1, S::OutputColorKey, co_port, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 8, "1", theme_engine, theme, S::in_port_label));

    y += S::PORT_DY;
    addChild(Center(createThemedColorOutput(Vec(x, y), my_module, JackModule::OUT_JACK_2, S::OutputColorKey, co_port, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 8, "2", theme_engine, theme, S::in_port_label));

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y - S::U1), theme_engine, theme, "Core link");
    addChild(link = createIndicatorCentered(30.f,box.size.y-9.f, RampGray(G_50), "[connection]"));
    link->setFill(false);

    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ModuleBroker::get()->addHostPickerMenu(createMenu(), my_module);
        });
    }
    addChild(link_button);

    // init
    sync_labels();

    if (my_module) {
        my_module->set_chem_ui(this);
        if (!chem_host) {
            onConnectHost(my_module->chem_host);
        }
    }
}


void JackUi::setThemeName(const std::string& name, void * context)
{
    Base::setThemeName(name, context);
}

void JackUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    onConnectionChange(ChemDevice::Haken, host ? host->host_connection(ChemDevice::Haken) : nullptr);
}

void JackUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    if (connection) {
        link->describe(connection->info.friendly(NameFormat::Long));
        link->setLook(PORT_BLUE);
    } else {
        link->describe("[not connected]");
        link->setLook(RampGray(G_50), false);
    }
}

void JackUi::onPresetChange()
{
}

void JackUi::sync_labels()
{
    if (my_module) {
        float current = my_module->getParam(JackModule::P_ASSIGN_JACK_1).getValue();
        if (current != last_1) {
            last_1 = current;
            auto q = dynamic_cast<JackQuantity*>(my_module->getParamQuantity(JackModule::P_ASSIGN_JACK_1));
            if (q) {
                assign_1_label->text(q->getDisplayValueString());
            }
        }

        current = my_module->getParam(JackModule::P_ASSIGN_JACK_2).getValue();
        if (current != last_2) {
            last_2 = current;
            auto q = dynamic_cast<JackQuantity*>(my_module->getParamQuantity(JackModule::P_ASSIGN_JACK_2));
            if (q) {
                assign_2_label->text(q->getDisplayValueString());
            }
        }

        if (chem_host) {
            auto em = chem_host->host_matrix();
            if (!em) return;
            auto pt = em->get_pedal_types();
            auto p1 = pt & Haken::bPedType0;
            auto p2 = (pt & Haken::bPedType1) >> Haken::sPedType1;
            if (last_p1 != p1) {
                pedal_image_1->set_index(p1);
                last_p1 = p1;
            }
            if (last_p2 != p2) {
                pedal_image_2->set_index(p2);
                last_p2 = p2;
            }
        }
    }
}

void JackUi::step()
{
    Base::step();
    if (!my_module) return;
    bind_host(my_module);
    if (chem_host) {
        auto em = chem_host->host_matrix();
        if (em) {
            level_1->level = uint8_t(em->get_jack_1() >> 7);
            level_2->level = uint8_t(em->get_jack_2() >> 7);
        }
    }
    sync_labels();
}

void JackUi::draw(const DrawArgs& args)
{
    Base::draw(args);
}

void JackUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
    Base::appendContextMenu(menu);
}
