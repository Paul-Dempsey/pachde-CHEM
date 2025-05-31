#include "MidiPad.hpp"
#include "../../services/colors.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/widgets.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;

// -- UI --------------------------------------
constexpr const float PANEL_WIDTH = 125.f;
//constexpr const float CENTER = PANEL_WIDTH*.5f;
constexpr const float GRID_CY = 135.f;

MidiPadUi::MidiPadUi(MidiPadModule *module) :
    my_module(module)
{
    setModule(module);
    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);
    
    bool browsing = !module;
    float x = 15.f;
    float y = 20.f;
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

    edit_button = Center(createThemedButton<EditButton>(Vec(12.5f, GRID_CY), theme_engine, theme, "Edit"));
    edit_button->set_sticky(true);
    if (my_module) {
        edit_button->setHandler([=](bool c, bool s){ edit_mode(!my_module->editing); });
    }
    addChild(edit_button);

    addChild(createLightCentered<TinyLight<YellowLight>>(Vec(22, GRID_CY - 6.5f), my_module, MidiPadModule::L_EDITING));

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

void MidiPadUi::edit_mode(bool editing)
{
    if (!my_module) return;
    my_module->editing = editing;
    edit_button->describe(editing ? "Stop editing pads" : "Edit pads");

    if (editing) {
        if (-1 == edit_pad) {
            auto pad = my_module->first_pad();
            if (pad) {
                set_edit_pad(pad->id);
            } else {
                set_edit_pad(0);
            }
        }

        float y = GRID_CY;
        auto theme = theme_engine.getTheme(getThemeName());
        name_palette = Center(createThemedButton<Palette1Button>(Vec(96.5,y), theme_engine, theme, "Text color"));
        name_palette->setHandler([=](bool,bool) {
            ui::Menu* menu = createMenu();
            auto picker = new ColorPickerMenu();
            picker->set_color(get_pad_text_color());
            picker->set_on_new_color([=](PackedColor color) {
                set_pad_text_color(color);
            });
            menu->addChild(picker);
        });
        addChild(name_palette);

        pad_palette = Center(createThemedButton<Palette2Button>(Vec(110.5,y), theme_engine, theme, "Pad color"));
        pad_palette->setHandler([=](bool,bool) {
            ui::Menu* menu = createMenu();
            auto picker = new ColorPickerMenu();
            picker->set_color(get_pad_color());
            picker->set_on_new_color([=](PackedColor color) {
                set_pad_color(color);
            });
            menu->addChild(picker);
        });
        addChild(pad_palette);

        y -= 6.f;
        auto pad = get_pad(edit_pad);
        name_field = createThemedTextInput(30.f, y, 55.f, 14.f, theme_engine, theme, pad->name, 
            [=](std::string text){ on_name_text_changed(text); },
            nullptr,
            "Pad name");
        name_field->text_height = 12.f;
        addChild(name_field);

        y += 16.f;
        //midi_field = createThemedTextInput(7.5, y, 105.f, 12.f, theme_engine, theme, pad->def,
        //    [=](std::string text){ on_midi_text_changed(text); },
        //    nullptr,
        //    "Pad Midi");
        //midi_field->text_height = 10.f;
        midi_field = createWidget<MultiTextInput>(Vec(3.5, y));
        midi_field->box.size = Vec(113, 112);
        midi_field->set_on_enter( [=](std::string text){ on_midi_text_changed(text); });
        midi_field->placeholder = "Pad Midi";
        midi_field->setText(pad->def);
        addChild(midi_field);

        y += 112;
        addChild(status = createLabel<TipLabel>(Vec(3.5, y), PANEL_WIDTH - 7, "", theme_engine, theme, S::warning_label));
        name_field->nextField = midi_field;
        midi_field->nextField = name_field;
        name_field->prevField = midi_field;
        midi_field->prevField = name_field;

    } else {
        if (midi_field) {
            on_midi_text_changed(midi_field->getText());
        }
        auto pad = get_pad(edit_pad);
        if (pad && !pad->def.empty()) {
            pad->compile();
        }

        for (int i = 0; i < 16; ++i) {
            pad_ui[i]->selected = false;
        }
        if (name_palette) {
            name_palette->requestDelete();
            name_palette = nullptr;
        }
        if (pad_palette) {
            pad_palette->requestDelete();
            pad_palette = nullptr;
        }
        if (name_field) {
            name_field->requestDelete();
            name_field = nullptr;
        }
        if (midi_field) {
            midi_field->requestDelete();
            midi_field = nullptr;
        }
        if (status) {
            status->requestDelete();
            status = nullptr;
        }
    }
}

void MidiPadUi::set_edit_pad(int id)
{
    assert(in_range(id, 0, 15));
    if (-1 != edit_pad) {
        on_midi_text_changed(midi_field->getText());
        auto pad = get_pad(edit_pad);
        if (pad && !pad->def.empty()) {
            pad->compile();
        }
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
    if (name_field) {
        name_field->setText(pad->name);
    }
    if (midi_field) {
        midi_field->setText(pad->def);
    }
    if (status) {
        status->text("");
        status->describe("");
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

void MidiPadUi::on_name_text_changed(std::string text)
{
    assert(in_range(edit_pad, 0, 15));
    get_pad(edit_pad)->name = text;
    pad_ui[edit_pad]->on_pad_change(true, false);
}

void MidiPadUi::on_midi_text_changed(std::string text)
{
    assert(in_range(edit_pad, 0, 15));
    auto pad = get_pad(edit_pad);
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
    pad_ui[edit_pad]->on_pad_change(false, true);
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
