#include "Kinetic.hpp"
#include "services/rack-help.hpp"
#include "em/wrap-HakenMidi.hpp"
using namespace pachde;

KineticModule::KineticModule(ChemId id, uint8_t cc) :
    chem_id(id),
    my_cc(cc),
    modulation(this, id)
{
}

void KineticModule::init()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    EmccPortConfig cfg[] = {
    };
    modulation.configure(Params::P_MOD_AMOUNT, 1, cfg);
}

void KineticModule::dataFromJson(json_t* root)
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
    // j = json_object_get(root, "mod-amount");
    // if (j) {
    //     getParam(P_MOD_AMOUNT).setValue(json_real_value(j));
    // }
    if (!device_claim.empty()) {
        ModuleBroker::get()->try_bind_client(this);
    }
}

json_t* KineticModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    //json_object_set_new(root, "mod-amount", json_real(getParam(P_MOD_AMOUNT).getValue()));
    return root;
}

void KineticModule::do_message(PackedMidiMessage message)
{
    if (Haken::ccStat1 != message.bytes.status_byte) return;
    if (as_u8(chem_id) == midi_tag(message)) return;
    // if (my_cc == midi_cc(message)) {
    //     modulation.set_em_and_param_low(P_VALUE, midi_cc_value(message), true);
    // }
}

// IChemClient
::rack::engine::Module* KineticModule::client_module() { return this; }
std::string KineticModule::client_claim() { return device_claim; }

void KineticModule::onConnectHost(IChemHost* host)
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

void KineticModule::onPresetChange()
{
    if (!connected()) return;
    update_from_em();
}

void KineticModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

bool KineticModule::connected()
{
    if (!chem_host) return false;
    auto conn = chem_host->host_connection(ChemDevice::Haken);
    return conn && conn->identified();
}

void KineticModule::onPortChange(const PortChangeEvent &e)
{
    getParam(P_MOD_AMOUNT).setValue(0.f);
    modulation.pull_mod_amount();
}

void KineticModule::update_from_em()
{
    if (!connected()) return;
    //modulation.set_em_and_param_low(P_VALUE, from_em(), true);
}

void KineticModule::process_params(const ProcessArgs &args)
{
    // if (getParamInt(getParam(P_MAX))) {
    //     getParam(P_VALUE).setValue(unipolar_7_to_rack(127));
    //     chem_host->host_haken()->control_change(chem_id, Haken::ch1, my_cc, 127);
    //     getParam(P_MAX).setValue(0);
    // } else if (getParamInt(getParam(P_MIN))) {
    //     getParam(P_VALUE).setValue(0);
    //     chem_host->host_haken()->control_change(chem_id, Haken::ch1, my_cc, 0);
    //     getParam(P_MIN).setValue(0);
    // }
    // modulation.mod_target = IN_MOD;
    modulation.pull_mod_amount();
}

void KineticModule::process(const ProcessArgs& args)
{
    check_bind_host_ready(this, args);

    if (!chem_host || chem_host->host_busy()) return;

    if (0 == ((args.frame + id) % 45)) {
        process_params(args);
    }
    if (modulation.sync_params_ready(args)) {
        modulation.sync_send();
    }

}

Model *modelKineticA = createModel<KineticAModule, KineticAUi>("chem-kinetic-a");
Model *modelKineticB = createModel<KineticBModule, KineticBUi>("chem-kinetic-b");

