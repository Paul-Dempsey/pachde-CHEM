#include "Sostenuto2.hpp"
#include "../../services/rack-help.hpp"
#include "../../em/wrap-HakenMidi.hpp"
using namespace pachde;

Sostenuto2Module::Sostenuto2Module()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    dp4(configParam(P_SOSTENUTO, 0.f, 10.f, 0.f, "Sostenuto2"));
    configButton(P_MAX_SOS, "Max sostenuto");
    configButton(P_MIN_SOS, "No sostenuto");
}

void Sostenuto2Module::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    json_t* j = json_object_get(root, "haken-device");
    if (j) {
        device_claim = json_string_value(j);
    }
    ModuleBroker::get()->try_bind_client(this);
}

json_t* Sostenuto2Module::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    return root;
}

void Sostenuto2Module::doMessage(PackedMidiMessage message)
{
    if (Haken::ccStat1 != message.bytes.status_byte) return;
    if (as_u8(ChemId::Sostenuto2) == midi_tag(message)) return;

    switch (midi_cc(message)) {
    case Haken::ccSos2: {
        getParam(P_SOSTENUTO).setValue(unipolar_7_to_rack(message.bytes.data2));
        return;
    }

    default:
        return;
    }
}

// IChemClient
::rack::engine::Module* Sostenuto2Module::client_module() { return this; }
std::string Sostenuto2Module::client_claim() { return device_claim; }

void Sostenuto2Module::onConnectHost(IChemHost* host)
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

void Sostenuto2Module::onPresetChange()
{
    if (!connected()) return;
    update_from_em();
}

void Sostenuto2Module::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

bool Sostenuto2Module::connected()
{
    if (!chem_host) return false;
    auto conn = chem_host->host_connection(ChemDevice::Haken);
    return conn && conn->identified();
}

void Sostenuto2Module::update_from_em()
{
    if (!connected()) return;
    auto em = chem_host->host_matrix();
    auto val = unipolar_7_to_rack(em->get_sos2());
    getParam(P_SOSTENUTO).setValue(unipolar_7_to_rack(val));
}

void Sostenuto2Module::process_params(const ProcessArgs &args)
{
    if (getParamInt(getParam(P_MAX_SOS))) {
        getParam(P_SOSTENUTO).setValue(unipolar_7_to_rack(127));
        chem_host->host_haken()->control_change(ChemId::Sostenuto2, Haken::ch1, Haken::ccSos2, 127);
        getParam(P_MAX_SOS).setValue(0);
    } else if (getParamInt(getParam(P_MIN_SOS))) {
        getParam(P_SOSTENUTO).setValue(0);
        chem_host->host_haken()->control_change(ChemId::Sostenuto2, Haken::ch1, Haken::ccSos2, 0);
        getParam(P_MIN_SOS).setValue(0);
    } else {
        auto p = unipolar_rack_to_unipolar_7(getParam(P_SOSTENUTO).getValue());
        auto em = chem_host->host_matrix()->get_sos2();
        if (p != em) {
            chem_host->host_haken()->control_change(ChemId::Sostenuto2, Haken::ch1, Haken::ccSos2, p);
        }
    }
}

void Sostenuto2Module::process(const ProcessArgs& args)
{
    find_and_bind_host(this, args);

    if (!chem_host || chem_host->host_busy()) return;

    if (0 == ((args.frame + id) % 45)) {
        process_params(args);
    }
}

Model *modelSostenuto2 = createModel<Sostenuto2Module, Sostenuto2Ui>("chem-sos2");

