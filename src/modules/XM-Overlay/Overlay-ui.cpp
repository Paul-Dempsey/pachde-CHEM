#include "Overlay.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/widgets.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;

// -- UI --------------------------------------
const char * DEFAULT_TITLE{"Overlay Synth"};

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
        title_widget->set_text(DEFAULT_TITLE);
        title_widget->set_text_color(0xffe6e6e6);
    }

    auto set_preset_button = Center(createThemedButton<ChicletButton>(Vec(box.size.x*.5f, 24.f), theme_engine, theme, "Select overlay preset"));
    if (my_module) {
        set_preset_button->setHandler([=](bool c, bool f) {
            if (!chem_host) return;
            auto haken = chem_host->host_haken();
            if (my_module->overlay_preset) {
                haken->select_preset(ChemId::Overlay, my_module->overlay_preset->id);
            }
        });
    }
    addChild(set_preset_button);

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
        auto logo = new Logo(0.25f);
        logo->box.pos = Vec(box.size.x * .5f, 60);
        addChild(Center(logo));
    }

    // init
    if (my_module) {
        my_module->set_chem_ui(this);
        onConnectHost(my_module->chem_host);
    }
}

void OverlayUi::setThemeName(const std::string& name, void * context)
{
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
}

void OverlayUi::appendContextMenu(Menu *menu)
{
    if (!module) return;

    menu->addChild(createMenuLabel<HamburgerTitle>("Overlay"));

    std::string label{"Overlay preset: "};
    auto over = my_module ? my_module->overlay_preset : nullptr;
    label.append(over ? over->summary() : "<not configured>");
    menu->addChild(createMenuLabel(label));

    menu->addChild(createMenuLabel(format_string("Macros: %u", my_module ? my_module->macros.size() : 0)));

    auto preset = my_module ? my_module->live_preset : nullptr;
    menu->addChild(createMenuItem("Use live preset", preset ? preset->summary() : "<none>", [=](){
       my_module->overlay_preset = my_module->live_preset;
       my_module->preset_connected = true;
       my_module->notify_connect_preset();
    }, !preset));

    menu->addChild(createMenuItem("Reset", "", [=](){
        if (module) my_module->reset();
        bg_widget->color = 0;
        title_widget->set_text_color(0xffe6e6e6);
        title_widget->set_text(DEFAULT_TITLE);
    }));

    menu->addChild(createSubmenuItem("Title", my_module ? my_module->title : "<none>",
        [=](Menu *menu) {
            menu->addChild(createTextInput<TextInputMenu>(0, 0, 150, 0, my_module ? my_module->title : "<title>", [=](std::string text) { set_title(text); }));
        }));


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
    Base::appendContextMenu(menu);
}
