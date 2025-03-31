#pragma once
#include "../../plugin.hpp"
#include "../../chem.hpp"
#include "../../services/colors.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../widgets/indicator-widget.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/symbol-set.hpp"
#include "../../widgets/theme-button.hpp"
#include "../../widgets/tip-label-widget.hpp"
#include "jack-param.hpp"
#include "shift-param.hpp"

using namespace pachde;

struct JackUi;

struct JackModule : ChemModule, IChemClient
{
    std::string device_claim;
    JackUi* ui() { return reinterpret_cast<JackUi*>(chem_ui); }

    bool glow_knobs;
    bool host_connection;
    int last_assign_1;
    int last_assign_2;
    int last_keep;
    int last_shift;
    int last_action;

    JackModule();
    ~JackModule() {
        if (chem_host) {
            chem_host->unregister_chem_client(this);
        }
    }

    bool connected() { return chem_host && host_connection; }

    // IChemClient
    rack::engine::Module* client_module() override;
    std::string client_claim() override;
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;

    // ----  Rack  ------------------------------------------

    enum Params {
        P_ASSIGN_JACK_1,
        P_ASSIGN_JACK_2,

        P_MIN_JACK_1,
        P_MAX_JACK_1, 
        P_MIN_JACK_2,
        P_MAX_JACK_2, 
        P_SHIFT,
        P_SHIFT_ACTION,
        P_KEEP,
        NUM_PARAMS
    };
    enum Inputs {
        NUM_INPUTS
    };
    enum Outputs {
        OUT_JACK_1,
        OUT_JACK_2,
        NUM_OUTPUTS
    };
    enum Lights {
        NUM_LIGHTS
    };

    void dataFromJson(json_t* root) override;
    json_t* dataToJson() override;

    void pull_jack_data();
    int update_send(std::vector<PackedMidiMessage>& stream_data, int paramId, uint8_t haken_id, int last_value);
    void process_params(const ProcessArgs& args);
    void process(const ProcessArgs& args) override;
};

// -- Jack UI -----------------------------------

// TODO: adapt HC 1 PedalParam for better menu

struct JackUi : ChemModuleWidget, IChemClient
{
    using Base = ChemModuleWidget;

    IChemHost* chem_host{nullptr};
    JackModule* my_module{nullptr};

    LinkButton*   link_button{nullptr};
    IndicatorWidget* link{nullptr};

    TextLabel* assign_1_label;
    TextLabel* assign_2_label;
    SymbolProvider symbols;
    SymbolSetWidget* pedal_image_1;
    SymbolSetWidget* pedal_image_2;

    float last_1, last_2;
    int last_p1, last_p2;

    JackUi(JackModule *module);

    bool connected();

    // IChemClient
    ::rack::engine::Module* client_module() override { return my_module; }
    std::string client_claim() override { return my_module ? my_module->device_claim : ""; }
    void onConnectHost(IChemHost* host) override;
    void onPresetChange() override;
    void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) override;
    
    // ChemModuleWidget
    std::string panelFilename() override { return asset::plugin(pluginInstance, "res/panels/CHEM-jack.svg"); }
    void setThemeName(const std::string& name, void * context) override;

    void sync_labels();
    void step() override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

