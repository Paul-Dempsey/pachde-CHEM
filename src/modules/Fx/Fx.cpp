#include "Fx.hpp"
using namespace pachde;

FxModule::FxModule()
:   chem_host(nullptr),
    ui(nullptr),
    glow_knobs(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configParam(P_R1, 0.f, 127.f, 0.f, "R1");
    configParam(P_R2, 0.f, 127.f, 0.f, "R2");
    configParam(P_R3, 0.f, 127.f, 0.f, "R3");
    configParam(P_R4, 0.f, 127.f, 0.f, "R4");
    configParam(P_R5, 0.f, 127.f, 0.f, "R5");
    configParam(P_R6, 0.f, 127.f, 0.f, "R6");
    configParam(P_MIX, 0.f, Haken::max14, 0.f, "Mix");
    configParam(P_ATTENUVERT, -100.f, 100.f, 0.f, "Input attenuverter", "%");

    configSwitch(P_ENABLE, 0.f, 1.f, 1.f, "Enable Fx", {"on", "off"});
    configSwitch(P_EFFECT, 0.f, 6.f, 0.f, "Effect", {"Short reverb", "Mod delay", "Swept delay", "Analog echo", "Delay with LPF", "Delay with HPF", "Long reverb"});

    configInput(IN_R1, "R1");
    configInput(IN_R2, "R2");
    configInput(IN_R3, "R3");
    configInput(IN_R4, "R4");
    configInput(IN_R5, "R5");
    configInput(IN_R6, "R6");
    configInput(IN_MIX, "Recirculator Mix");
    configInput(IN_ENABLE, "Enable Recirculator");

    configLight(L_ENABLE, "Enable Fx");
}

void FxModule::dataFromJson(json_t* root)
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

json_t* FxModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    return root;
}

// IChemClient
::rack::engine::Module* FxModule::client_module() { return this; }
std::string FxModule::client_claim() { return device_claim; }

void FxModule::onConnectHost(IChemHost* host)
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

void FxModule::onPresetChange()
{
    if (ui) ui->onPresetChange();
}

void FxModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (ui) ui->onConnectionChange(device, connection);
}

void FxModule::process(const ProcessArgs& args)
{
    if (!chem_host && !device_claim.empty()) {
        if (poll_host.process(args.sampleTime) > 2.f) {
            auto broker = ModuleBroker::get();
            broker->try_bind_client(this);
        }
    }
}

Model *modelFx = createModel<FxModule, FxUi>("chem-fx");

