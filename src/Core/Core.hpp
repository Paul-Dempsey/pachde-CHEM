#include "../chem.hpp"
#include "../chem-core.hpp"
#include "../em/EaganMatrix.hpp"
#include "../services/haken-midi.hpp"
#include "../services/midi-devices.hpp"
#include "../services/midi-io.hpp"
#include "../widgets/MidiPicker.hpp"
#include "../widgets/label-widget.hpp"
#include "../widgets/indicator-widget.hpp"
#include "../widgets/blip-widget.hpp"
#include "haken-task.hpp"
//#include "midi-input-worker.hpp"

using namespace pachde;

struct CoreModuleWidget;
struct CoreModule;

struct RelayMidiToEM : IDoMidi
{
    CoreModule* core{nullptr};
    void doMessage(PackedMidiMessage message) override;
};

struct CoreModule : ChemModule, IChemHost, IMidiDeviceNotify, IHandleEmEvents, IHakenTaskEvents
{
    CoreModuleWidget* ui = nullptr;

    MidiDeviceHolder haken_midi;
    MidiDeviceHolder controller1;
    MidiDeviceHolder controller2;

    MidiInput haken_midi_in;
    MidiInput controller1_midi_in;
    MidiInput controller2_midi_in;

    HakenMidiOutput haken_midi_out;

    //MidiInputWorker midi_input_queue;

    RelayMidiToEM to_em;
    MidiLog midilog;
    
    EaganMatrix em;
    bool log_midi;
    bool in_reboot;
    bool heartbeat;
    uint64_t loop;

    std::vector<IChemClient*> chem_clients;
    HakenTasks tasks;

    // ------------------------------------------
    CoreModule();
    virtual ~CoreModule();

    void enable_logging(bool enable);
    void logMessage(const char *prefix, const char *info) {
        midilog.logMessage(prefix, info);
    }

    ChemDevice DeviceIdentifier(const MidiDeviceHolder* holder)
    {
        if (holder == &haken_midi) return ChemDevice::Haken;
        if (holder == &controller1) return ChemDevice::Midi1;
        if (holder == &controller2) return ChemDevice::Midi2;
        return ChemDevice::Unknown;
    }

    bool isHakenConnected() { return (haken_midi_out.output.deviceId != -1) && (nullptr != haken_midi.connection); }
    bool isController1Connected() { return (-1 != controller1_midi_in.deviceId) && (nullptr != controller1.connection); }
    bool isController2Connected() { return (-1 != controller2_midi_in.deviceId) && (nullptr != controller2.connection); }

    void reboot();
    void send_midi_rate(HakenMidiRate rate);
    void restore_midi_rate();

    // IMidiDeviceNotify
    void onMidiDeviceChange(const MidiDeviceHolder* source) override;

    // IChemHost
    void register_chem_client(IChemClient* client) override;
    void unregister_chem_client(IChemClient* client) override;
    bool host_has_client_model(IChemClient* client) override;
    bool host_has_client(IChemClient* client) override;
    //bool host_ready() override { return this->em.ready; }
    std::shared_ptr<MidiDeviceConnection> host_connection(ChemDevice device) override;
    std::string host_claim() override {
        return haken_midi.getClaim();
    }
    const PresetDescription* host_preset() override {
        return &em.preset;
    }
    void notify_connection_changed(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection);
    void notify_preset_changed();
    
    // IHandleEmEvents
    void onLoopDetect(uint8_t cc, uint8_t value) override;
    void onEditorReply(uint8_t reply) override;
    void onHardwareChanged(uint8_t hardware, uint16_t firmware_version) override;
    void onPresetChanged() override;
    void onUserComplete() override;
    void onSystemComplete() override;
    void onTaskMessage(uint8_t code) override;
    void onLED(uint8_t led) override;

    // IHakenTaskEvents
    void onHakenTaskChange(HakenTask task) override;

    // ----  Rack  ------------------------------------------

    enum Params {
       NUM_PARAMS
    };
    enum Inputs {
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
        L_PULSE,
        //L_LED,
        NUM_LIGHTS
    };

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;

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
    StaticTextLabel* haken_device_label = nullptr;
    StaticTextLabel* controller1_device_label = nullptr;
    StaticTextLabel* controller2_device_label = nullptr;
    StaticTextLabel* preset_label = nullptr;
    StaticTextLabel* firmware_label = nullptr;
    //StaticTextLabel* task_status_label = nullptr;
    StaticTextLabel* em_status_label = nullptr;

    Blip* blip = nullptr;
    IndicatorWidget* mididevice_indicator = nullptr;
    IndicatorWidget* heartbeat_indicator = nullptr;
    IndicatorWidget* updates_indicator = nullptr;
    IndicatorWidget* userpresets_indicator = nullptr;
    IndicatorWidget* systempresets_indicator = nullptr;
    IndicatorWidget* presetinfo_indicator = nullptr;
    IndicatorWidget* lastpreset_indicator = nullptr;
    IndicatorWidget* syncdevices_indicator = nullptr;

    IndicatorWidget* widget_for_task(HakenTask task);


    NVGcolor theme_colors[ThemeColor::THEME_COLOR_SIZE];
    const NVGcolor& themeColor(ThemeColor index);
    const NVGcolor& taskStateColor(TaskState state);
    void set_theme_colors(const std::string& theme = "");

    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/CHEM-core.svg"); }
    MidiPicker* createMidiPicker(float x, float y, bool isEM, const char *tip, MidiDeviceHolder* device, std::shared_ptr<SvgTheme> theme);

    void createScrews(std::shared_ptr<SvgTheme> theme);
    void createMidiPickers(std::shared_ptr<SvgTheme> theme);
    void createIndicatorsCentered(float x, float y, float spread);
    void createRoundingLeds(float x, float y, float spread);
    void resetIndicators();

    CoreModuleWidget(CoreModule *module);
    virtual ~CoreModuleWidget();

    // IChemClient
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
    void onUserComplete() override;
    void onSystemComplete() override;
    void onTaskMessage(uint8_t code) override;
    void onLED(uint8_t led) override;

    // IHakenTaskEvents
    void onHakenTaskChange(HakenTask task) override;

    void setThemeName(const std::string& name) override;
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
