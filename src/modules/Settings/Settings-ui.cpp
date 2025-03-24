#include "Settings.hpp"
#include "../../services/colors.hpp"
#include "../../widgets/hamburger.hpp"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/theme-knob.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace pachde;
using SM = SettingsModule;

// -- Settings UI -----------------------------------

constexpr const float CENTER = 255.f * .5f;

using HamParam = HamburgerUi<ParamWidget>;

bool SettingsUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return my_module->connected();
}

SettingsUi::SettingsUi(SettingsModule *module) :
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
    const float value_dx = 9.f;
    const float value_dy = 7.5f;
    const float label_dx = 9.f;
    const float row_dy = 14.f;
    x = CENTER - 18.f;
    y = 18;
    addChild(Center(createThemedParamButton<SurfaceDirectionParamButton>(Vec(x+18.f,y), my_module, SM::P_SURFACE_DIRECTION, theme_engine, theme)));

    auto value_style = LabelStyle{"setting", TextAlignment::Left, 14.f};
    auto label_style = LabelStyle{"label", TextAlignment::Right, 14.f};

    y += 15.f;
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_X));
    addChild(x_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 100.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 8.f, "X", theme_engine, theme, label_style));

    y += row_dy;
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_Y));
    addChild(y_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 100.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 8.f, "Y", theme_engine, theme, label_style));

    y += row_dy;
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_Z));
    addChild(z_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 120.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 8.f, "Z", theme_engine, theme, label_style));

    y += row_dy;
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_NOTE_PROCESSING));
    addChild(note_processing_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 120.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Note processing", theme_engine, theme, label_style));

    y += row_dy;
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_NOTE_PRIORITY));
    addChild(note_priority_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 120.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Note priority", theme_engine, theme, label_style));

    y += row_dy;
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_BASE_POLYPHONY));
    addChild(base_polyphony_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 100.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Base polyphony", theme_engine, theme, label_style));

    // y += row_dy;
    // addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_EXPAND_POLYPHONY));

    // y += row_dy;
    // addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_DOUBLE_COMPUTATION));

    y += row_dy;
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_MONO_MODE));
    addChild(mono_mode_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 140.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Mono mode", theme_engine, theme, label_style));

    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>(Vec(12.f, y + row_dy*.5), my_module,SM::P_MONO, SM::L_MONO, theme_engine, theme)));

    y += row_dy;
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_MONO_INTERVAL));
    addChild(mono_interval_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 100.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Interval", theme_engine, theme, label_style));

    y += row_dy;
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_OCTAVE_TYPE));
    addChild(octave_type_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 100.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Octave switch", theme_engine, theme, label_style));

    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>(Vec(12.f, y + row_dy*.5), my_module, SM::P_OCTAVE_SWITCH, SM::L_OCTAVE_SWITCH, theme_engine, theme)));

    y += row_dy;
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_OCTAVE_RANGE));
    addChild(octave_range_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 100.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Range", theme_engine, theme, label_style));
    
    y += row_dy;
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_TUNING));
    addChild(tuning_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 100.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 50.f, "Tuning", theme_engine, theme, label_style));

    y += row_dy;
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_ROUND_TYPE));
    addChild(round_type_value = createLabel<TextLabel>(Vec(x+value_dx, y-value_dy), 60.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Rounding", theme_engine, theme, label_style));
    
    y += row_dy;
    create_rounding_leds(12.f, y, 6.f);

    addChild(Center(createThemedParamButton<CheckParamButton>(Vec(x,y), my_module, SM::P_ROUND_INITIAL, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Round initial", theme_engine, theme, label_style));

    y += row_dy;
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Round rate", theme_engine, theme, label_style));

    auto center_label_style = LabelStyle{"label", TextAlignment::Center, 10.f};
    x = 200.f;
    y = 240.f;
    addChild(Center(createThemedParamButton<CheckParamButton>(Vec(x,y), my_module, SM::P_KEEP_MIDI, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x,y + 5.f), 50.f, "Keep", theme_engine, theme, center_label_style));

    x = 60.f;
    y = 228;
    addChild(createLabel<TextLabel>(Vec(x, y), 40.f, "MIDI", theme_engine, theme, center_label_style));
    y += 15.f;
    addChild(createLabel<TextLabel>(Vec(x, y), 50.f, "Surface", theme_engine, theme, center_label_style));
    y += 21.f;
    x = 96.5f;
    addChild(createLabel<TextLabel>(Vec(x, y), 30.f, "DSP",  theme_engine, theme, center_label_style));
    x += 30.f;
    addChild(createLabel<TextLabel>(Vec(x, y), 30.f, "CVC",  theme_engine, theme, center_label_style));
    x += 30.f;
    addChild(createLabel<TextLabel>(Vec(x, y), 40.f, "MIDI", theme_engine, theme, center_label_style));

    const float col_dsp = 97.f;
    const float col_cvc = 127.f;
    const float col_midi = 157.f;
    const float row_midi = 233.f;
    const float row_surface = 248.f;
    y = row_midi;
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_dsp,y), my_module, SM::P_MIDI_DSP, SM::L_MIDI_DSP, theme_engine, theme)));
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_cvc,y), my_module, SM::P_MIDI_CVC, SM::L_MIDI_CVC, theme_engine, theme)));
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_midi,y), my_module, SM::P_MIDI_MIDI, SM::L_MIDI_MIDI, theme_engine, theme)));
    y = row_surface;
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_dsp,y), my_module, SM::P_SURFACE_DSP, SM::L_SURFACE_DSP, theme_engine, theme)));
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_cvc,y), my_module, SM::P_SURFACE_CVC, SM::L_SURFACE_CVC, theme_engine, theme)));
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_midi,y), my_module, SM::P_SURFACE_MIDI, SM::L_SURFACE_MIDI, theme_engine, theme)));

    // inputs
    y = S::PORT_TOP;
    x = CENTER + S::PORT_DX - 5.f;
    addChild(createChemKnob<TrimPot>(Vec(x, y), module, SM::P_MOD_AMOUNT, theme_engine, theme));

    x = CENTER;
    addChild(Center(createThemedColorInput(Vec(x , y), my_module, SM::IN_ROUND_RATE, S::InputColorKey, PORT_CORN, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 25.f, "RATE", theme_engine, theme, S::in_port_label));

    x = CENTER - S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x , y), my_module, SM::IN_ROUND_INITIAL, S::InputColorKey, PORT_ORANGE, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 25.f, "INIT", theme_engine, theme, S::in_port_label));

    // footer
    addChild(warning_label = createLabel<TipLabel>(
        Vec(28.f, box.size.y - 22.f), box.size.x, "", theme_engine, theme, S::warning_label));
    warning_label->describe("[warning/status]");
    warning_label->glowing(true);

    addChild(haken_device_label = createLabel<TipLabel>(
        Vec(28.f, box.size.y - 13.f), 200.f, S::NotConnected, theme_engine, theme, S::haken_label));

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y - S::U1), theme_engine, theme, "Core link");

    if (my_module) {
        link_button->setHandler([=](bool ctrl, bool shift) {
            ui::Menu* menu = createMenu();
            menu->addChild(createMenuLabel("— Link to Core Module —"));
            auto broker = ModuleBroker::get();
            broker->addHostPickerMenu(menu, my_module);
        });
    }
    addChild(link_button);

    // Browsing UI

    if (browsing) {
        auto logo = new Logo(0.8f);
        logo->box.pos = Vec(CENTER + 10.f, 80.f);
        addChild(logo);
    }

    // init

    sync_labels();

    if (my_module) {
        my_module->set_chem_ui(this);
        onConnectHost(my_module->chem_host);
    }
}

void SettingsUi::create_rounding_leds(float x, float y, float spread)
{
    addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, y), my_module, SM::L_ROUND_Y)); x += spread;
    addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, y), my_module, SM::L_ROUND_INITIAL)); x += spread;
    addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, y), my_module, SM::L_ROUND)); x += spread;
    addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, y), my_module, SM::L_ROUND_RELEASE));
}

void SettingsUi::setThemeName(const std::string& name, void * context)
{
    Base::setThemeName(name, context);
}

void SettingsUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
    } else {
        haken_device_label->text(S::NotConnected);
    }
}

void SettingsUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    haken_device_label->text(connection ? connection->info.friendly(TextFormatLength::Short) : S::NotConnected);
    haken_device_label->describe(connection ? connection->info.friendly(TextFormatLength::Long) : S::NotConnected);
}

void sync_switch_label(Module* module, int param_id, TextLabel* label)
{
    auto pq = module->getParamQuantity(param_id);
    label->text(pq->getDisplayValueString());
}

void SettingsUi::sync_labels()
{
    if (my_module) {
        sync_switch_label(my_module, SM::P_X, x_value);
        sync_switch_label(my_module, SM::P_Y, y_value);
        sync_switch_label(my_module, SM::P_Z, z_value);
        sync_switch_label(my_module, SM::P_NOTE_PROCESSING, note_processing_value);
        sync_switch_label(my_module, SM::P_NOTE_PRIORITY, note_priority_value);
        sync_switch_label(my_module, SM::P_BASE_POLYPHONY, base_polyphony_value);
        sync_switch_label(my_module, SM::P_MONO_MODE, mono_mode_value);
        sync_switch_label(my_module, SM::P_MONO_INTERVAL, mono_interval_value);
        sync_switch_label(my_module, SM::P_OCTAVE_TYPE, octave_type_value);
        sync_switch_label(my_module, SM::P_OCTAVE_RANGE, octave_range_value);
        sync_switch_label(my_module, SM::P_ROUND_TYPE, round_type_value);
        sync_switch_label(my_module, SM::P_TUNING, tuning_value);
    }
}

void SettingsUi::onHoverKey(const HoverKeyEvent &e)
{
    if (my_module) {
        if (e.action == GLFW_PRESS && ((e.mods & RACK_MOD_MASK) == 0)) {
            switch (e.key) {
            case GLFW_KEY_0:
                e.consume(this);
                my_module->modulation.zero_modulation();
                return;
            // case GLFW_KEY_5:
            //     center_knobs();
            //     e.consume(this);
            //     return;
            }
        }
    }
    Base::onHoverKey(e);
}

void SettingsUi::step()
{
    Base::step();
    if (!my_module) return;
    bind_host(my_module);
    sync_labels();
}

void SettingsUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
    menu->addChild(new MenuSeparator);

    bool unconnected = (my_module->inputs.end() == std::find_if(my_module->inputs.begin(), my_module->inputs.end(), [](Input& in){ return in.isConnected(); }));
    menu->addChild(createMenuItem("Zero modulation", "0", [this](){
        my_module->modulation.zero_modulation();
    }, unconnected));

    Base::appendContextMenu(menu);
}
