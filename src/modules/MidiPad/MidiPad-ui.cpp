#include "MidiPad.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/widgets.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;

// -- UI --------------------------------------
constexpr const float PANEL_WIDTH = 120.f;
//constexpr const float CENTER = PANEL_WIDTH*.5f;
constexpr const float EDIT_ROW_CY = 180.f;

namespace pad_edit_constants {
    constexpr const float HEIGHT{380.f};
    constexpr const float WIDTH{PANEL_WIDTH};
    constexpr const float CENTER{WIDTH*.5f};
    constexpr const float PALETTE_DX{15.f};

};

struct PadEdit : OpaqueWidget
{
    using Base = OpaqueWidget;

    MidiPadUi* ui{nullptr};

    Palette1Button* name_palette{nullptr};
    Palette2Button* pad_palette{nullptr};
    TextLabel* coordinate{nullptr};
    TextInput* name_field{nullptr};
    //TextInput* midi_field{nullptr};
    MultiTextInput* midi_field{nullptr};
    TipLabel* status{nullptr};

    PadEdit() { 
        using namespace pad_edit_constants;
        box.size = Vec(WIDTH, HEIGHT);
    }

    void commit()
    {
        auto edit_pad = ui->edit_pad;

        assert(in_range(edit_pad, 0, 15));
        auto pad = ui->get_pad(edit_pad);
        pad->def = midi_field->getText();
        if (!pad->compile()) {
            status->text(pad->error_message);
            status->describe(pad->error_message);
            midi_field->cursor = pad->error_pos;
            midi_field->selection = pad->error_pos;
        } else {
            status->text("ok");
            status->describe("");
        }
        ui->pad_ui[edit_pad]->on_pad_change(false, true);
    }

    void set_edit_pad(int id)
    {
        auto pad = ui->get_pad(id);
        coordinate->text(default_pad_name[id]);
        name_field->setText(pad->name);
        midi_field->setText(pad->def);
        status->text("");
        status->describe("");
    }

    void on_name_text_changed(std::string text)
    {
        auto edit_pad = ui->edit_pad;
        assert(in_range(edit_pad, 0, 15));
        ui->get_pad(edit_pad)->name = text;
        ui->pad_ui[edit_pad]->on_pad_change(true, false);
    }


    void on_midi_text_changed(std::string text)
    {
        auto edit_pad = ui->edit_pad;

        assert(in_range(edit_pad, 0, 15));
        auto pad = ui->get_pad(edit_pad);
        pad->def = text;
        if (!pad->compile()) {
            status->text(pad->error_message);
            status->describe(pad->error_message);
            midi_field->cursor = pad->error_pos;
            midi_field->selection = pad->error_pos;
        } else {
            status->text("ok");
            status->describe("");
        }
        ui->pad_ui[edit_pad]->on_pad_change(false, true);
    }

    void init(MidiPadUi* mw)
    {
        using namespace pad_edit_constants;
        ui = mw;
        auto theme = theme_engine.getTheme(ui->getThemeName());
        LabelStyle mini_label_style{"ctl-label", TextAlignment::Center, 10.f, true};
        LabelStyle section_label_style{"ctl-label-hi", TextAlignment::Center, 14.f, true};
        float y;

        y = 30;
        addChild(coordinate = createLabel(Vec(CENTER,y), 40, "", theme_engine, theme, section_label_style));

        y = 48;
        name_field = createThemedTextInput(30.f, y, 60.f, 14.f, theme_engine, theme, "", 
            [=](std::string text){ on_name_text_changed(text); },
            nullptr,
            "Pad name");
        name_field->text_height = 12.f;
        addChild(name_field);

        y += 24.f;
        name_palette = Center(createThemedButton<Palette1Button>(Vec(CENTER - PALETTE_DX,y), theme_engine, theme, "Text color"));
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

        pad_palette = Center(createThemedButton<Palette2Button>(Vec(CENTER + PALETTE_DX,y), theme_engine, theme, "Pad color"));
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
        addChild(createLabel(Vec(CENTER - PALETTE_DX,y),25, "text", theme_engine, theme, mini_label_style));
        addChild(createLabel(Vec(CENTER + PALETTE_DX,y),25, "pad", theme_engine, theme, mini_label_style));

        y += 16.f;
        //midi_field = createThemedTextInput(7.5, y, 105.f, 12.f, theme_engine, theme, pad->def,
        //    [=](std::string text){ on_midi_text_changed(text); },
        //    nullptr,
        //    "Pad Midi");
        //midi_field->text_height = 10.f;
        midi_field = createWidget<MultiTextInput>(Vec(3.5, y));
        midi_field->box.size = Vec(113, 132);
        midi_field->set_on_enter( [=](std::string text){ on_midi_text_changed(text); });
        midi_field->placeholder = "Pad Midi";
        addChild(midi_field);

        y += 112;
        addChild(status = createLabel<TipLabel>(Vec(3.5, y), WIDTH - 7, "", theme_engine, theme, S::warning_label));
        name_field->nextField = midi_field;
        midi_field->nextField = name_field;
        name_field->prevField = midi_field;
        midi_field->prevField = name_field;
    }

};

MidiPadUi::MidiPadUi(MidiPadModule *module) :
    my_module(module)
{
    setModule(module);
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());

    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);

    auto edit_panel = createWidget<PanelBackgroundWidget>(Vec(0,0));
    edit_panel->track();
    addChildBottom(edit_panel);

    bool browsing = !module;
    float x = 15.f;
    float y = 32.f;
    for (int i = 0; i < 16; ++i) {
        auto w = createWidget<PadWidget>(Vec(x,y));
        w->init(i, my_module ? my_module->pad_defs[i] : nullptr, my_module, 
            theme_engine, theme,
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

    edit_button = Center(createThemedButton<EditButton>(Vec(box.size.x*.5, EDIT_ROW_CY), theme_engine, theme, "Edit"));
    edit_button->set_sticky(true);
    if (my_module) {
        edit_button->setHandler([=](bool c, bool s){ edit_mode(!my_module->editing); });
    }
    addChild(edit_button);

    addChild(createLightCentered<TinyLight<YellowLight>>(Vec(box.size.x*.5 + 12.5, EDIT_ROW_CY - 6.5f), my_module, MidiPadModule::L_EDITING));

    // inputs
    const float INPUT_TOP_CY = 292.5f;
    const float INPUT_ROW_CX = 28.f;
    const float INPUT_DY = 19.5f;
    const float INPUT_DX = 25.f;
    const NVGcolor co_port = PORT_CORN;

    x = INPUT_ROW_CX;
    y = INPUT_TOP_CY;
    for (int i = 0; i < 16; ++i) {
        addChild(Center(createThemedColorInput(Vec(x, y), my_module, i, S::InputColorKey, co_port, theme_engine, theme)));
        if (3 == i % 4) {
            x = INPUT_ROW_CX;
            y += INPUT_DY;
        } else {
            x += INPUT_DX;
        }
    }

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

    if (browsing) {
    }

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
    }
}

void MidiPadUi::set_edit_pad(int id)
{
    assert(in_range(id, 0, 15));
    assert(my_module->editing);
    if (-1 != edit_pad) {
        edit_ui->commit();
    }
    edit_pad = id;
    my_module->edit_pad = id;
    my_module->ensure_pad(id);
    auto pad = get_pad(id);
    if (nullptr == pad_ui[id]->pad) {
        pad_ui[id]->set_pad(pad);
    }
    for (int i = 0; i < 16; ++i) {
        pad_ui[i]->selected = (i == edit_pad);
    }
    if (edit_ui) {
        edit_ui->set_edit_pad(id);
    }
}

void MidiPadUi::on_click_pad(int id)
{
    if (!my_module) return;

    if (my_module->editing) {
        set_edit_pad(id);
        return;
    }

    auto pad = get_pad(id);
    if (pad 
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

void MidiPadUi::setThemeName(const std::string& name, void * context)
{
    Base::setThemeName(name, context);
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

void MidiPadUi::step()
{
    Base::step();
    if (!my_module) return;
    bind_host(my_module);

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
