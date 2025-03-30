#include "proto.hpp"
using namespace pachde;
#include "../../em/wrap-HakenMidi.hpp"
#include "../../services/rack-help.hpp"

ProtoModule::ProtoModule() :
//    modulation(this, ChemId::Unknown),
    glow_knobs(false),
    in_mat_poke(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

//    dp4(configParam(P_MOD_AMOUNT, -100.f, 100.f, 0.f, "Modulation amount", "%"));

    // EmccPortConfig cfg[] =         {
    // };
    // modulation.configure(Params::P_MOD_AMOUNT, NUM_MOD_PARAMS, cfg);

}

void ProtoModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    json_t* j = json_object_get(root, "haken-device");
    if (j) {
        device_claim = json_string_value(j);
    }
    if (!device_claim.empty()) {
        //modulation.mod_from_json(root);
    }
    j = json_object_get(root, "glow-knobs");
    if (j) {
        glow_knobs = json_boolean_value(j);
    }
    ModuleBroker::get()->try_bind_client(this);
}

json_t* ProtoModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    if (!device_claim.empty()) {
        //modulation.mod_to_json(root);
    }
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    return root;
}

// IChemClient
::rack::engine::Module* ProtoModule::client_module() { return this; }
std::string ProtoModule::client_claim() { return device_claim; }

void ProtoModule::onConnectHost(IChemHost* host)
{
    onConnectHostModuleImpl(this, host);
}

void ProtoModule::onPresetChange()
{
//    auto em = chem_host->host_matrix();

    // modulation.set_em_and_param_low(P_R1, em->get_r1(), true);
    // getParam(P_EFFECT).setValue(em->get_recirculator_type());

    //if (chem_ui) ui()->onPresetChange();
}

void ProtoModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

void ProtoModule::do_message(PackedMidiMessage message)
{
    if (as_u8(ChemId::Unknown) == midi_tag(message)) return;

    switch (message.bytes.status_byte) {
    case Haken::ccStat1: {
        in_mat_poke = false;

        int param = -1;
        switch (midi_cc(message)) {
//        case Haken::ccReci1: param = P_R1; break;

        default: return;
        };
        assert(param != -1);
//        modulation.set_em_and_param_low(param, midi_cc_value(message), true);
    } break;

    case Haken::sData: //(ch16 key pressure)
        if (in_mat_poke) {
            switch (message.bytes.data1) {

            } return;
        }
        break;

    case Haken::ccStat16:
        if (Haken::ccStream == midi_cc(message)) {
            in_mat_poke = Haken::s_Mat_Poke == midi_cc_value(message);
        }
        break;
    default:
        break;
    }

}

void ProtoModule::process_params(const ProcessArgs& args)
{
    //modulation.pull_mod_amount();
}

void ProtoModule::process(const ProcessArgs& args)
{
    ChemModule::process(args);
    if (!chem_host || chem_host->host_busy()) return;

    if (((args.frame + id) % 41) == 0) {
        process_params(args);
    }

    // if (modulation.sync_params_ready(args)) {
    //     modulation.sync_send();
    // }    

    if (0 == ((args.frame + id) % 47)) {
        //modulation.update_mod_lights();
    }
}

Model *modelProto = createModel<ProtoModule, ProtoUi>("chem-proto");

