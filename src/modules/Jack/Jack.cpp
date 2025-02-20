#include "Jack.hpp"
using namespace pachde;

JackModule::JackModule()
:   chem_host(nullptr),
    ui(nullptr),
    glow_knobs(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    // What happened to recirculator assignments that were in 10.09?
    // Todo: adapt HC-1 PedalParam
    std::vector<std::string> labels = {
        "(free)",
        "Pre-level",
        "Post-level",
        "Audio in",
        "Macro i",
        "Macro ii",
        "Macro ii",
        "Macro iv",
        "Macro v",
        "Macro vi",
        "Sustain",
        "Sos 1",
        "Sos 2",
        "Oct switch",
        "Mono switch",
        "Round rate",
        "Round initial",
        "Oct stretch",
        "Fine tune",
        "Advance"
    };
    configSwitch(Params::P_ASSIGN_JACK_1, 0.f, 20.f, 10.f, "Jack 1 assign", labels);
    configSwitch(Params::P_ASSIGN_JACK_2, 0.f, 20.f, 12.f, "Jack 2 assign", labels);

    auto pq = configParam(Params::P_MIN_JACK_1, 0.f, 10.f, 0.f, "Jack 1 minimum");
    pq->displayPrecision = 2;
    pq = configParam(Params::P_MAX_JACK_1, 0.f, 10.f, 10.f, "Jack 1 maximum");
    pq->displayPrecision = 2;

    pq = configParam(Params::P_MIN_JACK_2, 0.f, 10.f, 0.f, "Jack 1 minimum");
    pq->displayPrecision = 2;
    pq = configParam(Params::P_MAX_JACK_2, 0.f, 10.f, 10.f, "Jack 1 maximum");
    pq->displayPrecision = 2;

    configOutput(Outputs::OUT_JACK_1, "Jack 1");
    configOutput(Outputs::OUT_JACK_2, "Jack 2");
}

void JackModule::dataFromJson(json_t* root)
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

json_t* JackModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    return root;
}

// IChemClient
::rack::engine::Module* JackModule::client_module() { return this; }
std::string JackModule::client_claim() { return device_claim; }

void JackModule::onConnectHost(IChemHost* host)
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

void JackModule::onPresetChange()
{
    if (ui) ui->onPresetChange();
}

void JackModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (ui) ui->onConnectionChange(device, connection);
}

void JackModule::process(const ProcessArgs& args)
{

}

Model *modelJack = createModel<JackModule, JackUi>("chem-jack");

