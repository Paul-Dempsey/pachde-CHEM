#include "Pre.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/click-region-widget.hpp"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

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
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);
    float x, y;
    bool browsing = !module;

    comp_type = module ? getParamIndex(my_module->getParamQuantity(PreModule::P_SELECT)) : 0;

    addChild(selector = createThemedParam<SelectorWidget>(Vec(3.5f, 66.f), my_module, PreModule::P_SELECT, theme_engine, theme));
    addChild(effect_label = createLabel<TextLabel>(Vec(CENTER + 3.5f, 58.f), 100.f, "", theme_engine, theme, LabelStyle{"ctl-label", TextAlignment::Center, 16.f, true}));

    // knobs
    x = CENTER;
    addChild(knobs[K_PRE_LEVEL] = createChemKnob<YellowKnob>(Vec(x, 35.f), my_module, PreModule::P_PRE_LEVEL, theme_engine, theme));
    addChild(tracks[K_PRE_LEVEL] = createTrackWidget(knobs[K_PRE_LEVEL], theme_engine, theme));

    addChild(knobs[K_MIX] = createChemKnob<BlueKnob>(Vec(x, 96.f), my_module, PreModule::P_MIX, theme_engine, theme));
    addChild(tracks[K_MIX] = createTrackWidget(knobs[K_MIX], theme_engine, theme));

    addChild(mix_light = createLightCentered<SmallSimpleLight<GreenLight>>(Vec(x + 22.f, 96.f-9.f), my_module, PreModule::L_MIX));
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, theme->name);

    const float PARAM_TOP = 135.f;
    const float PARAM_DY = 54.f;
    const float label_offset = 18.f;
    LabelStyle knob_label_style ={"ctl-label", TextAlignment::Center, 14.f, false};

    y = PARAM_TOP;
    addChild(knobs[K_THRESH_DRIVE] = createChemKnob<BasicKnob>(Vec(x, y), my_module, PreModule::P_THRESHOLD_DRIVE, theme_engine, theme));
    addChild(tracks[K_THRESH_DRIVE] = createTrackWidget(knobs[K_THRESH_DRIVE], theme_engine, theme));
    addChild(top_knob_label= createLabel<TextLabel>(Vec(x,y + label_offset), 100.f, "", theme_engine, theme, knob_label_style));

    y += PARAM_DY;
    addChild(knobs[K_ATTACK_X] = createChemKnob<BasicKnob>(Vec(x, y), my_module, PreModule::P_ATTACK, theme_engine, theme));
    addChild(tracks[K_ATTACK_X] = createTrackWidget(knobs[K_ATTACK_X], theme_engine, theme));
    addChild(mid_knob_label= createLabel<TextLabel>(Vec(x,y + label_offset), 100.f, "", theme_engine, theme, knob_label_style));

    y += PARAM_DY;
    addChild(knobs[K_RATIO_MAKEUP] = createChemKnob<BasicKnob>(Vec(x, y), my_module, PreModule::P_RATIO_MAKEUP, theme_engine, theme));
    addChild(tracks[K_RATIO_MAKEUP] = createTrackWidget(knobs[K_RATIO_MAKEUP], theme_engine, theme));
    addChild(bot_knob_label= createLabel<TextLabel>(Vec(x,y + label_offset), 100.f, "", theme_engine, theme, knob_label_style));

    // inputs
    auto co_port = PORT_CORN;
    y = S::PORT_TOP;
    x = CENTER - S::PORT_DX;
    addChild(knobs[K_MODULATION] = createChemKnob<TrimPot>(Vec(x, y), module, PreModule::P_MOD_AMOUNT, theme_engine, theme));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x , y), my_module, PreModule::IN_PRE_LEVEL, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 35, "LVL", theme_engine, theme, S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PreModule::IN_PRE_LEVEL, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PreModule::L_PRE_LEVEL_MOD));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PreModule::IN_ATTACK, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(in_attack_x = createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 20, "", theme_engine, theme, S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PreModule::IN_ATTACK, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PreModule::L_ATTACK_MOD));

    y += S::PORT_DY;
    x = CENTER - S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PreModule::IN_MIX, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 20, "MIX", theme_engine, theme, S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PreModule::IN_MIX, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PreModule::L_MIX_MOD));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PreModule::IN_THRESHOLD_DRIVE, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(in_thresh_drive = createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 20, "", theme_engine, theme, S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PreModule::IN_THRESHOLD_DRIVE, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PreModule::L_THRESHOLD_DRIVE_MOD));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PreModule::IN_RATIO_MAKEUP, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(in_ratio_makeup = createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 20, "", theme_engine, theme, S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -S::CLICK_DY, S::CLICK_WIDTH, S::CLICK_HEIGHT, PreModule::IN_RATIO_MAKEUP, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PreModule::L_RATIO_MAKEUP_MOD));
    
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
    }

    // init

    if (!my_module || my_module->glow_knobs) {
        glowing_knobs(true);
    }

    comp_type = -1; // force refresh
    sync_labels();

    if (my_module) {
        my_module->set_chem_ui(this);
        onConnectHost(my_module->chem_host);
    }
}

void PreUi::glowing_knobs(bool glow) {
    for (int i = 0; i < PreModule::NUM_KNOBS; ++i) {
        knobs[i]->glowing(glow);
    }
    selector->bright = glow;
}

void PreUi::setThemeName(const std::string& name, void * context)
{
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, name);
    Base::setThemeName(name, context);
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
    if (device != ChemDevice::Haken) return;
    haken_device_label->text(connection ? connection->info.friendly(TextFormatLength::Short) : S::NotConnected);
    haken_device_label->describe(connection ? connection->info.friendly(TextFormatLength::Long) : S::NotConnected);
}

void PreUi::onPresetChange()
{
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

void PreUi::step()
{
    Base::step();
    if (!my_module) return;
    sync_labels();
    knobs[K_MODULATION]->enable(my_module->modulation.has_target());

    for (int i = 0; i < K_MODULATION; ++i) {
        tracks[i]->set_value(my_module->modulation.get_port(i).modulated());
        tracks[i]->set_active(my_module->getInput(i).isConnected());
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

void PreUi::draw(const DrawArgs& args)
{
    Base::draw(args);
}

void PreUi::appendContextMenu(Menu *menu)
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
