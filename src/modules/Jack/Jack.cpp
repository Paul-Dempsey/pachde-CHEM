#include "Jack.hpp"
#include "../../services/rack-help.hpp"

using namespace pachde;

JackModule::JackModule()
:   glow_knobs(false),
    host_connection(false),
    last_assign_1(-1),
    last_assign_2(-1),
    last_keep(-1),
    last_shift(-1),
    last_action(-1)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configJackParam(this, 1, Params::P_ASSIGN_JACK_1, "Jack 1 assign");
    configJackParam(this, 2, Params::P_ASSIGN_JACK_2, "Jack 2 assign");

    dp2(configParam(Params::P_MIN_JACK_1, 0.f, 10.f, 0.f, "Jack 1 minimum"));
    dp2(configParam(Params::P_MAX_JACK_1, 0.f, 10.f, 10.f, "Jack 1 maximum"));
    dp2(configParam(Params::P_MIN_JACK_2, 0.f, 10.f, 0.f, "Jack 1 minimum"));
    dp2(configParam(Params::P_MAX_JACK_2, 0.f, 10.f, 10.f, "Jack 1 maximum"));

    configShiftParam(this, Params::P_SHIFT, "Jack shift");
    configSwitch(Params::P_SHIFT_ACTION, 0.f, 2.f, 0.f, "Jack shift action", {
        "Switch",
        "Toggle",
        "Instant"
    });
    configSwitch(Params::P_KEEP, 0.f, 1.f, 0.f, "Keep pedal settings when changing presets", {"off", "on"});

    configOutput(Outputs::OUT_JACK_1, "Jack 1");
    configOutput(Outputs::OUT_JACK_2, "Jack 2");
}

void JackModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    json_read_string(root, "haken-device", device_claim);
    json_read_bool(root, "glow-knobs", glow_knobs);

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
    onConnectHostModuleImpl(this, host);
}

void JackModule::onPresetChange()
{
    pull_jack_data();
    if (chem_ui) ui()->onPresetChange();
}

void JackModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    host_connection = connection ? connection->identified() : false;
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

void JackModule::pull_jack_data()
{
    assert(chem_host);
    auto em = chem_host->host_matrix();
    if (!em) return;
    last_assign_1 = em->get_jack_1_assign();
    last_assign_2 = em->get_jack_2_assign();
    getParam(P_ASSIGN_JACK_1).setValue(last_assign_1);
    getParam(P_ASSIGN_JACK_2).setValue(last_assign_2);
    getParam(P_MIN_JACK_1).setValue(unipolar_7_to_rack(em->get_pedal_1_min()));
    getParam(P_MAX_JACK_1).setValue(unipolar_7_to_rack(em->get_pedal_1_max()));
    getParam(P_MIN_JACK_2).setValue(unipolar_7_to_rack(em->get_pedal_2_min()));
    getParam(P_MAX_JACK_2).setValue(unipolar_7_to_rack(em->get_pedal_2_max()));

    last_shift = em->get_jack_shift();
    getParam(P_SHIFT).setValue(last_shift);

    last_keep = em->is_keep_pedals();
    getParam(P_KEEP).setValue(last_keep);
}

int JackModule::update_send(std::vector<PackedMidiMessage>& stream_data, int paramId, uint8_t haken_id, int last_value) {
    int param_value = getParamInt(getParam(paramId));
    if ((-1 != last_value) && (last_value != param_value)) {
        stream_data.push_back(MakeStreamData(ChemId::Jack, haken_id, U8(param_value)));
    }
    return param_value;
}

void JackModule::process_params(const ProcessArgs &args)
{
    assert(chem_host);
    std::vector<PackedMidiMessage> stream_data;

    last_assign_1 = update_send(stream_data, P_ASSIGN_JACK_1, Haken::idPedal1, last_assign_1);
    last_assign_2 = update_send(stream_data, P_ASSIGN_JACK_2, Haken::idPedal2, last_assign_2);
    last_shift    = update_send(stream_data, P_SHIFT,         Haken::idJackShift, last_shift);
    last_action   = update_send(stream_data, P_SHIFT_ACTION,  Haken::idSwTogInst, last_action);
    last_keep     = update_send(stream_data, P_KEEP,          Haken::idPresPed, last_keep);

    if (!stream_data.empty()) {
        auto haken = chem_host->host_haken();
        if (haken->log) {
            haken->log->log_message("Jack", "Pedal assign");
        }
        haken->begin_stream(ChemId::Jack, Haken::s_Mat_Poke);
        for (auto msg: stream_data) {
            haken->send_message(msg);
        }
        haken->end_stream(ChemId::Jack); // optional for poke streams
    }
}

void JackModule::process(const ProcessArgs &args)
{
    ChemModule::process(args);
    if (!connected()) return;
    if (!chem_host || chem_host->host_busy()) return;

    if (0 == ((args.frame + id) % 45)) {
        process_params(args);
    }

    auto em = chem_host->host_matrix();
    if (em) {
        uint16_t j = em->get_jack_1();
        getOutput(OUT_JACK_1).setVoltage(j ? unipolar_14_to_rack(j) : 0.f);
    
        j = em->get_jack_2();
        getOutput(OUT_JACK_2).setVoltage(j ? unipolar_14_to_rack(j) : 0.f);
    }
}

Model *modelJack = createModel<JackModule, JackUi>("chem-jack");

