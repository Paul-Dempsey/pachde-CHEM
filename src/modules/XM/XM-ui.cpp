#include "XM.hpp"
//#include "../../em/em-hardware.h"
#include "add-remove-button.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;

std::string macro_number_to_string(uint8_t mn) {
    if (mn < 7) return "(unset)";
    return format_string("%d", mn);
}
uint8_t macro_number_from_string(std::string s)
{
    auto n = 0;
    bool valid{true};
    for (auto ch : s) {
        if (std::isdigit(ch)) {
            n = (n*10) + (ch - '0');
        } else if (' ' == ch) {
            // skip
        } else {
            valid = false;
            break;
        }
    }
    return valid ? clamp(n, 7, 90) : 0;
}

constexpr const float PANEL_WIDTH   = 45.f;
constexpr const float CENTER        = PANEL_WIDTH*.5f;
constexpr const float KNOBS_TOP     = 28;
constexpr const float KNOB_DY       = 32;
// constexpr const float KNOB_LABEL_DY = 9;
constexpr const float PORT_TOP_CY   = 286;
constexpr const float PORT_DY       = 19.f;
constexpr const float PORT_MOD_DX   = 10.F;
constexpr const float PORT_MOD_DY   = 8.F;
constexpr const float PORT_OFFSET   = 9.f;

static const char * const KN{"12345678"};

struct EditWireframe: OpaqueWidget 
{
    using Base = OpaqueWidget;
    XMUi* ui{nullptr};

    EditWireframe() {
        box.size = Vec(PANEL_WIDTH, 380.f);
    }

    void set_ui(XMUi* w) {
        ui = w; 
        auto panel = createWidget<PanelBackgroundWidget>(Vec(0,15));
        panel->box.size.x = PANEL_WIDTH;
        panel->box.size.y = 380.f - 15.f - 12.f;
        addChild(panel);

        for (ssize_t i = 0; i < 8; ++i) {
            auto pos = ui->knob_center(i);
            auto cr = Center(createClickRegion(pos.x, pos.y, 28.f, 24.f, i, [=](int i, int mods) {
                ui->set_edit_item(i);
            }));
            addChild(cr);
        }
    }

    void draw(const DrawArgs& args) override
    {
        Base::draw(args);

        auto vg = args.vg;
        auto w = ui->edit_style.width();
        auto co = ui->edit_style.nvg_stroke_color();
        auto font = GetPluginFontSemiBold();
        if (!FontOk(font)) return;

        auto md = ui->get_edit_macro();

        nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
        SetTextStyle(vg, font, RampGray(G_0), 16);
        for (int i = 0; i < 8; ++i) {
            auto xy{ui->knob_center(i)};
            // knob
            auto perm = ui->get_persistent_macro(i);
            if (perm) {
                Dot(vg, xy.x + 9.5, xy.y - 9.5, co, true);
            }

            if (md && md->knob_id == i) {
                Circle(vg, xy.x, xy.y, 8, co);
                OpenCircle(vg, xy.x, xy.y, 8, co, w);
                nvgFillColor(vg, RampGray(G_0));
            } else {
                OpenCircle(vg, xy.x, xy.y, 8, co, w);
                nvgFillColor(vg, co);
            }
            nvgText(vg, xy.x, xy.y, KN+i,KN+i+1);
            
            if (md && (md->knob_id == i) && md->modulated) {
                // input
                xy = ui->input_center(i);
                OpenCircle(vg, xy.x, xy.y, 4.5, co, w*.5);
            }
        }

        // mod knob
        if (ui->get_modulation()) {
            OpenCircle(vg, CENTER, PORT_TOP_CY, 8, co, w*.5);
        }

        FittedBoxRect(vg, 0, 0, box.size.x, box.size.y, co, Fit::Inside, w);
        Line(vg, w, 275, box.size.x - w, 275, co, w);
    }
};

namespace me_constants {
    constexpr const float WIDTH{135.f};
    constexpr const float HEIGHT{380.f};
    constexpr const float me_CENTER{72.5f};
    constexpr const float TOP{18.f};
    constexpr const float LEFT_AXIS{40.f};
    constexpr const float FULL_WIDTH{WIDTH - 12.f};
    constexpr const float PALETTE_DX{15.f};
    constexpr const float LABEL_OFFSET_DX{5.f};
    constexpr const float ROW_DY{20.f};
    constexpr const float SMALL_ROW_DY{16.f};
    //constexpr const float LABEL_DY = 1.5f;
    constexpr const float OPT_CX{50.f};
    constexpr const float MINMAX_DX{13.f};
};

struct MacroEdit : OpaqueWidget, IApplyTheme
{
    using Base = OpaqueWidget;

    XMUi* ui{nullptr};
    MacroDescription macro{0,0};

    TextInput* title_entry{nullptr};
    Palette1Button * palette_fg{nullptr};
    Palette2Button * palette_bg{nullptr};
    
    TextLabel* knob_id{nullptr};
    TextInput* name_entry{nullptr};
    TextInput* macro_entry{nullptr};
    PlusMinusButton* input_port{nullptr};
    PlusMinusButton* modulation_button{nullptr};
    std::vector<StateIndicatorWidget*> range_options;

    PackedColor hi_color{0xffd9d9d9};

    MacroEdit() 
    {
        using namespace me_constants;
        box.size = Vec{WIDTH, HEIGHT};
    }

    ~MacroEdit() {
		APP->scene->rack->touchedParam = nullptr;
    }

    void set_ui(XMUi* w) { ui = w; }
    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override;
    void update_from_macro();
    void set_from_macro(const MacroDescription& mac);
    void set_range(MacroRange range);
    void create_ui(int knob_index, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme);
    static MacroEdit* createMacroEdit(Vec pos, XMUi* ui, Module * module, int knob_index, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme);
    void draw(const DrawArgs& args) override;
};

struct MacroMenu: Hamburger
{
    using Base = Hamburger;

    MacroEdit* edit{nullptr};
    void set_edit(MacroEdit* w) { edit = w; }

    XMUi* ui{nullptr};
    void set_ui(XMUi* w) { ui = w; }

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
                        edit->macro.macro_number = num;
                        edit->update_from_macro();
                        APP->event->setSelectedWidget(edit);
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

bool MacroEdit::applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) 
{
    auto style = theme->getStyle("ctl-label-hi");
    if (style) {
        hi_color = style->fillWithOpacity();
    } else {
        hi_color = 0xffd9d9d9;
    }
    return false;
}

void MacroEdit::update_from_macro()
{
    knob_id->text(std::string(1, KN[macro.knob_id]));
    name_entry->setText (macro.name);
    macro_entry->setText(macro_number_to_string(macro.macro_number));
    input_port->set_plus(!macro.modulated);

    int i = 0;
    for (auto r: range_options) {
        r->set_state(i == int(macro.range));
        ++i;
    }

    if (ui->my_module) {
        ui->my_module->getParam(XMModule::P_RANGE_MIN).setValue(macro.min);
        ui->my_module->getParam(XMModule::P_RANGE_MAX).setValue(macro.max);
    }
}

void MacroEdit::set_from_macro(const MacroDescription& mac)
{
    macro.init(mac);
    update_from_macro();
}

void MacroEdit::set_range(MacroRange range)
{
    macro.set_range(range);
    update_from_macro();
}

void MacroEdit::create_ui(int knob_index, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
    using namespace me_constants;

    LabelStyle mini_label_style{"ctl-label", TextAlignment::Center, 10.f, true};
    LabelStyle r_label_style{"ctl-label",  TextAlignment::Right, 14.f, false};
    LabelStyle header_style{"ctl-label-hi", TextAlignment::Center, 16.f, true};

    auto panel = createWidget<PanelBackgroundWidget>(Vec(0,0));
    addChild(panel);
    panel->expand_to_parent();

    TextButton* close = createWidget<TextButton>(Vec(box.size.x - 14.f, 1.5f));
    close->set_text("x");
    close->setHandler([=](bool, bool){ ui->set_edit_mode(false); });
    addChild(close);

    float x, y;
    x = LEFT_AXIS;
    y = TOP;

    addChild(createLabel(Vec(7.5f,y), FULL_WIDTH, "Section title:", theme_engine, theme, S::control_label_left));

    y += SMALL_ROW_DY;
    title_entry = createTextInput(7.5, y, FULL_WIDTH, 14.f, ui->get_header_text(),
        [=](std::string text){ ui->set_header_text(text); });
    addChild(title_entry);

    y += SMALL_ROW_DY;
    palette_fg = Center(createThemedButton<Palette1Button>(Vec(me_CENTER - PALETTE_DX, y + 6.f), theme_engine, theme, "Text color"));
    palette_fg->setHandler([=](bool,bool) {
        ui::Menu* menu = createMenu();
        auto picker = new ColorPickerMenu();
        picker->set_color(ui->get_header_text_color());
        picker->set_on_new_color([=](PackedColor color) {
            ui->set_header_text_color(color);
        });
        menu->addChild(picker);
    });
    addChild(palette_fg);

    palette_bg = Center(createThemedButton<Palette2Button>(Vec(me_CENTER + PALETTE_DX, y + 6.f), theme_engine, theme, "Background color"));
    palette_bg->setHandler([=](bool,bool) {
        ui::Menu* menu = createMenu();
        auto picker = new ColorPickerMenu();
        picker->set_color(ui->get_header_color());
        picker->set_on_new_color([=](PackedColor color) {
            ui->set_header_color(color);
        });
        menu->addChild(picker);
    });
    addChild(palette_bg);

    y += 12.5f;
    addChild(createLabel(Vec(me_CENTER - PALETTE_DX,y),25, "text", theme_engine, theme, mini_label_style));
    addChild(createLabel(Vec(me_CENTER + PALETTE_DX,y),25, "bg", theme_engine, theme, mini_label_style));

    macro.module_id = ui->module->id;
    macro.knob_id = knob_index;

    y += ROW_DY;
    addChild(knob_id = createLabel(Vec(me_CENTER, y), 40.f, std::string(1, KN[knob_index]), engine, theme, header_style));

    y += 4.f + ROW_DY;
    addChild(createLabel(Vec(x - LABEL_OFFSET_DX, y), 50.f, "Name", engine, theme, r_label_style));
    name_entry = createThemedTextInput(x, y, 90, 14,
        engine, theme,
        "",
        [=](std::string s){ macro.name = s; },
        nullptr,
        "<knob label>");
    addChild(name_entry);

    y += ROW_DY;
    addChild(createLabel(Vec(x - LABEL_OFFSET_DX, y), 60.f, "Macro", engine, theme, r_label_style));
    addChild(macro_entry = createThemedTextInput(x, y, 65, 14,
        engine, theme,
        "",
        [=](std::string s) { macro.macro_number = macro_number_from_string(s); }, 
        nullptr,
        "<macro number 7-90>"
        ));

    auto mm = createThemedWidget<MacroMenu>(Vec(x + 72.f, y + 1.5f), theme_engine, theme);
    mm->set_ui(ui);
    mm->set_edit(this);
    mm->describe("Available macros");
    addChild(mm);

    y += ROW_DY;
    input_port = createWidget<PlusMinusButton>(Vec(x,y));
    input_port->applyTheme(theme_engine, theme);
    input_port->set_plus(!macro.modulated);
    input_port->setHandler([=](bool, bool) {
        macro.modulated = input_port->plus;
    });
    addChild(input_port);
    addChild(createLabel(Vec(x + 16.f, y), 60.f, "Input port", engine, theme, S::control_label_left));

    y += ROW_DY;
    addChild(createLabel(Vec(x - LABEL_OFFSET_DX, y), 60.f, "Range", engine, theme, r_label_style));

    StateIndicatorWidget* indicator{nullptr};
    x = OPT_CX;
    MacroRange range{macro.range};

    indicator = createIndicatorCentered(x, y + 7.f, range == MacroRange::Bipolar, "Bipolar");
    indicator->applyTheme(theme_engine, theme);
    addChild(indicator);
    range_options.push_back(indicator);
    addChild(createLabel(Vec(x + 6.f, y), 50, "Bipolar", engine, theme, S::control_label_left));
    addChild(createClickRegion(x -6.f, y - 1.f, 60, 14, int(MacroRange::Bipolar), [=](int id, int) { 
        set_range(MacroRange(id)); 
    }));

    y += SMALL_ROW_DY;
    indicator = createIndicatorCentered(x, y + 7.f, range == MacroRange::Unipolar, "Unipolar");
    indicator->applyTheme(theme_engine, theme);
    addChild(indicator);
    range_options.push_back(indicator);
    addChild(createLabel(Vec(x + 6.f, y), 50, "Unipolar", engine, theme, S::control_label_left));
    addChild(createClickRegion(x -6.f, y - 1.f, 60, 14, int(MacroRange::Unipolar), [=](int id, int) { 
        set_range(MacroRange(id)); 
    }));

    y += SMALL_ROW_DY;
    indicator = createIndicatorCentered(x, y + 7.f, range == MacroRange::Custom, "Custom");
    indicator->applyTheme(theme_engine, theme);
    addChild(indicator);
    range_options.push_back(indicator);
    addChild(createLabel(Vec(x + 6.f, y), 50, "Custom", engine, theme, S::control_label_left));
    addChild(createClickRegion(x -6.f, y - 1.f, 60, 14, int(MacroRange::Custom), [=](int id, int) { 
        set_range(MacroRange(id)); 
    }));

    y += ROW_DY + 8.f;
    addChild(createChemKnob<GreenTrimPot>(Vec(me_CENTER - MINMAX_DX, y), ui->my_module, XMModule::P_RANGE_MIN, theme_engine, theme));
    addChild(createLabel(Vec(me_CENTER - MINMAX_DX, y + 9.5f), 25, "min", theme_engine, theme, mini_label_style));
    addChild(createChemKnob<GreenTrimPot>(Vec(me_CENTER + MINMAX_DX, y), ui->my_module, XMModule::P_RANGE_MAX, theme_engine, theme));
    addChild(createLabel(Vec(me_CENTER + MINMAX_DX, y + 9.5f), 25, "max", theme_engine, theme, mini_label_style));

    y += ROW_DY + 8.f;
    x = LEFT_AXIS;
    auto resetButton = Center(createThemedButton<DotButton>(Vec(x,y+ 7.f),engine, theme, "Reset"));
    resetButton->setHandler([=](bool,bool) { macro.clear(); update_from_macro(); });
    addChild(resetButton);
    addChild(createLabel(Vec(x + 10.f, y), 60.f, "Reset", theme_engine, theme, S::control_label_left));

    y = PORT_TOP_CY;
    x = 12.f;
    modulation_button = Center(createWidget<PlusMinusButton>(Vec(x,y)));
    modulation_button->applyTheme(theme_engine, theme);
    modulation_button->set_plus(!ui->get_modulation());
    modulation_button->setHandler([=](bool, bool) {
        ui->set_modulation(!ui->get_modulation());
    });
    addChild(modulation_button);
    addChild(createLabel(Vec(x + 10.f, y- 7.f), 60.f, "Modulation", engine, theme, S::control_label_left));

    // tab order
    title_entry->nextField = name_entry;
    name_entry->nextField = macro_entry;
    macro_entry->nextField = this;

    title_entry->prevField = this;
    name_entry->prevField = title_entry;
    macro_entry->prevField = name_entry;
};

MacroEdit* MacroEdit::createMacroEdit(Vec pos, XMUi* ui, Module * module, int knob_index, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
    MacroEdit* w = createWidget<MacroEdit>(pos);
    w->set_ui(ui);
    w->macro.module_id = module ? module->id : -1;
    w->macro.knob_id = knob_index;
    w->create_ui(knob_index, engine, theme);
    return w;
}

void MacroEdit::draw(const DrawArgs& args)
{
    auto vg = args.vg;

    Base::draw(args);

    auto w = ui->edit_style.width();
    auto hw = w*.5;
    auto co =ui->edit_style.nvg_stroke_color();
    Line(vg, 0, hw, box.size.x-hw, hw, co, w);
    Line(vg, box.size.x- hw, hw, box.size.x-hw, box.size.y-hw, co, w);
    Line(vg, 0, box.size.y-hw, box.size.x-hw, box.size.y-hw, co, w);

    auto kc = knob_id->box.getCenter();
    co = fromPacked(hi_color);
    OpenCircle(vg, kc.x, kc.y, 8.f, co, .85f);
    Line(vg, 7.5f, kc.y, kc.x - 8, kc.y, co, .85f);
    Line(vg, kc.x + 8, kc.y, box.size.x - 7.5f, kc.y, co, .85f);

    Line(vg, w*2, 275, box.size.x - w*2, 275, co, w);

}

// ====================================================================

XMUi::XMUi(XMModule *module) :
    my_module(module)
{
    setModule(module);
    bool browsing = !module;

    title_bg = browsing ? GetPackedStockColor(StockColor::pachde_blue_medium) : my_module->title_bg;
    title_fg = browsing ? GetPackedStockColor(StockColor::Gray_75p) : my_module->title_fg;

    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);

//    float x, y;

    title_bar = createWidget<Swatch>(Vec(0, 0));
    title_bar->box.size = Vec(PANEL_WIDTH, 15.f);
    title_bar->color = title_bg;
    addChild(title_bar);

    LabelStyle no_style{"", TextAlignment::Center, 12.f, true};
    title = createLabel(Vec(CENTER, 1.5), PANEL_WIDTH, browsing ? "XM": my_module->title, theme_engine, nullptr, no_style);
    title->color(fromPacked(title_fg));
    addChild(title);

    update_main_ui(theme);

    // inputs

    // Browsing UI
    if (browsing) {
        // all inputs
        addChild(createChemKnob<TrimPot>(Vec(CENTER, PORT_TOP_CY), my_module, XMModule::P_MODULATION, theme_engine, theme));
        
        const NVGcolor co_port = PORT_CORN;
        float axis = -1.f;
        for (int i = 0; i < 8; ++i) {
            auto pos = input_center(i);
            addChild(Center(createThemedColorInput(pos, my_module, i, S::InputColorKey, co_port, theme_engine, theme)));
            pos.x += axis*PORT_MOD_DX;
            pos.y -= PORT_MOD_DY;
            addChild(Center(createLight<TinySimpleLight<GreenLight>>(pos, my_module, i)));
            if (i == 3) {
                axis = 1.f;
            }
        }

        auto logo = new OpaqueLogo(.35f);
        logo->box.pos = Vec(CENTER, box.size.y*.3);
        addChild(Center(logo));
    }

    addChild(createLightCentered<TinyLight<GreenLight>>(Vec(CENTER-5, box.size.y - 3.5f), my_module, XMModule::L_OVERLAY));
    addChild(createLightCentered<TinyLight<BlueLight>>(Vec(CENTER+5, box.size.y - 3.5f), my_module, XMModule::L_CORE));

    // init
    if (!my_module || my_module->glow_knobs) {
        glowing_knobs(true);
    }

    if (my_module) {
        my_module->set_chem_ui(this);
//        onConnectHost(my_module->chem_host);
    }
}

void XMUi::update_main_ui(std::shared_ptr<SvgTheme> theme)
{
    if (!my_module) return;

    {
        memset(knobs, 0, sizeof(knobs));
        memset(labels, 0, sizeof(labels));
        std::vector<Widget*> removals;
        for (Widget* child: children) {
            if (child->box.pos.y > 15.f && child->box.pos.y< box.size.x -15.f) {
                removals.push_back(child);
            }
        }
        for (Widget* child: removals) {
            removeChild(child);
        }
    }
    
    TrimPot* knob{nullptr};
    if (get_modulation()) {
        knob = createChemKnob<TrimPot>(Vec(CENTER, PORT_TOP_CY), my_module, XMModule::P_MODULATION, theme_engine, theme);
        knobs[XMModule::P_MODULATION] = knob;
        addChild(knob);
    }

    draw_placeholders = my_module->my_macros.data.empty();

    const NVGcolor co_port = PORT_CORN;
    float axis = -1.f;
    for (auto macro : my_module->my_macros.data)
    {
        int i = macro->knob_id;
        axis = (i < 4) ? -1.f : 1.f;
        auto pos = knob_center(i);
        knob = createChemKnob<TrimPot>(pos, my_module, i, theme_engine, theme);
        knobs[i] = knob;
        addChild(knob);

        auto label = createLabel(Vec(pos.x, pos.y + 8.f), 45, macro->name, theme_engine, theme, S::med_control_label);
        labels[i] = label;
        addChild(label);

        if (macro->modulated) {
            pos = input_center(i);
            addChild(Center(createThemedColorInput(pos, my_module, i, S::InputColorKey, co_port, theme_engine, theme)));
            pos.x += axis*PORT_MOD_DX;
            pos.y -= PORT_MOD_DY;
            addChild(Center(createLight<TinySimpleLight<GreenLight>>(pos, my_module, i)));
        }
    }
}

void XMUi::glowing_knobs(bool glow)
{
    for (auto mw : knobs) {
        if (mw) mw->glowing(glow);
    }
}

void XMUi::center_knobs()
{
    if (!my_module) return;
}

Vec XMUi::knob_center(int index)
{
    return Vec(CENTER, KNOBS_TOP + (KNOB_DY * index));
}

Vec XMUi::input_center(int index)
{
    return Vec{
        index < 4 ? CENTER - PORT_OFFSET : CENTER + PORT_OFFSET, 
        PORT_TOP_CY + PORT_DY + ((index % 4) * PORT_DY)};
}

void XMUi::set_edit_mode(bool edit)
{
    editing = edit;
    if (editing) {
        if (!wire_frame) {
            wire_frame = createWidget<EditWireframe>(Vec(0,0));
            wire_frame->set_ui(this);
            addChild(wire_frame);
            box.size.x += me_constants::WIDTH;
            APP->scene->rack->setModulePosForce(this, box.pos);
        }
        set_edit_item(0);
    } else {
        if (edit_macro) {
            commit_macro();
            edit_macro->requestDelete();
            edit_macro = nullptr;
        }
        if (wire_frame) {
            removeChild(wire_frame);
            delete wire_frame;
            wire_frame = nullptr;
        }
        update_main_ui(theme_engine.getTheme(getThemeName()));
        box.size.x = PANEL_WIDTH;
        APP->scene->rack->setModulePosForce(this, box.pos);
    }
}

void XMUi::commit_macro()
{
    if (edit_macro)
    {
        auto overlay = get_overlay();
        MacroDescription& macro = edit_macro->macro;
        if (macro.valid()) {
            auto md = std::make_shared<MacroDescription>(macro.module_id, macro.knob_id);
            md->init(macro);
            my_module->my_macros.add_update(md);
            if (overlay) {
                overlay->overlay_add_update_macro(md);
            }
        } else {
            my_module->my_macros.remove(macro.module_id, macro.knob_id);
            if (overlay) {
                overlay->overlay_remove_macro(macro.module_id, macro.knob_id);
            }
        }
    }
}

void XMUi::set_edit_item(int index)
{
    assert(in_range(index, 0, 7));
    if (!edit_macro) {
        edit_macro = MacroEdit::createMacroEdit(Vec(45.f, 0), this, my_module, index, theme_engine, theme_engine.getTheme(getThemeName()));
        addChild(edit_macro);
    } else {
        commit_macro();
    }
    auto md = get_persistent_macro(index);
    if (md) {
        edit_macro->set_from_macro(*md);
    } else {
        edit_macro->macro.clear();
        edit_macro->macro.knob_id = index;
        edit_macro->update_from_macro();
    }
}

MacroDescription *XMUi::get_edit_macro()
{
    return edit_macro ? &edit_macro->macro : nullptr;
}

// void XMUi::setThemeName(const std::string& name, void * context)
// {
//     auto theme = theme_engine.getTheme(name);
//     Base::setThemeName(name, context);
// }

void XMUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    onConnectionChange(ChemDevice::Haken, host ? host->host_connection(ChemDevice::Haken) : nullptr);
}

void XMUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    //onConnectionChangeUiImpl(this, device, connection);
}

void XMUi::onHoverKey(const HoverKeyEvent &e)
{
    if (my_module) {
        if (e.action == GLFW_PRESS && ((e.mods & RACK_MOD_MASK) == 0)) {
            switch (e.key) {
            case GLFW_KEY_0:
                e.consume(this);
                //my_module->modulation.zero_modulation();
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

void  XMUi::xm_clear() {
    if (my_module) {
        title_fg = my_module->title_fg;
        title_bg = my_module->title_bg;
    }
    
    this->title->text("");
    title_bar->color = title_bg;
    title->color(fromPacked(title_fg));
}

std::string XMUi::get_header_text() { return title->getText(); }
PackedColor XMUi::get_header_color() { return title_bg; }
PackedColor XMUi::get_header_text_color() { return title_fg; }

void XMUi::set_header_color(PackedColor color)
{
    title_bg = color;
    title_bar->color = color;
    if (my_module) my_module->title_bg = color;
}

void XMUi::set_header_text_color(PackedColor color)
{
    title_fg = color;
    title->color(fromPacked(color));
    if (my_module) my_module->title_fg = color;
}

void XMUi::set_header_text(std::string title)
{
    this->title->text(title);
    if (my_module) my_module->title = title;
}

std::shared_ptr<MacroDescription> XMUi::get_persistent_macro(int index)
{
    if (!my_module) return nullptr;
    return my_module->my_macros.get_macro(my_module->id, index);
}

bool XMUi::get_modulation()
{
    return my_module ? my_module->has_mod_knob : true;
}

void XMUi::set_modulation(bool mod)
{
    if (my_module) my_module->has_mod_knob = mod;
}

void XMUi::step()
{
    Base::step();
    if (!my_module) return;

    bool left = panelBorder->left;
    bool right = panelBorder->right;
    if (!left) {
        auto e = my_module->getLeftExpander().module;
        left = e && e->getModel() == modelXM;
    }
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

void XMUi::setThemeName(const std::string& name, void * context)
{
    auto theme = theme_engine.getTheme(name);
    edit_style.apply_theme(theme);
    placeholder_style.apply_theme(theme);

    Base::setThemeName(name, context);
}

void XMUi::draw(const DrawArgs& args)
{
    Base::draw(args);

    if (my_module && draw_placeholders) {
        auto vg = args.vg;
        auto co = placeholder_style.nvg_stroke_color();
        auto w = placeholder_style.width();
        OpenCircle(vg, CENTER, PORT_TOP_CY, 8, co, w);
        for (int i = 0; i < 8; ++i) {
            auto pos{knob_center(i)};
            OpenCircle(vg, pos.x, pos.y, 8, co, w);
            pos = input_center(i);
            OpenCircle(vg, pos.x, pos.y, 4.5, co, w);
        }
    }
}

void XMUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
    menu->addChild(new MenuSeparator);
    menu->addChild(createCheckMenuItem("Edit", "", 
        [=](){ return editing; },
        [=](){ set_edit_mode(!editing); },
        !get_overlay()
    ));

   // if (editing) return;
    
    // bool unconnected = (my_module->inputs.end() == std::find_if(my_module->inputs.begin(), my_module->inputs.end(), [](Input& in){ return in.isConnected(); }));
    // menu->addChild(createMenuItem("Zero modulation", "0", [this](){
    //     my_module->modulation.zero_modulation();
    // }, unconnected));

    menu->addChild(new MenuSeparator);
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

