#include "XM-Edit.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/color-picker.hpp"
#include "../../widgets/click-region-widget.hpp"
//#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;
using MOD = XMEditModule;

// -- UI --------------------------------------

std::string macro_number_text(uint8_t mn) {
    if (mn < 7) return "(unset)";
    return format_string("%d", mn);
}

XMEditUi::XMEditUi(XMEditModule *module) :
    my_module(module)
{
    setModule(module);
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);

    if (my_module) {
        Module* xm = get_xm_module();
        client = dynamic_cast<IExtendedMacro*>(xm);
    }

    float x, y;
    bool browsing = !module;

    const float PANEL_WIDTH{120.f};
    const float CENTER{PANEL_WIDTH*.5f};
    const float LEFT_AXIS{12.f};
    const float FULL_WIDTH{PANEL_WIDTH - 2*LEFT_AXIS};
    const float LABEL_DY{16.f};
    const float PALETTE_DX{15.f};

    addChild(createLightCentered<SmallLight<GreenLight>>(Vec(49.5, 15.5), my_module, MOD::L_XM));

    x = LEFT_AXIS;
    y = 20;
    addChild(createLabel(Vec(x,y), FULL_WIDTH, "Module title", theme_engine, theme, S::control_label_left));

    y += LABEL_DY;
    title_entry = createWidget<TextInput>(Vec(x,y));
    title_entry->box.size = Vec(PANEL_WIDTH - 2*LEFT_AXIS, 14.f);
    title_entry->setText(client ? client->get_title() : "");
    title_entry->set_on_change([=](std::string text){ on_title_change(text); });
    addChild(title_entry);

    y += 18.f;
    palette_fg = Center(createThemedButton<Palette1Button>(Vec(CENTER - PALETTE_DX, y + 6.f), theme_engine, theme, "Text color"));
    palette_fg->setHandler([=](bool,bool) {
        ui::Menu* menu = createMenu();
        auto picker = new ColorPickerMenu();
        picker->set_color(get_title_text_color());
        picker->set_on_new_color([=](PackedColor color) {
            set_title_text_color(color);
        });
        menu->addChild(picker);
    });
    addChild(palette_fg);

    palette_bg = Center(createThemedButton<Palette2Button>(Vec(CENTER + PALETTE_DX, y + 6.f), theme_engine, theme, "Background color"));
    palette_bg->setHandler([=](bool,bool) {
        ui::Menu* menu = createMenu();
        auto picker = new ColorPickerMenu();
        picker->set_color(get_title_background_color());
        picker->set_on_new_color([=](PackedColor color) {
            set_title_background_color(color);
        });
        menu->addChild(picker);
    });
    addChild(palette_bg);
    y += 12.5f;
    auto mini_label_style = LabelStyle{"ctl-label", TextAlignment::Center, 10.f, true};
    addChild(createLabel(Vec(CENTER - PALETTE_DX,y),25, "text", theme_engine, theme, mini_label_style));
    addChild(createLabel(Vec(CENTER + PALETTE_DX,y),25, "bg", theme_engine, theme, mini_label_style));

    y = 94.f;
    x = CENTER;
    tab_header = new TabHeader(Vec(x,y), 8);
    tab_header->set_on_item_change([=](int item) { on_item_change(item); });
    tab_header->applyTheme(theme_engine, theme);
    addChild(Center(tab_header));

    current_macro = client ? client->get_macro(knob_index) : nullptr;

    y += 16.f;
    x = LEFT_AXIS;
    addChild(createThemedParamButton<CheckParamButton>(Vec(x,y), my_module, MOD::P_ADD_REMOVE, theme_engine, theme));
    addChild(createLabel(Vec(36.f, y), 40.f, "Macro", theme_engine, theme, S::control_label_left));
    //>> macro menu
    auto value_style = LabelStyle{"setting", TextAlignment::Left, 12.f};
    std::string text = macro_number_text(current_macro ? current_macro->macro_number : INVALID_MACRO);
    addChild(macro_number = createLabel(Vec(box.size.x - 32.f, y + 1.5f), 35, text, theme_engine, theme, value_style));

    y = 130.f;
    x = LEFT_AXIS;
    addChild(createLabel(Vec(x,y), 40, "Name", theme_engine, theme, S::control_label_left));

    y += LABEL_DY;
    name_entry = createWidget<TextInput>(Vec(x,y));
    name_entry->box.size = Vec(PANEL_WIDTH - 2*LEFT_AXIS, 14.f);
    name_entry->set_on_change([=](std::string text){ on_name_change(text); });
    if (current_macro) {
        name_entry->setText(current_macro->name);
    }
    addChild(name_entry);

    y = 168.f;
    addChild(createLabel(Vec(x,y), 35, "Range", theme_engine, theme, S::control_label_left));

    StateIndicatorWidget* indicator{nullptr};
    float x_opt = 38.f;
    MacroRange range = current_macro ? current_macro->range : MacroRange::Unipolar;
    
    y += LABEL_DY + 6.f;
    indicator = createIndicatorCentered(x_opt, y, range == MacroRange::Bipolar, "Bipolar");
    indicator->applyTheme(theme_engine, theme);
    addChild(indicator);
    range_options.push_back(indicator);
    addChild(createLabel(Vec(x_opt + 6.f, y- 7.f), 50, "Bipolar", theme_engine, theme, S::control_label_left));
    addChild(createClickRegion(x_opt -6.f, y - 6.f, 60, 14, int(MacroRange::Bipolar), [=](int id, int) { set_range(MacroRange(id)); }));

    y += 14.f;
    indicator = createIndicatorCentered(x_opt, y, range == MacroRange::Unipolar, "Unipolar");
    indicator->applyTheme(theme_engine, theme);
    addChild(indicator);
    range_options.push_back(indicator);
    addChild(createLabel(Vec(x_opt + 6.f, y- 7.f), 50, "Unipolar", theme_engine, theme, S::control_label_left));
    addChild(createClickRegion(x_opt -6.f, y - 6.f, 60, 14, int(MacroRange::Unipolar), [=](int id, int) { set_range(MacroRange(id)); }));

    y += 14.f;
    indicator = createIndicatorCentered(x_opt, y, range == MacroRange::Custom, "Custom");
    indicator->applyTheme(theme_engine, theme);
    addChild(indicator);
    range_options.push_back(indicator);
    addChild(createLabel(Vec(x_opt + 6.f, y- 7.f), 50, "Custom", theme_engine, theme, S::control_label_left));
    addChild(createClickRegion(x_opt -6.f, y - 6.f, 60, 14, int(MacroRange::Custom), [=](int id, int) { set_range(MacroRange(id)); }));

    y += 18.f;
    addChild(createChemKnob<GreenTrimPot>(Vec(CENTER - PALETTE_DX, y), my_module, MOD::P_RANGE_MIN, theme_engine, theme));
    addChild(createChemKnob<GreenTrimPot>(Vec(CENTER + PALETTE_DX, y), my_module, MOD::P_RANGE_MAX, theme_engine, theme));

    y += 16.f;
    x = LEFT_AXIS;
    addChild(createThemedParamButton<CheckParamButton>(Vec(x,y), my_module, MOD::P_INPUT, theme_engine, theme));
    addChild(createLabel(Vec(x + 14.f, y), 60.f, "Input port", theme_engine, theme, S::control_label_left));

    y += 24.f;
    addChild(createThemedParamButton<CheckParamButton>(Vec(x,y), my_module, MOD::P_MOD, theme_engine, theme));
    addChild(createLabel(Vec(x + 14.f, y), 60.f, "Modulation", theme_engine, theme, S::control_label_left));

    // footer
    LabelStyle status_style{"warning", TextAlignment::Center, 10.f};
    addChild(status = createLabel(Vec(CENTER, 348.f), PANEL_WIDTH, "", theme_engine, theme, status_style));

    addChild(link = createIndicatorCentered(22.f,box.size.y-9.f, RampGray(G_50), "[connection]"));
    link->setFill(false);
    link_button = createThemedButton<LinkButton>(Vec(3.5f, box.size.y - S::U1), theme_engine, theme, "Core link");
    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ModuleBroker::get()->addHostPickerMenu(createMenu(), my_module);
        });
    }
    addChild(link_button);

    // Browsing UI
    if (browsing) {
    }

    if (my_module) {
        my_module->set_chem_ui(this);
        if (!chem_host) {
            onConnectHost(my_module->chem_host);
        }
    }
}

void XMEditUi::on_title_change(std::string text)
{
    title = text;
    if (client) client->set_header_text(text);
}

void XMEditUi::on_name_change(std::string text)
{
    if (current_macro) {
        current_macro->name = text;
        if (client) client->on_macro_change(current_macro->index);
    }
}

void XMEditUi::set_range(MacroRange range)
{
    if (current_macro) {
        current_macro->range = range;
        if (client) client->on_macro_change(current_macro->index);
    }
    set_range_ui(range);
}

void XMEditUi::set_range_ui(MacroRange range)
{
    int r = 0;
    for (auto ri: range_options) {
        ri->set_state(MacroRange(r) == range);
        ++r;
    }
}

void XMEditUi::set_title_background_color(PackedColor color)
{
    title_bg_color = color;
    if (client) client->set_header_color(color);
}

PackedColor XMEditUi::get_title_background_color()
{
    return title_bg_color;
}

void XMEditUi::set_title_text_color(PackedColor color)
{
    title_fg_color = color;
    if (client) client->set_header_text_color(color);
}

PackedColor XMEditUi::get_title_text_color()
{
    return title_fg_color;
}

void XMEditUi::setThemeName(const std::string& name, void * context)
{
    current_style.apply_theme(theme_engine.getTheme(name));
    Base::setThemeName(name, context);
}

Module* XMEditUi::get_xm_module()
{
    if (!my_module) return nullptr;
    Module * mod = my_module->getLeftExpander().module;
    if (mod && mod->getModel() == modelXM) {
        return mod;
    }
    return nullptr;
}

void XMEditUi::on_client_change()
{
    knob_index = 0;
    on_item_change(knob_index);
    if (client) {
        title_entry->setText(client->get_title());
        status->text("");
    } else {
        title_entry->setText("");
        status->text("[no XM connected]");
    }
    tab_header->set_current_item(knob_index);
    refresh_macro_controls();
}

void XMEditUi::on_item_change(int item)
{
    knob_index = item;
    if (client) {
        client->set_macro_edit(item);
        refresh_macro_controls();
    }
}

void XMEditUi::refresh_macro_controls()
{
    current_macro = client ? client->get_macro(knob_index) : nullptr;
    if (current_macro) {
        macro_number->text(macro_number_text(current_macro->macro_number));
        name_entry->setText(current_macro->name);
        if (my_module) {
            my_module->getParam(MOD::P_ADD_REMOVE).setValue(current_macro->active());
        }
        set_range_ui(current_macro->range);
    } else {
        macro_number->text(macro_number_text(INVALID_MACRO));
        name_entry->setText("");
        if (my_module) {
            my_module->getParam(MOD::P_ADD_REMOVE).setValue(1.f);
        }
        set_range_ui(MacroRange::Bipolar);
    }
}

void XMEditUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    onConnectionChange(ChemDevice::Haken, host ? host->host_connection(ChemDevice::Haken) : nullptr);
}

void XMEditUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    if (connection) {
        link->describe(connection->info.friendly(TextFormatLength::Long));
        link->setFill(true);
        link->setColor(PORT_BLUE);
    } else {
        link->describe("[not connected]");
        link->setFill(false);
        link->setColor(RampGray(G_50));
    }}

void XMEditUi::step()
{
    Base::step();
    if (!my_module) return;
    Module* xm = get_xm_module();
    auto new_client = dynamic_cast<IExtendedMacro*>(xm);
    if (new_client != client) {
        client = new_client;
        on_client_change();
    }
    panelBorder->setPartners(nullptr != xm, false);
}

void XMEditUi::draw(const DrawArgs& args)
{
    Base::draw(args);
    if (!client) return;
    auto vg = args.vg;
    auto co = current_style.nvg_color();
    float y = 32.f + (knob_index * 32.f);
    nvgBeginPath(vg);
    nvgMoveTo(vg, 4.5f, y - 4.5f);
    nvgLineTo(vg, 0, y);
    nvgLineTo(vg, 4.5f, y + 4.5);
    nvgClosePath(vg);
    nvgFillColor(vg, co);
    nvgFill(vg);
    //Line (vg, .75f, y - 14.f, .75f, y + 16.f, co, 1.5f);
}

void XMEditUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
    menu->addChild(new MenuSeparator);

    Base::appendContextMenu(menu);
}
