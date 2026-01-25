#include "Convo.hpp"
#include "services/colors.hpp"
#include "em/em-hardware.h"
#include "widgets/click-region-widget.hpp"
#include "widgets/logo-widget.hpp"
#include "widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace pachde;

bool ConvoUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    if (chem_host->host_busy()) return false;
    return true;
}

constexpr const float PANEL_WIDTH = 285.f;
constexpr const float CENTER = PANEL_WIDTH * .5f;
constexpr const float PORT_DX = 26.f;

ConvoUi::ConvoUi(ConvoModule *module) :
    my_module(module)
{
    setModule(module);
    if (module) module->set_chem_ui(this);
    auto theme = getSvgTheme();
    auto panel = createThemedPanel(panelFilename(), &module_svgs);
    panelBorder = attachPartnerPanelBorder(panel);
    setPanel(panel);
    if (style::show_screws()) {
        createScrews();
    }
    float x, y;
    bool browsing = !module;

    y = 32.f;
    addChild(createLabel(Vec(7.5f, y), "Pre", &heading_style, 25.f));
    addChild(createLabel(Vec(204.f, y), "Post", &heading_style, 25.f));

    // knobs

    // Pre/Post Mix/Index
    y = 38.f;
    const float PP_CENTER = 114.f;
    const float knob_dx = 42.f;
    const float center_dx = 22.f;
    float pcx[] {
        PP_CENTER - center_dx - knob_dx,
        PP_CENTER - center_dx,
        PP_CENTER + center_dx,
        PP_CENTER + center_dx + knob_dx
    };
    for (int p = 0; p < 4; ++p) {
        if (p & 1) {
            addChild(knobs[p] = createChemKnob<BasicKnob>(Vec(pcx[p], y), &module_svgs, my_module, p));
        } else {
            addChild(knobs[p] = createChemKnob<BlueKnob>(Vec(pcx[p], y), &module_svgs, my_module, p));
        }
        addChild(tracks[p] = createTrackWidget(knobs[p]));
    }
    addChild(extend_button = Center(createThemedParamLightButton<MediumRoundParamButton, SmallSimpleLight<RedLight>>(
        Vec(258.f, 26.f), &module_svgs, my_module, CM::P_PHASE_CANCEL, CM::L_PHASE_CANCEL)));
    addChild(createLabelCentered(Vec(258.f, 42.f), "Phase", &S::control_label, 36));

    const float KX = 32.f;
    x = KX;
    const float K_DX = 42.f;
    const float WL_DX = 50.f;
    const float IR_DX = 50.f;
    const float ROW_DY = 51.f;
    const float kdx[] { K_DX, K_DX, WL_DX, K_DX, IR_DX, 0.f};
    const char* labels[] { "Length", "Tuning", "Width", "Left", "Right", "IR" };
    for (int i = 0; i < 6; ++i) {
        addChild(createLabelCentered(Vec(x, 68.f), labels[i], &S::control_label, 34));
        x += kdx[i];
    }

    x = KX;
    y = 104.f;
    int iwi = W_IDX_KR;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 6; ++j) {
            auto wi = widget_info[iwi++];
            addChild(knobs[wi.param] = createChemKnob<BasicKnob>(Vec(x, y), &module_svgs, my_module, wi.param));
            if (wi.input >= 0) {
                addChild(tracks[wi.input] = createTrackWidget(knobs[wi.param]));
            }
            x += kdx[j];
        }
        addChild(ir_labels[i] = createLabelCentered(Vec(142, y + 17.f), "", &ir_style, 150.f));
        x = KX;
        y += ROW_DY;
    }

    addChild(knobs[CM::P_MOD_AMOUNT] = createChemKnob<TrimPot>(Vec(210.f, S::PORT_TOP), &module_svgs, my_module, CM::P_MOD_AMOUNT));

    // inputs
    auto co_port = PORT_CORN;
    float click_width = 32.25;
    float click_height = 18.f;
    float click_dy = 1.5f;
    const float PP_LEFT = 244;
    const float PORT_MOD_DX = 15.f;
    const float PORT_MOD_DY = .75f;

    // Main inputs
    const float pdx[] { 34.f, 34.f, 42.f, 34.f, 0.f};
    y = S::PORT_TOP + S::PORT_LABEL_DY + 6.f;
    iwi = W_IDX_KR;
    for (int i = 0; i < 4; ++i) {
        x = KX - 10.f + (i * 12.f);
        for (int j = 0; j < 5; ++j) {
            auto wi = widget_info[iwi++];
            while (wi.input < 0) {
                wi = widget_info[iwi++];
            }
            addChild(Center(createThemedColorInput(Vec(x, y), &module_svgs, my_module, wi.input, S::InputColorKey, co_port)));
            addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - PORT_MOD_DX, y - PORT_MOD_DY), my_module, wi.light));
            if (my_module) {
                addChild(Center(createClickRegion(x, y -click_dy, click_width, click_height, wi.param, [=](int id, int mods) { my_module->set_modulation_target(id); })));
            }
            x += (pdx[j]);
        }
        x = KX;
        y += S::PORT_DY *.5f;
    }

    // Pre/Post
    click_width = PORT_DX;
    click_height = 34.f;
    click_dy = 7.f;
    y = S::PORT_TOP;
    x = PP_LEFT;
    iwi = W_IDX_PP;
    const char * port_name[] {"MIX", "IDX"};
    for (int i = 0; i < 4; ++i, ++iwi) {
        auto wi = widget_info[iwi];
        addChild(Center(createThemedColorInput(Vec(x, y), &module_svgs, my_module, wi.input, S::InputColorKey, co_port)));
        addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, wi.light));
        addChild(createLabelCentered(Vec(x, y + S::PORT_LABEL_DY), port_name[i & 1], &S::in_port_label, 35.f));
        if (my_module) {
            addChild(Center(createClickRegion(x, y -click_dy, click_width, click_height, wi.param, [=](int id, int mods) { my_module->set_modulation_target(id); })));
        }
        if (1 == i) {
            y += S::PORT_DY;
            x = PP_LEFT;
        } else {
            x += PORT_DX;
        }
    }

    // footer

    addChild(haken_device_label = createLabel<TipLabel>(
        Vec(28.f, box.size.y - 13.f), S::NotConnected, &S::haken_label, 200.f));

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y - S::U1), &module_svgs, "Core link");
    if (my_module) {
        link_button->set_handler([=](bool ctrl, bool shift) {
            ModuleBroker::get()->addHostPickerMenu(createMenu(), my_module);
        });
    }
    addChild(link_button);

    // Browsing UI

    if (browsing && S::show_browser_logo()) {
        addChild(createWidgetCentered<OpaqueLogo>(Vec(CENTER, 136)));
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
        onConnectHost(my_module->chem_host);
    }
}

ConvoUi::~ConvoUi() {
    if (my_module) {
        my_module->set_chem_ui(nullptr);
    }
}

void ConvoUi::glowing_knobs(bool glow) {
    for (int i = 0; i < ConvoModule::NUM_KNOBS; ++i) {
        knobs[i]->glowing(glow);
    }
}

void ConvoUi::createScrews()
{
    addChild(createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, 0), &module_svgs));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0), &module_svgs));
    //addChild(createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), &module_svgs));
}

void ConvoUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    onConnectionChange(ChemDevice::Haken, host ? host->host_connection(ChemDevice::Haken) : nullptr);
}

void ConvoUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    onConnectionChangeUiImpl(this, device, connection);
}

void ConvoUi::step()
{
    Base::step();
    if (!my_module) return;
    bind_host(my_module);

    knobs[CM::P_MOD_AMOUNT]->enable(my_module->modulation.has_target());

    for (int p = 0; p < 4; p++) {
        float v = my_module->getParam(CM::P_1_TYPE+p).getValue();
        if (v != last_ir[p]) {
            last_ir[p] = v;
            auto sq = my_module->getParamQuantity(CM::P_1_TYPE+p);
            if (sq) {
                ir_labels[p]->set_text(sq->getDisplayValueString());
            }
        }
    }

    for (int id = 0; id < CM::P_MOD_AMOUNT; ++id) {
        auto wi = param_info(id);
        if (wi.input < 0) continue;
        tracks[wi.input]->set_value(my_module->modulation.get_port(id).modulated());
        tracks[wi.input]->set_active(my_module->getInput(wi.input).isConnected());
    }
}

void ConvoUi::onHoverKey(const HoverKeyEvent &e)
{
    if (my_module) {
        if (e.action == GLFW_PRESS && ((e.mods & RACK_MOD_MASK) == 0)) {
            switch (e.key) {
            case GLFW_KEY_0:
                e.consume(this);
                my_module->modulation.zero_modulation();
                return;
            // case GLFW_KEY_5:
            //     center_knobs();
            //     e.consume(this);
            //     return;
            }
        }
    }
    Base::onHoverKey(e);
}

void ConvoUi::draw(const DrawArgs &args)
{
    Base::draw(args);
    if (!my_module) return;
    Vec pos{15.f,18.f};
    if (my_module->init_from_em) {
        Circle(args.vg, VEC_ARGS(pos), 2, PORT_GREEN);
    } else {
        OpenCircle(args.vg, VEC_ARGS(pos), 2, RampGray(Ramp::G_45));
    }
}

void ConvoUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
    menu->addChild(new MenuSeparator);
    bool unconnected = (my_module->inputs.end() == std::find_if(my_module->inputs.begin(), my_module->inputs.end(), [](Input& in){ return in.isConnected(); }));
    menu->addChild(createMenuItem("Zero modulation", "0", [this](){
            my_module->modulation.zero_modulation();
    }, unconnected));
    menu->addChild(createCheckMenuItem("Glowing knobs", "",
        [this](){ return my_module->glow_knobs; },
        [this](){
            my_module->glow_knobs = !my_module->glow_knobs;
            glowing_knobs(my_module->glow_knobs);
        }
    ));
    Base::appendContextMenu(menu);
}
