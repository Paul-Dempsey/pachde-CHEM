#pragma once
#include "chem.hpp"
#include "chem-core.hpp"
#include "chem-task.hpp"
#include "em/EaganMatrix.hpp"
#include "em/preset-list.hpp"
#include "preset-enum.hpp"
#include "preset-file-info.hpp"
#include "relay-midi.hpp"
#include "services/em-midi-port.hpp"
#include "services/HakenMidiOutput.hpp"
#include "services/midi-devices.hpp"
#include "services/midi-io.hpp"
#include "services/svg-query.hpp"
#include "widgets/widgets.hpp"
#include "wxyz.hpp"

using namespace pachde;
using namespace eaganmatrix;

struct CoreModuleWidget;
struct CoreModule;
struct CoreMenu : Hamburger
{
    using Base = Hamburger;

    CoreModuleWidget* ui{nullptr};
    void setUi(CoreModuleWidget* w) { ui = w; }
    void appendContextMenu(ui::Menu* menu) override;
    void onHoverKey(const HoverKeyEvent& e) override;
};

enum GatherFlags : uint8_t {
    None    = 0,
    System  = 1,
    User    = (1 << 1),
    Quick   = (1 << 2),
    Full    = (1 << 3),
    Ids     = (1 << 4),
    Presets = (1 << 5)
};
constexpr const GatherFlags QuickUserPresets{static_cast<GatherFlags>(GatherFlags::Quick + GatherFlags::User + GatherFlags::Presets)};
constexpr const GatherFlags QuickSystemPresets{static_cast<GatherFlags>(GatherFlags::Quick + GatherFlags::System + GatherFlags::Presets)};
constexpr const GatherFlags FullUserPresets{static_cast<GatherFlags>(GatherFlags::Full + GatherFlags::User + GatherFlags::Presets)};
constexpr const GatherFlags FullSystemPresets{static_cast<GatherFlags>(GatherFlags::Full + GatherFlags::System + GatherFlags::Presets)};

inline bool gather_quick   (GatherFlags g) { return 0 != (g & GatherFlags::Quick); }
inline bool gather_full    (GatherFlags g) { return 0 != (g & GatherFlags::Full); }
inline bool gather_system  (GatherFlags g) { return 0 != (g & GatherFlags::System); }
inline bool gather_user    (GatherFlags g) { return 0 != (g & GatherFlags::User); }
inline bool gather_ids     (GatherFlags g) { return 0 != (g & GatherFlags::Ids); }
inline bool gather_presets (GatherFlags g) { return 0 != (g & GatherFlags::Presets); }
inline bool gather_valid(GatherFlags g)
{
    if (GatherFlags::None == g) return true;
    if ((GatherFlags::Quick + GatherFlags::Full) == (g & (GatherFlags::Quick + GatherFlags::Full))) return false;
    if ((GatherFlags::User + GatherFlags::System) ==  (g & (GatherFlags::User + GatherFlags::System))) return false;
    if ((GatherFlags::Ids + GatherFlags::Presets) ==  (g & (GatherFlags::Ids + GatherFlags::Presets))) return false;
    return true;
}

struct CoreModule : ChemModule, IChemHost, IMidiDeviceNotify, IHandleEmEvents, IDoMidi, IPresetList
{
    Modulation modulation;

    MidiDeviceHolder haken_device;
    MidiDeviceHolder controller1;
    MidiDeviceHolder controller2;

    MidiInput haken_midi_in { ChemId::Haken };
    MidiInput controller1_midi_in { ChemId::Midi1 };
    MidiInput controller2_midi_in { ChemId::Midi2 };

    HakenMidiOutput haken_midi_out;
    HakenMidi haken_midi;
    RelayMidi midi_relay;
    MidiLog* midi_log{nullptr};

    EaganMatrix em;
    bool disconnected{false};
    bool is_busy{false};
    bool in_reboot{false};
    bool in_preset_request{false};
    // ui options
    bool glow_knobs{false};

    // Music (Note) processing
    //int music_outs{0}; // count of connected wxyz outputs
    //bool wxyz_connected() { return 0 != music_outs; }
    MusicMidiToCV mm_to_cv;

    std::shared_ptr<PresetList> user_presets{nullptr};
    std::shared_ptr<PresetList> system_presets{nullptr};
    std::vector<IPresetListClient*> preset_list_clients;
    GatherFlags gathering{GatherFlags::None};
    PresetIdListBuilder* id_builder{nullptr};
    PresetListBuildCoordinator* full_build{nullptr};
    bool stop_scan{false};

    std::vector<std::shared_ptr<PresetFileInfo>> user_preset_file_infos;
    std::string stash_user_preset_file; // for repopulating custom user preset file

    WallTimer ticker;
    RecurringChemTasks recurring_tasks;
    ChemStartupTasks startup_tasks;
    ChemTask::State start_states[4]{ChemTask::State::Untried};

    std::vector<IChemClient*> chem_clients;

    OctaveShiftLeds octave;
    RoundingLeds round_leds;

    // ------------------------------------------
    CoreModule();
    virtual ~CoreModule();

    CoreModuleWidget* ui() { return reinterpret_cast<CoreModuleWidget*>(chem_ui); };

    bool is_logging() { return nullptr != midi_log; }
    void enable_logging(bool enable);
    void log_message(const char *prefix, const char *info) {
        if (midi_log) midi_log->log_message(prefix, info);
    }
    void log_message(const char *prefix, const std::string& info) {
        if (midi_log) midi_log->log_message(prefix, info);
    }
    #define LOG_MSG(prefix, ...) if (is_logging()) midi_log->log_message((prefix), __VA_ARGS__)

    std::string device_name(ChemDevice which);

    bool is_haken_connected() { return (haken_midi_out.output.deviceId >= 0) && (nullptr != haken_device.connection); }
    bool is_controller_1_connected() { return (controller1_midi_in.deviceId >= 0) && (nullptr != controller1.connection); }
    bool is_controller_2_connected() { return (controller2_midi_in.deviceId >= 0) && (nullptr != controller2.connection); }

    void reboot();
    void update_from_em();
    void connect_midi(bool on_off);
    void init_osmose();
    void reset_tasks();
    PresetId prev_next_id(ssize_t increment);
    void next_preset();
    void prev_preset();
    void clear_presets(eaganmatrix::PresetTab which);
    PresetResult load_preset_file(eaganmatrix::PresetTab which, bool busy_load = false);
    PresetResult load_quick_user_presets();
    PresetResult load_quick_system_presets();
    PresetResult load_full_system_presets();
    PresetResult load_full_user_presets();
    PresetResult scan_osmose_presets(uint8_t page);
    void notify_preset_list_changed(eaganmatrix::PresetTab which);
    void update_user_preset_file_infos();

    // IPresetList
    void register_preset_list_client(IPresetListClient* client) override;
    void unregister_preset_list_client(IPresetListClient* client) override;
    std::shared_ptr<PresetList> host_user_presets() override;
    std::shared_ptr<PresetList> host_system_presets() override;

    PresetResult end_scan();
    void load_lists();

    // IDoMidi
    void do_message(PackedMidiMessage message) override;

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
        if (disconnected) return nullptr;
        if (em.in_preset) return nullptr;
        if (em.preset.valid()) return &em.preset;
        return nullptr;
    }
    HakenMidi* host_haken() override {
        if (disconnected) return nullptr;
        return &haken_midi;
    }
    EaganMatrix* host_matrix() override {
        if (disconnected) return nullptr;
        return &em;
    }
    bool host_busy() override {
        return is_busy
            || disconnected
            || in_preset_request
            || in_reboot
            || !em.ready
            || em.busy()
            || !startup_tasks.completed()
            || gathering
            ;
    }
    IPresetList* host_ipreset_list() override { return this; }
    void request_preset(ChemId tag, PresetId id) override;

    void notify_connection_changed(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection);
    void notify_preset_changed();

    // IHandleEmEvents
    void onEditorReply(uint8_t reply) override;
    void onTaskMessage(uint8_t code) override;
    void onHardwareChanged(uint8_t hardware, uint16_t firmware_version) override;
    void onPresetBegin() override;
    void onPresetChanged() override;
    void onUserBegin() override;
    void onUserComplete() override;
    void onSystemBegin() override;
    void onSystemComplete() override;
    void onMahlingBegin() override;
    void onMahlingComplete() override;

    // ----  Rack  ------------------------------------------

    enum Params {
        P_C1_MUSIC_FILTER,
        P_C2_MUSIC_FILTER,
        P_C1_MUTE,
        P_C2_MUTE,
        P_NOTHING,
        P_C1_CHANNEL_MAP,
        P_C2_CHANNEL_MAP,
        P_ATTENUATION,
        NUM_PARAMS
    };
    enum Inputs {
        IN_C1_MUTE_GATE,
        IN_C2_MUTE_GATE,
        NUM_INPUTS
    };
    enum Outputs {
        OUT_READY,
        OUT_W,
        OUT_X,
        OUT_Y,
        OUT_Z,
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
        L_C1_CHANNEL_MAP,
        L_C2_CHANNEL_MAP,
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
    // virtual void onSetMaster(const SetMasterEvent& e) override;
    // void onUnsetMaster(const UnsetMasterEvent& e) override;
    void onRandomize(const RandomizeEvent& e) override;
    //virtual void onSave(const SaveEvent& e) override;

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;

    void onRandomize() override {}
    void onPortChange(const PortChangeEvent& e) override;
    void process_params(const ProcessArgs &args);
    void processLights(const ProcessArgs &args);
    void process_gather(const ProcessArgs &args);
    void process(const ProcessArgs &args) override;
};

enum ThemeColor {
    coHakenMidiIn,
    coHakenMidiOut,
    coC1MidiIn,
    coC2MidiIn,
    coTask0,
    coTaskPending,
    coTaskComplete,
    coTaskWaiting,
    coTaskBroken,
    coWeird,
    THEME_COLOR_SIZE
};

struct CoreModuleWidget : ChemModuleWidget, IChemClient, IHandleEmEvents
{
    using Base = ChemModuleWidget;

    CoreModuleWidget(CoreModule *module);
    virtual ~CoreModuleWidget();

    CoreModule* my_module = nullptr;
    bool spinning{false};

    MidiPicker* haken_picker{nullptr};
    MidiPicker* controller1_picker{nullptr};
    MidiPicker* controller2_picker{nullptr};
    TextLabel* haken_device_label{nullptr};
    TextLabel* controller1_device_label{nullptr};
    TextLabel* controller2_device_label{nullptr};
    TipLabel* preset_label{nullptr};
    TextLabel* firmware_label{nullptr};
    TextLabel* em_status_label{nullptr};
    TextButton* stop_button{nullptr};
    BlueKnob* attenuation_knob{nullptr};
    Blip* em_led{nullptr};;
    IndicatorWidget* mididevice_indicator{nullptr};
    IndicatorWidget* heartbeat_indicator{nullptr};
    IndicatorWidget* em_init_indicator{nullptr};
    IndicatorWidget* presetinfo_indicator{nullptr};
    IndicatorWidget* user_presets_indicator{nullptr};
    IndicatorWidget* system_presets_indicator{nullptr};

    IndicatorWidget* widget_for_task(ChemTaskId task);

    NVGcolor theme_colors[ThemeColor::THEME_COLOR_SIZE];
    const NVGcolor& themeColor(ThemeColor index);
    const NVGcolor& taskStateColor(ChemTask::State state);
    void set_theme_colors(const std::string& theme = "");

    MidiPicker* createMidiPicker(Vec pos, const char *tip, MidiDeviceHolder* device, MidiDeviceHolder* haken_device);

    void createMidiPickers(::svg_query::BoundsIndex& bounds);
    void createRoundingLeds(Vec pos, float spread);
    void create_stop_button();
    void remove_stop_button();
    void createIndicatorsCentered(Vec pos, float spread);
    void updateIndicators();
    void connect_midi(bool on);
    void open_user_preset_file();
    void save_as_user_preset_file();

    void show_busy(bool busy);
    bool showing_busy() { return spinning; }
    void glowing_knobs(bool glow);

    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-core.svg"); }
    void createScrews() override;

    // IChemClient
    IChemHost* chem_host;
    ::rack::engine::Module* client_module() override;
    std::string client_claim() override;
    void onConnectHost(IChemHost* host) override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    // IHandleEmEvents
    void onHardwareChanged(uint8_t hardware, uint16_t firmware_version) override;
    void onPresetChanged() override;
    void onTaskMessage(uint8_t code) override;
    void onLED(uint8_t led) override;

    void hot_reload() override;
    void setThemeName(const std::string& name, void *context) override;

    void drawMidiAnimation(const DrawArgs& args, bool halo);
    void drawLayer(const DrawArgs& args, int layer) override;
    void draw(const DrawArgs& args) override;
    void step() override;
    //void appendContextMenu(Menu *menu) override;
};


inline const NVGcolor &CoreModuleWidget::themeColor(ThemeColor which)
{
    assert(which < ThemeColor::THEME_COLOR_SIZE);
    return theme_colors[static_cast<size_t>(which)];
}


