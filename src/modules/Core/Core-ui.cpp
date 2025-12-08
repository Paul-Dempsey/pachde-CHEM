#include "Core.hpp"
#include "services/ModuleBroker.hpp"
#include "services/kv-store.hpp"
#include "services/open-file.hpp"
#include "widgets/draw-button.hpp"
#include "widgets/theme-button.hpp"
#include "widgets/theme-knob.hpp"
#include "em/preset-meta.hpp"
using EME = IHandleEmEvents::EventMask;
const char * preset_list_file_dialog_filter = "Preset list (.json):json;Any (*):*";
constexpr const float LOGO_CENTER{62.f};

CoreModuleWidget::~CoreModuleWidget()
{
    if (my_module) {
        my_module->em.unsubscribeEMEvents(this);
    }
    if (chem_host) {
        chem_host->unregister_chem_client(this);
    }
}

// Layout
constexpr const int MODULE_WIDTH = 165;
constexpr const float CENTER = static_cast<float>(MODULE_WIDTH) * .5f;
constexpr const float NAV_ROW = 144.f;
constexpr const float PICKER_TOP = 222.f;
constexpr const float PICKER_INTERVAL = 36.5f;
constexpr const float PICKER_LABEL_OFFSET = 12.5f;
constexpr const float MIDI_ANIMATION_OFFSET = 27.75f;
constexpr const float ROUND_LIGHT_SPREAD = 6.f;
constexpr const int MIDI_ANIMATION_MARGIN = 8.f;
namespace S = pachde::style;

CoreModuleWidget::CoreModuleWidget(CoreModule *module) :
    my_module(module),
    chem_host(nullptr)
{
    setModule(module);

    if (my_module) {
        my_module->set_chem_ui(this);
    }
    IHandleEmEvents::em_event_mask =
        EME::HardwareChanged
        + EME::PresetChanged
        + EME::TaskMessage
        + EME::LED
    ;
    IHandleEmEvents::module_id = ChemId::Core;

    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);

    set_theme_colors();

    if (style::show_screws()) {
        createScrews(theme);
    }
    createMidiPickers(theme);
    float r_col = box.size.x - RACK_GRID_WIDTH * 1.5f;

    auto useless = createChemKnob<UselessKnob>(Vec(CENTER, LOGO_CENTER), my_module, CoreModule::P_NOTHING, theme_engine, theme);
    useless->speed = .5f;
    useless->bright = true;
    useless->minAngle = 0;
    useless->maxAngle = 2*M_PI;
    addChild(useless);

    createIndicatorsCentered(CENTER, PICKER_TOP - 10.f, 9.f);

    auto menu = Center(createThemedWidget<CoreMenu>(Vec(box.size.x - RACK_GRID_WIDTH, PICKER_TOP - 14.f), theme_engine, theme));
    menu->setUi(this);
    menu->describe("Core actions menu");
    addChild(menu);

    LabelStyle style{"curpreset", TextAlignment::Center, 16.f, true};
    addChild(preset_label = createLabel<TipLabel>(
        Vec(CENTER, 118.f), box.size.x, "[preset]", theme_engine, theme, style));
    preset_label->glowing(true);

    createRoundingLeds(CENTER + 40.f, NAV_ROW, ROUND_LIGHT_SPREAD);
    create_octave_shift_leds(this, CENTER - 40.f, NAV_ROW, 4.5f, my_module, CoreModule::L_OCT_SHIFT_FIRST);

    auto prev = createWidgetCentered<PrevButton>(Vec(CENTER - 9.5f, NAV_ROW));
    if (my_module) {
        prev->describe("Select previous preset");
        prev->setHandler([this](bool c, bool s){ my_module->prev_preset(); });
    }
    prev->applyTheme(theme_engine, theme);
    addChild(prev);

    auto next = createWidgetCentered<NextButton>(Vec(CENTER + 9.f, NAV_ROW));
    if (my_module) {
        next->describe("Select next preset");
        next->setHandler([this](bool c, bool s){ my_module->next_preset(); });
    }
    next->applyTheme(theme_engine, theme);
    addChild(next);

    float x, y;
    x = RACK_GRID_WIDTH * 1.5f;
    y = 30.f;
    addChild(createLightCentered<TinyLight<BlueLight>>(Vec(x, y), my_module, CoreModule::L_READY));
    addChild(em_led = createBlipCentered(r_col, y, "EM LED"));

    x = CENTER;
    y = 178.f;
    addChild(attenuation_knob = createChemKnob<BlueKnob>(Vec(x, y), my_module, CoreModule::P_ATTENUATION, theme_engine, theme));

    y += 18.f;
    LabelStyle status_style{"brand", TextAlignment::Center, 10.f, false};
    addChild(em_status_label = createLabel<TextLabel>(Vec(CENTER, y), box.size.x - 15.f, "", theme_engine, theme, status_style));

    const NVGcolor co_port = PORT_CORN;
    y = S::PORT_TOP + S::PORT_DY;
    x = RACK_GRID_WIDTH+2;
    addChild(Center(createThemedColorOutput(Vec(x, y), my_module, CoreModule::OUT_READY, "ready-ring", PORT_MAGENTA, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 18.f, "OK", theme_engine, theme, S::in_port_label));
    x = 125.f;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, CoreModule::IN_C1_MUTE_GATE, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 18.f, "M1", theme_engine, theme, S::in_port_label));
    x += 25.f;
    addChild(Center(createThemedColorInput(Vec(x, y), my_module, CoreModule::IN_C2_MUTE_GATE, S::InputColorKey, co_port, theme_engine, theme)));
    addChild(createLabel<TextLabel>(Vec(x, y + S::PORT_LABEL_DY), 18.f, "M2", theme_engine, theme, S::in_port_label));

    if (my_module) {
        my_module->set_chem_ui(this);
        my_module->register_chem_client(this);
        my_module->em.subscribeEMEvents(this);
    }
}

void CoreModuleWidget::createScrews(std::shared_ptr<SvgTheme> theme)
{
    addChild(createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, 0), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
    addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
}

void CoreModuleWidget::createMidiPickers(std::shared_ptr<SvgTheme> theme)
{
    LabelStyle style{"dytext", TextAlignment::Center, 10.f};
    addChild(firmware_label = createLabel<TextLabel>(
        Vec(CENTER, box.size.y - 12.5f), 140.f, "v00.00", theme_engine,
        theme, style));

    const float midi_x = 7.5f;
    LabelStyle midi_style{"midi-name", TextAlignment::Left, 12.f};
    float y = PICKER_TOP;
    haken_picker = createMidiPicker(midi_x, y, "HAKEN device", &my_module->haken_device, &my_module->haken_device, theme);
    std::string text = (my_module) ? my_module->device_name(ChemDevice::Haken) : "[Eagan Matrix Device]";
    addChild(haken_device_label = createLabel<TextLabel>(
        Vec(S::UHALF, y + PICKER_LABEL_OFFSET), MODULE_WIDTH - S::UHALF, text, theme_engine, theme, midi_style));

    y += PICKER_INTERVAL;
    controller1_picker = createMidiPicker(midi_x, y, "MIDI controller #1", &my_module->controller1, &my_module->haken_device, theme);
    text = (my_module) ? my_module->device_name(ChemDevice::Midi1) : "";
    addChild(controller1_device_label = createLabel<TextLabel>(
        Vec(S::UHALF, y + PICKER_LABEL_OFFSET), MODULE_WIDTH - S::UHALF, text, theme_engine, theme, midi_style));

    addChild(Center(createThemedParamLightButton<SmallRoundParamButton, TinySimpleLight<GreenLight>>(
        Vec(121.f, y + 6.f), my_module, CoreModule::P_C1_CHANNEL_MAP, CoreModule::L_C1_CHANNEL_MAP, theme_engine, theme)));
    addChild(Center(createThemedParamLightButton<SmallRoundParamButton, TinySimpleLight<GreenLight>>(
        Vec(138.f, y + 6.f), my_module, CoreModule::P_C1_MUSIC_FILTER, CoreModule::L_C1_MUSIC_FILTER, theme_engine, theme)));
    addChild(Center(createThemedParamLightButton<SmallRoundParamButton, TinySimpleLight<RedLight>>(
        Vec(155.f, y + 6.f), my_module, CoreModule::P_C1_MUTE, CoreModule::L_C1_MUTE, theme_engine, theme)));

    y += PICKER_INTERVAL;
    controller2_picker = createMidiPicker(midi_x, y, "MIDI controller #2", &my_module->controller2, &my_module->haken_device, theme);
    text = (my_module) ? my_module->device_name(ChemDevice::Midi2) : "";
    addChild(controller2_device_label = createLabel<TextLabel>(
        Vec(S::UHALF, y + PICKER_LABEL_OFFSET), MODULE_WIDTH - S::UHALF, text, theme_engine, theme, midi_style));
    addChild(Center(createThemedParamLightButton<SmallRoundParamButton, TinySimpleLight<GreenLight>>(
        Vec(121.f, y + 6.f), my_module, CoreModule::P_C2_CHANNEL_MAP, CoreModule::L_C2_CHANNEL_MAP, theme_engine, theme)));
    addChild(Center(createThemedParamLightButton<SmallRoundParamButton, TinySimpleLight<GreenLight>>(
        Vec(138.f, y + 6.f), my_module, CoreModule::P_C2_MUSIC_FILTER, CoreModule::L_C2_MUSIC_FILTER, theme_engine, theme)));
    addChild(Center(createThemedParamLightButton<SmallRoundParamButton, TinySimpleLight<RedLight>>(
        Vec(155.f, y + 6.f), my_module, CoreModule::P_C2_MUTE, CoreModule::L_C2_MUTE, theme_engine, theme)));

    float x = 18.f;
    y = PICKER_TOP - 14.f;
    auto w = Center(createThemedButton<SmallRoundButton>(Vec(x,y), theme_engine, theme, "Reset MIDI\n(" RACK_MOD_CTRL_NAME "+Click to clear)"));
    if (my_module) {
        w->setHandler([=](bool ctrl, bool shift) {
            if (ctrl) {
                auto broker = MidiDeviceBroker::get();
                assert(broker);
                my_module->haken_device.clear();
                my_module->haken_midi_out.clear();
                my_module->controller1.clear();
                my_module->controller2.clear();
                my_module->haken_midi_out.enable();
                my_module->controller1_midi_in.enable();
                my_module->controller2_midi_in.enable();
            } else {
                // drop connections
                my_module->haken_device.connect(nullptr);
                my_module->controller1.connect(nullptr);
                my_module->controller2.connect(nullptr);
                my_module->haken_midi_in.reset();
                my_module->haken_midi_out.clear();
                my_module->haken_midi_out.enable();
                my_module->controller1_midi_in.reset();
                my_module->controller1_midi_in.enable();
                my_module->controller2_midi_in.reset();
                my_module->controller2_midi_in.enable();
            }
        });
    }
    addChild(w);
}

void CoreModuleWidget::createRoundingLeds(float cx, float cy, float spread)
{
    float x = cx - 1.5 * spread;
    addChild(createLightCentered<TinySimpleLight<RedLight>>(Vec(x, cy), my_module, CoreModule::L_ROUND_Y)); x += spread;
    addChild(createLightCentered<TinySimpleLight<RedLight>>(Vec(x, cy), my_module, CoreModule::L_ROUND_INITIAL)); x += spread;
    addChild(createLightCentered<TinySimpleLight<RedLight>>(Vec(x, cy), my_module, CoreModule::L_ROUND)); x += spread;
    addChild(createLightCentered<TinySimpleLight<RedLight>>(Vec(x, cy), my_module, CoreModule::L_ROUND_RELEASE));
}

void CoreModuleWidget::create_stop_button()
{
    if (!stop_button) {
        stop_button = createWidget<TextButton>(Vec(CENTER, NAV_ROW));
        stop_button->box.size.x = 80.f;
        stop_button->box.size.y = 18.f;
        stop_button->set_text("stop scan");
        stop_button->setHandler([=](bool, bool){
            my_module->stop_scan = true;
            remove_stop_button();
            em_status_label->text("Preset scan stopped");
        });
        addChild(Center(stop_button));
    }
}

void CoreModuleWidget::remove_stop_button()
{
    if (stop_button) {
        stop_button->requestDelete();
        stop_button = nullptr;
    }
}

void CoreModuleWidget::createIndicatorsCentered(float x, float y, float spread)
{
    auto co = themeColor(coTask0);
    x -= (spread * 5) *.5f;
    addChild(mididevice_indicator      = createIndicatorCentered(x, y, co, "Midi device")); x += spread;
    addChild(heartbeat_indicator       = createIndicatorCentered(x, y, co, "Heartbeat")); x += spread;
    addChild(em_init_indicator         = createIndicatorCentered(x, y, co, "Updates")); x += spread;
    addChild(presetinfo_indicator      = createIndicatorCentered(x, y, co, "Initial preset info")); x += spread;
    addChild(system_presets_indicator  = createIndicatorCentered(x, y, co, "System preset info")); x += spread;
    addChild(user_presets_indicator    = createIndicatorCentered(x, y, co, "User preset info"));
}

void CoreModuleWidget::updateIndicators()
{
    if (my_module) {
        for (size_t i = 0; i < sizeof(my_module->start_states)/sizeof(my_module->start_states[0]); ++i) {
            auto w = widget_for_task(ChemTaskId(i));
            if (my_module->disconnected) {
                w->setLook(themeColor(ThemeColor::coTask0), false);
            } else {
                auto state = my_module->start_states[i];
                w->setLook(taskStateColor(state), ChemTask::State::Untried != state);
            }
        }
        system_presets_indicator->setLook(taskStateColor(my_module->system_presets->empty()
            ? ChemTask::State::Untried
            : (gather_system(my_module->gathering) ? ChemTask::State::Pending : ChemTask::State::Complete)));
        user_presets_indicator->setLook(taskStateColor(my_module->user_presets->empty()
            ? ChemTask::State::Untried
            : (gather_user(my_module->gathering) ? ChemTask::State::Pending : ChemTask::State::Complete)));
    } else {
        mididevice_indicator->setLook(taskStateColor(ChemTask::State::Complete));
        heartbeat_indicator->setLook(taskStateColor(ChemTask::State::Complete));
        em_init_indicator->setLook(taskStateColor(ChemTask::State::Complete));
        presetinfo_indicator->setLook(taskStateColor(ChemTask::State::Pending));
        user_presets_indicator->setLook(taskStateColor(ChemTask::State::Untried));
        system_presets_indicator->setLook(taskStateColor(ChemTask::State::Untried));
    }
}

// void CoreModuleWidget::resetIndicators()
// {
//     auto co = themeColor(coTask0);
//     mididevice_indicator   ->setColor(co); mididevice_indicator   ->setFill(false);
//     heartbeat_indicator    ->setColor(co); heartbeat_indicator    ->setFill(false);
//     em_init_indicator      ->setColor(co); em_init_indicator      ->setFill(false);
//     presetinfo_indicator   ->setColor(co); presetinfo_indicator   ->setFill(false);
// }
void CoreModuleWidget::connect_midi(bool on)
{
    if (my_module) my_module->connect_midi(on);
    if (!on) {
        em_led->describe("MIDI Disconnected");
        em_led->set_light_color(red_light);
        em_led->set_brightness(1.0);
        attenuation_knob->enable(false);
    } else {
        em_led->describe("EM LED");
        attenuation_knob->enable(true);
    }
}

void CoreModuleWidget::show_busy(bool busy)
{
    if (busy) {
        if (!spinning) {
            startSpinner(this, Vec(box.size.x*.5, LOGO_CENTER));
        }
        spinning = true;
    } else {
        stopSpinner(this);
        spinning = false;
        em_status_label->text("");
    }
}

void CoreModuleWidget::glowing_knobs(bool glow)
{
    attenuation_knob->glowing(glow);
}

void CoreModuleWidget::save_as_user_preset_file()
{
    if (!my_module) return;
    if (!my_module->user_presets) return;
    if (my_module->user_presets->empty()) return;

    std::string folder = asset::user(pluginInstance->slug.c_str());
    std::string path;
    bool ok = saveFileDialog(folder, preset_list_file_dialog_filter, my_module->user_presets->filename, path);
    if (ok) {
        auto ext = system::getExtension(path);
        if (ext.empty()) {
            path.append(".json");
        }
        auto hardware = my_module->em.get_hardware();
        assert(hardware);
        my_module->user_presets->save(path, hardware);
        my_module->update_user_preset_file_infos();
    }
}

void CoreModuleWidget::open_user_preset_file()
{
    if (!my_module) return;
    if (!my_module->user_presets) return;

    std::string folder = asset::user(pluginInstance->slug.c_str());
    std::string path;
    bool ok = openFileDialog(folder, preset_list_file_dialog_filter, my_module->user_presets->filename, path);
    if (ok) {
        //TODO: check hardware
        my_module->user_presets->load(path);
        my_module->update_user_preset_file_infos();
    }
}

MidiPicker* CoreModuleWidget::createMidiPicker(float x, float y, const char *tip, MidiDeviceHolder* device, MidiDeviceHolder* haken_device, std::shared_ptr<SvgTheme> theme)
{
    auto picker = createThemedWidget<MidiPicker>(Vec(x, y), theme_engine, theme);
    picker->describe(tip);
    if (my_module) {
        picker->setDeviceHolder(device, haken_device);
    }
    addChild(picker);
    return picker;
}

void CoreModuleWidget::set_theme_colors(const std::string& theme)
{
    auto name = theme.empty() ? getThemeName() : theme;
    theme_colors[ThemeColor::coHakenMidiIn] = ColorFromTheme(name, "haken-in", nvgRGB(0x54, 0xa7, 0x54));
    theme_colors[ThemeColor::coHakenMidiOut] = ColorFromTheme(name, "haken-out", nvgRGB(0x45, 0x56, 0xe7));

    auto co = nvgRGB(0xe7, 0x44, 0xbe);
    theme_colors[ThemeColor::coC1MidiIn]     = ColorFromTheme(name, "c1-in", co);
    theme_colors[ThemeColor::coC2MidiIn]     = ColorFromTheme(name, "c2-in", co);
    theme_colors[ThemeColor::coTask0]        = ColorFromTheme(name, "tsk-0",     RampGray(G_45));
    theme_colors[ThemeColor::coTaskPending]  = ColorFromTheme(name, "tsk-pend",  nvgRGB(0xaa, 0x40, 0xbf));
    theme_colors[ThemeColor::coTaskComplete] = ColorFromTheme(name, "tsk-comp",  nvgRGB(0x54, 0xa7, 0x54));
    theme_colors[ThemeColor::coTaskWaiting]  = ColorFromTheme(name, "tsk-wait",  nvgRGB(0x93, 0x99, 0x94));
    theme_colors[ThemeColor::coTaskBroken]   = ColorFromTheme(name, "tsk-broke", nvgRGB(250, 0, 0));
    theme_colors[ThemeColor::coWeird] = GetStockColor(StockColor::Chartreuse);
}

void CoreModuleWidget::setThemeName(const std::string& name, void * context)
{
    set_theme_colors(name);
    ChemModuleWidget::setThemeName(name, context);
}

// IChemClient
rack::engine::Module* CoreModuleWidget::client_module()
{
    return my_module;
}
std::string CoreModuleWidget::client_claim()
{
    return my_module ? my_module->haken_device.get_claim() : "";
}
void CoreModuleWidget::onConnectHost(IChemHost* host)
{
    chem_host = host;
}

void CoreModuleWidget::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    std::string nothing = "";
    std::string name = connection ? connection->info.friendly(NameFormat::Short) : nothing;
    switch (device) {
    case ChemDevice::Haken:
        em_status_label->text(nothing);
        preset_label->text(nothing);
        preset_label->describe(nothing);
        haken_device_label->text(name);
        break;
    case ChemDevice::Midi1: controller1_device_label->text(name); break;
    case ChemDevice::Midi2: controller2_device_label->text(name); break;
    default: break;
    }
}

// IHandleEmEvents
void CoreModuleWidget::onHardwareChanged(uint8_t hardware, uint16_t firmware_version)
{
    firmware_label->text(format_string("%s v%03.2f", ShortHardwareName(hardware), firmware_version/100.f));
}

void CoreModuleWidget::onPresetChanged()
{
    auto preset = my_module->host_preset();
    if (preset) {
        preset_label->text(preset->name);
        preset_label->describe(preset->meta_text());
    } else {
        preset_label->text("");
        preset_label->describe("");
    }
}

void CoreModuleWidget::onTaskMessage(uint8_t code)
{
    if (my_module && my_module->gathering) return;
    switch (code) {
        case Haken::archiveOk:
        case Haken::eraseMessage:
        case Haken::endSysNames:
        case Haken::endUserNames:
            em_status_label->text("");
            break;
        case Haken::archiveFail:
            em_status_label->text("Archive Fail");
            break;
        case Haken::reduceGain:
            em_status_label->text("Reduce Gain");
            break;
        case Haken::reducePoly:
            em_status_label->text("Reduce Polyphony");
            break;
        case Haken::inFactCalib:
            em_status_label->text("Factory Calibration...");
            break;
        case Haken::midiLoopback:
            em_status_label->text("MIDI Loop Detected");
            break;
        case Haken::createLed:
            em_led->set_light_color(yellow_light);
            em_led->set_brightness(1.0);
            break;
        case Haken::helloWorld:
            em_status_label->text("The EaganMatrix says hello.");
            break;
        case Haken::testErr:
            em_status_label->text("MIDI test failed");
            break;
        case Haken::beginSysNames:
            em_status_label->text("Gathering System presets...");
            break;
        case Haken::beginUserNames:
            em_status_label->text("Gathering User presets...");
            break;
        case Haken::rxOver:
        case Haken::txOver:
        case Haken::rxSynErr:
        case Haken::rxBitErr:
            em_status_label->text("MIDI comms error");
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
    NVGcolor co = no_light;
    float bright = 1.0;
    switch (led) {
    case Haken::ledOff: bright = 0.f; break;
    case Haken::ledBlue:        co = blue_light; break;
    case Haken::ledRed:         co = red_light; break;
    case Haken::ledBrightGreen: co = bright_green_light; break;
    case Haken::ledGreen:       co = green_light; break;
    case Haken::ledWhite:       co = white_light; break;
    case Haken::ledYellow:      co = yellow_light; break;
    case Haken::ledPurple:      co = purple_light; break;
    case Haken::ledBlueGreen:   co = blue_green_light; break;
    }
    em_led->set_light_color(co);
    em_led->set_brightness(bright);
}

IndicatorWidget *CoreModuleWidget::widget_for_task(ChemTaskId task)
{
    switch (task) {
    case ChemTaskId::Heartbeat: return heartbeat_indicator;
    case ChemTaskId::HakenDevice: return mididevice_indicator;
    case ChemTaskId::EmInit: return em_init_indicator;
    case ChemTaskId::PresetInfo: return presetinfo_indicator;
    default: break;
    }
    return nullptr;
}

const NVGcolor& CoreModuleWidget::taskStateColor(ChemTask::State state)
{
    switch (state) {
    case ChemTask::State::Untried:  return themeColor(ThemeColor::coTask0);
    case ChemTask::State::Pending:  return themeColor(ThemeColor::coTaskPending);
    case ChemTask::State::Complete: return themeColor(ThemeColor::coTaskComplete);
    case ChemTask::State::Broken:   return themeColor(ThemeColor::coTaskBroken);
    default: return themeColor(ThemeColor::coWeird);
    }
}

void CoreModuleWidget::step()
{
    updateIndicators();
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
    if (my_module->is_controller_1_connected()) {
        co = themeColor(ThemeColor::coC1MidiIn);
        x = midi_animation_cx(my_module->controller1_midi_in.count());
        drawDotHalo(args.vg, x, y, co, halo);
    }

    y += PICKER_INTERVAL;
    if (my_module->is_controller_2_connected()) {
        co = themeColor(ThemeColor::coC2MidiIn);
        x = midi_animation_cx(my_module->controller2_midi_in.count());
        drawDotHalo(args.vg, x, y, co, halo);
    }
}

bool connected (CoreModule* core) {
    return core ? core->is_haken_connected() : false;
}

void CoreModuleWidget::drawLayer(const DrawArgs& args, int layer)
{
    ChemModuleWidget::drawLayer(args, layer);
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

//  ----  CoreMenu  ----

void CoreMenu::onHoverKey(const HoverKeyEvent& e)
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

void CoreMenu::appendContextMenu(ui::Menu* menu)
{
    auto my_module = ui->my_module;
    if (!my_module) return;
    bool busy = my_module->host_busy();

    menu->addChild(createMenuLabel<HamburgerTitle>("Core Actions"));

    menu->addChild(createSubmenuItem("Presets", "", [this, my_module, busy](Menu* menu) {

        // User Presets section

        std::string preset_file = my_module->user_presets ? system::getFilename(my_module->user_presets->filename) : "";
        bool no_file = preset_file.empty();
        if (no_file) {
            preset_file = "(none)";
        }
        menu->addChild(createMenuLabel(format_string("File: %s", preset_file.c_str())));

        bool no_presets = (nullptr ==  my_module->user_presets) ||  my_module->user_presets->empty();
        menu->addChild(createMenuItem("Save as...", "", [=]() {
            ui->save_as_user_preset_file();
        }, busy || no_file || no_presets));

        menu->addChild(createMenuItem(format_string("Open...", preset_file.c_str()), "", [=]() {
            ui->open_user_preset_file();
        }, busy || no_file));

        menu->addChild(new MenuSeparator);

        menu->addChild(createMenuItem("Clear User presets", "", [=]() {
            my_module->clear_presets(PresetTab::User);
        }, busy));
        if (my_module->em.is_osmose()) {
            menu->addChild(createMenuItem("Scan User preset database", "(page 1)", [=]() {
                my_module->load_full_user_presets();
            }, busy));
            menu->addChild(createSubmenuItem("Scan more User pages", "", [=](Menu* menu) {
                menu->addChild(createMenuItem("Scan and append Page 2", "", [=]() {
                    my_module->scan_osmose_presets(91);
                }, busy));
                menu->addChild(createMenuItem("Scan and append Page 3", "", [=]() {
                    my_module->scan_osmose_presets(92);
                }, busy));
                menu->addChild(createMenuItem("Scan and append Page 4", "", [=]() {
                    my_module->scan_osmose_presets(93);
                }, busy));
                menu->addChild(createMenuItem("Scan and append Page 5", "", [=]() {
                    my_module->scan_osmose_presets(94);
                }, busy));
                // menu->addChild(createMenuItem("Page 6", "", [my_module]() {}));
                // menu->addChild(createMenuItem("Page 7", "", [my_module]() {}));
                // menu->addChild(createMenuItem("Page 8", "", [my_module]() {}));
                // menu->addChild(createMenuItem("Page 9", "", [my_module]() {}));
                // menu->addChild(createMenuItem("Page 10", "", [my_module]() {}));
            }));
        } else {
            menu->addChild(createMenuItem("Quick scan - User presets", "", [=]() {
                my_module->load_quick_user_presets();
            }, busy));
            menu->addChild(createMenuItem("Full scan - User preset database", "", [=]() {
                my_module->load_full_user_presets();
            }, busy));
        }

        // System Presets section

        menu->addChild(new MenuSeparator);

        preset_file = my_module->system_presets ? system::getFilename(my_module->system_presets->filename) : "";
        no_file = preset_file.empty();
        if (no_file) {
            preset_file = "(none)";
        }

        menu->addChild(createMenuLabel(format_string("File: %s", preset_file.c_str())));

        menu->addChild(createMenuItem("Clear System presets", "", [=]() {
            my_module->clear_presets(PresetTab::System);
        }, busy));

        if (my_module->em.is_osmose()) {
            menu->addChild(createMenuItem("Scan System preset database", "", [=]() {
                my_module->load_full_system_presets();
            }, busy));
        } else {
            menu->addChild(createMenuItem("Quick scan - System presets", "", [=]() {
                my_module->load_quick_system_presets();
            }, busy));
            menu->addChild(createMenuItem("Full scan - System preset database", "", [=]() {
                my_module->load_full_system_presets();
            }, busy));
        }
    }));

    menu->addChild(createCheckMenuItem(
        "Log MIDI", "",
        [my_module]() { return my_module->is_logging(); },
        [my_module]() { my_module->enable_logging(!my_module->is_logging()); }));

    menu->addChild(createCheckMenuItem("Disconnect MIDI", "",
        [=](){ return my_module->disconnected; },
        [=](){ ui->connect_midi(my_module->disconnected); },
        my_module->disconnected ? false : busy
    ));

    bool logos = style::show_browser_logo();
    menu->addChild(createCheckMenuItem("Show logos in browser", "",
        [=](){ return logos; },
        [=](){
            auto kv = get_plugin_kv_store();
            if (kv && kv->load()) {
                const char* key = "browser-logo";
                kv->update(key, KVStore::bool_text(!logos));
            }
        },
        busy
    ));

    menu->addChild(createCheckMenuItem("Glowing knobs", "",
        [my_module](){ return my_module->glow_knobs; },
        [this, my_module](){
            my_module->glow_knobs = !my_module->glow_knobs;
            ui->glowing_knobs(my_module->glow_knobs);
        }
    ));

    if (my_module->em.is_surface()) {
        menu->addChild(createSubmenuItem("Calibration", "", [=](Menu* menu) {
            menu->addChild(createMenuItem("Reset calibration", "", [my_module]() {
                my_module->haken_midi.reset_calibration(ChemId::Core);
            }, busy));
            menu->addChild(createMenuItem("Refine calibration", "", [my_module]() {
                my_module->haken_midi.refine_calibration(ChemId::Core);
            }, busy));
            menu->addChild(createMenuItem("Factory calibration", "", [my_module]() {
                my_module->haken_midi.factory_calibration(ChemId::Core);
            }, busy));
            menu->addChild(new MenuSeparator);
            menu->addChild(createMenuItem("Surface alignment", "", [my_module]() {
                my_module->haken_midi.surface_alignment(ChemId::Core);
            }, busy));
        }));
    }

    menu->addChild(createSubmenuItem("Haken Requests", "", [=](Menu* menu) {
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Editor Hello", "", [my_module]() {
            my_module->haken_midi.editor_present(ChemId::Core);
        }, busy));
        menu->addChild(createMenuItem("ConText", "", [my_module]() {
            my_module->haken_midi.request_con_text(ChemId::Core);
        }, busy));
        menu->addChild(createMenuItem("Updates", "", [my_module]() {
            my_module->haken_midi.request_updates(ChemId::Core);
        }));
        menu->addChild(createMenuItem("Configuration", "", [my_module]() {
            my_module->haken_midi.request_configuration(ChemId::Core);
        }, busy));
        menu->addChild(createMenuItem("User presets", "", [my_module]() {
            my_module->haken_midi.request_user(ChemId::Core);
        }, busy));
        menu->addChild(createMenuItem("System presets", "", [my_module]() {
            my_module->haken_midi.request_system(ChemId::Core);
        }, busy));
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Remake Mahling data", "", [my_module]() {
            my_module->haken_midi.remake_mahling(ChemId::Core);
        }, busy));

    }));

}