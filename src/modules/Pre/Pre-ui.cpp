#include "Pre.hpp"
#include "services/colors.hpp"
#include "em/em-hardware.h"
#include "widgets/click-region-widget.hpp"
#include "widgets/logo-widget.hpp"
#include "widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace pachde;

// -- Pre UI -----------------------------------

constexpr const float CENTER = 45.f;

bool PreUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return my_module->connected();
}

enum K {
    K_PRE_LEVEL,
    K_MIX,
    K_THRESH_DRIVE,
    K_ATTACK_X,
    K_RATIO_MAKEUP,
    K_MODULATION
};

PreUi::PreUi(PreModule *module) :
    my_module(module),
    comp_type(-1)
{
    setModule(module);
    auto theme = getSvgTheme();
    auto panel = createThemedPanel(panelFilename(), &module_svgs);
    panelBorder = attachPartnerPanelBorder(panel);
    setPanel(panel);
    float x, y;
    bool browsing = !module;

    comp_type = module ? getParamIndex(my_module->getParamQuantity(PreModule::P_SELECT)) : 0;

    addChild(selector = createParam<SelectorWidget>(Vec(3.5f, 66.f), my_module, PreModule::P_SELECT));
    addChild(effect_label = createLabel<TextLabel>(Vec(CENTER + 3.5f, 58.f), 90.f, "", LabelStyle{"ctl-label", TextAlignment::Center, 16.f, true}));

    // knobs
    x = CENTER;
    addChild(knobs[K_PRE_LEVEL] = createChemKnob<YellowKnob>(Vec(x, 35.f), &module_svgs, my_module, PreModule::P_PRE_LEVEL));
    addChild(tracks[K_PRE_LEVEL] = createTrackWidget(knobs[K_PRE_LEVEL]));

    addChild(knobs[K_MIX] = createChemKnob<BlueKnob>(Vec(x, 96.f), &module_svgs, my_module, PreModule::P_MIX));
    addChild(tracks[K_MIX] = createTrackWidget(knobs[K_MIX]));

    addChild(mix_light = createLightCentered<SmallSimpleLight<GreenLight>>(Vec(x + 22.f, 96.f-9.f), my_module, PreModule::L_MIX));
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, theme);

    const float PARAM_TOP = 135.f;
    const float PARAM_DY = 54.f;
    const float label_offset = 18.f;
    LabelStyle knob_label_style ={"ctl-label", TextAlignment::Center, 14.f, false};

    y = PARAM_TOP;
    addChild(knobs[K_THRESH_DRIVE] = createChemKnob<BasicKnob>(Vec(x, y), &module_svgs, my_module, PreModule::P_THRESHOLD_DRIVE));
    addChild(tracks[K_THRESH_DRIVE] = createTrackWidget(knobs[K_THRESH_DRIVE]));
    addChild(top_knob_label= createLabel<TextLabel>(Vec(x,y + label_offset), 90.f, "", knob_label_style));

    y += PARAM_DY;
    addChild(knobs[K_ATTACK_X] = createChemKnob<BasicKnob>(Vec(x, y), &module_svgs, my_module, PreModule::P_ATTACK));
    addChild(tracks[K_ATTACK_X] = createTrackWidget(knobs[K_ATTACK_X]));
    addChild(mid_knob_label= createLabel<TextLabel>(Vec(x,y + label_offset), 90.f, "", knob_label_style));

    y += PARAM_DY;
    addChild(knobs[K_RATIO_MAKEUP] = createChemKnob<BasicKnob>(Vec(x, y), &module_svgs, my_module, PreModule::P_RATIO_MAKEUP));
    addChild(tracks[K_RATIO_MAKEUP] = createTrackWidget(knobs[K_RATIO_MAKEUP]));
    addChild(bot_knob_label= createLabel<TextLabel>(Vec(x,y + label_offset), 90.f, "", knob_label_style));

    // inputs
    auto co_port = PORT_CORN;
    y = S::PORT_TOP;
    x = CENTER - S::PORT_DX;
    addChild(knobs[K_MODULATION] = createChemKnob<TrimPot>(Vec(x, y), &module_svgs, module, PreModule::P_MOD_AMOUNT));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x , y), &module_svgs, my_module, PreModule::IN_PRE_LEVEL, S::InputColorKey, co_port)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 20.f, "LVL", S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PreModule::IN_PRE_LEVEL, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PreModule::L_PRE_LEVEL_MOD));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), &module_svgs, my_module, PreModule::IN_ATTACK, S::InputColorKey, co_port)));
    addChild(in_attack_x = createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 20.f, "", S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PreModule::IN_ATTACK, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PreModule::L_ATTACK_MOD));

    y += S::PORT_DY;
    x = CENTER - S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), &module_svgs, my_module, PreModule::IN_MIX, S::InputColorKey, co_port)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 20.f, "MIX", S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PreModule::IN_MIX, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PreModule::L_MIX_MOD));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), &module_svgs, my_module, PreModule::IN_THRESHOLD_DRIVE, S::InputColorKey, co_port)));
    addChild(in_thresh_drive = createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 20, "", S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PreModule::IN_THRESHOLD_DRIVE, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PreModule::L_THRESHOLD_DRIVE_MOD));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), &module_svgs, my_module, PreModule::IN_RATIO_MAKEUP, S::InputColorKey, co_port)));
    addChild(in_ratio_makeup = createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 20, "", S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PreModule::IN_RATIO_MAKEUP, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PreModule::L_RATIO_MAKEUP_MOD));

    // footer

    addChild(haken_device_label = createLabel<TipLabel>(Vec(28.f, box.size.y - 13.f), 200.f, S::NotConnected, S::haken_label));

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y - S::U1), &module_svgs, "Core link");
    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ModuleBroker::get()->addHostPickerMenu(createMenu(), my_module);
        });
    }
    addChild(link_button);

    // Browsing UI

    if (browsing && S::show_browser_logo()) {
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

    comp_type = -1; // force refresh
    sync_labels();

    if (my_module) {
        my_module->set_chem_ui(this);
        if (!chem_host) {
            onConnectHost(my_module->chem_host);
        }
    }
}

void PreUi::glowing_knobs(bool glow) {
    for (int i = 0; i < PreModule::NUM_KNOBS; ++i) {
        knobs[i]->glowing(glow);
    }
    selector->bright = glow;
}

void PreUi::center_knobs()
{
    if (!my_module) return;
    for (int i = 1; i < 5; ++i) {
        my_module->getParam(i).setValue(5.0f);
    }
}

void PreUi::setThemeName(const std::string& name, void * context)
{
    Base::setThemeName(name, context);
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, getSvgTheme());
}

void PreUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
    } else {
        haken_device_label->text(S::NotConnected);
    }
}

void PreUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    onConnectionChangeUiImpl(this, device, connection);
}

void PreUi::sync_labels()
{
    if (my_module) {
        auto pq = my_module->getParamQuantity(PreModule::P_SELECT);
        if (pq) {
            auto current = getParamIndex(pq);
            if (current == comp_type) return;
            comp_type = current;
        }
    }
    if (comp_type == -1) comp_type = 0;
    if (comp_type > 0){
        effect_label   ->text("Tanh");

        top_knob_label ->text("Drive");
        mid_knob_label ->text("—");
        knobs[K_ATTACK_X]->enable(false);
        bot_knob_label ->text("Makeup");

        in_attack_x    ->text("—");
        in_thresh_drive->text("D");
        in_ratio_makeup->text("M");
    } else {
        effect_label   ->text("Compressor");

        top_knob_label ->text("Threshhold");
        mid_knob_label ->text("Attack");
        knobs[K_ATTACK_X]->enable(true);
        bot_knob_label ->text("Ratio");

        in_attack_x    ->text("A");
        in_thresh_drive->text("TH");
        in_ratio_makeup->text("R");
    }

}

void PreUi::onHoverKey(const HoverKeyEvent &e)
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

void PreUi::step()
{
    Base::step();
    if (!my_module) return;
    bind_host(my_module);

    sync_labels();
    knobs[K_MODULATION]->enable(my_module->modulation.has_target());

    for (int i = 0; i < K_MODULATION; ++i) {
        tracks[i]->set_value(my_module->modulation.get_port(i).modulated());
        tracks[i]->set_active(my_module->getInput(i).isConnected());
    }

}

void PreUi::appendContextMenu(Menu *menu)
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
