#include "Convo.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/click-region-widget.hpp"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

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
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);

    float x, y;
    bool browsing = !module;
    LabelStyle heading_style{"ctl-label-hi", TextAlignment::Left, 14.f, true};

    y = 32.f;
    addChild(createLabel<TextLabel>(Vec(7.5f, y), 25.f, "Pre", theme_engine, theme,  heading_style));
    addChild(createLabel<TextLabel>(Vec(204.f, y), 25.f, "Post", theme_engine, theme, heading_style));

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
            addChild(knobs[p] = createChemKnob<BasicKnob>(Vec(pcx[p], y), my_module, p, theme_engine, theme));
        } else {
            addChild(knobs[p] = createChemKnob<BlueKnob>(Vec(pcx[p], y), my_module, p, theme_engine, theme));
        }
        addChild(tracks[p] = createTrackWidget(knobs[p], theme_engine, theme));
    }
    addChild(extend_button = Center(createThemedParamLightButton<SmallRoundParamButton, SmallSimpleLight<RedLight>>(
        Vec(258.f, 28.f), my_module, CM::P_EXTEND, CM::L_EXTEND, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(258.f, 42.f), 36, "Extend", theme_engine, theme, S::control_label));

    const float KX = 34.f;
    x = KX;
    const float K_DX = 42.f;
    const float WL_DX = 50.f;
    const float IR_DX = 50.f;
    const float ROW_DY = 50.f;
    const float kdx[] { K_DX, K_DX, WL_DX, K_DX, IR_DX, 0.f};
    const char* labels[] { "Length", "Tuning", "Width", "Left", "Right", "IR" };
    for (int i = 0; i < 6; ++i) {
        addChild(createLabel<TextLabel>(Vec(x, 68.f), 30, labels[i], theme_engine, theme, S::control_label));
        x += kdx[i];
    }

    x = KX;
    y = 104.f;
    int iwi = W_IDX_KR;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 6; ++j) {
            auto wi = widget_info[iwi++];
            addChild(knobs[wi.param] = createChemKnob<BasicKnob>(Vec(x, y), my_module, wi.param, theme_engine, theme));
            if (wi.input >= 0) {
                addChild(tracks[wi.input] = createTrackWidget(knobs[wi.param], theme_engine, theme));
            }
            x += kdx[j];
        }
        x = KX;
        y += ROW_DY;
    }

    addChild(knobs[CM::P_MOD_AMOUNT] = createChemKnob<TrimPot>(Vec(210.f, S::PORT_TOP), my_module, CM::P_MOD_AMOUNT, theme_engine, theme));

    // inputs
    auto co_port = PORT_CORN;
    float click_width = 32.25;
    float click_height = 18.f;
    float click_dy = 1.5f;
    const float PP_LEFT = 244;
    const float PORT_MOD_DX = 15.f;
    const float PORT_MOD_DY = 0.f;

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
            addChild(Center(createThemedColorInput(Vec(x, y), my_module, wi.input, S::InputColorKey, co_port, theme_engine, theme)));
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
        addChild(Center(createThemedColorInput(Vec(x, y), my_module, wi.input, S::InputColorKey, co_port, theme_engine, theme)));
        addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, wi.light));
        addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 35.f, port_name[i & 1], theme_engine, theme, S::in_port_label));
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
    addChild(warning_label = createLabel<TipLabel>(
        Vec(28.f, box.size.y - 22.f), box.size.x, "", theme_engine, theme, S::warning_label));
    warning_label->describe("[warning/status]");
    warning_label->glowing(true);

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
        auto logo = new Logo(0.35f);
        logo->box.pos = Vec(CENTER - logo->box.size.x*.5, S::PORT_TOP + S::PORT_DY *.5f);
        addChild(logo);
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

void ConvoUi::glowing_knobs(bool glow) {
    for (int i = 0; i < ConvoModule::NUM_KNOBS; ++i) {
        knobs[i]->glowing(glow);
    }
}

void ConvoUi::setThemeName(const std::string& name, void * context)
{
    Base::setThemeName(name, context);
}

void ConvoUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
    } else {
        haken_device_label->text(S::NotConnected);
    }
}

void ConvoUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    haken_device_label->text(connection ? connection->info.friendly(TextFormatLength::Short) : S::NotConnected);
    haken_device_label->describe(connection ? connection->info.friendly(TextFormatLength::Long) : S::NotConnected);
}

void ConvoUi::step()
{
    Base::step();
    if (!my_module) return;

    knobs[CM::P_MOD_AMOUNT]->enable(my_module->modulation.has_target());

    for (int id = 0; id < CM::P_MOD_AMOUNT; ++id) {
        auto wi = param_info(id);
        if (wi.input < 0) continue;
        tracks[wi.input]->set_value(my_module->modulation.get_port(id).modulated());
        tracks[wi.input]->set_active(my_module->getInput(wi.input).isConnected());
    }

#ifdef LAYOUT_HELP
    if (hints != layout_hinting) {
        layout_hinting = hints;
        for (auto child: children) {
            auto tr = dynamic_cast<ClickRegion*>(child);
            if (tr) tr->visible = layout_hinting;
        }
    }
#endif

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
