#include "Post.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/click-region-widget.hpp"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;

// -- Post UI -----------------------------------

constexpr const float ONEU = 15.f;
constexpr const float HALFU = 7.5f;
constexpr const ssize_t SSIZE_0 = 0;
constexpr const float CENTER = 52.5f;

bool PostUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

enum K { 
    K_POST_LEVEL, 
    K_MIX, 
    K_TILT, 
    K_FREQUENCY, 
    K_MODULATION
};

PostUi::PostUi(PostModule *module) :
    my_module(module)
{
    setModule(module);
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);
    float x, y;
    //bool browsing = !module;
    LabelStyle knob_label_style ={"ctl-label", TextAlignment::Center, 14.f, false};

    // knobs
    x = CENTER;
    addChild(knobs[K_POST_LEVEL] = createChemKnob<YellowKnob>(Vec(x, 35.f), module, PostModule::P_POST_LEVEL, theme_engine, theme));
    addChild(tracks[K_POST_LEVEL] = createTrackWidget(knobs[K_POST_LEVEL], theme_engine, theme));

    // MUTE
    y = 68.f;
    addChild(Center(createThemedParamLightButton<LargeRoundParamButton, SmallLight<RedLight>>(
        Vec(x, y), my_module, PostModule::P_MUTE, PostModule::L_MUTE, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x,y + 14.f), 50.f, "Mute", theme_engine, theme, knob_label_style));
    
    y = 104.f;
    addChild(effect_label = createStaticTextLabel<StaticTextLabel>(Vec(x, y), 100.f, "EQ", theme_engine, theme, LabelStyle{"ctl-label", TextAlignment::Center, 16.f, true}));
    y += 34;
    addChild(knobs[K_MIX] = createChemKnob<BlueKnob>(Vec(x, y), module, PostModule::P_MIX, theme_engine, theme));
    addChild(tracks[K_MIX] = createTrackWidget(knobs[K_MIX], theme_engine, theme));
    addChild(mix_light = createLightCentered<SmallSimpleLight<GreenLight>>(Vec(x + 22.f, y-9.f), my_module, PostModule::L_MIX));
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, theme->name);

    y += 40;
    const float PARAM_DY = 54.5f;
    const float label_offset = 18.f;

    addChild(knobs[K_TILT] = createChemKnob<BasicKnob>(Vec(x, y), module, PostModule::P_TILT, theme_engine, theme));
    addChild(tracks[K_TILT] = createTrackWidget(knobs[K_TILT], theme_engine, theme));
    addChild(top_knob_label= createStaticTextLabel<StaticTextLabel>(Vec(x,y + label_offset), 50.f, "Tilt", theme_engine, theme, knob_label_style));
    y += PARAM_DY;
    addChild(knobs[K_FREQUENCY] = createChemKnob<BasicKnob>(Vec(x, y), module, PostModule::P_FREQUENCY, theme_engine, theme));
    addChild(tracks[K_FREQUENCY] = createTrackWidget(knobs[K_FREQUENCY], theme_engine, theme));
    addChild(mid_knob_label= createStaticTextLabel<StaticTextLabel>(Vec(x,y + label_offset), 80.f, "Frequency", theme_engine, theme, knob_label_style));
    y += PARAM_DY;

    // if (browsing) {
    //     auto logo = new Logo(0.35f);
    //     logo->box.pos = Vec(CENTER - logo->box.size.x*.5, y - 16.f);
    //     addChild(logo);
    // }

    // inputs
    const NVGcolor co_port = PORT_CORN;
    const float click_width = 32.f;
    const float click_height = 21.f;
    const float click_dy = 14.f;
    y = S::PORT_TOP;
    x = CENTER - S::PORT_DX;
    addChild(knobs[K_MODULATION] = createChemKnob<TrimPot>(Vec(x, y), module, PostModule::P_MOD_AMOUNT, theme_engine, theme));
    
    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PostModule::IN_MUTE, S::InputColorKey, PORT_RED, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 36.f, "MUTE", theme_engine, theme, S::in_port_label));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PostModule::IN_POST_LEVEL, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 36.f, "LVL", theme_engine, theme, S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -click_dy, click_width, click_height, PostModule::IN_POST_LEVEL, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PostModule::L_POST_LEVEL_MOD));

    y += S::PORT_DY;
    x = CENTER - S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PostModule::IN_MIX, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 36.f, "MIX", theme_engine, theme, S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -click_dy, click_width, click_height, PostModule::IN_MIX, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PostModule::L_MIX_MOD));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PostModule::IN_TILT, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 36.f, "TLT", theme_engine, theme, S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -click_dy, click_width, click_height, PostModule::IN_TILT, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PostModule::L_TILT_MOD));

    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PostModule::IN_FREQUENCY, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 36.f, "FRQ", theme_engine, theme, S::in_port_label));
    if (my_module) { addChild(Center(createClickRegion(x, y -click_dy, click_width, click_height, PostModule::IN_FREQUENCY, [=](int id, int mods) { my_module->set_modulation_target(id); })));}
    addChild(createLight<TinySimpleLight<GreenLight>>(Vec(x - S::PORT_MOD_DX, y - S::PORT_MOD_DY), my_module, PostModule::L_FREQUENCY_MOD));

    // footer

    addChild(warning_label = createStaticTextLabel<TipLabel>(
        Vec(S::CORE_LINK_TEXT, box.size.y - 22.f), box.size.x, "", theme_engine, theme, S::warning_label));
    warning_label->describe("[warning/status]");
    warning_label->glowing(true);

    addChild(haken_device_label = createStaticTextLabel<TipLabel>(
        Vec(S::CORE_LINK_TEXT, box.size.y - S::CORE_LINK_TEXT_DY), 200.f, S::NotConnected, theme_engine, theme, S::haken_label));

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y-ONEU), theme_engine, theme, "Core link");

    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ui::Menu* menu = createMenu();
            menu->addChild(createMenuLabel("— Link to Core Module —"));
            auto broker = ModuleBroker::get();
            broker->addHostPickerMenu(menu, my_module);
        });
    }
    addChild(link_button);

    // init

    if (!my_module || my_module->glow_knobs) {
        glowing_knobs(true);
    }

    if (my_module) {
        my_module->set_chem_ui(this);
        onConnectHost(my_module->chem_host);
    }
}

void PostUi::glowing_knobs(bool glow) {
    for (int i = 0; i < PostModule::NUM_KNOBS; ++i) {
        knobs[i]->glowing(glow);
    }
}

void PostUi::setThemeName(const std::string& name, void * context)
{
    applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, name);
    Base::setThemeName(name, context);
}

void PostUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
        //onPresetChange();
    } else {
        haken_device_label->text(S::NotConnected);
    }
}

void PostUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    haken_device_label->text(connection ? connection->info.friendly(TextFormatLength::Short) : S::NotConnected);
    haken_device_label->describe(connection ? connection->info.friendly(TextFormatLength::Long) : S::NotConnected);
}

void PostUi::onPresetChange()
{
}

void PostUi::step()
{
    Base::step();
    if (!my_module) return;

    knobs[K_MODULATION]->enable(my_module->modulation.has_target());

    for (int i = 0; i <= K_FREQUENCY; ++i) {
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

void PostUi::draw(const DrawArgs& args)
{
    Base::draw(args);
}

void PostUi::appendContextMenu(Menu *menu)
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
