#include "Pre.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace pachde;
namespace fs = ghc::filesystem;

// -- Pre UI -----------------------------------

constexpr const float CENTER = 52.5f;

bool PreUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

enum K { 
    K_PRE_LEVEL, 
    K_MIX, 
    K_THRESH_DRIVE, 
    K_ATTACK_X, 
    K_RATIO_MAKEUP,
    K_ATTENUVERTER
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

    addChild(selector = createThemedParam<SelectorWidget>(Vec(3.5f, 68.f), my_module, PreModule::P_SELECT, theme_engine, theme));
    addChild(effect_label = createStaticTextLabel<StaticTextLabel>(Vec(CENTER, 60.f), 100.f, "", theme_engine, theme, LabelStyle{"ctl-label", TextAlignment::Center, 16.f, true}));
#ifndef NDEBUG
    selector->host = "Pre";
#endif

    // knobs
    x = CENTER;
    addChild(knobs[K_PRE_LEVEL] = createChemKnob<YellowKnob>(Vec(x, 35.f), my_module, PreModule::P_PRE_LEVEL, theme_engine, theme));
    addChild(knobs[K_MIX]       = createChemKnob<BlueKnob>(Vec(x, 96.f), my_module, PreModule::P_MIX, theme_engine, theme));

    const float PARAM_TOP = 135.f;
    const float PARAM_DY = 54.5f;
    const float label_offset = 18.f;
    LabelStyle knob_label_style ={"ctl-label", TextAlignment::Center, 14.f, false};

    y = PARAM_TOP;
    addChild(knobs[K_THRESH_DRIVE] = createChemKnob<BasicKnob>(Vec(x, y), my_module, PreModule::P_THRESHOLD_DRIVE, theme_engine, theme));
    addChild(top_knob_label= createStaticTextLabel<StaticTextLabel>(Vec(x,y + label_offset), 100.f, "", theme_engine, theme, knob_label_style));

    y += PARAM_DY;
    addChild(knobs[K_ATTACK_X] = createChemKnob<BasicKnob>(Vec(x, y), my_module, PreModule::P_ATTACK_, theme_engine, theme));
    addChild(mid_knob_label= createStaticTextLabel<StaticTextLabel>(Vec(x,y + label_offset), 100.f, "", theme_engine, theme, knob_label_style));

    y += PARAM_DY;
    addChild(knobs[K_RATIO_MAKEUP] = createChemKnob<BasicKnob>(Vec(x, y), my_module, PreModule::P_RATIO_MAKEUP, theme_engine, theme));
    addChild(bot_knob_label= createStaticTextLabel<StaticTextLabel>(Vec(x,y + label_offset), 100.f, "", theme_engine, theme, knob_label_style));

    // inputs
    auto co_port = PORT_CORN;
    y = S::PORT_TOP;
    x = CENTER - S::PORT_DX;
    addChild(knobs[K_ATTENUVERTER] = createChemKnob<TrimPot>(Vec(x, y), module, PreModule::P_ATTENUVERT, theme_engine, theme));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x , y), my_module, PreModule::IN_PRE_LEVEL, co_port, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 35, "LVL", theme_engine, theme, S::in_port_label));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PreModule::IN_ATTACK, co_port, theme_engine, theme)));
    addChild(in_attack_x = createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 20, "", theme_engine, theme, S::in_port_label));

    y += S::PORT_DY;
    x = CENTER - S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PreModule::IN_MIX, co_port, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 20, "MIX", theme_engine, theme, S::in_port_label));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PreModule::IN_THRESHOLD, co_port, theme_engine, theme)));
    addChild(in_thresh_drive = createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 20, "", theme_engine, theme, S::in_port_label));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PreModule::IN_RATIO, co_port, theme_engine, theme)));
    addChild(in_ratio_makeup = createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 20, "", theme_engine, theme, S::in_port_label));
    
    // footer
    addChild(warning_label = createStaticTextLabel<TipLabel>(
        Vec(28.f, box.size.y - 22.f), box.size.x, "", theme_engine, theme, S::warning_label));
    warning_label->describe("[warning/status]");
    warning_label->glowing(true);

    addChild(haken_device_label = createStaticTextLabel<TipLabel>(
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
        my_module->ui = this;
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
//    blip->set_light_color(ColorFromTheme(getThemeName(), "warning", nvgRGB(0xe7, 0xe7, 0x45)));
    Base::setThemeName(name, context);
}

void PreUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        //onPresetChange();
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
