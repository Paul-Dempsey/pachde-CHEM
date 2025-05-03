#include "Overlay.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/click-region-widget.hpp"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/text-input.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;

// -- UI --------------------------------------

OverlayUi::OverlayUi(OverlayModule *module) :
    my_module(module)
{
    setModule(module);
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);
    
    bool browsing = !module;

    bg_widget = createWidget<Swatch>(Vec(0.f, 15.f));
    bg_widget->box.size = Vec(30.f, 350.f);
    if (my_module) {
        bg_widget->color = my_module->bg_color;
    }
    addChild(bg_widget);

    addChild(Center(createLight<SmallLight<GreenLight>>(Vec(15.f, 15.f), my_module, OverlayModule::L_CONNECTED)));

    title_widget = createWidget<VText>(Vec(0, 15));
    title_widget->set_size(Vec(30.f, 350.f));
    title_widget->set_text_height(28.f);
    addChild(title_widget);
    if (my_module) {
        title_widget->set_text_color(my_module->fg_color);
        title_widget->set_text(my_module->title);
    } else {
        title_widget->set_text_color(parse_color("hsl(0,0%,65%)"));
    }

    // inputs

    // footer


    link_button = createThemedButton<LinkButton>(Vec(3.5f, box.size.y-15.f), theme_engine, theme, "Core link");
    addChild(link = createIndicatorCentered(22.f,box.size.y-9.f, RampGray(G_50), "[connection]"));
    link->setFill(false);
    
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
    // if (!my_module || my_module->glow_knobs) {
    //     glowing_knobs(true);
    // }

    if (my_module) {
        my_module->set_chem_ui(this);
        onConnectHost(my_module->chem_host);
    }
}

// void OverlayUi::glowing_knobs(bool glow) {
//     // for (int i = 0; i < OverlayModule::NUM_KNOBS; ++i) {
//     //     knobs[i]->glowing(glow);
//     // }
// }

// void OverlayUi::center_knobs()
// {
//     if (!my_module) return;
// }

void OverlayUi::setThemeName(const std::string& name, void * context)
{
    //applyLightTheme<SmallSimpleLight<GreenLight>>(mix_light, name);
    Base::setThemeName(name, context);
}

void OverlayUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    onConnectionChange(ChemDevice::Haken, host ? host->host_connection(ChemDevice::Haken) : nullptr);
}

void OverlayUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (connection) {
        link->describe(connection->info.friendly(TextFormatLength::Long));
        link->setFill(true);
        link->setColor(PORT_BLUE);
    } else {
        link->describe("[not connected]");
        link->setFill(false);
        link->setColor(RampGray(G_50));
    }
}

bool OverlayUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

void OverlayUi::set_bg_color(PackedColor color)
{
    if (bg_widget) bg_widget->color = color;
    if (!my_module) return;
    my_module->bg_color = color;
}

void OverlayUi::set_fg_color(PackedColor color)
{
    if (title_widget) title_widget->set_text_color(color);
    if (!my_module) return;
    my_module->fg_color = color;
}

void OverlayUi::set_title(std::string text)
{
    if (title_widget) title_widget->set_text(text);
    if (!my_module) return;
    my_module->title = text;
}

void OverlayUi::set_preset(std::string preset)
{
    if (!my_module) return;
    my_module->overlay_preset = preset;
}

void OverlayUi::onHoverKey(const HoverKeyEvent &e)
{
    if (my_module) {
        if (e.action == GLFW_PRESS && ((e.mods & RACK_MOD_MASK) == 0)) {
            switch (e.key) {
            // case GLFW_KEY_0:
            //     e.consume(this);
            //     my_module->modulation.zero_modulation();
            //     return;
            // case GLFW_KEY_5:
            //     center_knobs();
            //     e.consume(this);
            //     return;
            }
        }
    }
    Base::onHoverKey(e);
}

void OverlayUi::step()
{
    Base::step();
    if (!my_module) return;
    bind_host(my_module);
    bool left = panelBorder->left;
    bool right = panelBorder->right;
    if (!right) {
        auto e = my_module->getRightExpander().module;
        right = e && (e->getModel() == modelXM);
    }
    panelBorder->setPartners(left, right);

    //knobs[K_MODULATION]->enable(my_module->modulation.has_target());

    // for (int i = 0; i < K_MODULATION; ++i) {
    //     tracks[i]->set_value(my_module->modulation.get_port(i).modulated());
    //     tracks[i]->set_active(my_module->getInput(i).isConnected());
    // }

}

void OverlayUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
    menu->addChild(new MenuSeparator);
    menu->addChild(createSubmenuItem("Preset", "", 
        [=](Menu *menu) {
            TextInputMenu *editField = new TextInputMenu();
            editField->box.size.x = 150;
            editField->setText(my_module ? my_module->overlay_preset : "<preset>");
            editField->set_on_change([=](std::string text) { set_preset(text); });
            menu->addChild(editField);
        }));
    menu->addChild(createSubmenuItem("Title", "", 
        [=](Menu *menu) {
            TextInputMenu *editField = new TextInputMenu();
            editField->box.size.x = 150;
            editField->setText(my_module ? my_module->title : "<title>");
            editField->set_on_change([=](std::string text) { set_title(text); });
            menu->addChild(editField);
        }));

    menu->addChild(new MenuSeparator);

    menu->addChild(createSubmenuItem("Background", "", [=](Menu* menu) {
        auto picker = new ColorPickerMenu();
        picker->set_color(get_bg_color());
        picker->set_on_new_color([=](PackedColor color) {
            set_bg_color(color);
        });
        menu->addChild(picker);
    }));

    menu->addChild(createSubmenuItem("Text color", "", [=](Menu* menu) {
        auto picker = new ColorPickerMenu();
        picker->set_color(get_fg_color());
        picker->set_on_new_color([=](PackedColor color) {
            set_fg_color(color);
        });
        menu->addChild(picker);
    }));

    // bool unconnected = (my_module->inputs.end() == std::find_if(my_module->inputs.begin(), my_module->inputs.end(), [](Input& in){ return in.isConnected(); }));
    // menu->addChild(createMenuItem("Zero modulation", "0", [this](){
    //     my_module->modulation.zero_modulation();
    // }, unconnected));

    // menu->addChild(createMenuItem("Center knobs", "5", [this](){ center_knobs(); }));

    // menu->addChild(createCheckMenuItem("Glowing knobs", "", 
    //     [this](){ return my_module->glow_knobs; },
    //     [this](){
    //         my_module->glow_knobs = !my_module->glow_knobs; 
    //         glowing_knobs(my_module->glow_knobs);
    //     }
    // ));
    Base::appendContextMenu(menu);
}
