#include "proto.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/click-region-widget.hpp"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"
#include "../../widgets/hamburger.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;

// -- UI --------------------------------------

struct RawQuantity : Quantity {
    float value;
    void setValue(float v) override { value = v; }
    float getValue() override { return value; }
    float getMinValue() override { return -INFINITY; }
    float getMaxValue() override { return INFINITY; }
};

ProtoUi::ProtoUi(ProtoModule *module) :
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

    // //const float PANEL_WIDTH = 300.f;
    // //const float CENTER = PANEL_WIDTH*.5f;
    auto label_style = LabelStyle{"ctl-label", TextAlignment::Left, 12.f};
    y = 24.f;
    x = 15.f;
    addChild(label = createLabel<TextLabel>(Vec(x,y), 100.f, "", theme_engine, theme, label_style));
    std::vector<std::string> items {
        "Acoustic",
        "Aggressive",
        "Airy",
        "Analog",
        "Arpeggio",
        "Big",
        "Bright",
        "Chords",
        "Clean",
        "Dark",
        "Digital",
        "Distorted",
        "Dry",
        "Echo",
        "Electric",
        "Ensemble",
        "Evolving",
        "FM",
        "Hybrid",
        "Icy",
        "Intimate",
        "Lo-fi",
        "Looping",
        "Layered",
        "Morphing",
        "Metallic",
        "Nature",
        "Noise",
        "Random",
        "Reverberant",
        "Sound Design",
        "Stereo",
        "Shaking",
        "Simple",
        "Soft",
        "Strumming",
        "Synthetic",
        "Warm",
        "Woody"
    };
    bits = new BitsWidget("Character", 10, 42.f, items, theme_engine, theme,
        [=](uint64_t state){
            label->text(format_string("x%0.8llx", state));
        });
    bits->box.pos.x = 15.f;
    bits->box.pos.y = 36;
    bits->setVisible(false);
    addChild(bits);

    addChild(Center(createClickRegion(bits->box.pos.x + bits->box.size.x * .5f, 30.f, 12.f, 12.f, 0, [=](int, int){
        bits->setVisible(!bits->isVisible());
    })));


    // inputs

    // footer

    addChild(haken_device_label = createLabel<TipLabel>(
        Vec(28.f, box.size.y - 13.f), 200.f, S::NotConnected, theme_engine, theme, S::haken_label));

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y - S::U1), theme_engine, theme, "Core link");

    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ModuleBroker::get()->addHostPickerMenu(createMenu(), my_module);
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

    if (my_module) {
        my_module->set_chem_ui(this);
        onConnectHost(my_module->chem_host);
    }
}

void ProtoUi::glowing_knobs(bool glow) {
    // for (int i = 0; i < ProtoModule::NUM_KNOBS; ++i) {
    //     knobs[i]->glowing(glow);
    // }
}

void ProtoUi::center_knobs()
{
    if (!my_module) return;
}

void ProtoUi::setThemeName(const std::string& name, void * context)
{
    //applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, name);
    Base::setThemeName(name, context);
}

void ProtoUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    onConnectionChange(ChemDevice::Haken, host ? host->host_connection(ChemDevice::Haken) : nullptr);
}

void ProtoUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    onConnectionChangeUiImpl(this, device, connection);
}

bool ProtoUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

void ProtoUi::onHoverKey(const HoverKeyEvent &e)
{
    // if (my_module) {
    //     if (e.action == GLFW_PRESS && ((e.mods & RACK_MOD_MASK) == 0)) {
    //         switch (e.key) {
    //         case GLFW_KEY_0:
    //             e.consume(this);
    //             //my_module->modulation.zero_modulation();
    //             return;
    //         case GLFW_KEY_5:
    //             center_knobs();
    //             e.consume(this);
    //             return;
    //         }
    //     }
    // }
    Base::onHoverKey(e);
}

void ProtoUi::step()
{
    Base::step();
    if (!my_module) return;
    bind_host(my_module);

}

void ProtoUi::draw(const DrawArgs &args)
{
    Base::draw(args);
    auto vg = args.vg;
    Circle(vg, bits->box.pos.x + bits->box.size.x * .5f, 30.f, 5.5f, nvgHSL(200.f/360.f, .5f, .5f));
}

void ProtoUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
//    menu->addChild(new MenuSeparator);

//    menu->addChild(createMenuItem("Center knobs", "5", [this](){ center_knobs(); }));

    // menu->addChild(createCheckMenuItem("Glowing knobs", "", 
    //     [this](){ return my_module->glow_knobs; },
    //     [this](){
    //         my_module->glow_knobs = !my_module->glow_knobs; 
    //         glowing_knobs(my_module->glow_knobs);
    //     }
    // ));
    Base::appendContextMenu(menu);
}
