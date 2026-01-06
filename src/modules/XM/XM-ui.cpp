#include "XM.hpp"
#include "add-remove-button.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;

std::string macro_number_to_string(uint8_t mn)
{
    if (!in_range(mn, U8(7), U8(90))) return "";
    return format_string("%d", mn);
}

uint8_t macro_number_from_string(std::string s)
{
    int n = 0;
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
    return (valid && in_range(n, 7, 90)) ? n : 0;
}

constexpr const float PANEL_WIDTH   = 45.f;
constexpr const float CENTER        = PANEL_WIDTH*.5f;
constexpr const float KNOBS_TOP     = 28;
constexpr const float KNOB_DY       = 32;
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
        auto theme = ui->getSvgTheme();
        auto panel = createWidget<PanelBackgroundWidget>(Vec(0,15));
        panel->track();
        addChild(panel);

        for (ssize_t i = 0; i < 8; ++i) {
            auto pos = ui->knob_center(i);
            auto cr = Center(createClickRegion(pos.x, pos.y, 28.f, 24.f, i, [=](int i, int mods) {
                ui->set_edit_item(i);
            }));
            addChild(cr);
        }
        applyChildrenTheme(this, ui->getSvgTheme());
    }

    void draw(const DrawArgs& args) override
    {
        Base::draw(args);

        auto vg = args.vg;
        auto w = ui->edit_style.width();
        auto co = ui->edit_style.nvg_stroke_color();
        auto cofill = ui->edit_style.nvg_color();
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
                Circle(vg, xy.x, xy.y, 8, cofill);
                OpenCircle(vg, xy.x, xy.y, 8, co, w);
                nvgFillColor(vg, RampGray(G_0));
            } else {
                OpenCircle(vg, xy.x, xy.y, 8, co, w);
                nvgFillColor(vg, co);
            }
            nvgText(vg, xy.x, xy.y, KN+i,KN+i+1);

            if (md && (md->knob_id == i) && md->cv_port) {
                // input
                xy = ui->input_center(i);
                OpenCircle(vg, xy.x, xy.y, 4.5, co, w*.5);
            }
        }

        // mod knob
        if (!md || md->modulation) {
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
    constexpr const float OPT_CX{50.f};
    constexpr const float MINMAX_DX{13.f};
};

struct MacroEdit : OpaqueWidget, IThemed
{
    using Base = OpaqueWidget;

    XMUi* ui{nullptr};
    MacroDescription macro{-1,-1};

    TextInput* title_entry{nullptr};
    Palette1Button* palette_fg{nullptr};
    Palette2Button* palette_bg{nullptr};

    LabelStyle mini_label_style{"ctl-label", HAlign::Center, 10.f, true};
    LabelStyle r_label_style{"ctl-label",  HAlign::Right, 14.f, false};
    LabelStyle header_style{"ctl-label-hi", HAlign::Center, 16.f, true};

    TextLabel* knob_id{nullptr};
    TextInput* name_entry{nullptr};
    TextInput* macro_entry{nullptr};
    PlusMinusButton* add_input_port{nullptr};
    PlusMinusButton* add_modulation{nullptr};
    std::vector<StateIndicatorWidget*> range_options;
    GlowKnob* min_knob{nullptr};
    GlowKnob* max_knob{nullptr};
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
    void applyTheme(std::shared_ptr<SvgTheme> theme) override;
    void update_from_macro();
    void set_from_macro(const MacroDescription& mac);
    void set_range(MacroRange range);
    void create_ui(int knob_index, std::shared_ptr<SvgTheme> theme);
    static MacroEdit* createMacroEdit(Vec pos, XMUi* ui, Module * module, int knob_index);
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
            std::string text = "Preset ";
            if (conf) text.append(conf->summary());
            menu->addChild(createMenuLabel(text));

            auto live = overlay->overlay_live_preset();
            if (!live || (conf->id.key() != live->id.key())) {
                menu->addChild(createMenuLabel("Live preset is not the configured preset"));
                return;
            }

            switch (overlay->overlay_macros_ready()) {

            case MacroReadyState::Available: {
                auto macros{overlay->overlay_macro_usage()};
                if (macros.empty()) goto NO_MACROS;
                std::vector<uint8_t> used;
                overlay->overlay_used_macros(&used);
                for (const MacroUsage& mu: macros) {
                    auto num = mu.macro_number;
                    if (num < 7 || used.cend() != std::find(used.cbegin(), used.cend(), num)) {
                        menu->addChild(createMenuLabel(mu.to_string()));
                    } else {
                        menu->addChild(createMenuItem(mu.to_string(), "", [=](){
                            edit->macro.macro_number = num;
                            edit->update_from_macro();
                            APP->event->setSelectedWidget(edit);
                        }));
                    }
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

void MacroEdit::applyTheme(std::shared_ptr<SvgTheme> theme)
{
    auto style = theme->getStyle("ctl-label-hi");
    hi_color = style ? style->fillWithOpacity() : 0xffd9d9d9;
}

void MacroEdit::update_from_macro()
{
    knob_id->set_text(std::string(1, KN[macro.knob_id]));
    name_entry->setText (macro.name);
    macro_entry->setText(macro_number_to_string(macro.macro_number));
    add_input_port->set_plus(!macro.cv_port);
    add_modulation->set_plus(!macro.modulation);

    int i = 0;
    for (auto r: range_options) {
        r->set_state(i == int(macro.range));
        ++i;
    }

    min_knob->enable(true);
    max_knob->enable(true);
    auto mod = ui->my_module;
    if (mod) {
        mod->getParam(XMModule::P_RANGE_MIN).setValue(macro.min);
        mod->getParam(XMModule::P_RANGE_MAX).setValue(macro.max);
    }
    min_knob->enable(macro.range == MacroRange::Custom);
    max_knob->enable(macro.range == MacroRange::Custom);
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

void MacroEdit::create_ui(int knob_index, std::shared_ptr<SvgTheme> theme)
{
    using namespace me_constants;

    TextButton* close = createWidget<TextButton>(Vec(box.size.x - 14.f, 1.5f));
    close->set_text("x");
    close->setHandler([=](bool, bool){ ui->set_edit_mode(false); });
    addChild(close);

    float x, y;
    x = LEFT_AXIS;
    y = TOP;

    addChild(createLabel(Vec(7.5f,y), "Title:", &S::control_label_left, FULL_WIDTH));

    y += SMALL_ROW_DY;
    title_entry = createThemedTextInput(7.5f, y, FULL_WIDTH, 14.f,
        ui->get_header_text(),
        [=](std::string text){ ui->set_header_text(text); },
        nullptr,
        "module name");
    addChild(title_entry);

    y += SMALL_ROW_DY;
    palette_fg = Center(createThemedButton<Palette1Button>(Vec(me_CENTER - PALETTE_DX, y + 6.f), &ui->module_svgs, "Text color"));
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

    palette_bg = Center(createThemedButton<Palette2Button>(Vec(me_CENTER + PALETTE_DX, y + 6.f), &ui->module_svgs, "Background color"));
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
    addChild(createLabelCentered(Vec(me_CENTER - PALETTE_DX,y), "text", &mini_label_style, 25.f));
    addChild(createLabelCentered(Vec(me_CENTER + PALETTE_DX,y), "bg", &mini_label_style, 25.f));

    macro.module_id = ui->module->id;
    macro.knob_id = knob_index;

    y += ROW_DY;
    addChild(knob_id = createLabelCentered(Vec(me_CENTER, y), std::string(1, KN[knob_index]), &header_style, 40.f));

    y += 4.f + ROW_DY;
    addChild(createLabelRight(Vec(x - LABEL_OFFSET_DX, y), "Name", &r_label_style, 50.f));
    name_entry = createThemedTextInput(x, y, 90, 14,
        "",
        [=](std::string s){ macro.name = s; },
        nullptr,
        "knob name");
    addChild(name_entry);

    y += ROW_DY;
    addChild(createLabelRight(Vec(x - LABEL_OFFSET_DX, y), "Macro", &r_label_style, 60.f));
    addChild(macro_entry = createThemedTextInput(x, y, 65, 14,
        "",
        [=](std::string s) { macro.macro_number = macro_number_from_string(s); },
        nullptr,
        "macro #"
        ));

    auto mm = createWidget<MacroMenu>(Vec(x + 72.f, y + 1.5f));
    mm->set_ui(ui);
    mm->set_edit(this);
    mm->describe("Available macros");
    addChild(mm);

    y += ROW_DY;
    add_input_port = createWidget<PlusMinusButton>(Vec(x,y));
    add_input_port->applyTheme(theme);
    add_input_port->set_plus(!macro.cv_port);
    add_input_port->setHandler([=](bool, bool) {
        macro.cv_port = add_input_port->plus;
    });
    addChild(add_input_port);
    addChild(createLabel(Vec(x + 16.f, y), "Input jack", &S::control_label_left, 60.f));

    y += ROW_DY;
    add_modulation = createWidget<PlusMinusButton>(Vec(x,y));
    add_modulation->applyTheme(theme);
    add_modulation->set_plus(!macro.modulation);
    add_modulation->setHandler([=](bool, bool) {
        macro.modulation = add_modulation->plus;
    });
    addChild(add_modulation);
    addChild(createLabel(Vec(x + 16.f, y), "Modulation", &S::control_label_left, 60.f));

    y += ROW_DY;
    addChild(createLabel(Vec(x - LABEL_OFFSET_DX, y), "Range", &r_label_style, 60.f));

    StateIndicatorWidget* indicator{nullptr};
    x = OPT_CX;
    MacroRange range{macro.range};

    indicator = createIndicatorCentered(x, y + 7.f, range == MacroRange::Bipolar, "Bipolar");
    indicator->applyTheme(theme);
    addChild(indicator);
    range_options.push_back(indicator);
    addChild(createLabel(Vec(x + 6.f, y), "Bipolar", &S::control_label_left, 50));
    addChild(createClickRegion(x -6.f, y - 1.f, 60, 14, int(MacroRange::Bipolar), [=](int id, int) {
        set_range(MacroRange(id));
    }));

    y += SMALL_ROW_DY;
    indicator = createIndicatorCentered(x, y + 7.f, range == MacroRange::Unipolar, "Unipolar");
    indicator->applyTheme(theme);
    addChild(indicator);
    range_options.push_back(indicator);
    addChild(createLabel(Vec(x + 6.f, y), "Unipolar", &S::control_label_left, 50));
    addChild(createClickRegion(x -6.f, y - 1.f, 60, 14, int(MacroRange::Unipolar), [=](int id, int) {
        set_range(MacroRange(id));
    }));

    y += SMALL_ROW_DY;
    indicator = createIndicatorCentered(x, y + 7.f, range == MacroRange::Custom, "Custom");
    indicator->applyTheme(theme);
    addChild(indicator);
    range_options.push_back(indicator);
    addChild(createLabel(Vec(x + 6.f, y), "Custom", &S::control_label_left, 50));
    addChild(createClickRegion(x -6.f, y - 1.f, 60, 14, int(MacroRange::Custom), [=](int id, int) {
        set_range(MacroRange(id));
    }));

    y += ROW_DY + 8.f;
    addChild(min_knob = createChemKnob<GreenTrimPot>(Vec(me_CENTER - MINMAX_DX, y), &ui->module_svgs, ui->my_module, XMModule::P_RANGE_MIN));
    addChild(createLabelCentered(Vec(me_CENTER - MINMAX_DX, y + 9.5f), "min", &mini_label_style, 25));
    addChild(max_knob = createChemKnob<GreenTrimPot>(Vec(me_CENTER + MINMAX_DX, y), &ui->module_svgs, ui->my_module, XMModule::P_RANGE_MAX));
    addChild(createLabelCentered(Vec(me_CENTER + MINMAX_DX, y + 9.5f), "max", &mini_label_style, 25));

    y += ROW_DY + 8.f;
    x = LEFT_AXIS;
    auto resetButton = Center(createThemedButton<DotButton>(Vec(x,y+ 7.f), &ui->module_svgs, "Reset"));
    resetButton->setHandler([=](bool,bool) { macro.clear(); update_from_macro(); });
    addChild(resetButton);
    addChild(createLabel(Vec(x + 10.f, y), "Reset", &S::control_label_left, 60));

    // tab order
    title_entry->nextField = name_entry;
    name_entry->nextField = macro_entry;
    macro_entry->nextField = this;

    title_entry->prevField = this;
    name_entry->prevField = title_entry;
    macro_entry->prevField = name_entry;

    applyChildrenTheme(this, theme);
};

MacroEdit* MacroEdit::createMacroEdit(Vec pos, XMUi* ui, Module * module, int knob_index)
{
    MacroEdit* w = createWidget<MacroEdit>(pos);
    w->set_ui(ui);
    w->macro.module_id = module ? module->id : -1;
    w->macro.knob_id = knob_index;
    w->create_ui(knob_index, ui->getSvgTheme());
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

    //Line(vg, w*2, 275, box.size.x - w*2, 275, co, w);

}

// ====================================================================

XMUi::XMUi(XMModule *module) :
    my_module(module)
{
    setModule(module);
    bool browsing = !module;

    title_bg = browsing ? GetPackedStockColor(StockColor::pachde_blue_medium) : my_module->title_bg;
    title_fg = browsing ? GetPackedStockColor(StockColor::Gray_75p) : my_module->title_fg;

    auto theme = getSvgTheme();
    auto panel = createThemedPanel(panelFilename(), &module_svgs);
    panelBorder = attachPartnerPanelBorder(panel);
    setPanel(panel);
    edit_style.apply_theme(theme);
    placeholder_style.apply_theme(theme);

    {
        auto flyout_background = createWidget<PanelBackgroundWidget>(Vec(0,0));
        flyout_background->track();
        flyout_background->applyTheme(theme);
        addChildBottom(flyout_background);
    }

    title_bar = createWidget<Swatch>(Vec(0, 0));
    title_bar->box.size = Vec(PANEL_WIDTH, 15.f);
    title_bar->color = title_bg;
    addChild(title_bar);

    title = createLabelCentered(Vec(CENTER, 1.5), browsing ? "XM": my_module->title, &no_style, PANEL_WIDTH);
    title->set_color(title_fg);
    addChild(title);

    update_main_ui(theme);

    // Browsing UI
    if (browsing) {

        addChild(createChemKnob<TrimPot>(Vec(CENTER, PORT_TOP_CY), &module_svgs, my_module, XMModule::P_MODULATION));

        const NVGcolor co_port = PORT_CORN;
        float axis = -1.f;
        Vec pos;
        TrimPot* knob{nullptr};
        TrackWidget* track{nullptr};
        for (int i = 0; i < 8; ++i) {
            if (i < 2 || i > 3) {
                pos = knob_center(i);
                knob = createChemKnob<TrimPot>(pos, &module_svgs, nullptr, i);
                addChild(knob);
                track = createTrackWidget(knob);
                addChild(track);
            }

            pos = input_center(i);
            addChild(Center(createThemedColorInput(pos, &module_svgs, my_module, i, S::InputColorKey, co_port)));
            pos.x += axis*PORT_MOD_DX;
            pos.y -= PORT_MOD_DY;
            addChild(Center(createLight<TinySimpleLight<GreenLight>>(pos, my_module, i)));
            if (i == 3) {
                axis = 1.f;
            }
        }
        if (S::show_browser_logo()) {
            auto logo = new OpaqueLogo(.35f);
            logo->box.pos = Vec(CENTER, box.size.y*.3);
            addChild(Center(logo));
        }
    }

    addChild(createLightCentered<TinyLight<GreenLight>>(Vec(CENTER-5, box.size.y - 3.5f), my_module, XMModule::L_OVERLAY));
    addChild(createLightCentered<TinyLight<BlueLight>>(Vec(CENTER+5, box.size.y - 3.5f), my_module, XMModule::L_CORE));

    module_svgs.changeTheme(theme);
    applyChildrenTheme(this, theme);

    // init
    if (!my_module || my_module->glow_knobs) {
        glowing_knobs(true);
    }

    if (my_module) {
        my_module->set_chem_ui(this);
//        onConnectHost(my_module->chem_host);
    }
}

void XMUi::clear_dynamic_ui()
{
    memset(knobs, 0, sizeof(knobs));
    memset(labels, 0, sizeof(labels));
    memset(tracks, 0, sizeof(tracks));

    for (int i = 0; i < 8; ++i) {
        auto pc = port_click[i];
        if (pc) {
            removeChild(pc);
            delete pc;
        }
        auto pl = port_light[i];
        if (pl) {
            removeChild(pl);
            delete pl;
        }
    }
    memset(port_click, 0, sizeof(port_click));
    memset(port_light, 0, sizeof(port_light));

    std::vector<Widget*> removals;
    for (Widget* child: children) {
        if (in_range(child->box.pos.y, 6.f, PORT_TOP_CY)) {
            removals.push_back(child);
        }
    }
    for (Widget* child: removals) {
        removeChild(child);
        delete child;
    }
}

void XMUi::update_main_ui(std::shared_ptr<SvgTheme> theme)
{
    if (!my_module) return;

    clear_dynamic_ui();

    TrimPot* knob{nullptr};
    TrackWidget* track{nullptr};

    if (has_mod_knob()) {
        knob = createChemKnob<TrimPot>(Vec(CENTER, PORT_TOP_CY), &module_svgs, my_module, XMModule::P_MODULATION);
        knobs[XMModule::P_MODULATION] = knob;
        addChild(knob);
    }

    draw_placeholders = my_module->my_macros.data.empty();
    my_module->update_param_info();

    bool inputs[8]{false};
    const NVGcolor co_port = PORT_CORN;
    float axis = -1.f;
    for (auto macro : my_module->my_macros.data) {
        int i = macro->knob_id;
        axis = (i < 4) ? -1.f : 1.f;
        auto pos = knob_center(i);
        if (macro->has_param_knob()) {
            knob = createChemKnob<TrimPot>(pos, &module_svgs, my_module, i);
            knobs[i] = knob;
            addChild(knob);
        }

        auto label = createLabelCentered(Vec(pos.x, pos.y + 8.f), macro->name, &S::med_label, 45);
        labels[i] = label;
        addChild(label);

        if (macro->cv_port) {
            inputs[i] = true;
            if (macro->has_param_knob()) {
                track = createTrackWidget(knob);
            } else {
                auto angle = 0.83 * M_PI;
                track = Center(createTrackWidget<TrackIndicator>(pos, 12.f, -angle, angle));
            }
            track->dot_radius = 1.75;
            track->set_min_max_value(-1.f, 1.f);
            tracks[i] = track;
            addChild(track);

            pos = input_center(i);
            if (nullptr == ports[i]) {
                ports[i] = Center(createThemedColorInput(pos, &module_svgs, my_module, i, S::InputColorKey, co_port));
                addChild(ports[i]);
            }
            port_click[i] = Center(createClickRegion(pos.x + 2 * axis, pos.y, 22.f, 19.5f, i, [=](int id, int mods) { my_module->set_modulation_target(id); }));
            addChild(port_click[i]);

            pos.x += axis * PORT_MOD_DX;
            pos.y -= PORT_MOD_DY;
            port_light[i] = Center(createLight<TinySimpleLight<GreenLight>>(pos, my_module, i));
            addChild(port_light[i]);
        } else {
            auto port = ports[macro->knob_id];
            if (port) {
                ports[macro->knob_id] = nullptr;
                port->requestDelete();
            }
        }
    }
    for (int i = 0; i < 8; ++i) {
        if (ports[i] && !inputs[i]) {
            ports[i]->requestDelete();
            ports[i] = nullptr;
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
    my_module->center_knobs();
}

Vec XMUi::knob_center(int index)
{
    return Vec(CENTER, KNOBS_TOP + (KNOB_DY * index));
}

Vec XMUi::input_center(int index)
{
    return Vec {
        index < 4 ? CENTER - PORT_OFFSET : CENTER + PORT_OFFSET,
        PORT_TOP_CY + PORT_DY + ((index % 4) * PORT_DY)
    };
}

void XMUi::save_module_positions()
{
    fill_right_module_positions(module_positions, this, me_constants::WIDTH);
}

void XMUi::restore_module_positions()
{
    restore_right_module_positions(module_positions, this);
    module_positions.clear();
}

void XMUi::set_edit_mode(bool edit)
{
    if (edit) {
        if (my_module && my_module->overlay) {
            my_module->overlay->overlay_client_pause(my_module, true);
        }
        clear_dynamic_ui();
        if (!wire_frame) {
            wire_frame = createWidget<EditWireframe>(Vec(0,0));
            wire_frame->set_ui(this);
            addChild(wire_frame);

            save_module_positions();
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
        update_main_ui(getSvgTheme());

        box.size.x = PANEL_WIDTH;
        APP->scene->rack->setModulePosForce(this, box.pos);
        restore_module_positions();

        if (my_module && my_module->overlay) {
            my_module->overlay->overlay_client_pause(my_module, false);
        }
    }
    editing = edit;
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
        edit_macro = MacroEdit::createMacroEdit(Vec(45.f, 0), this, my_module, index);
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

MacroDescription* XMUi::get_edit_macro()
{
    return edit_macro ? &edit_macro->macro : nullptr;
}

void XMUi::onHoverKey(const HoverKeyEvent &e)
{
    if (my_module) {
        if (e.action == GLFW_PRESS && ((e.mods & RACK_MOD_MASK) == 0)) {
            switch (e.key) {
            case GLFW_KEY_F2:
                set_edit_mode(!editing);
                e.consume(this);
                return;

            // case GLFW_KEY_0:
            //     //zero_modulation();
            //     e.consume(this);
            //     return;

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

    this->title->set_text("");
    title_bar->color = title_bg;
    title->format->color = title_fg;
}

std::string XMUi::get_header_text() { return title->get_text(); }
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
    title->format->color = color;
    if (my_module) my_module->title_fg = color;
}

void XMUi::set_header_text(std::string title)
{
    this->title->set_text(title);
    if (my_module) my_module->title = title;
}

std::shared_ptr<MacroDescription> XMUi::get_persistent_macro(int index)
{
    if (!my_module) return nullptr;
    return my_module->my_macros.get_macro(my_module->id, index);
}

bool XMUi::has_mod_knob()
{
    return my_module ? my_module->has_mod_knob() : true;
}

void XMUi::setThemeName(const std::string& name, void * context)
{
    Base::setThemeName(name, context);
    auto theme = getSvgTheme();
    edit_style.apply_theme(theme);
    placeholder_style.apply_theme(theme);
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

    for (auto macro : my_module->my_macros.data) {
        int i = macro->knob_id;
        if ((i == my_module->mod_target) && knobs[XMModule::P_MODULATION]) {
            knobs[XMModule::P_MODULATION]->enable(macro->modulation);
        }
        if (tracks[i]) {
            tracks[i]->set_active(my_module->getInput(i).isConnected());
            tracks[i]->set_value(macro->mod_value);
        }
    }
    if (my_module->overlay) {
        auto info = my_module->getLightInfo(XMModule::L_OVERLAY);
        auto title = my_module->overlay->overlay_title();
        info->name = title.empty() ? "Overlay" : title + " connected";
    }
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
    menu->addChild(createCheckMenuItem("Edit", "F2",
        [=](){ return editing; },
        [=](){ set_edit_mode(!editing); },
        !get_overlay()
    ));

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

