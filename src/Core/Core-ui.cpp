#include "Core.hpp"

const float PICKER_TOP = 130.f;
const float PICKER_INTERVAL = 45.f;

CoreModuleWidget::CoreModuleWidget(CoreModule *module)
{
    my_module = module;
    setModule(module);

    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);

    this->panelBorder = new PartnerPanelBorder();
    replacePanelBorder(panel, this->panelBorder);
    setPanel(panel);

    addChild(createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, 0), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));

    haken_picker       = createMidiPicker(7.5f, PICKER_TOP,                         true, "Choose HAKEN device",      &my_module->haken_midi);
    controller1_picker = createMidiPicker(7.5f, PICKER_TOP + PICKER_INTERVAL,       false, "Choose MIDI controller #1", &my_module->controller1);
    controller2_picker = createMidiPicker(7.5f, PICKER_TOP + 2.f * PICKER_INTERVAL, false, "Choose MIDI controller #2", &my_module->controller2);

    addChild(createLightCentered<MediumLight<BlueLight>>(Vec(RACK_GRID_WIDTH*1.5f, RACK_GRID_WIDTH*2), my_module, CoreModule::L_READY));
    addChild(Center(createThemedColorOutput(Vec(RACK_GRID_WIDTH+2, RACK_GRID_HEIGHT - 60), my_module, CoreModule::OUT_READY, PORT_MAGENTA, theme_engine, theme)));
}

MidiPicker* CoreModuleWidget::createMidiPicker(float x, float y, bool isEM, const char *tip, MidiDeviceHolder* device)
{
    auto picker = createWidget<MidiPicker>(Vec(x, y));
    picker->describe(tip);
    picker->setCallback(device);
    picker->setConnection(device->connection);
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
    // if (my_module && my_module->ready) {
    //     Dot(args.vg, 15, 15, COLOR_GREEN_HI, true);
    // }
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