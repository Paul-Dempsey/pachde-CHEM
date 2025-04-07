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

constexpr const float CENTER = 225.f * .5f;

bool SettingsUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return my_module->connected();
}

void SettingsUi::createScrews(std::shared_ptr<SvgTheme> theme)
{
    addChild(createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, 0), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2*RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
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

    if (S::show_screws()) {
        createScrews(theme);
    }

    float x, y;
    bool browsing = !module;
    const float value_dx = 9.f;
    const float value_dy = 7.5f;
    const float label_dx = 9.f;
    const float row_dy = 14.f;
    const float menu_axis = CENTER - 18.f;
    x = menu_axis;
    y = 20;
    addChild(Center(createThemedParamButton<SurfaceDirectionParamButton>(Vec(x+18.f,y), my_module, SM::P_SURFACE_DIRECTION, theme_engine, theme)));

    auto value_style = LabelStyle{"setting", TextAlignment::Left, 14.f};
    auto label_style = LabelStyle{"ctl-label", TextAlignment::Right, 14.f};

    y += 15.f;
    addChild(Center(createThemedParam<BendMenu>(Vec(x,y), my_module, SM::P_X, theme_engine, theme)));
    addChild(x_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 100.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 8.f, "X", theme_engine, theme, label_style));

    y += row_dy;
    addChild(Center(createThemedParam<YMenu>(Vec(x,y), my_module, SM::P_Y, theme_engine, theme)));
    addChild(y_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 100.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 8.f, "Y", theme_engine, theme, label_style));

    y += row_dy;
    addChild(Center(createThemedParam<ZMenu>(Vec(x,y), my_module, SM::P_Z, theme_engine, theme)));
    addChild(z_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 120.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 8.f, "Z", theme_engine, theme, label_style));

    y += row_dy;
    addChild(Center(createThemedParam<HamParam>(Vec(x,y), my_module, SM::P_BASE_POLYPHONY, theme_engine, theme)));
    addChild(base_polyphony_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 10.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x- label_dx,y-value_dy), 80.f, "Polyphony", theme_engine, theme, label_style));

    y += row_dy;
    addChild(Center(createThemedParamButton<CheckParamButton>(Vec(x,y), my_module, SM::P_EXPAND_POLYPHONY, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Expand", theme_engine, theme, label_style));

    y += row_dy;
    addChild(Center(createThemedParamButton<CheckParamButton>(Vec(x,y), my_module, SM::P_DOUBLE_COMPUTATION, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "2x rate", theme_engine, theme, label_style));

    x = menu_axis;
    y += row_dy;
    addChild(Center(createThemedParam<HamParam>(Vec(x,y), my_module, SM::P_NOTE_PROCESSING, theme_engine, theme)));
    addChild(note_processing_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 120.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Note processing", theme_engine, theme, label_style));

    y += row_dy;
    addChild(Center(createThemedParam<HamParam>(Vec(x,y), my_module, SM::P_NOTE_PRIORITY, theme_engine, theme)));
    addChild(note_priority_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 120.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Note priority", theme_engine, theme, label_style));

    y += row_dy;
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>(Vec(12.f, y), my_module,SM::P_MONO, SM::L_MONO, theme_engine, theme)));
    addChild(Center(createThemedParam<HamParam>(Vec(x,y), my_module, SM::P_MONO_MODE, theme_engine, theme)));
    addChild(mono_mode_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 140.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Mono mode", theme_engine, theme, label_style));

    y += row_dy;
    addChild(Center(createThemedParam<HamParam>(Vec(x,y), my_module, SM::P_MONO_INTERVAL, theme_engine, theme)));
    addChild(mono_interval_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 100.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Mono interval", theme_engine, theme, label_style));

    y += row_dy;
    addChild(Center(createThemedParam<TuningMenu>(Vec(x,y), my_module, SM::P_TUNING, theme_engine, theme)));
    addChild(tuning_value = createLabel<TextLabel>(Vec(x+value_dx,y-value_dy), 100.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 50.f, "Tuning", theme_engine, theme, label_style));

    y += row_dy;
    create_rounding_leds(this, 20.f, y, 6.f, my_module, SM::L_ROUND_Y);

    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_ROUND_TYPE));
    addChild(round_type_value = createLabel<TextLabel>(Vec(x+value_dx, y-value_dy), 60.f, "", theme_engine, theme, value_style));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Rounding", theme_engine, theme, label_style));
    
    y += row_dy;

    addChild(Center(createThemedParamButton<CheckParamButton>(Vec(x,y), my_module, SM::P_ROUND_INITIAL, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Round initial", theme_engine, theme, label_style));

    y += row_dy;
    addChild(round_rate_slider = createSlider<FillHSlider>(Vec(x-5.f, y-6.f), 120.f, my_module, SM::P_ROUND_RATE, theme_engine, theme));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Round rate", theme_engine, theme, label_style));

    y += row_dy;
    addChild(Center(createThemedParamButton<CheckParamButton>(Vec(x,y), my_module, SM::P_KEEP_MIDI, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 50.f, "Keep MIDI", theme_engine, theme, label_style));

    y += row_dy;
    addChild(Center(createThemedParamButton<CheckParamButton>(Vec(x,y), my_module, SM::P_KEEP_SURFACE, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x-label_dx,y-value_dy), 80.f, "Keep Surface", theme_engine, theme, label_style));

    // routing
    x = 42.5f;
    y = 267;
    addChild(createLabel<TextLabel>(Vec(x, y), 40.f, "MIDI", theme_engine, theme, S::med_control_label));
    y += 15.f;
    addChild(createLabel<TextLabel>(Vec(x, y), 50.f, "Surface", theme_engine, theme, S::med_control_label));
    y += 22.f;
    x = 81.5f;
    addChild(createLabel<TextLabel>(Vec(x, y), 30.f, "DSP",  theme_engine, theme, S::med_control_label));
    x += 30.f;
    addChild(createLabel<TextLabel>(Vec(x, y), 30.f, "CVC",  theme_engine, theme, S::med_control_label));
    x += 30.f;
    addChild(createLabel<TextLabel>(Vec(x, y), 40.f, "MIDI", theme_engine, theme, S::med_control_label));

    const float col_dsp = 82.5f;
    const float col_cvc = col_dsp + 30.f;
    const float col_midi = col_cvc + 30.f;
    const float row_midi = 273.f;
    const float row_surface = 288.f;
    y = row_midi;
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_dsp,y), my_module, SM::P_MIDI_DSP, SM::L_MIDI_DSP, theme_engine, theme)));
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_cvc,y), my_module, SM::P_MIDI_CVC, SM::L_MIDI_CVC, theme_engine, theme)));
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_midi,y), my_module, SM::P_MIDI_MIDI, SM::L_MIDI_MIDI, theme_engine, theme)));
    y = row_surface;
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_dsp,y), my_module, SM::P_SURFACE_DSP, SM::L_SURFACE_DSP, theme_engine, theme)));
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_cvc,y), my_module, SM::P_SURFACE_CVC, SM::L_SURFACE_CVC, theme_engine, theme)));
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_midi,y), my_module, SM::P_SURFACE_MIDI, SM::L_SURFACE_MIDI, theme_engine, theme)));

    // inputs
    y = S::PORT_TOP + S::PORT_DY;
    x = CENTER + S::PORT_DX - 5.f;
    addChild(mod_knob = createChemKnob<TrimPot>(Vec(x, y), module, SM::P_MOD_AMOUNT, theme_engine, theme));

    x = CENTER;
    addChild(Center(createThemedColorInput(Vec(x , y), my_module, SM::IN_ROUND_RATE, S::InputColorKey, PORT_CORN, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 25.f, "RATE", theme_engine, theme, S::in_port_label));

    x = CENTER - S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x , y), my_module, SM::IN_ROUND_INITIAL, S::InputColorKey, PORT_GRASS, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 25.f, "INIT", theme_engine, theme, S::in_port_label));

    // footer
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
        logo->box.pos = Vec(CENTER, 80.f);
        addChild(logo);
    }

    // init

    sync_labels();

    if (my_module) {
        my_module->set_chem_ui(this);
        onConnectHost(my_module->chem_host);
    }
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
    auto text = pq->getDisplayValueString();
    if (text.empty()) {
        label->text(format_string("?%f", pq->getValue()));
    } else {
        label->text(text);
    }
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
                my_module->zero_modulation();
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
    if (my_module->getInput(my_module->rounding_port.input_id).isConnected()) {
        round_rate_slider->set_modulation(my_module->rounding_port.modulated());
        mod_knob->enable(true);
    } else {
        round_rate_slider->set_modulation(NAN);
        mod_knob->enable(false);
    }
    sync_labels();
}

void SettingsUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
    menu->addChild(new MenuSeparator);

    bool unconnected = (my_module->inputs.end() == std::find_if(my_module->inputs.begin(), my_module->inputs.end(), [](Input& in){ return in.isConnected(); }));
    menu->addChild(createMenuItem("Zero modulation", "0", [this](){
        my_module->zero_modulation();
    }, unconnected));

    Base::appendContextMenu(menu);
}
