#include "Post.hpp"
using namespace pachde;
#include "../../services/em-param-quantity.hpp"
#include "../../em/wrap-HakenMidi.hpp"

PostModule::PostModule()
:   chem_host(nullptr),
    ui(nullptr),
    glow_knobs(false),
    attenuator_target(IN_INVALID)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configU14ccEmParam(Haken::ch1, Haken::ccPost, this, Params::P_POST_LEVEL,  0.f, 10.f, 5.f, "Post-level");
    configU7EmParam(Haken::ch1, Haken::ccEqMix,  this, Params::P_MIX,          0.f, 10.f, 0.f, "EQ Mix");
    configU7EmParam(Haken::ch1, Haken::ccEqTilt, this, Params::P_TILT,         0.f, 10.f, 5.f, "EQ Tilt");
    configU7EmParam(Haken::ch1, Haken::ccEqFreq, this, Params::P_FREQUENCY,    0.f, 10.f, 5.f, "EQ Frequency");
    configParam(P_ATTENUVERT, -100.f, 100.f, 0.f, "Input attenuverter", "%")->displayPrecision = 4;

    configSwitch(P_MUTE, 0.f, 1.f, 0.f, "Mute", { "Voice", "Mute" });

    configInput(IN_POST_LEVEL, "Post-level");
    configInput(IN_MIX,        "EQ Mix");
    configInput(IN_TILT,       "EQ Tilt");
    configInput(IN_FREQUENCY,  "EQ Frequency");
    configInput(IN_MUTE,       "Mute trigger");

    configLight(L_MIX, "EQ active");
    configLight(L_MUTE, "Mute");
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

void PostModule::process_params(const ProcessArgs& args)
{
    auto v = getParam(Params::P_MUTE).getValue();
    v = (v > 0.5f) ? 1.f : 0.f;
    getLight(Lights::L_MUTE).setBrightnessSmooth(v, 45.f);

    v = getParam(Params::P_MIX).getValue()* .1f;
    getLight(Lights::L_MIX).setBrightnessSmooth(v, 45.f);

}

void PostModule::process(const ProcessArgs& args)
{
    if (!chem_host || chem_host->host_busy()) return;

    if (0 == ((args.frame + id) % 45)) {
        process_params(args);
    }

    if (attenuator_target != Inputs::IN_INVALID) {
        // 
    }
}

Model *modelPost = createModel<PostModule, PostUi>("chem-post");

