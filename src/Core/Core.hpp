#include "../chem.hpp"
#include "../services/em_device.hpp"
#include "../widgets/MidiPicker.hpp"

using namespace pachde;

struct MidiDeviceHolder : IMidiDeviceHolder
{
    std::shared_ptr<pachde::MidiDeviceConnection> connection = nullptr;
    std::string device_claim;
    Module * module = nullptr;

    MidiDeviceHolder(Module * parent) {
        module = parent;
    }

    // IMidiDeviceHolder
    void setMidiDeviceClaim(const std::string& claim) override
    {
        if (0 == device_claim.compare(claim)) return;
        if (!device_claim.empty()) {
            if (module) {
                MidiDeviceBroker::get()->revoke_claim(module->getId());
            }
        }
        device_claim = claim;
    }
    const std::string& getMidiDeviceClaim() override  { return device_claim; }
};

struct CoreModule : ChemModule
{
    bool ready = false;
    MidiDeviceHolder haken_midi;
    MidiDeviceHolder controller1;
    MidiDeviceHolder controller2;

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
        NUM_LIGHTS
    };

    CoreModule();
    void processLights(const ProcessArgs &args);
    void process(const ProcessArgs &args) override;
};

struct CoreModuleWidget : ChemModuleWidget
{
    CoreModule* my_module = nullptr;
    MidiPicker* haken_picker = nullptr;
    MidiPicker* controller1_picker = nullptr;
    MidiPicker* controller2_picker = nullptr;
    
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/CHEM-core.svg"); }
    MidiPicker* createMidiPicker(float x, float y, bool isEM, const char *tip, MidiDeviceHolder* device);

    CoreModuleWidget(CoreModule *module);

    void draw(const DrawArgs& args) override;
    void step() override;
    void appendContextMenu(Menu *menu) override;
};

