#include "Core.hpp"

constexpr const float PICKER_TOP = 150.f;
constexpr const float PICKER_INTERVAL = 40.f;
constexpr const float PICKER_LABEL_OFFSET = 14.f;
constexpr const float ROUND_LIGHT_SPREAD = 4.f;
constexpr const float U1 = 15.f;
constexpr const float UHALF = 7.5f;

CoreModuleWidget::~CoreModuleWidget()
{
    if (my_module) {
        my_module->ui = nullptr;
    }
}

CoreModuleWidget::CoreModuleWidget(CoreModule *module)
{
    my_module = module;
    setModule(module);

    if (my_module) {
        my_module->ui = this;
    }

    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);

    this->panelBorder = new PartnerPanelBorder();
    replacePanelBorder(panel, this->panelBorder);
    setPanel(panel);

    float x, y;

    addChild(createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, 0), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH *.5f, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - RACK_GRID_WIDTH - 7.5f, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));

    addChild(preset_label = createStaticTextLabel<StaticTextLabel>(
        Vec(box.size.x *.5f, 3.5f), box.size.x, "<preset>",
        theme_engine, theme, "curpreset", TextAlignment::Center, 16.f, false
        ));

    y = PICKER_TOP; 
    haken_picker = createMidiPicker(UHALF, y, true, "Choose HAKEN device", &my_module->haken_midi);
    std::string text = "<Eagan Matrix Device>";
    if (my_module) {
        if (my_module->haken_midi.connection) {
            text = my_module->haken_midi.connection->info.friendly(TextFormatLength::Short);
        } else if (!my_module->haken_midi.device_claim.empty()) {
            text = my_module->haken_midi.device_claim;
        }
    }
    addChild(haken_device_label = createStaticTextLabel<StaticTextLabel>(
        Vec(UHALF, y + PICKER_LABEL_OFFSET), 160.f, text,
        theme_engine, theme, "dytext", TextAlignment::Left, 14.f, false
        ));
    y += PICKER_INTERVAL;
    controller1_picker = createMidiPicker(UHALF, y, false, "Choose MIDI controller #1", &my_module->controller1);
    addChild(controller1_device_label = createStaticTextLabel<StaticTextLabel>(
        Vec(UHALF, y + PICKER_LABEL_OFFSET), 120.f, "",
        theme_engine, theme, "dytext", TextAlignment::Left, 14.f, false
        ));

    y += PICKER_INTERVAL;
    controller2_picker = createMidiPicker(UHALF, y, false, "Choose MIDI controller #2", &my_module->controller2);
    addChild(controller2_device_label = createStaticTextLabel<StaticTextLabel>(
        Vec(UHALF, y + PICKER_LABEL_OFFSET), 120.f, "",
        theme_engine, theme, "dytext", TextAlignment::Left, 14.f, false
        ));

    x = 18.f;
    y += PICKER_INTERVAL;
    auto w = Center(createThemedButton<SquareButton>(Vec(x,y), theme_engine, theme));
    w->describe("Reset MIDI\n(Ctrl+Click to clear)");
    if (my_module) {
        w->setHandler([=](bool ctrl, bool shift) {
            if (ctrl) {
                // clear claims and revoke connections
                // todo: integrate claim revocation somewhere in Core, or MidiDeviceHolder
                auto broker = MidiDeviceBroker::get();
                assert(broker);
                broker->revokeClaim(&my_module->haken_midi);
                my_module->haken_midi.clear();
                broker->revokeClaim(&my_module->controller1);
                my_module->controller1.clear();
                broker->revokeClaim(&my_module->controller2);
                my_module->controller2.clear();
            } else {
                // drop connections
                my_module->haken_midi.connect(nullptr);
                my_module->controller1.connect(nullptr);
                my_module->controller2.connect(nullptr);
            }
        });
    }
    addChild(w);

    addChild(createLightCentered<MediumLight<BlueLight>>(Vec(RACK_GRID_WIDTH * 1.5f, 30), my_module, CoreModule::L_READY));

    // rounding leds
    x = RACK_GRID_WIDTH * 1.5f - 7.5f;
    y = 38.f;
    addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, y), my_module, CoreModule::L_ROUND_Y)); x += ROUND_LIGHT_SPREAD;
    addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, y), my_module, CoreModule::L_ROUND_INITIAL)); x += ROUND_LIGHT_SPREAD;
    addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, y), my_module, CoreModule::L_ROUND)); x += ROUND_LIGHT_SPREAD;
    addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, y), my_module, CoreModule::L_ROUND_RELEASE));

    addChild(Center(createThemedColorOutput(Vec(RACK_GRID_WIDTH+2, RACK_GRID_HEIGHT - 50), my_module, CoreModule::OUT_READY, PORT_MAGENTA, theme_engine, theme)));

}

MidiPicker* CoreModuleWidget::createMidiPicker(float x, float y, bool isEM, const char *tip, MidiDeviceHolder* device)
{
    auto picker = createWidget<MidiPicker>(Vec(x, y));
    picker->describe(tip);
    picker->setDeviceHolder(device);
    picker->is_em = isEM;
    addChild(picker);
    return picker;
}

void CoreModuleWidget::step()
{
    ChemModuleWidget::step();
    if (my_module) {
        if (my_module->pending_connection()) {
            auto broker = MidiDeviceBroker::get();
            if (broker) {
                broker->sync();
            }
        }
    }
}

void CoreModuleWidget::draw(const DrawArgs& args)
{
    ChemModuleWidget::draw(args);
    if (my_module && my_module->ready) {
    //     Dot(args.vg, 15, 15, COLOR_GREEN_HI, true);
        //MIDI Animation
        float y = 128.f;
        float cx = static_cast<float>((my_module->haken_midi_in.received_count / 15) % 165);

        auto co = fromPacked(theme_engine.getFillColor(getThemeName(), "midi-in"));

        CircularHalo(args.vg, cx, y, 2.f, 8.5f, co);
        Circle(args.vg, cx, y, 2.25f, co);
       //float cx = static_cast<float>((my_module->haken_midi_out.sent_count / 15) % 165);
    }
}

void CoreModuleWidget::appendContextMenu(Menu *menu)
{
    ChemModuleWidget::appendContextMenu(menu);

    if (my_module) {
        menu->addChild(createCheckMenuItem(
            "Ready", "",
            [this]() { return my_module->ready; },
            [this]() { my_module->ready = !my_module->ready; }));
    }
}