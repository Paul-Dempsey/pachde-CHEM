#include "Post.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;
namespace fs = ghc::filesystem;

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
    K_ATTENUVERTER
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
    bool browsing = !module;

    addChild(effect_label = createStaticTextLabel<StaticTextLabel>(Vec(CENTER, 62.f), 100.f, "EQ", theme_engine, theme, LabelStyle{"ctl-label", TextAlignment::Center, 16.f, true}));

    // knobs
    x = CENTER;
    addChild(knobs[K_POST_LEVEL] = createChemKnob<YellowKnob>(Vec(x, 35.f), module, PostModule::P_POST_LEVEL, theme_engine, theme));
    addChild(knobs[K_MIX] = createChemKnob<BlueKnob>(Vec(x, 96.f), module, PostModule::P_MIX, theme_engine, theme));

    const float PARAM_TOP = 136.f;
    const float PARAM_DY = 54.5f;
    const float label_offset = 18.f;
    LabelStyle knob_label_style ={"ctl-label", TextAlignment::Center, 14.f, false};

    y = PARAM_TOP;
    addChild(knobs[K_TILT] = createChemKnob<BasicKnob>(Vec(x, y), module, PostModule::P_TILT, theme_engine, theme));
    addChild(top_knob_label= createStaticTextLabel<StaticTextLabel>(Vec(x,y + label_offset), 100.f, "Tilt", theme_engine, theme, knob_label_style));
    y += PARAM_DY;
    addChild(knobs[K_FREQUENCY] = createChemKnob<BasicKnob>(Vec(x, y), module, PostModule::P_FREQUENCY, theme_engine, theme));
    addChild(mid_knob_label= createStaticTextLabel<StaticTextLabel>(Vec(x,y + label_offset), 100.f, "Frequency", theme_engine, theme, knob_label_style));
    y += PARAM_DY;

    if (browsing) {
        auto logo = new Logo(0.35f);
        logo->box.pos = Vec(CENTER - logo->box.size.x*.5, y - 16.f);
        addChild(logo);
    }

    // inputs
    const NVGcolor co_port = PORT_CORN;
    y = S::PORT_TOP;
    x = CENTER - S::PORT_DX;
    addChild(knobs[K_ATTENUVERTER] = createChemKnob<TrimPot>(Vec(x, y), module, PostModule::P_ATTENUVERT, theme_engine, theme));
    
    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PostModule::IN_MUTE, PORT_RED, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 36.f, "MUTE", theme_engine, theme, S::in_port_label));
    
    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PostModule::IN_POST_LEVEL, co_port, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 36.f, "LVL", theme_engine, theme, S::in_port_label));

    y += S::PORT_DY;
    x = CENTER - S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PostModule::IN_MIX, co_port, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 36.f, "MIX", theme_engine, theme, S::in_port_label));
    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PostModule::IN_TILT, co_port, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 36.f, "TLT", theme_engine, theme, S::in_port_label));
    x += S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, PostModule::IN_FREQUENCY, co_port, theme_engine, theme)));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x, y + S::PORT_LABEL_DY), 36.f, "FRQ", theme_engine, theme, S::in_port_label));

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
        my_module->ui = this;
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
//    blip->set_light_color(ColorFromTheme(getThemeName(), "warning", nvgRGB(0xe7, 0xe7, 0x45)));
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

// void PostUi::step()
// {
//     Base::step();
    
// }

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
