#include "Pre.hpp"
using namespace pachde;

PreModule::PreModule()
:   chem_host(nullptr),
    ui(nullptr),
    glow_knobs(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configInput(IN_PRE_LEVEL, "Pre-level");
    configInput(IN_MIX,       "Mix");
    configInput(IN_THRESHOLD, "Threshold");
    configInput(IN_ATTACK,    "Attack");
    configInput(IN_RATIO,     "Ratio");
    
    configParam(P_PRE_LEVEL,       0.f, 10.f,  5.f, "Pre-level")->displayPrecision = 4;
    configParam(P_MIX,             0.f, 10.f,  0.f, "Mix")->displayPrecision = 4;
    configParam(P_THRESHOLD_DRIVE, 0.f, 10.f, 10.f, "Threshold")->displayPrecision = 4;
    configParam(P_ATTACK_,         0.f, 10.f,  5.f, "Attack")->displayPrecision = 4;
    configParam(P_RATIO_MAKEUP,    0.f, 10.f,  5.f, "Ratio")->displayPrecision = 4;
    configParam(P_ATTENUVERT,   -100.f, 100.f, 0.f, "Input attenuverter", "%")->displayPrecision = 4;

    configSwitch(P_SELECT, 0.f, 1.f, 0.f, "Select Compressor or Tanh", { "Compressor", "Tanh"});
}

void PreModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    json_t* j = json_object_get(root, "haken-device");
    if (j) {
        device_claim = json_string_value(j);
    }
    j = json_object_get(root, "glow-knobs");
    if (j) {
        glow_knobs = json_boolean_value(j);
    }
    ModuleBroker::get()->try_bind_client(this);
}

json_t* PreModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    return root;
}

// IChemClient
::rack::engine::Module* PreModule::client_module() { return this; }
std::string PreModule::client_claim() { return device_claim; }

void PreModule::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (!host) {
        device_claim = "";
        if (ui) ui->onConnectHost(nullptr);
        return;
    }
    auto conn = chem_host->host_connection(ChemDevice::Haken);
    if (conn) {
        device_claim = conn->info.claim();
    }
    if (ui) ui->onConnectHost(host);
}

void PreModule::onPresetChange()
{
    if (ui) ui->onPresetChange();
}

void PreModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (ui) ui->onConnectionChange(device, connection);
}

void PreModule::process(const ProcessArgs& args)
{
}

Model *modelPre = createModel<PreModule, PreUi>("chem-pre");

