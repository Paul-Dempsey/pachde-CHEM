#include "Jack.hpp"
#include "../../widgets/uniform-style.hpp"
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
constexpr const float PEDAL_DY = 24.f;

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
    bool browsing = !module;

    auto assign_style = S::pedal_label;
    assign_style.align = TextAlignment::Center;
    assign_style.height = 10.f;

    LabelStyle knob_label_style ={"ctl-label", TextAlignment::Center, 10.f, false};
    
    x = CENTER;
    y = 40.f;
    addChild(Center(createThemedParam<FlipSwitch>(Vec(x, y), my_module, JackModule::P_ASSIGN_JACK_1, theme_engine, theme)));
    y += LABEL_DY;
    addChild(assign_1_label = createStaticTextLabel<StaticTextLabel>(Vec(x, y), box.size.x, "", theme_engine, theme, assign_style));

    y += KNOBS_DY;
    addChild(createChemKnob<TrimPot>(Vec(12.f,y), my_module, JackModule::P_MIN_JACK_1, theme_engine, theme));
    addChild(createChemKnob<TrimPot>(Vec(32.f,y), my_module, JackModule::P_MAX_JACK_1, theme_engine, theme));
    y += LABEL_DY;
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(12, y), 20.f, "min", theme_engine, theme, knob_label_style));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(32, y), 20.f, "max", theme_engine, theme, knob_label_style));

    y += PEDAL_DY;
    pedal_image_1 = new SymbolSetWidget(&symbols);
    pedal_image_1->box.pos = Vec(CENTER, y);
    pedal_image_1->set_index(0);
    addChild(Center(pedal_image_1));

    y = 150.f;
    addChild(Center(createThemedParam<FlipSwitch>(Vec(x, y), my_module, JackModule::P_ASSIGN_JACK_2, theme_engine, theme)));
    y += LABEL_DY;
    addChild(assign_2_label = createStaticTextLabel<StaticTextLabel>(Vec(x, y), box.size.x, "", theme_engine, theme, assign_style));

    y += KNOBS_DY;
    addChild(createChemKnob<TrimPot>(Vec(12.f, y), my_module, JackModule::P_MIN_JACK_2, theme_engine, theme));
    addChild(createChemKnob<TrimPot>(Vec(32.f, y), my_module, JackModule::P_MAX_JACK_2, theme_engine, theme));
    y += LABEL_DY;
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(12, y), 20.f, "min", theme_engine, theme, knob_label_style));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(32, y), 20.f, "max", theme_engine, theme, knob_label_style));

    y += PEDAL_DY;
    pedal_image_2 = new SymbolSetWidget(&symbols);
    pedal_image_2->box.pos = Vec(CENTER, y);
    pedal_image_2->set_index(0);
    addChild(Center(pedal_image_2));

    y += 34.f;
    addChild(Center(createThemedParamLightButton<SmallRoundParamButton, SmallSimpleLight<GreenLight>>(
        Vec(x, y), my_module, JackModule::P_KEEP, JackModule::L_KEEP, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y+9.f), 80.f, "Keep", theme_engine, theme, S::control_label));

    // outputs
    auto co_port = PORT_ORANGE;
    x = CENTER;
    y = S::PORT_TOP;

    addChild(Center(createThemedColorOutput(Vec(x , y), my_module, JackModule::OUT_JACK_1, S::OutputColorKey, co_port, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 16, "1", theme_engine, theme, S::in_port_label));

    y += S::PORT_DY;
    addChild(Center(createThemedColorOutput(Vec(x, y), my_module, JackModule::OUT_JACK_2, S::OutputColorKey, co_port, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 16, "2", theme_engine, theme, S::in_port_label));

    // footer
    addChild(warning_label = createStaticTextLabel<TipLabel>(
        Vec(7.5f, box.size.y - 22.f), box.size.x, "", theme_engine, theme, S::warning_label));
    warning_label->describe("[warning/status]");
    warning_label->glowing(true);

    addChild(haken_device_label = createStaticTextLabel<TipLabel>(
        Vec(20.f, box.size.y - 13.f), 200.f, S::NotConnected, theme_engine, theme, S::haken_label));

    link_button = createThemedButton<LinkButton>(Vec(5.f, box.size.y - S::U1), theme_engine, theme, "Core link");
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
    sync_labels();

    if (my_module) {
        my_module->ui = this;
        my_module->set_chem_ui(this);
        onConnectHost(my_module->chem_host);
    }
}


void JackUi::setThemeName(const std::string& name, void * context)
{
//    blip->set_light_color(ColorFromTheme(getThemeName(), "warning", nvgRGB(0xe7, 0xe7, 0x45)));
    Base::setThemeName(name, context);
}

void JackUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        //onPresetChange();
    } else {
        haken_device_label->text(S::NotConnected);
    }
}

void JackUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    haken_device_label->text(connection ? connection->info.friendly(TextFormatLength::Short) : S::NotConnected);
    haken_device_label->describe(connection ? connection->info.friendly(TextFormatLength::Long) : S::NotConnected);
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
            auto q = dynamic_cast<SwitchQuantity*>(my_module->getParamQuantity(JackModule::P_ASSIGN_JACK_1));
            if (q) {
                assign_1_label->text(q->getDisplayValueString());
            }
        }

        current = my_module->getParam(JackModule::P_ASSIGN_JACK_2).getValue();
        if (current != last_2) {
            last_2 = current;
            auto q = dynamic_cast<SwitchQuantity*>(my_module->getParamQuantity(JackModule::P_ASSIGN_JACK_2));
            if (q) {
                assign_2_label->text(q->getDisplayValueString());
            }
        }

        if (chem_host) {
            auto pt = chem_host->host_matrix()->get_pedal_types();
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
