#include "Macro.hpp"
using namespace pachde;

MacroModule::MacroModule()
:   chem_host(nullptr),
    ui(nullptr),
    glow_knobs(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    configInput(IN_M1, "Macro 1 (i)");
    configInput(IN_M2, "Macro 2 (ii)");
    configInput(IN_M3, "Macro 3 (iii)");
    configInput(IN_M4, "Macro 4 (iv)");
    configInput(IN_M5, "Macro 5 (v)");
    configInput(IN_M6, "Macro 6 (vi)");

    configParam(P_M1, 0.f, Haken::max14, 0.f, "Macro 1 (i)");
    configParam(P_M2, 0.f, Haken::max14, 0.f, "Macro 2 (ii)");
    configParam(P_M3, 0.f, Haken::max14, 0.f, "Macro 3 (iii)");
    configParam(P_M4, 0.f, Haken::max14, 0.f, "Macro 4 (iv)");
    configParam(P_M5, 0.f, Haken::max14, 0.f, "Macro 5 (v)");
    configParam(P_M6, 0.f, Haken::max14, 0.f, "Macro 6 (vi)");
}

void MacroModule::dataFromJson(json_t* root)
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

json_t* MacroModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    return root;
}

// IChemClient
::rack::engine::Module* MacroModule::client_module() { return this; }
std::string MacroModule::client_claim() { return device_claim; }

void MacroModule::onConnectHost(IChemHost* host)
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

void MacroModule::onPresetChange()
{
    if (ui) ui->onPresetChange();
}

void MacroModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (ui) ui->onConnectionChange(device, connection);
}

void MacroModule::process(const ProcessArgs& args)
{
    if (!chem_host && !device_claim.empty()) {
        if (poll_host.process(args.sampleTime) > 2.f) {
            auto broker = ModuleBroker::get();
            broker->try_bind_client(this);
        }
    }
}

Model *modelMacro = createModel<MacroModule, MacroUi>("chem-macro");

