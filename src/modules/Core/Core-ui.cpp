#include "Core.hpp"
#include "../../services/ModuleBroker.hpp"

// Layout
constexpr const float U1 = 15.f;
constexpr const float UHALF = 7.5f;
constexpr const int MODULE_WIDTH = 165;
constexpr const float PICKER_TOP = 200.f;
constexpr const float PICKER_INTERVAL = 42.f;
constexpr const float PICKER_LABEL_OFFSET = 14.f;
constexpr const float MIDI_ANIMATION_OFFSET = 30.f;
constexpr const float ROUND_LIGHT_SPREAD = 4.f;
constexpr const int MIDI_ANIMATION_MARGIN = 8.f;

CoreModuleWidget::~CoreModuleWidget()
{
    if (my_module) {
        my_module->em.unsubscribeEMEvents(this);
        my_module->tasks.unsubscribeChange(this);
        my_module->ui = nullptr;
    }
    if (chem_host) {
        chem_host->unregister_chem_client(this);
    }
}

CoreModuleWidget::CoreModuleWidget(CoreModule *module) :
    my_module(module),
    chem_host(nullptr)
{
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

    LabelStyle style{"curpreset", TextAlignment::Center, 16.f, true};
    addChild(preset_label = createStaticTextLabel<TipLabel>(
        Vec(box.size.x *.5f, 145.f), box.size.x, "[preset]", theme_engine, theme, style));
    preset_label->glowing(true);

    addChild(createLightCentered<TinyLight<BlueLight>>(Vec(RACK_GRID_WIDTH * 1.5f, 30), my_module, CoreModule::L_READY));
    addChild(blip = createBlipCentered(box.size.x - RACK_GRID_WIDTH * 1.5f, 30, "LED"));

    float x, y;
    x = RACK_GRID_WIDTH * 1.5f;
    y = 42;
    addChild(createLightCentered<TinySimpleLight<YellowLight>>(Vec(x, y), my_module, CoreModule::L_PULSE));
    addChild(Center(createThemedColorOutput(Vec(RACK_GRID_WIDTH+2, RACK_GRID_HEIGHT - 50.f), my_module, CoreModule::OUT_READY, PORT_MAGENTA, theme_engine, theme)));

    style.key = "brand";
    style.align = TextAlignment::Center;
    style.height = 10.f;
    style.bold = false;
    // addChild(task_status_label = createStaticTextLabel<StaticTextLabel>(
    //     Vec(UHALF, y), box.size.x - 15.f, "", theme_engine, theme, style));
    // y -= 16.f;
    y = 3.75;
    addChild(em_status_label = createStaticTextLabel<StaticTextLabel>(
        Vec(box.size.x*.5f, y), box.size.x - 15.f, "", theme_engine, theme, style));

    if (my_module) {
        my_module->register_chem_client(this);
        my_module->em.subscribeEMEvents(this);
        // sync task updates that occurred before now
        for (HakenTask id = HakenTask::First; id < HakenTask::End; id = next_task(id)) {
            onHakenTaskChange(id);
        }
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
    haken_picker = createMidiPicker(UHALF, y, true, "Choose HAKEN device", &my_module->haken_device, theme);
    std::string text = "[Eagan Matrix Device]";
    if (my_module) {
        if (my_module->haken_device.connection) {
            text = my_module->haken_device.connection->info.friendly(TextFormatLength::Short);
        } else if (!my_module->haken_device.device_claim.empty()) {
            text = my_module->haken_device.device_claim;
        }
    }
    LabelStyle style{"dytext", TextAlignment::Center, 10.f};
    addChild(firmware_label = createStaticTextLabel<StaticTextLabel>(
        Vec(box.size.x * .5f, box.size.y - 12.5f), 140.f, "v00.00", theme_engine,
        theme, style));

    style.align = TextAlignment::Left;
    style.height = 14.f;

    addChild(haken_device_label = createStaticTextLabel<StaticTextLabel>(
        Vec(UHALF, y + PICKER_LABEL_OFFSET), 160.f, text, theme_engine, theme, style));

    y += PICKER_INTERVAL;
    controller1_picker = createMidiPicker(UHALF, y, false, "Choose MIDI controller #1", &my_module->controller1, theme);
    addChild(controller1_device_label = createStaticTextLabel<StaticTextLabel>(
        Vec(UHALF, y + PICKER_LABEL_OFFSET), 120.f, "", theme_engine, theme, style));

    y += PICKER_INTERVAL;
    controller2_picker = createMidiPicker(UHALF, y, false, "Choose MIDI controller #2", &my_module->controller2, theme);
    addChild(controller2_device_label = createStaticTextLabel<StaticTextLabel>(
        Vec(UHALF, y + PICKER_LABEL_OFFSET), 120.f, "", theme_engine, theme, style));

    float x = 18.f;
    y = PICKER_TOP - 16.f;
    auto w = Center(createThemedButton<SmallRoundButton>(Vec(x,y), theme_engine, theme, "Reset MIDI\n(Ctrl+Click to clear)"));
    if (my_module) {
        w->setHandler([=](bool ctrl, bool shift) {
            if (ctrl) {
                // clear claims and revoke connections
                // todo: integrate claim revocation somewhere in Core, or MidiDeviceHolder
                auto broker = MidiDeviceBroker::get();
                assert(broker);
                broker->revokeClaim(&my_module->haken_device);
                broker->revokeClaim(&my_module->controller1);
                broker->revokeClaim(&my_module->controller2);
                my_module->haken_device.clear();
                my_module->controller1.clear();
                my_module->controller2.clear();
            } else {
                // drop connections
                my_module->haken_device.connect(nullptr);
                my_module->controller1.connect(nullptr);
                my_module->controller2.connect(nullptr);
                my_module->haken_midi_in.reset();
                my_module->haken_midi_out.output.reset();
                my_module->haken_midi_out.output.channel = -1;
                my_module->controller1_midi_in.reset();
                my_module->controller2_midi_in.reset();
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

MidiPicker* CoreModuleWidget::createMidiPicker(float x, float y, bool isEM, const char *tip, MidiDeviceHolder* device, std::shared_ptr<SvgTheme> theme)
{
    auto picker = createThemedWidget<MidiPicker>(Vec(x, y), theme_engine, theme);
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
    theme_colors[ThemeColor::coTaskUnscheduled]   = ColorFromTheme(name, "tsk-nsch", RampGray(G_50));
    theme_colors[ThemeColor::coTaskPending]       = ColorFromTheme(name, "tsk-pend", nvgRGB(0xaa, 0x40, 0xbf));
    theme_colors[ThemeColor::coTaskComplete]      = ColorFromTheme(name, "tsk-comp", nvgRGB(0x54, 0xa7, 0x54));
    theme_colors[ThemeColor::coTaskDone]          = ColorFromTheme(name, "tsk-done", nvgRGB(0x77, 0xbb, 0x77));
    theme_colors[ThemeColor::coTaskWaiting]       = ColorFromTheme(name, "tsk-wait", nvgRGB(0x93, 0x99, 0x94));
    theme_colors[ThemeColor::coTaskNotApplicable] = ColorFromTheme(name, "tsk-na",   nvgRGB(0x93, 0x99, 0x94));
    theme_colors[ThemeColor::coTaskBroken]        = ColorFromTheme(name, "tsk-broke", nvgRGB(250, 0, 0));

    theme_colors[ThemeColor::coWeird] = GetStockColor(StockColor::Chartreuse);
}

void CoreModuleWidget::setThemeName(const std::string& name)
{
    set_theme_colors(name);
    ChemModuleWidget::setThemeName(name);
}

// IChemClient
rack::engine::Module* CoreModuleWidget::client_module()
{
    return my_module; 
}
std::string CoreModuleWidget::client_claim()
{
    return my_module ? my_module->haken_device.getClaim() : "";
}
void CoreModuleWidget::onConnectHost(IChemHost* host)
{
    chem_host = host;
}

void CoreModuleWidget::onPresetChange()
{}

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
    auto preset = my_module->host_preset();
    preset_label->text(preset->name);
    preset_label->describe(preset->summary());
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
            em_status_label->text("System preset names");
            break;
        case Haken::beginUserNames:
            em_status_label->text("User preset names");
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
    blip->set_brightness(1.f); 
    switch (led) {
    case Haken::ledOff:         blip->set_light_color(no_light); blip->set_brightness(0.f); break;
    case Haken::ledBlue:        blip->set_light_color(blue_light); break;
    case Haken::ledRed:         blip->set_light_color(red_light); break;
    case Haken::ledBrightGreen: blip->set_light_color(bright_green_light); break;
    case Haken::ledGreen:       blip->set_light_color(green_light); break;
    case Haken::ledWhite:       blip->set_light_color(white_light); break;
    case Haken::ledYellow:      blip->set_light_color(yellow_light); break;
    case Haken::ledPurple:      blip->set_light_color(purple_light); break;
    case Haken::ledBlueGreen:   blip->set_light_color(blue_green_light); break;
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
    case TaskState::NotApplicable: return themeColor(ThemeColor::coTaskNotApplicable);
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
            if (id == HakenTask::SyncDevices && !ModuleBroker::get()->is_primary(my_module)) {
                widget->describe("Sync: Not primary");
                widget->setColor(RampGray(G_40));
                widget->setFill(false);
            } else {
                auto text = format_string("%s: %s", TaskName(id), TaskStateName(task->state));
                widget->describe(text);
                widget->setColor(taskStateColor(task->state));
                widget->setFill((task->state != TaskState::Unscheduled) && (task->state != TaskState::NotApplicable));
            }
        }
    }
}

void CoreModuleWidget::step()
{
    ChemModuleWidget::step();
}

inline float midi_animation_cx(uint64_t count) {
    return static_cast<float>(MIDI_ANIMATION_MARGIN + ((count / 20) % (MODULE_WIDTH - 2*MIDI_ANIMATION_MARGIN)));
}

void drawDotHalo(NVGcontext* vg, float x, float y, const NVGcolor& co, bool halo)
{
    const float dotr = 1.75f;
    const float halo_inner = 1.7f;
    const float halo_outer = 4.5f;

    if (halo) {
        CircularHalo(vg, x, y, halo_inner, halo_outer, co);
    }
    Circle(vg, x, y, dotr, co);    
}

void CoreModuleWidget::drawMidiAnimation(const DrawArgs& args, bool halo)
{
    float y = PICKER_TOP + MIDI_ANIMATION_OFFSET;
    float x = midi_animation_cx(my_module->haken_midi_out.count());
    NVGcolor co = themeColor(ThemeColor::coHakenMidiOut);
    drawDotHalo(args.vg, x, y, co, halo);

    co = themeColor(ThemeColor::coHakenMidiIn);
    x = midi_animation_cx(my_module->haken_midi_in.count());
    drawDotHalo(args.vg, x, y, co, halo);

    y += PICKER_INTERVAL;
    if (my_module->isController1Connected()) {
        co = themeColor(ThemeColor::coC1MidiIn);
        x = midi_animation_cx(my_module->controller1_midi_in.count());
        drawDotHalo(args.vg, x, y, co, halo);
    }

    y += PICKER_INTERVAL;
    if (my_module->isController2Connected()) {
        co = themeColor(ThemeColor::coC2MidiIn);
        x = midi_animation_cx(my_module->controller2_midi_in.count());
        drawDotHalo(args.vg, x, y, co, halo);
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
    if (my_module) {
        menu->addChild(new MenuSeparator);
        menu->addChild(createCheckMenuItem(
            "Log MIDI", "",
            [this]() { return my_module->is_logging; },
            [this]() { my_module->enable_logging(!my_module->is_logging); }));

        menu->addChild(createSubmenuItem("Themes", "", [=](Menu* menu) {
            ChemModuleWidget::appendContextMenu(menu);
        }));
        menu->addChild(createMenuItem("Previous sys preset", "", [this]() {
            my_module->haken_midi.previous_system_preset();
        }));
        menu->addChild(createMenuItem("Next sys preset", "", [this]() {
            my_module->haken_midi.next_system_preset();
        }));

        menu->addChild(createSubmenuItem("Calibration", "", [=](Menu* menu) {
            menu->addChild(createMenuItem("Reset calibration", "", [this]() {
                my_module->haken_midi.reset_calibration();
            }));
            menu->addChild(createMenuItem("Refine calibration", "", [this]() {
                my_module->haken_midi.refine_calibration();
            }));
            menu->addChild(createMenuItem("Factory calibration", "", [this]() {
                my_module->haken_midi.factory_calibration();
            }));
            menu->addChild(new MenuSeparator);
            menu->addChild(createMenuItem("Surface alignment", "", [this]() {
                my_module->haken_midi.surface_alignment();
            }));
        }));

        menu->addChild(createSubmenuItem("Haken Requests", "", [=](Menu* menu) {
            menu->addChild(new MenuSeparator);
            menu->addChild(createMenuItem("Editor Hello", "", [this]() {
                my_module->haken_midi.editor_present();
            }));
            menu->addChild(createMenuItem("ConText", "", [this]() {
                my_module->haken_midi.request_con_text();
            }));
            menu->addChild(createMenuItem("Updates", "", [this]() {
                my_module->haken_midi.request_updates();
            }));
            menu->addChild(createMenuItem("Configuration", "", [this]() {
                my_module->haken_midi.request_configuration();
            }));
            menu->addChild(createMenuItem("User presets", "", [this]() {
                my_module->haken_midi.request_user();
            }));
            menu->addChild(createMenuItem("System presets", "", [this]() {
                my_module->haken_midi.request_system();
            }));
            menu->addChild(new MenuSeparator);
            menu->addChild(createMenuItem("Remake Mahling data", "", [this]() {
                my_module->haken_midi.remake_mahling();
            }));

        }));
    
    }
}