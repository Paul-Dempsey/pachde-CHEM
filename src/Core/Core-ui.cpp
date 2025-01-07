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
    NVGcolor co;

    addChild(createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, 0), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH *.5f, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - RACK_GRID_WIDTH - 7.5f, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));

    co = fromPacked(theme_engine.getFillColor(getThemeName(), "curpreset"));
    if (!isColorVisible(co)) { co = COLOR_GREEN; }
    addChild(preset_label = createStaticTextLabel<StaticTextLabel>(
        Vec(box.size.x *.5f, 3.5f), box.size.x, "<preset>",
        TextAlignment::Center, 16.f, true, co
        ));
    preset_label->_label->_text_key = "curpreset"; 

    y = PICKER_TOP; 
    co = fromPacked(theme_engine.getFillColor(getThemeName(), "dytext"));
    haken_picker = createMidiPicker(UHALF, y, true, "Choose HAKEN device", &my_module->haken_midi);
    addChild(haken_device_label = createStaticTextLabel<StaticTextLabel>(
        Vec(UHALF, y + PICKER_LABEL_OFFSET), 120.f, "<Eagan Matrix Device>",
        TextAlignment::Left, 14.f, false, co
        ));
    y += PICKER_INTERVAL;
    controller1_picker = createMidiPicker(UHALF, y, false, "Choose MIDI controller #1", &my_module->controller1);
    addChild(controller1_device_label = createStaticTextLabel<StaticTextLabel>(
        Vec(UHALF, y + PICKER_LABEL_OFFSET), 120.f, "",
        TextAlignment::Left, 14.f, false, co
        ));

    y += PICKER_INTERVAL;
    controller2_picker = createMidiPicker(UHALF, y, false, "Choose MIDI controller #2", &my_module->controller2);
    addChild(controller2_device_label = createStaticTextLabel<StaticTextLabel>(
        Vec(UHALF, y + PICKER_LABEL_OFFSET), 120.f, "",
        TextAlignment::Left, 14.f, false, co
        ));

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