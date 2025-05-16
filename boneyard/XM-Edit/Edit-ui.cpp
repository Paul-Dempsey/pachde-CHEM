#include "Edit.hpp"
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

struct MacroMenu: Hamburger
{
    using Base = Hamburger;

    XMEditUi* ui{nullptr};
    void set_ui(XMEditUi* w) { ui = w; }

    void appendContextMenu(ui::Menu* menu) override
    {
        menu->addChild(createMenuLabel<HamburgerTitle>("Macros"));
        if (!ui->my_module) return;
        auto overlay = ui->get_overlay();
        if (overlay) {
            auto conf = overlay->overlay_configured_preset();
            if (!conf) {
                menu->addChild(createMenuLabel("Overlay has no configured preset"));
                return;
            }
            auto live = overlay->overlay_live_preset();
            if (!live || conf->name.compare(live->name)) {
                menu->addChild(createMenuLabel("Live preset is not the configured preset"));
                return;
            }
            std::string text = "Preset ";
            text.append(live->name);
            menu->addChild(createMenuLabel(text));

            switch (overlay->overlay_macros_ready()) {

            case MacroReadyState::Available: {
                auto macros{overlay->overlay_macro_usage()};
                if (macros.empty()) goto NO_MACROS;
                for (const MacroUsage& mu: macros) {
                    auto num = mu.macro_number;
                    menu->addChild(createMenuItem(mu.to_string(), "", [=](){
                        ui->set_macro_number(num);
                        APP->event->setSelectedWidget(ui);
                    }));
                }
                } break;

            case MacroReadyState::Unavailable:
NO_MACROS:
                menu->addChild(createMenuItem("Load preset macros", "", [=]() {
                    overlay->overlay_request_macros();
                    APP->event->setSelectedWidget(ui);
                }));
                break;

            case MacroReadyState::InProgress:
                menu->addChild(createMenuLabel("Gathering macros...please wait"));
                break;
            }
        } else {
            menu->addChild(createMenuLabel("Overlay not connected"));
        }
    }
};

constexpr const float PANEL_WIDTH{120.f};
constexpr const float CENTER{PANEL_WIDTH*.5f};
constexpr const float LEFT_AXIS{10.f};
constexpr const float FULL_WIDTH{PANEL_WIDTH - 2*LEFT_AXIS};
constexpr const float LABEL_DY{14.75f};
constexpr const float PALETTE_DX{15.f};
constexpr const float MINMAX_DX{13.f};
constexpr const float LIGHT_CY{15.5f};

XMEditUi::XMEditUi(XMEditModule *module) :
    my_module(module)
{
    setModule(module);
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);

    IExtendedMacro* client = get_client();

    float x, y;
    bool browsing = !module;

    addChild(createLightCentered<SmallLight<GreenLight>>(Vec(CENTER - 10.f, LIGHT_CY), my_module, MOD::L_XM));
    addChild(createLightCentered<SmallLight<WhiteLight>>(Vec(CENTER,       LIGHT_CY), my_module, MOD::L_OVERLAY));
    addChild(createLightCentered<SmallLight<BlueLight>> (Vec(CENTER + 10.f, LIGHT_CY), my_module, MOD::L_CORE));

    x = LEFT_AXIS;
    y = 20;
    addChild(createLabel(Vec(x,y), FULL_WIDTH, "Section title:", theme_engine, theme, S::control_label_left));

    y += LABEL_DY;
    title_entry = createTextInput(x, y, PANEL_WIDTH - 2*LEFT_AXIS, 14.f, client ? client->xm_get_header_text() : "",
        [=](std::string text){ on_title_change(text); });
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

    y += 24.f;
    x = CENTER;
    tab_header = new TabHeader(Vec(x,y), 8);
    tab_header->set_on_item_change([=](int item) { on_item_change(item); });
    tab_header->applyTheme(theme_engine, theme);
    addChild(Center(tab_header));

    current_macro = client ? client->xm_get_macro(knob_index) : nullptr;

    y += 14.f;
    x = LEFT_AXIS;

    add_remove = createThemedButton<PlusMinusButton>(Vec(x,y),theme_engine, theme);
    add_remove->setHandler([=](bool, bool){
        on_add_remove();
    });
    addChild(add_remove);
    addChild(createLabel(Vec(x + 14.f, y), 65.f, "Add/Remove", theme_engine, theme, S::control_label_left));

    y +=  15.f;
    addChild(createLabel(Vec(x,y), 40, "Name:", theme_engine, theme, S::control_label_left));
    y += LABEL_DY;
    name_entry = createThemedTextInput(x, y, PANEL_WIDTH - 2*LEFT_AXIS, 14.f, 
        theme_engine, theme,
        current_macro ? current_macro->name: "",
        [=](std::string text){ on_name_change(text); },
        nullptr,
        "<knob label>"
    );
    addChild(name_entry);

    y += 18.f;
    addChild(createLabel(Vec(x, y), 40.f, "Macro", theme_engine, theme, S::control_label_left));
    auto mm = createThemedWidget<MacroMenu>(Vec(CENTER- 5.f,y+1.5f), theme_engine, theme);
    mm->set_ui(this);
    mm->describe("Available macros");
    addChild(mm);

    auto value_style = LabelStyle{"setting", TextAlignment::Left, 12.f};
    std::string text = macro_number_text(current_macro ? current_macro->macro_number : INVALID_MACRO);
    addChild(macro_number = createLabel(Vec(box.size.x - 40.f, y + 1.5f), 35, text, theme_engine, theme, value_style));

    y += 18.f;
    addChild(createLabel(Vec(x,y), 35, "Range:", theme_engine, theme, S::control_label_left));

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

    y += 13.f;
    indicator = createIndicatorCentered(x_opt, y, range == MacroRange::Unipolar, "Unipolar");
    indicator->applyTheme(theme_engine, theme);
    addChild(indicator);
    range_options.push_back(indicator);
    addChild(createLabel(Vec(x_opt + 6.f, y- 7.f), 50, "Unipolar", theme_engine, theme, S::control_label_left));
    addChild(createClickRegion(x_opt -6.f, y - 6.f, 60, 14, int(MacroRange::Unipolar), [=](int id, int) { set_range(MacroRange(id)); }));

    y += 13.f;
    indicator = createIndicatorCentered(x_opt, y, range == MacroRange::Custom, "Custom");
    indicator->applyTheme(theme_engine, theme);
    addChild(indicator);
    range_options.push_back(indicator);
    addChild(createLabel(Vec(x_opt + 6.f, y - 7.f), 50, "Custom", theme_engine, theme, S::control_label_left));
    addChild(createClickRegion(x_opt -6.f, y - 6.f, 60, 14, int(MacroRange::Custom), [=](int id, int) { set_range(MacroRange(id)); }));

    y += 16.f;
    addChild(createChemKnob<GreenTrimPot>(Vec(CENTER - MINMAX_DX, y), my_module, MOD::P_RANGE_MIN, theme_engine, theme));
    addChild(createLabel(Vec(CENTER - MINMAX_DX, y + 10.5f), 25, "min", theme_engine, theme, mini_label_style));
    addChild(createChemKnob<GreenTrimPot>(Vec(CENTER + MINMAX_DX, y), my_module, MOD::P_RANGE_MAX, theme_engine, theme));
    addChild(createLabel(Vec(CENTER + MINMAX_DX, y + 10.5f), 25, "max", theme_engine, theme, mini_label_style));

    y += 24.f;
    x = LEFT_AXIS;
    addChild(createThemedParamButton<CheckParamButton>(Vec(x,y), my_module, MOD::P_INPUT, theme_engine, theme));
    addChild(createLabel(Vec(x + 14.f, y), 60.f, "Input port", theme_engine, theme, S::control_label_left));

    y += 24.f;
    addChild(createThemedParamButton<CheckParamButton>(Vec(x,y), my_module, MOD::P_MOD, theme_engine, theme));
    addChild(createLabel(Vec(x + 14.f, y), 60.f, "Modulation", theme_engine, theme, S::control_label_left));

    y += 30.f;
    auto clear_button = createThemedButton<SquareButton>(Vec(x,y), theme_engine, theme, "Reset");
    clear_button->setHandler([=](bool, bool){ clear_xm(); });
    addChild(clear_button);
    addChild(createLabel(Vec(x + clear_button->box.size.x + 5.f,y), 50, "Reset", theme_engine, theme, S::control_label_left));

    title_entry->nextField = name_entry;
    title_entry->prevField = this;
    name_entry->nextField = this;
    name_entry->prevField = title_entry;

    // footer
    LabelStyle status_style{"warning", TextAlignment::Center, 10.f};
    addChild(status = createLabel(Vec(CENTER, 348.f), PANEL_WIDTH, "", theme_engine, theme, status_style));

    // addChild(link = createIndicatorCentered(22.f,box.size.y-9.f, RampGray(G_50), "[connection]"));
    // link->setFill(false);
    // link_button = createThemedButton<LinkButton>(Vec(3.5f, box.size.y - S::U1), theme_engine, theme, "Core link");
    // if (my_module) {
    //     link_button->setHandler([=](bool ctrl, bool shift) {
    //         ModuleBroker::get()->addHostPickerMenu(createMenu(), my_module);
    //     });
    // }
    // addChild(link_button);

    // Browsing UI
    if (browsing) {
        auto logo = new Logo(0.5f);
        logo->box.pos = Vec(CENTER, y + 36.f);
        addChild(Center(logo));
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
    auto client = get_client();
    if (client) client->xm_set_header_text(text);
}

void XMEditUi::on_name_change(std::string text)
{
    if (current_macro) {
        current_macro->name = text;
        auto client = get_client();
        if (client) client->xm_on_macro_change(current_macro->knob_id);
    }
}

void XMEditUi::set_range(MacroRange range)
{
    uint16_t min;
    uint16_t max;
    if (current_macro) {
        min = current_macro->min;
        max = current_macro->max;
    } else if (my_module) {
        min = getParamInt(my_module->getParam(MOD::P_RANGE_MIN));
        max = getParamInt(my_module->getParam(MOD::P_RANGE_MAX));
    }
    switch (range) {
    case MacroRange::Bipolar:
        min = 0;
        max = Haken::max14;
        break;
    case MacroRange::Unipolar:
        min = Haken::zero14;
        max = Haken::max14;
        break;
    case MacroRange::Custom:
        break;
    }
    if (my_module){
        my_module->getParam(MOD::P_RANGE_MIN).setValue(min);
        my_module->getParam(MOD::P_RANGE_MAX).setValue(max);
    }
    if (current_macro) {
        current_macro->range = range;
        current_macro->min = min;
        current_macro->max = max;
        auto client = get_client();
        if (client) client->xm_on_macro_change(current_macro->knob_id);
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

void XMEditUi::set_macro_number(uint8_t num)
{
    if (current_macro) {
        current_macro->macro_number = num;
        auto client = get_client();
        if (client) client->xm_on_macro_change(current_macro->knob_id);
    }
    refresh_macro_controls();
}

void XMEditUi::clear_xm()
{
    knob_index = 0;

    auto client = get_client();
    if (client) client->xm_clear();
    client->xm_set_macro_edit(knob_index);
    tab_header->set_current_item(knob_index);
    title = client->xm_get_header_text();
    title_bg_color = client->xm_get_header_color();
    title_fg_color = client->xm_get_header_text_color();
    refresh_macro_controls();
}

void XMEditUi::set_title_background_color(PackedColor color)
{
    title_bg_color = color;
    auto client = get_client();
    if (client) client->xm_set_header_color(color);
}

PackedColor XMEditUi::get_title_background_color()
{
    return title_bg_color;
}

void XMEditUi::set_title_text_color(PackedColor color)
{
    title_fg_color = color;
    auto client = get_client();
    if (client) client->xm_set_header_text_color(color);
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
    return my_module ? my_module->get_xm_module() : nullptr;
}

void XMEditUi::on_add_remove()
{
    auto client = get_client();
    if (client) {
        if (add_remove->plus) {
            client->xm_add_macro(knob_index);
        } else {
            client->xm_remove_macro(knob_index);
        }
    }
}

void XMEditUi::on_client_change()
{
    knob_index = 0;
    on_item_change(knob_index);
    auto client = get_client();
    if (client) {
        title_entry->setText(client->xm_get_header_text());
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
    tab_header->set_current_item(item);
    auto client = get_client();
    if (client) {
        client->xm_set_macro_edit(item);
        refresh_macro_controls();
    }
}

void XMEditUi::refresh_macro_controls()
{
    auto client = get_client();
    current_macro = client ? client->xm_get_macro(knob_index) : nullptr;
    if (current_macro) {
        add_remove->set_plus(false);
        macro_number->text(macro_number_text(current_macro->macro_number));
        name_entry->setText(current_macro->name);
        set_range_ui(current_macro->range);
        if (my_module) {
            my_module->getParam(MOD::P_RANGE_MIN).setValue(current_macro->min);
            my_module->getParam(MOD::P_RANGE_MAX).setValue(current_macro->max);
        }
    } else {
        add_remove->set_plus(true);
        macro_number->text(macro_number_text(INVALID_MACRO));
        name_entry->setText("");
        set_range_ui(MacroRange::Bipolar);
        if (my_module) {
            my_module->getParam(MOD::P_RANGE_MIN).setValue(0);
            my_module->getParam(MOD::P_RANGE_MAX).setValue(Haken::max14);
        }
    }
}

void XMEditUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    onConnectionChange(ChemDevice::Haken, host ? host->host_connection(ChemDevice::Haken) : nullptr);
}

void XMEditUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    // if (device != ChemDevice::Haken) return;
    // if (connection) {
    //     link->describe(connection->info.friendly(TextFormatLength::Long));
    //     link->setFill(true);
    //     link->setColor(PORT_BLUE);
    // } else {
    //     link->describe("[not connected]");
    //     link->setFill(false);
    //     link->setColor(RampGray(G_50));
    // }
}

void XMEditUi::step()
{
    Base::step();
    if (!my_module) return;
    bind_host(my_module);
    panelBorder->setPartners(nullptr != get_xm_module(), false);
    if (!my_module->ready()) {
        current_macro = nullptr;
    } else if (!current_macro) {
        IExtendedMacro* client = get_client();
        current_macro = client ? client->xm_get_macro(knob_index) : nullptr;
    }
}

void XMEditUi::onSelectKey(const SelectKeyEvent &e)
{
    if ((e.action == GLFW_PRESS || e.action == GLFW_REPEAT))
    {
        switch (e.key) {
        case GLFW_KEY_1:
        case GLFW_KEY_2:
        case GLFW_KEY_3:
        case GLFW_KEY_4:
        case GLFW_KEY_5:
        case GLFW_KEY_6:
        case GLFW_KEY_7:
        case GLFW_KEY_8:
            on_item_change(e.key - GLFW_KEY_1);
            e.consume(this);
            return;

        case GLFW_KEY_LEFT: {
            int item = knob_index - 1;
            if (item < 0) item = 7;
            on_item_change(item);
            e.consume(this);
            return;
        } break;

        case GLFW_KEY_RIGHT:{
            int item = (knob_index + 1) % 8;
            on_item_change(item);
            e.consume(this);
            return;
        } break;

        case GLFW_KEY_TAB: {
            auto sel = APP->event->getSelectedWidget();
            if (sel == this) {
                APP->event->setSelectedWidget(title_entry);
                e.consume(this);
                return;
            }
        } break;

        case GLFW_KEY_ESCAPE:
            APP->event->setSelectedWidget(nullptr);
            e.consume(this);
            return;
        }
    }
    Base::onSelectKey(e);
}

void XMEditUi::draw(const DrawArgs& args)
{
    Base::draw(args);
    if (!get_client()) return;
    if (my_module && !my_module->ready()) return;
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
}

void XMEditUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
//    menu->addChild(new MenuSeparator);
    Base::appendContextMenu(menu);
}
