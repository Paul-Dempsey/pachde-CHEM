#include "Core.hpp"

// Layout
constexpr const int MODULE_WIDTH = 165;
constexpr const float PICKER_TOP = 128.f;
constexpr const float PICKER_INTERVAL = 42.f;
constexpr const float PICKER_LABEL_OFFSET = 14.f;
constexpr const int MIDI_ANIMATION_MARGIN = 3;
constexpr const float MIDI_ANIMATION_OFFSET = 30.f;
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

    set_theme_colors();

    createScrews(theme);
    createMidiPickers(theme);
    createRoundingLeds(RACK_GRID_WIDTH * 1.5f - 7.5f, 50.f, ROUND_LIGHT_SPREAD);
    createIndicatorsCentered((box.size.x * 0.5f) - ((6.f + 7.f * 9.f) * .5f), 354.f, 9.f);

    addChild(preset_label = createStaticTextLabel<StaticTextLabel>(
        Vec(box.size.x *.5f, 3.5f), box.size.x, "<preset>",
        theme_engine, theme, "curpreset", TextAlignment::Center, 12.f, false
        ));

    addChild(createLightCentered<TinyLight<BlueLight>>(Vec(RACK_GRID_WIDTH * 1.5f, 30), my_module, CoreModule::L_READY));
    addChild(blip = createBlipCentered(box.size.x - RACK_GRID_WIDTH * 1.5f, 30, "LED"));

    float x, y;
    x = RACK_GRID_WIDTH * 1.5f;
    y = 42;
    addChild(createLightCentered<TinySimpleLight<YellowLight>>(Vec(x, y), my_module, CoreModule::L_PULSE));

    addChild(Center(createThemedColorOutput(Vec(RACK_GRID_WIDTH+2, RACK_GRID_HEIGHT - 50.f), my_module, CoreModule::OUT_READY, PORT_MAGENTA, theme_engine, theme)));

    y = RACK_GRID_HEIGHT - 50.f - 25.f;
    // addChild(task_status_label = createStaticTextLabel<StaticTextLabel>(
    //     Vec(UHALF, y), box.size.x - 15.f, "",
    //     theme_engine, theme, "brand", TextAlignment::Left, 10.f, false
    //     ));
    // y -= 16.f;
    addChild(em_status_label = createStaticTextLabel<StaticTextLabel>(
        Vec(UHALF, y), box.size.x - 15.f, "",
        theme_engine, theme, "brand", TextAlignment::Left, 10.f, false
        ));

    if (my_module) {
        my_module->register_client(this);
        my_module->em.subscribeEMEvents(this);
        my_module->tasks.subscribeChange(this);
    }
}

void CoreModuleWidget::createScrews(std::shared_ptr<SvgTheme> theme)
{
    addChild(createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, 0), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH *.5f, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - RACK_GRID_WIDTH - 7.5f, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
}

void CoreModuleWidget::createMidiPickers(std::shared_ptr<SvgTheme> theme)
{
    float y = PICKER_TOP;
    haken_picker = createMidiPicker(UHALF, y, true, "Choose HAKEN device", &my_module->haken_midi);

    std::string text = "<Eagan Matrix Device>";
    if (my_module) {
        if (my_module->haken_midi.connection) {
            text = my_module->haken_midi.connection->info.friendly(TextFormatLength::Short);
        } else if (!my_module->haken_midi.device_claim.empty()) {
            text = my_module->haken_midi.device_claim;
        }
    }

    addChild(firmware_label = createStaticTextLabel<StaticTextLabel>(
        Vec(box.size.x * .5f, box.size.y - 12.5f), 140.f, "v00.00", theme_engine,
        theme, "dytext", TextAlignment::Center, 10.f, false
        ));

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

    float x = 18.f;
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
}

void CoreModuleWidget::createRoundingLeds(float x, float y, float spread)
{
    addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, y), my_module, CoreModule::L_ROUND_Y)); x += spread;
    addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, y), my_module, CoreModule::L_ROUND_INITIAL)); x += spread;
    addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, y), my_module, CoreModule::L_ROUND)); x += spread;
    addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, y), my_module, CoreModule::L_ROUND_RELEASE));
}

void CoreModuleWidget::createIndicatorsCentered(float x, float y, float spread)
{
    auto co = themeColor(coTaskUnscheduled);
    addChild(mididevice_indicator    = createIndicatorCentered(x, y, co, TaskName(HakenTask::MidiDevice))); x += spread;
    addChild(heartbeat_indicator     = createIndicatorCentered(x, y, co, TaskName(HakenTask::HeartBeat))); x += spread;
    addChild(updates_indicator       = createIndicatorCentered(x, y, co, TaskName(HakenTask::Updates))); x += spread;
    addChild(userpresets_indicator   = createIndicatorCentered(x, y, co, TaskName(HakenTask::UserPresets))); x += spread;
    addChild(systempresets_indicator = createIndicatorCentered(x, y, co, TaskName(HakenTask::SystemPresets))); x += spread;
    addChild(presetinfo_indicator    = createIndicatorCentered(x, y, co, TaskName(HakenTask::PresetInfo))); x += spread;
    addChild(lastpreset_indicator    = createIndicatorCentered(x, y, co, TaskName(HakenTask::LastPreset))); x += spread;
    addChild(syncdevices_indicator   = createIndicatorCentered(x, y, co, TaskName(HakenTask::SyncDevices)));
}

void CoreModuleWidget::resetIndicators()
{
    auto co = themeColor(coTaskUnscheduled);
    mididevice_indicator   ->setColor(co); mididevice_indicator   ->setFill(false);
    heartbeat_indicator    ->setColor(co); heartbeat_indicator    ->setFill(false);
    updates_indicator      ->setColor(co); updates_indicator      ->setFill(false);
    userpresets_indicator  ->setColor(co); userpresets_indicator  ->setFill(false);
    systempresets_indicator->setColor(co); systempresets_indicator->setFill(false);
    presetinfo_indicator   ->setColor(co); presetinfo_indicator   ->setFill(false);
    lastpreset_indicator   ->setColor(co); lastpreset_indicator   ->setFill(false);
    syncdevices_indicator  ->setColor(co); syncdevices_indicator  ->setFill(false);
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

void CoreModuleWidget::set_theme_colors(const std::string& theme)
{
    auto name = theme.empty() ? getThemeName() : theme;
    theme_colors[ThemeColor::coHakenMidiIn] = ColorFromTheme(name, "haken-in", nvgRGB(0x54, 0xa7, 0x54));
    theme_colors[ThemeColor::coHakenMidiOut] = ColorFromTheme(name, "haken-out", nvgRGB(0x45, 0x56, 0xe7));

    auto co = nvgRGB(0xe7, 0x44, 0xbe);
    theme_colors[ThemeColor::coC1MidiIn] = ColorFromTheme(name, "c1-in", co);
    theme_colors[ThemeColor::coC2MidiIn] = ColorFromTheme(name, "c2-in", co);

    theme_colors[ThemeColor::coTaskUninitialized] = ColorFromTheme(name, "tsk-0", RampGray(G_45));
    theme_colors[ThemeColor::coTaskUnscheduled]   = ColorFromTheme(name, "tsk-x", RampGray(G_50));
    theme_colors[ThemeColor::coTaskPending]       = ColorFromTheme(name, "tsk-pend", nvgRGB(0xaa, 0x40, 0xbf));
    theme_colors[ThemeColor::coTaskComplete]      = ColorFromTheme(name, "tsk-comp", nvgRGB(0x54, 0xa7, 0x54));
    theme_colors[ThemeColor::coTaskDone]          = ColorFromTheme(name, "tsk-done", nvgRGB(0x77, 0xbb, 0x77));
    theme_colors[ThemeColor::coTaskWaiting]       = ColorFromTheme(name, "tsk-wait", nvgRGB(0x93, 0x99, 0x94));
    theme_colors[ThemeColor::coTaskBroken]        = ColorFromTheme(name, "tsk-broke", nvgRGB(250, 0, 0));

    theme_colors[ThemeColor::coWeird] = GetStockColor(StockColor::Chartreuse);
}

void CoreModuleWidget::setThemeName(const std::string& name)
{
    set_theme_colors(name);
    ChemModuleWidget::setThemeName(name);
}

// IChemClient
void CoreModuleWidget::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) 
{
    std::string nothing = "";
    resetIndicators();
    em_status_label->text(nothing);
    //task_status_label->text(nothing);
    preset_label->text(nothing);

    std::string name = connection ? connection->info.friendly(TextFormatLength::Short) : nothing;
    switch (device) {
    case ChemDevice::Haken: haken_device_label->text(name); break;
    case ChemDevice::Midi1: controller1_device_label->text(name); break;
    case ChemDevice::Midi2: controller2_device_label->text(name); break;
    default: break;
    }
}

// IHandleEmEvents
void CoreModuleWidget::onLoopDetect(uint8_t cc, uint8_t value)
{

}

void CoreModuleWidget::onEditorReply(uint8_t reply)
{
   //em_status_label->text(format_string("Editor reply: 0x%04x", reply));
}

void CoreModuleWidget::onHardwareChanged(uint8_t hardware, uint16_t firmware_version)
{
    firmware_label->text(format_string("%s v%03.2f", ShortHardwareName(hardware), firmware_version/100.f));
}

void CoreModuleWidget::onPresetChanged()
{
    preset_label->text(my_module->em.preset.name);
}

void CoreModuleWidget::onUserComplete() {}
void CoreModuleWidget::onSystemComplete() {}

void CoreModuleWidget::onTaskMessage(uint8_t code)
{
    switch (code) {
        case Haken::reduceGain:
            em_status_label->text("Reduce Gain");
            break;
        case Haken::reducePoly:
            em_status_label->text("Reduce Polyphony");
            break;
        case Haken::inFactCalib:
            em_status_label->text("Factory Calibration");
            break;
        case Haken::eraseMessage:
            em_status_label->text("");
            break;
        case Haken::midiLoopback:
            em_status_label->text("MIDI Loop Detected");
            break;
        case Haken::helloWorld:
            em_status_label->text("The EaganMatrix says hello.");
            break;
        case Haken::testErr:
            em_status_label->text("MIDI test failed");
            break;
        case Haken::endSysNames:
            em_status_label->text("");
            break;
        case Haken::beginSysNames:
            em_status_label->text("Begin system preset names");
            break;
        case Haken::beginUserNames:
            em_status_label->text("Begin user preset names");
            break;
        case Haken::endUserNames:
            em_status_label->text("");
            break;
        case Haken::rxOver:
        case Haken::txOver:
        case Haken::rxSynErr:
        case Haken::rxBitErr:
            em_status_label->text("MIDI receive error");
            break;
        case Haken::sensComm:
        case Haken::nanErr:
        case Haken::ceeSeq:
            em_status_label->text("EaganMatrix internal error");
            break;
    }
}

void CoreModuleWidget::onLED(uint8_t led)
{
    blip->setBrightness(1.f); 
    switch (led) {
    case Haken::ledOff:         blip->setLight(no_light); blip->setBrightness(0.f); break;
    case Haken::ledBlue:        blip->setLight(blue_light); break;
    case Haken::ledRed:         blip->setLight(red_light); break;
    case Haken::ledBrightGreen: blip->setLight(bright_green_light); break;
    case Haken::ledGreen:       blip->setLight(green_light); break;
    case Haken::ledWhite:       blip->setLight(white_light); break;
    case Haken::ledYellow:      blip->setLight(yellow_light); break;
    case Haken::ledPurple:      blip->setLight(purple_light); break;
    case Haken::ledBlueGreen:   blip->setLight(blue_green_light); break;
    }
}

IndicatorWidget* CoreModuleWidget::widget_for_task(HakenTask task)
{
    switch (task) {
    case HakenTask::MidiDevice:    return mididevice_indicator;
    case HakenTask::HeartBeat:     return heartbeat_indicator;
    case HakenTask::Updates:       return updates_indicator;
    case HakenTask::UserPresets:   return userpresets_indicator;
    case HakenTask::SystemPresets: return systempresets_indicator;
    case HakenTask::PresetInfo:    return presetinfo_indicator;
    case HakenTask::LastPreset:    return lastpreset_indicator;
    case HakenTask::SyncDevices:   return syncdevices_indicator;
    default: return nullptr;
    }
}
const NVGcolor& CoreModuleWidget::taskStateColor(TaskState state)
{
    switch (state) {
    case TaskState::Uninitialized: return themeColor(ThemeColor::coTaskUninitialized);
    case TaskState::Unscheduled:   return themeColor(ThemeColor::coTaskUnscheduled);
    case TaskState::Pending:       return themeColor(ThemeColor::coTaskPending);
    case TaskState::Complete:      return themeColor(ThemeColor::coTaskComplete);
    case TaskState::Done:          return themeColor(ThemeColor::coTaskDone);
    case TaskState::Waiting:       return themeColor(ThemeColor::coTaskWaiting);
    case TaskState::Broken:        return themeColor(ThemeColor::coTaskBroken);
    default: return themeColor(ThemeColor::coWeird);
    }
}

// IHakenTaskEvents
void CoreModuleWidget::onHakenTaskChange(HakenTask id)
{
    if ((id != HakenTask::End) && (id != HakenTask::None)) {
        auto task = my_module->tasks.get_task(id);
        if (task) {
            auto widget = widget_for_task(task->id);
            auto text = format_string("%s: %s", TaskName(id), TaskStateName(task->state));
            widget->describe(text);
            widget->setColor(taskStateColor(task->state));
            widget->setFill(task->state != TaskState::Unscheduled);
        }
    }
}

void CoreModuleWidget::step()
{
    ChemModuleWidget::step();
}

inline float midi_animation_cx(uint64_t count) {
    return static_cast<float>(MIDI_ANIMATION_MARGIN + ((count / 15) % (MODULE_WIDTH - 2*MIDI_ANIMATION_MARGIN)));
}

void CoreModuleWidget::drawMidiAnimation(const DrawArgs& args, bool halo)
{
    float y = PICKER_TOP + MIDI_ANIMATION_OFFSET;
    NVGcolor co = themeColor(ThemeColor::coHakenMidiOut);
    float cx = 0; //midi_animation_cx(my_module->haken_midi_out.count());
    if (halo) {
        CircularHalo(args.vg, cx, y, 2.f, 8.5f, co);
    }
    Circle(args.vg, cx, y, 2.25f, co);

    co = themeColor(ThemeColor::coHakenMidiIn);
    cx = midi_animation_cx(my_module->haken_midi_in.count());
    if (halo) {
        CircularHalo(args.vg, cx, y, 2.f, 8.5f, co);
    }
    Circle(args.vg, cx, y, 2.25f, co);

    y += PICKER_INTERVAL;
    if (my_module->isController1Connected()) {
        co = themeColor(ThemeColor::coC1MidiIn);
        cx = midi_animation_cx(my_module->controller1_midi_in.count());
        if (halo) {
            CircularHalo(args.vg, cx, y, 2.f, 8.5f, co);
        }
        Circle(args.vg, cx, y, 2.25f, co);
    }

    y += PICKER_INTERVAL;
    if (my_module->isController2Connected()) {
        co = themeColor(ThemeColor::coC2MidiIn);
        cx = midi_animation_cx(my_module->controller2_midi_in.count());
        if (halo) {
            CircularHalo(args.vg, cx, y, 2.f, 8.5f, co);
        }
        Circle(args.vg, cx, y, 2.25f, co);
    }
}



bool connected (CoreModule* core) {
    return core ? core->isHakenConnected() : false;
}

void CoreModuleWidget::drawLayer(const DrawArgs& args, int layer)
{
    Widget::drawLayer(args, layer);
    if (layer != 1) return;
    if (!my_module) return;

    if (connected(my_module) && rack::settings::rackBrightness < .95f) {
        drawMidiAnimation(args, true);
    }
}


void CoreModuleWidget::draw(const DrawArgs& args)
{
    ChemModuleWidget::draw(args);
    if (!my_module) return;

    if (connected(my_module) && rack::settings::rackBrightness >= .95f) {
        drawMidiAnimation(args, false);
    }

}

void CoreModuleWidget::appendContextMenu(Menu *menu)
{
    ChemModuleWidget::appendContextMenu(menu);
    if (my_module) {
menu->addChild(createSubmenuItem("Haken Requests", "", [=](Menu* menu) {
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Editor Present", "", [this]() {
            my_module->haken_midi_out.sendEditorPresent();
        }));
        menu->addChild(createMenuItem("RequestUpdates", "", [this]() {
            my_module->haken_midi_out.sendRequestUpdates();
        }));
        menu->addChild(createMenuItem("Request Configuration", "", [this]() {
            my_module->haken_midi_out.sendRequestConfiguration();
        }));
        menu->addChild(createMenuItem("Request ConText", "", [this]() {
            my_module->haken_midi_out.sendRequestConText();
        }));
        menu->addChild(createMenuItem("Request User", "", [this]() {
            my_module->haken_midi_out.sendRequestUser();
        }));
        menu->addChild(createMenuItem("Request System", "", [this]() {
            my_module->haken_midi_out.sendRequestSystem();
        }));
}));
//        menu->addChild(new MenuSeparator);
        menu->addChild(createCheckMenuItem(
            "Log MIDI", "",
            [this]() { return my_module->log_midi; },
            [this]() { my_module->enable_logging(!my_module->log_midi); }));
    
    }
}