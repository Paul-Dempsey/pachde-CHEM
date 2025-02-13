#include "Post.hpp"
using namespace pachde;

PostModule::PostModule()
:   chem_host(nullptr),
    ui(nullptr),
    glow_knobs(false),
    attenuator_target(IN_INVALID)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configParam(P_POST_LEVEL,   0.f, 127.f, 64.f, "Post level");
    configParam(P_MIX,          0.f, 127.f, 0.f, "EQ Mix");
    configParam(P_TILT,         0.f, 127.f, 64.f, "EQ Tilt");
    configParam(P_FREQUENCY,    0.f, 127.f, 0.f, "EQ Frequency");
    configParam(P_ATTENUATION,  0.f, 127.f, 64.f, "Input attenuator");

    configInput(IN_POST_LEVEL, "Post level");
    configInput(IN_MIX,        "EQ Mix");
    configInput(IN_TILT,       "EQ Tilt");
    configInput(IN_FREQUENCY,  "EQ Frequency");

    configLight(L_EQ, "EQ active");
}

void PostModule::onPortChange(const PortChangeEvent& e)
{
    if (e.type == Port::OUTPUT) return;
    if (e.connecting) {
        attenuator_target = e.portId;
    } else {
        attenuator_target = Inputs::IN_INVALID;
    }
}

void PostModule::dataFromJson(json_t* root)
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

json_t* PostModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    return root;
}

// IChemClient
::rack::engine::Module* PostModule::client_module() { return this; }
std::string PostModule::client_claim() { return device_claim; }

void PostModule::onConnectHost(IChemHost* host)
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

void PostModule::onPresetChange()
{
    if (ui) ui->onPresetChange();
}

void PostModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (ui) ui->onConnectionChange(device, connection);
}

void PostModule::process(const ProcessArgs& args)
{
    if (!chem_host && !device_claim.empty()) {
        if (poll_host.process(args.sampleTime) > 2.f) {
            auto broker = ModuleBroker::get();
            broker->try_bind_client(this);
        }
    }

    if (attenuator_target != Inputs::IN_INVALID) {
        // 
    }
}

Model *modelPost = createModel<PostModule, PostUi>("chem-post");

