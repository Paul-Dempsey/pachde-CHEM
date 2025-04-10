#include "../../chem.hpp"
#include "../../chem-core.hpp"
#include "../../em/EaganMatrix.hpp"
#include "../../services/HakenMidiOutput.hpp"
#include "../../services/midi-devices.hpp"
#include "../../services/midi-io.hpp"
#include "../../widgets/blip-widget.hpp"
#include "../../widgets/em-led-widget.hpp"
#include "../../widgets/indicator-widget.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/MidiPicker.hpp"
#include "../../widgets/slider-h-widget.hpp"
#include "../../widgets/spinner.hpp"
#include "../../widgets/tip-label-widget.hpp"
#include "../../widgets/uniform-style.hpp"
#include "haken-task.hpp"
#include "relay-midi.hpp"

using namespace pachde;

struct CoreModuleWidget;
struct CoreModule;

struct CoreModule : ChemModule, IChemHost, IMidiDeviceNotify, IHandleEmEvents, IHakenTaskEvents
{
    CoreModuleWidget* ui() { return reinterpret_cast<CoreModuleWidget*>(chem_ui); };

    MidiDeviceHolder haken_device;
    MidiDeviceHolder controller1;
    MidiDeviceHolder controller2;

    MidiInput haken_midi_in { ChemId::Haken };
    MidiInput controller1_midi_in { ChemId::Midi1 };
    MidiInput controller2_midi_in { ChemId::Midi2 };

    HakenMidiOutput haken_midi_out;

    HakenMidi haken_midi;
    HakenMidi& get_haken_midi() { return haken_midi; }

    RelayMidi midi_relay;
    MidiLog* midi_log;
    
    EaganMatrix em;
    bool is_busy;
    bool in_reboot;
    bool heartbeat;
    WallTimer ticker;

    std::vector<IChemClient*> chem_clients;
    HakenTasks tasks;

    PresetDescription last_preset;
    bool restore_last_preset;
    OctaveShiftLeds octave;
    RoundingLeds round_leds;
    
    // ------------------------------------------
    CoreModule();
    virtual ~CoreModule();

    bool is_logging() { return nullptr != midi_log; }
    void enable_logging(bool enable);
    void log_message(const char *prefix, const char *info) {
        if (midi_log) midi_log->log_message(prefix, info);
    }
    void log_message(const char *prefix, const std::string& info) {
        if (midi_log) midi_log->log_message(prefix, info);
    }

    std::string device_name(const MidiDeviceHolder& holder);
    std::string device_name(ChemDevice which);

    bool is_haken_connected() { return (haken_midi_out.output.deviceId >= 0) && (nullptr != haken_device.connection); }
    bool is_controller_1_connected() { return (controller1_midi_in.deviceId >= 0) && (nullptr != controller1.connection); }
    bool is_controller_2_connected() { return (controller2_midi_in.deviceId >= 0) && (nullptr != controller2.connection); }

    void reboot();
    void send_midi_rate(HakenMidiRate rate);
    void restore_midi_rate();

    // IMidiDeviceNotify
    void onMidiDeviceChange(const MidiDeviceHolder* source) override;

    IChemHost* get_host() override { return this; }

    // IChemHost
    void register_chem_client(IChemClient* client) override;
    void unregister_chem_client(IChemClient* client) override;
    bool host_has_client_model(IChemClient* client) override;
    bool host_has_client(IChemClient* client) override;
    std::shared_ptr<MidiDeviceConnection> host_connection(ChemDevice device) override;
    std::string host_claim() override {
        return haken_device.get_claim();
    }
    const PresetDescription* host_preset() override {
        return em.preset.id.valid() ? &em.preset : nullptr;
    }
    HakenMidi* host_haken() override {
        return &haken_midi;
    }
    EaganMatrix* host_matrix() override {
        return &em;
    }
    bool host_busy() override {
        return is_busy || in_reboot || !em.ready || em.busy();
    }
    void notify_connection_changed(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection);
    void notify_preset_changed();
    
    // IHandleEmEvents
    void onLoopDetect(uint8_t cc, uint8_t value) override;
    void onEditorReply(uint8_t reply) override;
    //void onHardwareChanged(uint8_t hardware, uint16_t firmware_version) override;
    void onPresetChanged() override;
    void onUserBegin() override;
    void onUserComplete() override;
    void onSystemBegin() override;
    void onSystemComplete() override;
    void onMahlingBegin() override;
    void onMahlingComplete() override;

    //void onTaskMessage(uint8_t code) override;
    //void onLED(uint8_t led) override;

    // IHakenTaskEvents
    void onHakenTaskChange(HakenTask task) override;

    // ----  Rack  ------------------------------------------

    enum Params {
        P_C1_MUSIC_FILTER,
        P_C2_MUSIC_FILTER,
        P_C1_MUTE,
        P_C2_MUTE,
        P_NOTHING,
        NUM_PARAMS
    };
    enum Inputs {
        IN_C1_MUTE_GATE,
        IN_C2_MUTE_GATE,
        NUM_INPUTS
    };
    enum Outputs {
        OUT_READY,
        NUM_OUTPUTS
    };
    enum Lights {
        L_READY,
        L_ROUND_Y,
        L_ROUND_INITIAL,
        L_ROUND,
        L_ROUND_RELEASE,
        L_C1_MUSIC_FILTER,
        L_C2_MUSIC_FILTER,
        L_C1_MUTE,
        L_C2_MUTE,
        L_OCT_SHIFT_FIRST,
        L_OCT_SHIFT_LAST = L_OCT_SHIFT_FIRST + 6,
        NUM_LIGHTS
    };

    //virtual void onAdd(const AddEvent& e) override;
    virtual void onRemove(const RemoveEvent& e) override;
    //virtual void onBypass(const BypassEvent& e) override;
    //virtual void onUnBypass(const UnBypassEvent& e) override;
    //virtual void onPortChange(const PortChangeEvent& e) override;
    //virtual void onSampleRateChange(const SampleRateChangeEvent& e) override;
    //virtual void onExpanderChange(const ExpanderChangeEvent& e) override;
    virtual void onReset(const ResetEvent& e) override;
    //virtual void onSetMaster(const SetMasterEvent& e) override;
    //virtual void onRandomize(const RandomizeEvent& e) override;
    //virtual void onSave(const SaveEvent& e) override;

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;

    void process_params(const ProcessArgs &args);
    void processLights(const ProcessArgs &args);
    void process(const ProcessArgs &args) override;
};

enum ThemeColor {
    coHakenMidiIn,
    coHakenMidiOut,
    coC1MidiIn,
    coC2MidiIn,
    coTaskUninitialized,
    coTaskUnscheduled,
    coTaskPending,
    coTaskComplete,
    coTaskDone,
    coTaskWaiting,
    coTaskNotApplicable,
    coTaskBroken,
    coWeird,
    THEME_COLOR_SIZE
};

struct CoreModuleWidget : ChemModuleWidget, IChemClient, IHandleEmEvents, IHakenTaskEvents
{
    CoreModule* my_module = nullptr;
    MidiPicker* haken_picker = nullptr;
    MidiPicker* controller1_picker = nullptr;
    MidiPicker* controller2_picker = nullptr;
    TextLabel* haken_device_label = nullptr;
    TextLabel* controller1_device_label = nullptr;
    TextLabel* controller2_device_label = nullptr;
    TipLabel* preset_label = nullptr;
    TextLabel* firmware_label = nullptr;
    //StaticTextLabel* task_status_label = nullptr;
    TextLabel* em_status_label = nullptr;

    Blip* blip = nullptr;
    IndicatorWidget* mididevice_indicator = nullptr;
    IndicatorWidget* heartbeat_indicator = nullptr;
    IndicatorWidget* updates_indicator = nullptr;
    IndicatorWidget* presetinfo_indicator = nullptr;
    IndicatorWidget* lastpreset_indicator = nullptr;
    IndicatorWidget* syncdevices_indicator = nullptr;

    IndicatorWidget* widget_for_task(HakenTask task);

    NVGcolor theme_colors[ThemeColor::THEME_COLOR_SIZE];
    const NVGcolor& themeColor(ThemeColor index);
    const NVGcolor& taskStateColor(TaskState state);
    void set_theme_colors(const std::string& theme = "");

    MidiPicker* createMidiPicker(float x, float y, const char *tip, MidiDeviceHolder* device, MidiDeviceHolder* haken_device, std::shared_ptr<SvgTheme> theme);
    
    void createMidiPickers(std::shared_ptr<SvgTheme> theme);
    void createIndicatorsCentered(float x, float y, float spread);
    void createRoundingLeds(float x, float y, float spread);
    void resetIndicators();
    
    CoreModuleWidget(CoreModule *module);
    virtual ~CoreModuleWidget();

    void show_busy(bool busy);

    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-core.svg"); }
    void createScrews(std::shared_ptr<SvgTheme> theme) override;

    // IChemClient
    IChemHost* chem_host;
    ::rack::engine::Module* client_module() override;
    std::string client_claim() override;
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    // IHandleEmEvents
    void onLoopDetect(uint8_t cc, uint8_t value) override;
    void onEditorReply(uint8_t reply) override;
    void onHardwareChanged(uint8_t hardware, uint16_t firmware_version) override;
    void onPresetChanged() override;
    void onTaskMessage(uint8_t code) override;
    void onLED(uint8_t led) override;

    // IHakenTaskEvents
    void onHakenTaskChange(HakenTask task) override;

    void setThemeName(const std::string& name, void *context) override;

    void drawMidiAnimation(const DrawArgs& args, bool halo);
    void drawLayer(const DrawArgs& args, int layer) override;
    void draw(const DrawArgs& args) override;
    void step() override;
    void appendContextMenu(Menu *menu) override;
};

inline const NVGcolor& CoreModuleWidget::themeColor(ThemeColor which) {
    assert(which < ThemeColor::THEME_COLOR_SIZE);
    return theme_colors[static_cast<size_t>(which)];
}
