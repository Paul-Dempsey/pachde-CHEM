#include "../chem.hpp"
#include "../chem-core.hpp"
#include "../services/midi_devices.hpp"
#include "../widgets/MidiPicker.hpp"
#include "../widgets/label_widget.hpp"
using namespace pachde;

struct MidiInput : midi::Input
{
    uint64_t received_count = 0;

    uint64_t received() { return received_count; }
    void onMessage(const midi::Message& message) override
    {
        ++received_count;
    }
};

struct CoreModuleWidget;

enum class MidiDevice { Unknown, Haken, Midi1, Midi2 };

struct CoreModule : ChemModule, IMidiDeviceNotify, IChemHost
{
    bool ready = false;
    CoreModuleWidget* ui = nullptr;

    MidiDeviceHolder haken_midi;
    MidiDeviceHolder controller1;
    MidiDeviceHolder controller2;

    MidiDevice MidiDeviceIdentifier(const MidiDeviceHolder* holder)
    {
        if (holder == &haken_midi) return MidiDevice::Haken;
        if (holder == &controller1) return MidiDevice::Midi1;
        if (holder == &controller2) return MidiDevice::Midi2;
        return MidiDevice::Unknown;
    }

    MidiInput haken_midi_in;
    midi::Output haken_midi_out;
    MidiInput controller1_midi_in;
    MidiInput controller2_midi_in;

    std::vector<IChemClient*> clients;

    bool pending_connection() { return !haken_midi.device_claim.empty() && nullptr == haken_midi.connection; }

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
        NUM_LIGHTS
    };

    CoreModule();
    virtual ~CoreModule();

    bool isHakenConnected() { return nullptr != haken_midi.connection; }
    bool isController1Connected() { return nullptr != controller1.connection; }
    bool isController2Connected() { return nullptr != controller2.connection; }

    void notify_connection_changed();

    // IMidiDeviceNotify
    void onMidiDeviceChange(const MidiDeviceHolder* source) override;

    // rack::Module
    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;
    void process(const ProcessArgs &args) override;

    // IChemHost
    void register_client(IChemClient* client) override;
    void unregister_client(IChemClient* client) override;
    bool host_has_client(IChemClient* client) override;
    bool host_ready() override { return this->ready; }
    std::shared_ptr<MidiDeviceConnection> host_connection() override {
        return haken_midi.connection;
    }
    const PresetDescription* host_preset() override {
        return nullptr;
    }

    // impl
    void processLights(const ProcessArgs &args);
};

struct CoreModuleWidget : ChemModuleWidget
{
    CoreModule* my_module = nullptr;
    MidiPicker* haken_picker = nullptr;
    MidiPicker* controller1_picker = nullptr;
    MidiPicker* controller2_picker = nullptr;
    StaticTextLabel* haken_device_label = nullptr;
    StaticTextLabel* controller1_device_label = nullptr;
    StaticTextLabel* controller2_device_label = nullptr;
    StaticTextLabel* preset_label = nullptr;

    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/CHEM-core.svg"); }
    MidiPicker* createMidiPicker(float x, float y, bool isEM, const char *tip, MidiDeviceHolder* device);

    CoreModuleWidget(CoreModule *module);
    virtual ~CoreModuleWidget();

    void draw(const DrawArgs& args) override;
    void step() override;
    void appendContextMenu(Menu *menu) override;
};

