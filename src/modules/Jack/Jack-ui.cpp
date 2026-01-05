#include "Jack.hpp"
#include "widgets/uniform-style.hpp"
#include "widgets/logo-widget.hpp"
#include "widgets/flip-switch.hpp"
#include "widgets/theme-knob.hpp"

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
    auto theme = getSvgTheme();
    auto panel = createThemedPanel(panelFilename(), &module_svgs);
    panelBorder = attachPartnerPanelBorder(panel);
    setPanel(panel);

    symbols.add_source("res/symbols/pedal-none.svg");
    symbols.add_source("res/symbols/pedal-switch.svg");
    symbols.add_source("res/symbols/pedal-expression.svg");
    symbols.add_source("res/symbols/pedal-damper.svg");
    symbols.add_source("res/symbols/pedal-tristate.svg");
    symbols.add_source("res/symbols/pedal-cv.svg");
    symbols.add_source("res/symbols/pedal-pot.svg");
    symbols.add_source("res/symbols/pedal-other.svg");
    symbols.loadSvg(&module_svgs);

    float x, y;


    x = CENTER;
    y = 34.f;
    addChild(createParamCentered<JackMenu>(Vec(x, y), my_module, JackModule::P_ASSIGN_JACK_1));
    y += 8.f;
    addChild(assign_1_label = createLabel(Vec(x, y), "", &S::pedal_label_center, box.size.x));

    y += KNOBS_DY;
    addChild(createChemKnob<TrimPot>(Vec(12.f,y), &module_svgs, my_module, JackModule::P_MIN_JACK_1));
    addChild(createChemKnob<TrimPot>(Vec(32.f,y), &module_svgs, my_module, JackModule::P_MAX_JACK_1));
    y += LABEL_DY;
    addChild(createLabel(Vec(12, y), "min",&knob_label_style, 20.f));
    addChild(createLabel(Vec(32, y), "max",&knob_label_style, 20.f));

    y += PEDAL_DY;
    pedal_image_1 = new SymbolSetWidget(&symbols);
    pedal_image_1->box.pos = Vec(CENTER, y);
    pedal_image_1->set_index(0);
    addChild(Center(pedal_image_1));

    y = 130.f;
    addChild(createParamCentered<JackMenu>(Vec(x, y), my_module, JackModule::P_ASSIGN_JACK_2));
    y += 8.f;
    addChild(assign_2_label = createLabel(Vec(x, y), "", &S::pedal_label_center, box.size.x));

    y += KNOBS_DY;
    addChild(createChemKnob<TrimPot>(Vec(12.f, y), &module_svgs, my_module, JackModule::P_MIN_JACK_2));
    addChild(createChemKnob<TrimPot>(Vec(32.f, y), &module_svgs, my_module, JackModule::P_MAX_JACK_2));
    y += LABEL_DY;
    addChild(createLabel(Vec(12, y), "min", &knob_label_style, 15.f));
    addChild(createLabel(Vec(32, y), "max", &knob_label_style, 15.f));

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
    addChild(createParamCentered<ShiftMenu>(Vec(x, y), my_module, JackModule::P_SHIFT));
    addChild(createLabel(Vec(x, y + 6.f), "Shift", &S::med_label, 40.f));

    y += 27.f;
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, JackModule::P_SHIFT_ACTION));
    addChild(createLabel(Vec(x, y + 6.f), "Action", &S::med_label, 40.f));

    y += 27.f;
    addChild(Center(createThemedParamButton<CheckParamButton>(Vec(x,y), &module_svgs, my_module, JackModule::P_KEEP)));
    addChild(createLabel(Vec(x, y + 6.f), "Keep", &S::med_label, 40.f));

    // outputs
    auto co_port = PORT_ORANGE;

    x = CENTER;
    y = S::PORT_TOP;
    addChild(level_1 = createWidgetCentered<LevelWidget>(Vec(CENTER - 14.25f, y - 5.f)));
    addChild(level_2 = createWidgetCentered<LevelWidget>(Vec(CENTER + 14.25f, y - 5.f + S::PORT_DY)));

    addChild(Center(createThemedColorOutput(Vec(x , y), &module_svgs, my_module, JackModule::OUT_JACK_1, S::OutputColorKey, co_port)));
    addChild(createLabel(Vec(x, y + S::PORT_LABEL_DY), "1", &S::out_port_label, 8));

    y += S::PORT_DY;
    addChild(Center(createThemedColorOutput(Vec(x, y), &module_svgs, my_module, JackModule::OUT_JACK_2, S::OutputColorKey, co_port)));
    addChild(createLabel(Vec(x, y + S::PORT_LABEL_DY), "2", &S::out_port_label, 8));

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y - S::U1), &module_svgs, "Core link");
    addChild(link = createIndicatorCentered(30.f,box.size.y-9.f, RampGray(G_50), "[connection]"));
    link->setFill(false);

    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ModuleBroker::get()->addHostPickerMenu(createMenu(), my_module);
        });
    }
    addChild(link_button);

    module_svgs.changeTheme(theme);
    applyChildrenTheme(this, theme);

    // init
    sync_labels();

    if (my_module) {
        my_module->set_chem_ui(this);
        if (!chem_host) {
            onConnectHost(my_module->chem_host);
        }
    }
}

JackUi::~JackUi(){
    if (my_module) my_module->set_chem_ui(nullptr);
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

void JackUi::sync_labels()
{
    if (!my_module || !my_module->initialized) return;

    float current = my_module->getParam(JackModule::P_ASSIGN_JACK_1).getValue();
    if (current != last_1) {
        last_1 = current;
        auto q = dynamic_cast<JackQuantity*>(my_module->getParamQuantity(JackModule::P_ASSIGN_JACK_1));
        if (q && (0 != q->getValue())) {
            assign_1_label->set_text(q->getDisplayValueString());
        } else {
            assign_1_label->set_text("");
        }
    }

    current = my_module->getParam(JackModule::P_ASSIGN_JACK_2).getValue();
    if (current != last_2) {
        last_2 = current;
        auto q = dynamic_cast<JackQuantity*>(my_module->getParamQuantity(JackModule::P_ASSIGN_JACK_2));
        assign_2_label->set_text(q && (0 != q->getValue()) ? q->getDisplayValueString() : "");
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

void JackUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
    Base::appendContextMenu(menu);
}
