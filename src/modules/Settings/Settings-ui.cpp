#include "Settings.hpp"
#include "services/colors.hpp"
#include "widgets/hamburger.hpp"
#include "widgets/logo-widget.hpp"
#include "widgets/menu-widgets.hpp"
#include "widgets/theme-knob.hpp"
#include "widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace pachde;
using SM = SettingsModule;

const char * AesInputName(uint8_t value) {
    switch (value) {
    case Haken::aesInputNone: return "(none)";
    case Haken::aesInputNonStd: return "nonstandard rate";
    case Haken::aesInput44p1: return "44.1 kHz";
    case Haken::aesInput48: return "48.0 kHz";
    case Haken::aesInput88p2: return "88.2 kHz";
    case Haken::aesInput96: return "96.0 kHz";
    case Haken::aesInput176p4: return "176.4 kHz";
    case Haken::aesInput192: return "192.0 kHz";
    default: return "(unknown)";
    }
}

struct AesMenu : Hamburger
{
    using Base = Hamburger;

    SettingsModule* settings{nullptr};

    void setModule(SettingsModule* m) { settings = m; }
    void onHoverKey(const HoverKeyEvent& e) override
    {
        switch (e.key) {
            case GLFW_KEY_ENTER:
            case GLFW_KEY_MENU:
            if (e.action == GLFW_RELEASE) {
                e.consume(this);
                createContextMenu();
                return;
            }
        }
        Base::onHoverKey(e);
    }
    void appendContextMenu(ui::Menu* menu) override{
        menu->addChild(createMenuLabel<HamburgerTitle>("AES Audio"));
        if (settings && settings->connected()) {
            auto em = settings->chem_host->host_matrix();
            auto text = format_string("AES in: %s", em ? AesInputName(em->get_aes_detect()) : "(Unavailable)");
            menu->addChild(createMenuLabel(text));
        } else {
            menu->addChild(createMenuLabel("(Unavailable)"));
        }
        menu->addChild(new MenuSeparator);
        auto pq = dynamic_cast<SwitchQuantity*>(settings->getParamQuantity(SM::P_AES3));
        if (pq) {
            int current = getParamInt(settings->getParam(SM::P_AES3));

            // 48k
            menu->addChild(new OptionMenuEntry(current == 2,
                createMenuItem(pq->labels[0], "",
                    [=](){ settings->getParam(SM::P_AES3).setValue(2.f); })));

            // 96k
            menu->addChild(new OptionMenuEntry(current == 0,
                createMenuItem(pq->labels[1], "",
                    [=](){ settings->getParam(SM::P_AES3).setValue(0.f); })));

            // Sync
            menu->addChild(new OptionMenuEntry(current == 1,
                createMenuItem(pq->labels[2], "",
                    [=](){ settings->getParam(SM::P_AES3).setValue(1.f); })));
        } else {
            menu->addChild(createMenuLabel("(Unavailable)"));
        }
    }

};

// -- Settings UI -----------------------------------

constexpr const float CENTER = 225.f * .5f;

bool SettingsUi::connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return my_module->connected();
}

void SettingsUi::createScrews()
{
    addChild(createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, 0), &module_svgs));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2*RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), &module_svgs));
}

SettingsUi::SettingsUi(SettingsModule *module) :
    my_module(module)
{
    setModule(module);
    auto theme = getSvgTheme();
    auto panel = createThemedPanel(panelFilename(), &module_svgs);
    panelBorder = attachPartnerPanelBorder(panel);
    setPanel(panel);

    if (S::show_screws()) {
        createScrews();
    }

    float x, y;
    bool browsing = !module;
    const float value_dx = 9.f;
    const float value_dy = 6.f;
    const float label_dx = 9.f;
    const float row_dy = 14.f;
    const float menu_axis = CENTER - 18.f;

    y = 20.f;
    addChild(Center(createThemedParamButton<SurfaceDirectionParamButton>(Vec(CENTER,y), &module_svgs, my_module, SM::P_SURFACE_DIRECTION)));

    y = 38.f;
    x = 58.f;
    addChild(createLabelRight(Vec(x-12.f,y-value_dy), "Middle C", &label_right_style, 50.f));
    addChild(createChemKnob<TrimPot>(Vec(x, y), &module_svgs, my_module, SM::P_MIDDLE_C));
    addChild(middle_c_value = createLabel(Vec(x+13.f,y-value_dy), "", &value_style, 100.f));

    x += 110.f;
    addChild(createLabelRight(Vec(x-12.f,y-value_dy), "Touch area", &label_right_style, 58.f));
    addChild(createChemKnob<TrimPot>(Vec(x, y), &module_svgs, my_module, SM::P_TOUCH_AREA));
    addChild(touch_area_value = createLabel(Vec(x+12.f,y-value_dy), "", &value_style, 100.f));

    x = menu_axis;
    y = 54.f;
    addChild(createLabelRight(Vec(x-label_dx,y-value_dy), "X", &label_right_style, 8.f));
    addChild(createParamCentered<BendMenu>(Vec(x,y), my_module, SM::P_X));
    addChild(x_value = createLabel(Vec(x+value_dx,y-value_dy), "", &value_style, 100.f));

    y += row_dy;
    addChild(createLabelRight(Vec(x-label_dx,y-value_dy), "Y", &label_right_style, 8.f));
    addChild(createParamCentered<YMenu>(Vec(x,y), my_module, SM::P_Y));
    addChild(y_value = createLabel(Vec(x+value_dx,y-value_dy), "", &value_style, 100.f));

    y += row_dy;
    addChild(createLabelRight(Vec(x-label_dx,y-value_dy), "Z", &label_right_style, 8.f));
    addChild(createParamCentered<ZMenu>(Vec(x,y), my_module, SM::P_Z));
    addChild(z_value = createLabel(Vec(x+value_dx,y-value_dy), "", &value_style, 120.f));


    x = menu_axis - 22.f;
    y += row_dy;
    addChild(createLabelRight(Vec(x-label_dx,y-value_dy), "Polyphony", &label_right_style, 80.f));
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_BASE_POLYPHONY));
    addChild(base_polyphony_value = createLabel(Vec(x+value_dx,y-value_dy), "", &value_style, 10.f));

    float x2 = 145.f;
    addChild(createLabelRight(Vec(x2,y-value_dy), "Fine tune", &label_right_style, 60.f));
    auto slider = createSlider<BasicHSlider>(Vec(x2 + 4.f, y-6.f), 64.f, my_module, SM::P_FINE);
    slider->increment = 1.f;
    addChild(slider);

    y += row_dy;
    addChild(createLabelRight(Vec(x-label_dx,y-value_dy), "Expand", &label_right_style, 80.f));
    addChild(Center(createThemedParamButton<CheckParamButton>(Vec(x,y), &module_svgs, my_module, SM::P_EXPAND_POLYPHONY)));

    addChild(createLabelRight(Vec(x2,y-value_dy), "Actuation", &label_right_style, 60.f));
    slider = createSlider<FillHSlider>(Vec(x2 + 4.f, y-6.f), 64.f, my_module, SM::P_ACTUATION);
    slider->increment = 1.f;
    addChild(slider);

    y += row_dy;
    addChild(createLabelRight(Vec(x-label_dx,y-value_dy), "2x rate", &label_right_style, 80.f));
    addChild(Center(createThemedParamButton<CheckParamButton>(Vec(x,y), &module_svgs, my_module, SM::P_DOUBLE_COMPUTATION)));

    addChild(createLabelRight(Vec(x2,y-value_dy), "Audio in", &label_right_style, 60.f));
    slider = createSlider<FillHSlider>(Vec(x2 + 4.f, y-6.f), 64.f, my_module, SM::P_AUDIO_IN);
    addChild(slider);

    x = menu_axis;
    y += row_dy;
    addChild(createLabelRight(Vec(x-label_dx,y-value_dy), "Note processing", &label_right_style, 80.f));
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_NOTE_PROCESSING));
    addChild(note_processing_value = createLabel(Vec(x+value_dx,y-value_dy), "", &value_style, 120.f));

    y += row_dy;
    addChild(createLabelRight(Vec(x-label_dx,y-value_dy), "Note priority", &label_right_style, 80.f));
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_NOTE_PRIORITY));
    addChild(note_priority_value = createLabel(Vec(x+value_dx,y-value_dy), "", &value_style, 120.f));

    y += row_dy;
    addChild(createLabelRight(Vec(x-label_dx,y-value_dy), "Mono mode", &label_right_style, 80.f));
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>(Vec(12.f, y), &module_svgs, my_module,SM::P_MONO, SM::L_MONO)));
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_MONO_MODE));
    addChild(mono_mode_value = createLabel(Vec(x+value_dx,y-value_dy), "", &value_style, 140.f));

    y += row_dy;
    addChild(createLabelRight(Vec(x-label_dx,y-value_dy), "Mono interval", &label_right_style, 80.f));
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_MONO_INTERVAL));
    addChild(mono_interval_value = createLabel(Vec(x+value_dx,y-value_dy), "", &value_style, 100.f));

    y += row_dy;
    addChild(createLabelRight(Vec(x-label_dx,y-value_dy), "Tuning", &label_right_style, 50.f));
    addChild(createParamCentered<TuningMenu>(Vec(x,y), my_module, SM::P_TUNING));
    addChild(tuning_value = createLabel(Vec(x+value_dx,y-value_dy), "", &value_style, 100.f));

    y += row_dy;
    create_rounding_leds(this, 20.f, y, 6.f, my_module, SM::L_ROUND_Y);

    addChild(createLabelRight(Vec(x-label_dx,y-value_dy), "Rounding", &label_right_style, 80.f));
    addChild(createParamCentered<HamParam>(Vec(x,y), my_module, SM::P_ROUND_TYPE));
    addChild(round_type_value = createLabel(Vec(x+value_dx, y-value_dy), "", &value_style, 60.f));

    y += row_dy;

    addChild(createLabelRight(Vec(x-label_dx,y-value_dy), "Round initial", &label_right_style, 80.f));
    addChild(Center(createThemedParamButton<CheckParamButton>(Vec(x,y), &module_svgs, my_module, SM::P_ROUND_INITIAL)));

    y += row_dy;
    addChild(createLabelRight(Vec(x-label_dx,y-value_dy), "Round rate", &label_right_style, 80.f));
    addChild(round_rate_slider = createSlider<FillHSlider>(Vec(x-5.f, y-6.f), 120.f, my_module, SM::P_ROUND_RATE));

    y += row_dy;
    addChild(createLabelRight(Vec(x-label_dx,y-value_dy), "Keep MIDI", &label_right_style, 50.f));
    addChild(Center(createThemedParamButton<CheckParamButton>(Vec(x,y), &module_svgs, my_module, SM::P_KEEP_MIDI)));

    y += row_dy;
    addChild(createLabelRight(Vec(x-label_dx,y-value_dy), "Keep Surface", &label_right_style, 80.f));
    addChild(Center(createThemedParamButton<CheckParamButton>(Vec(x,y), &module_svgs, my_module, SM::P_KEEP_SURFACE)));

    // routing
    x = 42.5f;
    y = 274.5f;
    addChild(createLabelCentered(Vec(x, y), "MIDI", &S::med_label, 40.f));
    y += 15.f;
    addChild(createLabelCentered(Vec(x, y), "Surface", &S::med_label, 50.f));
    y += 22.f;
    x = 81.5f;
    addChild(createLabelCentered(Vec(x, y), "DSP", &S::med_label, 30.f));
    x += 30.f;
    addChild(createLabelCentered(Vec(x, y), "CVC", &S::med_label, 30.f));
    x += 30.f;
    addChild(createLabelCentered(Vec(x, y), "MIDI", &S::med_label, 40.f));

    const float col_dsp = 82.5f;
    const float col_cvc = col_dsp + 30.f;
    const float col_midi = col_cvc + 30.f;
    const float row_midi = 280.5f;
    const float row_surface = row_midi + 15.f;
    y = row_midi;
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_dsp,y), &module_svgs, my_module, SM::P_MIDI_DSP, SM::L_MIDI_DSP)));
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_cvc,y), &module_svgs, my_module, SM::P_MIDI_CVC, SM::L_MIDI_CVC)));
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_midi,y), &module_svgs, my_module, SM::P_MIDI_MIDI, SM::L_MIDI_MIDI)));
    y = row_surface;
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_dsp,y), &module_svgs, my_module, SM::P_SURFACE_DSP, SM::L_SURFACE_DSP)));
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_cvc,y), &module_svgs, my_module, SM::P_SURFACE_CVC, SM::L_SURFACE_CVC)));
    addChild(Center(createThemedParamLightButton<DotParamButton, SmallSimpleLight<GreenLight>>( Vec(col_midi,y), &module_svgs, my_module, SM::P_SURFACE_MIDI, SM::L_SURFACE_MIDI)));

    x = 180.f;
    y = 274.5f;
    addChild(createLabelCentered(Vec(x, y), "AES3", &S::control_label_small, 40.f));
    y += 18.f;
    auto aes = createWidgetCentered<AesMenu>(Vec(x,y));
    aes->setModule(my_module);
    addChild(aes);

    // inputs
    y = S::PORT_TOP + S::PORT_DY;
    x = CENTER + S::PORT_DX - 5.f;
    addChild(mod_knob = createChemKnob<TrimPot>(Vec(x, y), &module_svgs, module, SM::P_MOD_AMOUNT));

    x = CENTER;
    addChild(Center(createThemedColorInput(Vec(x , y), &module_svgs, my_module, SM::IN_ROUND_RATE, S::InputColorKey, PORT_CORN)));
    addChild(createLabelCentered(Vec(x, y + S::PORT_LABEL_DY), "RATE", &S::in_port_label, 25.f));

    x = CENTER - S::PORT_DX;
    addChild(Center(createThemedColorInput(Vec(x , y), &module_svgs, my_module, SM::IN_ROUND_INITIAL, S::InputColorKey, PORT_GRASS)));
    addChild(createLabelCentered(Vec(x, y + S::PORT_LABEL_DY), "INIT", &S::in_port_label, 25.f));

    // footer
    addChild(haken_device_label = createLabel<TipLabel>(
        Vec(28.f, box.size.y - 13.f), S::NotConnected, &S::haken_label, 200.f));

    link_button = createThemedButton<LinkButton>(Vec(12.f, box.size.y - S::U1), &module_svgs, "Core link");

    if (my_module) {
        link_button->set_handler([=](bool ctrl, bool shift) {
            ModuleBroker::get()->addHostPickerMenu(createMenu(), my_module);
        });
    }
    addChild(link_button);

    // Browsing UI

    if (browsing && S::show_browser_logo()) {
        auto logo = new Logo(0.8f);
        logo->box.pos = Vec(CENTER, 144.f);
        addChild(logo);
    }

    module_svgs.changeTheme(theme);
    applyChildrenTheme(this, theme);

    // init

    sync_labels();

    if (my_module) {
        my_module->set_chem_ui(this);
        if (!chem_host) {
            onConnectHost(my_module->chem_host);
        }
    }
}

SettingsUi::~SettingsUi() {
    if (my_module) {
        my_module->set_chem_ui(nullptr);
    }
}

void SettingsUi::onConnectHost(IChemHost* host) {
    chem_host = host;
    if (chem_host) {
        onConnectionChange(ChemDevice::Haken, chem_host->host_connection(ChemDevice::Haken));
    } else {
        haken_device_label->set_text(S::NotConnected);
    }
}

void SettingsUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (device != ChemDevice::Haken) return;
    haken_device_label->set_text(connection ? connection->info.friendly(NameFormat::Short) : S::NotConnected);
    haken_device_label->describe(connection ? connection->info.friendly(NameFormat::Long) : S::NotConnected);
}

void sync_switch_label(Module* module, int param_id, TextLabel* label)
{
    auto pq = module->getParamQuantity(param_id);
    auto text = pq->getDisplayValueString();
    if (text.empty()) {
        label->set_text(format_string("?%f", pq->getValue()));
    } else {
        label->set_text(text);
    }
}

void SettingsUi::sync_labels()
{
    if (my_module) {
        auto value = my_module->em_values[SM::P_MIDDLE_C];
        middle_c_value->set_text(format_string("nn%d", value));

        value = my_module->em_values[SM::P_TOUCH_AREA];
        if (value <= 16) {
            touch_area_value->set_text("none");
        } else {
            touch_area_value->set_text(format_string("nn%d", value));
        }

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
