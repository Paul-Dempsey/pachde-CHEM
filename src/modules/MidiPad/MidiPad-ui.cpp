#include "MidiPad.hpp"
#include "services/colors.hpp"
#include "em/em-hardware.h"
#include "widgets/widgets.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;

// -- UI --------------------------------------
constexpr const float PANEL_WIDTH = 120.f;
//constexpr const float CENTER = PANEL_WIDTH*.5f;
constexpr const float EDIT_CY = 190.f;

namespace pad_edit_constants {
    constexpr const float HEIGHT{380.f};
    constexpr const float WIDTH{PANEL_WIDTH};
    constexpr const float CENTER{WIDTH*.5f};
    constexpr const float PALETTE_DX{12.f};
    constexpr const float SECTION_TOP{46};
};

struct PadEdit : OpaqueWidget
{
    using Base = OpaqueWidget;

    bool modified{false};
    bool initializing{false};
    int pad_id{-1};

    PackedColor hi_color{0xffd9d9d9};

    MidiPadUi* ui{nullptr};
    Palette1Button* name_palette{nullptr};
    Palette2Button* pad_palette{nullptr};
    TextLabel* coordinate{nullptr};
    TextInput* title_field{nullptr};
    TextInput* name_field{nullptr};
    //TextInput* midi_field{nullptr};
    MultiTextInput* midi_field{nullptr};
    TipLabel* status{nullptr};

    PadEdit() {
        using namespace pad_edit_constants;
        box.size = Vec(WIDTH, HEIGHT);
    }

    void commit(bool force = false)
    {
        if (-1 == pad_id) {
            status->text("");
            status->describe("");
            return;
        }

        assert(pad_id == ui->edit_pad);
        assert(in_range(pad_id, 0, 15));
        if (modified || force) {
            auto pad = ui->get_pad(pad_id);
            pad->def = midi_field->getText();
            pad->compile();
            set_status(pad);
            if (pad->ok) {
                status->text("OK");
            }
            ui->pad_ui[pad_id]->on_pad_change(true, true);
            modified = false;
        }
    }

    void send_pad()
    {
        commit(true);
        ui->send_pad(pad_id);
    }

    void clear_pad()
    {
        if (-1 == pad_id) {
            commit();
            return;
        }
        ui->remove_pad(pad_id);
        ui->my_module->ensure_pad(pad_id);
        ui->pad_ui[pad_id]->set_pad(ui->get_pad(pad_id));
        set_edit_pad(pad_id);
    }

    void set_edit_pad(int id)
    {
        commit();

        initializing = true;

        pad_id = id;
        auto pad = ui->get_pad(pad_id);
        coordinate->text(default_pad_name[pad_id]);
        name_field->setText(pad->name);
        midi_field->setText(pad->def);
        set_status(pad);
        modified = false;

        initializing = false;
    }

    void on_title_text_changed(std::string text)
    {
        if (!ui->my_module) return;
        ui->my_module->title = text;
        ui->title->text(text);
    }

    void on_name_text_changed(std::string text)
    {
        if (initializing) return;
        assert(pad_id == ui->edit_pad);
        assert(in_range(pad_id, 0, 15));

        modified = true;

        ui->get_pad(pad_id)->name = text;
        ui->pad_ui[pad_id]->on_pad_change(true, false);
    }

    void set_status(std::shared_ptr<pachde::MidiPad> pad)
    {
        if (!pad->ok) {
            status->text(ellipse_string(pad->error_message, 30));
            status->describe(pad->error_message);
            midi_field->cursor = pad->error_pos;
            midi_field->selection = pad->error_pos;
        } else {
            status->text("");
            status->describe("");
        }
    }

    void on_midi_text_changed(std::string text)
    {
        if (initializing) return;

        assert(pad_id == ui->edit_pad);
        assert(in_range(pad_id, 0, 15));

        modified = true;

        auto pad = ui->get_pad(pad_id);
        pad->def = text;
        pad->compile();
        set_status(pad);
        ui->pad_ui[pad_id]->on_pad_change(false, true);
    }

    void init(MidiPadUi* mw)
    {
        using namespace pad_edit_constants;
        initializing = true;
        ui = mw;
        auto theme = ui->getSvgTheme();
        LabelStyle mini_label_style{"ctl-label", TextAlignment::Center, 10.f, true};
        LabelStyle section_label_style{"ctl-label-hi", TextAlignment::Center, 14.f, true};
        {
            auto style = theme->getStyle("ctl-label-hi");
            hi_color = style ? style->fillWithOpacity() : 0xffd9d9d9;
        }
        float y;

        TextButton* close = createWidget<TextButton>(Vec(box.size.x - 20.f, 5.f));
        close->set_text("x");
        close->setHandler([=](bool, bool){ ui->edit_mode(false); });
        addChild(close);

        y = 22.f;
        title_field = createThemedTextInput(15.f, y, 90.f, 14.f, ui->my_module ? ui->my_module->title : "",
            [=](std::string text){ on_title_text_changed(text); },
            nullptr,
            "Module title");
        title_field->text_height = 12.f;
        addChild(title_field);

        y = SECTION_TOP;
        addChild(coordinate = createLabel(Vec(CENTER,y), 40, "", section_label_style));

        y += 18;

        addChild(createLabel(Vec(3.5, y), 28, "Name", ::pachde::style::control_label_left));

        name_field = createThemedTextInput(35.5f, y, 60.f, 14.f, "",
            [=](std::string text){ on_name_text_changed(text); },
            nullptr,
            "Pad name");
        name_field->text_height = 12.f;
        addChild(name_field);

        y += 24.f;

        // compile / test / clear buttons

        name_palette = Center(createThemedButton<Palette1Button>(Vec(CENTER - PALETTE_DX,y), &ui->module_svgs, "Text color"));
        name_palette->setHandler([=](bool,bool) {
            ui::Menu* menu = createMenu();
            auto picker = new ColorPickerMenu();
            picker->set_color(ui->get_pad_text_color());
            picker->set_on_new_color([=](PackedColor color) {
                ui->set_pad_text_color(color);
            });
            menu->addChild(picker);
        });
        addChild(name_palette);

        pad_palette = Center(createThemedButton<Palette2Button>(Vec(CENTER + PALETTE_DX,y), &ui->module_svgs, "Pad color"));
        pad_palette->setHandler([=](bool,bool) {
            ui::Menu* menu = createMenu();
            auto picker = new ColorPickerMenu();
            picker->set_color(ui->get_pad_color());
            picker->set_on_new_color([=](PackedColor color) {
                ui->set_pad_color(color);
            });
            menu->addChild(picker);
        });
        addChild(pad_palette);

        y += 6.5f;
        addChild(createLabel(Vec(CENTER - PALETTE_DX,y),25, "text", mini_label_style));
        addChild(createLabel(Vec(CENTER + PALETTE_DX,y),25, "pad", mini_label_style));

        y += 16.f;

        midi_field = createWidget<MultiTextInput>(Vec(3.5, y));
        midi_field->box.size = Vec(113, 132);
        midi_field->set_on_change( [=](std::string text){
            modified = true;
            status->text("");
            status->describe("");
        });
        midi_field->set_on_enter( [=](std::string text){ on_midi_text_changed(text); });
        midi_field->placeholder = "Pad Midi";
        addChild(midi_field);

        y += 146;
        float x = CENTER - 28.f;
        auto btn = Center(createThemedButton<SquareButton>(Vec(x,y), &ui->module_svgs, "Clear"));
        btn->latched = false;
        btn->setHandler([=](bool,bool) { clear_pad(); });
        addChild(btn);
        addChild(createLabel(Vec(x,y + 8.f),25, "clear", mini_label_style));

        x = CENTER;
        btn = createThemedButton<SquareButton>(Vec(x,y), &ui->module_svgs, "Compile definition to MIDI");
        btn->setHandler([=](bool,bool){ commit(true); });
        addChild(Center(btn));
        addChild(createLabel(Vec(x,y + 8.f),34, "compile", mini_label_style));

        x = CENTER + 28.f;
        btn = createThemedButton<SquareButton>(Vec(x,y), &ui->module_svgs, "Send MIDI (test)");
        btn->setHandler([=](bool,bool){ send_pad(); });
        addChild(Center(btn));
        addChild(createLabel(Vec(x,y + 8.f),25, "send", mini_label_style));

        y += 24.f;
        addChild(status = createLabel<TipLabel>(Vec(3.5, y), WIDTH - 7, "", S::warning_label));

        // tab order
        title_field->nextField = name_field;
        name_field->nextField = midi_field;
        midi_field->nextField = title_field;

        title_field->prevField = midi_field;
        name_field->prevField = title_field;
        midi_field->prevField = name_field;

        applyChildrenTheme(this, ui->getSvgTheme());
        initializing = false;
    }

    void draw(const DrawArgs& args) override
    {
        using namespace pad_edit_constants;
        Base::draw(args);
        auto co = fromPacked(hi_color);
        Line(args.vg, 0.f, SECTION_TOP + 7.5, CENTER - 20.f, SECTION_TOP + 7.5, co, .85f);
        Line(args.vg, CENTER + 20.f, SECTION_TOP + 7.5, box.size.x -2, SECTION_TOP + 7.5, co, .85f);
    }
};

MidiPadUi::MidiPadUi(MidiPadModule *module) :
    my_module(module)
{
    setModule(module);
    auto theme = getSvgTheme();

    {
        auto panel = createThemedPanel(panelFilename(), &module_svgs);
        panelBorder = attachPartnerPanelBorder(panel);
        setPanel(panel);
    }

    {
        auto flyout_background = createWidget<PanelBackgroundWidget>(Vec(0,0));
        flyout_background->track();
        flyout_background->applyTheme(theme);
        addChildBottom(flyout_background);
    }
    if (S::show_screws()) {
        addChild(createThemedWidget<ThemeScrew>(Vec(0, 0), &module_svgs));
        addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - RACK_GRID_WIDTH, 0), &module_svgs));
    }
    addChild(title = createLabel(Vec(box.size.x*.5f, 20.f), 120.f - 7.5f, my_module ? my_module->title : "Midi Pad", LabelStyle{"ctl-label-hi", TextAlignment::Center, 16.f, true}));

    float x = 15.f;
    float y = 60.f;
    for (int i = 0; i < 16; ++i) {
        auto w = createWidget<PadWidget>(Vec(x,y));
        w->init(i, my_module ? my_module->pad_defs[i] : nullptr, my_module,
            theme,
            [=](int id) { on_click_pad(id); });
        pad_ui[i] = w;
        addChild(w);
        if (3 == (i % 4)) {
            x = 15.f;
            y += 25.5f;
        } else {
            x += 25.5f;
        }
    }

    edit_button = Center(createThemedButton<EditButton>(Vec(box.size.x*.5, EDIT_CY), &module_svgs, "Edit (F2)"));
    edit_button->set_sticky(true);
    if (my_module) {
        edit_button->setHandler([=](bool c, bool s){ edit_mode(!my_module->editing); });
    }
    addChild(edit_button);

    addChild(createLightCentered<TinyLight<YellowLight>>(Vec(box.size.x*.5 + 12.5, EDIT_CY - 6.5f), my_module, MidiPadModule::L_EDITING));

    // inputs
    const float INPUT_TOP_CY = 292.5f;
    const float INPUT_ROW_CX = 28.f;
    const float INPUT_DY = 19.5f;
    const float INPUT_DX = 25.f;
    const NVGcolor co_port = PORT_CORN;

    x = INPUT_ROW_CX;
    y = INPUT_TOP_CY;
    for (int i = 0; i < 16; ++i) {
        addChild(Center(createThemedColorInput(Vec(x, y), &module_svgs, my_module, i, S::InputColorKey, co_port)));
        if (3 == i % 4) {
            x = INPUT_ROW_CX;
            y += INPUT_DY;
        } else {
            x += INPUT_DX;
        }
    }

    // footer
    addChild(haken_device_label = createLabel<TipLabel>(
        Vec(28.f, box.size.y - 13.f), 200.f, S::NotConnected, S::haken_label));

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y - S::U1), &module_svgs, "Core link");
    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ModuleBroker::get()->addHostPickerMenu(createMenu(), my_module);
        });
    }
    addChild(link_button);

    if (!my_module && S::show_browser_logo()) {
        auto logo = new OpaqueLogo(.55f);
        logo->box.pos = Vec(box.size.x*.5f, 232.f);
        addChild(Center(logo));
    }

    module_svgs.changeTheme(theme);
    applyChildrenTheme(this, theme);

    if (my_module) {
        my_module->set_chem_ui(this);
        onConnectHost(my_module->chem_host);
    }
}

void changePanelWidth(SvgPanel* panel, float width)
{
    //panel->sw->box.size.x = width;
    panel->fb->box.size.x = width;
	panel->panelBorder->box.size.x = width;
	panel->box.size.x = width;
	panel->fb->setDirty();
}

void MidiPadUi::refresh()
{
    if (!my_module) return;
    title->text(my_module->title);
    for (int i = 0; i < 16; ++i) {
        pad_ui[i]->set_pad(my_module->pad_defs[i]);
    }
}

void MidiPadUi::save_module_positions()
{
    fill_right_module_positions(module_positions, this, pad_edit_constants::WIDTH);
}

void MidiPadUi::restore_module_positions()
{
    restore_right_module_positions(module_positions, this);
    module_positions.clear();
}

void MidiPadUi::edit_mode(bool editing)
{
    if (!my_module) return;
    my_module->editing = editing;
    edit_button->describe(editing ? "Stop editing pads" : "Edit pads");

    if (editing) {
        int edit_id = edit_pad;
        if (-1 == edit_pad) {
            auto pad = my_module->first_pad();
            edit_id = pad ? pad->id : 0;
        }

        save_module_positions();

        float x = box.size.x;
        box.size.x += pad_edit_constants::WIDTH;
        changePanelWidth(dynamic_cast<SvgPanel*>(getPanel()), box.size.x);
        APP->scene->rack->setModulePosForce(this, box.pos);

        edit_ui = createWidget<PadEdit>(Vec(x, 0));
        edit_ui->init(this);
        addChild(edit_ui);
        set_edit_pad(edit_id);
    } else {
        for (int i = 0; i < 16; ++i) {
            pad_ui[i]->selected = false;
        }

        edit_ui->commit();
        edit_ui->requestDelete();
        edit_ui = nullptr;

        box.size.x = PANEL_WIDTH;
        changePanelWidth(dynamic_cast<SvgPanel*>(getPanel()), PANEL_WIDTH);
        APP->scene->rack->setModulePosForce(this, box.pos);

        restore_module_positions();
    }
}

void MidiPadUi::set_edit_pad(int id)
{
    assert(in_range(id, 0, 15));
    assert(my_module->editing);
    if (-1 != edit_pad) {
        edit_ui->commit();
    }
    my_module->edit_pad = id;
    my_module->ensure_pad(id);
    auto pad = get_pad(id);
    if (nullptr == pad_ui[id]->pad) {
        pad_ui[id]->set_pad(pad);
    }
    for (int i = 0; i < 16; ++i) {
        pad_ui[i]->selected = (i == id);
    }
    if (edit_ui) {
        edit_ui->set_edit_pad(id);
    }
    edit_pad = id;
}

void MidiPadUi::on_click_pad(int id)
{
    if (!my_module) return;

    if (my_module->editing) {
        set_edit_pad(id);
    } else {
        send_pad(id);
    }

}

void MidiPadUi::send_pad(int id)
{
    if (-1 == id) return;
    auto pad = get_pad(id);
    if (pad
        && pad->ok
        && !pad->midi.empty()
        && chem_host
        && !chem_host->host_busy()
    ) {
        auto haken = chem_host->host_haken();
        if (haken) {
            for (auto m: pad->midi) {
                haken->send_message(m);
            }
        }
    }
}
void MidiPadUi::remove_pad(int id)
{
    my_module->remove_pad(id);
}

PackedColor MidiPadUi::get_pad_color()
{
    if (-1 == edit_pad) return DEFAULT_PAD_COLOR;
    return my_module ? my_module->get_pad_color(edit_pad) : DEFAULT_PAD_COLOR;
}

void MidiPadUi::set_pad_color(PackedColor color)
{
    if (-1 == edit_pad) return;
    my_module->set_pad_color(edit_pad, color);
    // not needed: color used at draw time // pads[edit_pad]->on_pad_change(false, false);
}

PackedColor MidiPadUi::get_pad_text_color()
{
    if (-1 == edit_pad) return DEFAULT_PAD_TEXT_COLOR;
    return my_module ? my_module->get_pad_text_color(edit_pad) : DEFAULT_PAD_TEXT_COLOR;
}

void MidiPadUi::set_pad_text_color(PackedColor color)
{
    if (-1 == edit_pad) return;
    my_module->set_pad_text_color(edit_pad, color);
    pad_ui[edit_pad]->on_pad_change(false, false);
}

void MidiPadUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    onConnectionChange(ChemDevice::Haken, host ? host->host_connection(ChemDevice::Haken) : nullptr);
}

void MidiPadUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    onConnectionChangeUiImpl(this, device, connection);
}

bool MidiPadUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

void MidiPadUi::onHoverKey(const HoverKeyEvent &e)
{
    if (my_module) {
        if (e.action == GLFW_PRESS && ((e.mods & RACK_MOD_MASK) == 0)) {
            switch (e.key) {
            case GLFW_KEY_F2:
                edit_mode(!my_module->editing);
                e.consume(this);
                return;

            }
        }
    }
    Base::onHoverKey(e);
}

void MidiPadUi::step()
{
    Base::step();
    if (!my_module) return;
    bind_host(my_module);

    // When closed by clicking flyout close, the edit button is in
    // the wrong state (because it wasn't clicked). So we sync it up here.
    if (edit_button->latched != my_module->editing) {
        edit_button->latched = my_module->editing;
        edit_button->sync_frame();
    }

    //knobs[K_MODULATION]->enable(my_module->modulation.has_target());

    // for (int i = 0; i < K_MODULATION; ++i) {
    //     tracks[i]->set_value(my_module->modulation.get_port(i).modulated());
    //     tracks[i]->set_active(my_module->getInput(i).isConnected());
    // }
}

void MidiPadUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
//    menu->addChild(new MenuSeparator);

    Base::appendContextMenu(menu);
}
