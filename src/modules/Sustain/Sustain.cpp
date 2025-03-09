#include "Sustain.hpp"
#include "../../services/rack-help.hpp"
#include "../../em/wrap-HakenMidi.hpp"
using namespace pachde;

SustainModule::SustainModule()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    dp4(configParam(P_SUSTAIN, 0.f, 10.f, 0.f, "Sustain"));
    configButton(P_MAX_SUS, "Max sustain");
    configButton(P_MIN_SUS, "No sustain");
}

void SustainModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    json_t* j = json_object_get(root, "haken-device");
    if (j) {
        device_claim = json_string_value(j);
    }
    ModuleBroker::get()->try_bind_client(this);
}

json_t* SustainModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    return root;
}

void SustainModule::doMessage(PackedMidiMessage message)
{
    if (Haken::ccStat1 != message.bytes.status_byte) return;
    if (as_u8(ChemId::Sustain) == midi_tag(message)) return;

    switch (midi_cc(message)) {
    case Haken::ccSus: {
        getParam(P_SUSTAIN).setValue(unipolar_7_to_rack(message.bytes.data2));
        return;
    }

    default:
        return;
    }
}

// IChemClient
::rack::engine::Module* SustainModule::client_module() { return this; }
std::string SustainModule::client_claim() { return device_claim; }

void SustainModule::onConnectHost(IChemHost* host)
{
    chem_host = host;
    if (!host) {
        device_claim = "";
        if (chem_ui) ui()->onConnectHost(nullptr);
        return;
    }
    auto conn = chem_host->host_connection(ChemDevice::Haken);
    if (conn) {
        device_claim = conn->info.claim();
    }
    if (chem_ui) ui()->onConnectHost(host);
}

void SustainModule::onPresetChange()
{
    if (!connected()) return;
    update_from_em();
}

void SustainModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

bool SustainModule::connected()
{
    if (!chem_host) return false;
    auto conn = chem_host->host_connection(ChemDevice::Haken);
    return conn && conn->identified();
}

void SustainModule::update_from_em()
{
    if (!connected()) return;
    auto em = chem_host->host_matrix();
    auto sus = unipolar_7_to_rack(em->get_sustain());
    getParam(P_SUSTAIN).setValue(unipolar_7_to_rack(sus));
}

void SustainModule::process_params(const ProcessArgs &args)
{
    if (getParamInt(getParam(P_MAX_SUS))) {
        getParam(P_SUSTAIN).setValue(unipolar_7_to_rack(127));
        chem_host->host_haken()->control_change(ChemId::Sustain, Haken::ch1, Haken::ccSus, 127);
        getParam(P_MAX_SUS).setValue(0);
    } else if (getParamInt(getParam(P_MIN_SUS))) {
        getParam(P_SUSTAIN).setValue(0);
        chem_host->host_haken()->control_change(ChemId::Sustain, Haken::ch1, Haken::ccSus, 0);
        getParam(P_MIN_SUS).setValue(0);
    } else {
        auto p = unipolar_rack_to_unipolar_7(getParam(P_SUSTAIN).getValue());
        auto em = chem_host->host_matrix()->get_sustain();
        if (p != em) {
            chem_host->host_haken()->control_change(ChemId::Sustain, Haken::ch1, Haken::ccSus, p);
        }
    }
}

void SustainModule::process(const ProcessArgs& args)
{
    find_and_bind_host(this, args);

    if (!chem_host || chem_host->host_busy()) return;

    if (0 == ((args.frame + id) % 45)) {
        process_params(args);
    }
}

Model *modelSustain = createModel<SustainModule, SustainUi>("chem-sus");

