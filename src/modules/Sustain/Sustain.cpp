#include "Sustain.hpp"
#include "../../services/rack-help.hpp"
#include "../../em/wrap-HakenMidi.hpp"
using namespace pachde;

SusModule::SusModule(ChemId id, uint8_t cc) :
    chem_id(id),
    my_cc(cc),
    modulation(this, id)
{
}

void SusModule::init()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    dp4(configParam(P_VALUE, 0.f, 10.f, 0.f, this->param_name()));
    dp4(configParam(P_MOD_AMOUNT, -100.f, 100.f, 0.f, "Modulation amount", "%"));
    configButton(P_MIN, this->min_param_name());
    configButton(P_MAX, this->max_param_name());
    configInput(IN_MOD, this->param_name());
    EmccPortConfig cfg[] = {
        EmccPortConfig::cc(Params::P_VALUE, Inputs::IN_MOD, -1, Haken::ch1, my_cc, true)
    };
    modulation.configure(Params::P_MOD_AMOUNT, 1, cfg);
}

void SusModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    json_read_string(root, "haken-device", device_claim);
    json_read_bool(root, "glow-knobs", glow_knobs);
    getParam(P_MOD_AMOUNT).setValue(get_json_float(root, "mod-amount", 0.f));
    ModuleBroker::get()->try_bind_client(this);
}

json_t* SusModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    json_object_set_new(root, "mod-amount", json_real(getParam(P_MOD_AMOUNT).getValue()));
    return root;
}

void SusModule::do_message(PackedMidiMessage message)
{
    if (Haken::ccStat1 != message.bytes.status_byte) return;
    if (as_u8(chem_id) == midi_tag(message)) return;
    if (my_cc == midi_cc(message)) {
        modulation.set_em_and_param_low(P_VALUE, midi_cc_value(message), true);
    }
}

// IChemClient
::rack::engine::Module* SusModule::client_module() { return this; }
std::string SusModule::client_claim() { return device_claim; }

void SusModule::onConnectHost(IChemHost* host)
{
    onConnectHostModuleImpl(this, host);
}

void SusModule::onPresetChange()
{
    if (!connected()) return;
    update_from_em();
}

void SusModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

bool SusModule::connected()
{
    if (!chem_host) return false;
    auto conn = chem_host->host_connection(ChemDevice::Haken);
    return conn && conn->identified();
}

void SusModule::onPortChange(const PortChangeEvent &e)
{
    getParam(P_MOD_AMOUNT).setValue(0.f);
    modulation.pull_mod_amount();
}

void SusModule::update_from_em()
{
    if (!connected()) return;
    modulation.set_em_and_param_low(P_VALUE, from_em(), true);
}

void SusModule::process_params(const ProcessArgs &args)
{
    if (getParamInt(getParam(P_MAX))) {
        getParam(P_VALUE).setValue(unipolar_7_to_rack(127));
        chem_host->host_haken()->control_change(chem_id, Haken::ch1, my_cc, 127);
        getParam(P_MAX).setValue(0);
    } else if (getParamInt(getParam(P_MIN))) {
        getParam(P_VALUE).setValue(0);
        chem_host->host_haken()->control_change(chem_id, Haken::ch1, my_cc, 0);
        getParam(P_MIN).setValue(0);
    }
    modulation.mod_target = IN_MOD;
    modulation.pull_mod_amount();
}

void SusModule::process(const ProcessArgs& args)
{
    ChemModule::process(args);

    if (!host_connected(chem_host) || chem_host->host_busy()) return;

    if (0 == ((args.frame + id) % 45)) {
        process_params(args);
    }
    if (modulation.sync_params_ready(args)) {
        modulation.sync_send();
    }

}

Model *modelSustain = createModel<SustainModule, SustainUi>("chem-sus");
Model *modelSostenuto = createModel<SostenutoModule, SostenutoUi>("chem-sos");
Model *modelSostenuto2 = createModel<Sostenuto2Module, Sostenuto2Ui>("chem-sos2");

