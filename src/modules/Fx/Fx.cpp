#include "Fx.hpp"
using namespace pachde;
#include "../../em/wrap-HakenMidi.hpp"
#include "../../services/rack-help.hpp"

FxModule::FxModule() :
    modulation(this, ChemId::Fx),
    last_disable(-1),
    glow_knobs(false),
    in_mat_poke(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    dp4(configParam(Params::P_R1,  0.f, 10.f, 5.f, "R1"));
    dp4(configParam(Params::P_R2,  0.f, 10.f, 5.f, "R2"));
    dp4(configParam(Params::P_R3,  0.f, 10.f, 5.f, "R3"));
    dp4(configParam(Params::P_R4,  0.f, 10.f, 5.f, "R4"));
    dp4(configParam(Params::P_R5,  0.f, 10.f, 5.f, "R5"));
    dp4(configParam(Params::P_R6,  0.f, 10.f, 5.f, "R6"));
    dp4(configParam(Params::P_MIX, 0.f, 10.f, 5.f, "Mix"));
    dp4(configParam(P_MOD_AMOUNT, -100.f, 100.f, 0.f, "Modulation amount", "%"));

    configSwitch(P_DISABLE, 0.f, 1.f, 0.f, "Fx", {"on", "off"});
    configSwitch(P_EFFECT, 0.f, 6.f, 0.f, "Effect", {
        "Short reverb",
        "Mod delay",
        "Swept delay",
        "Analog echo",
        "Delay with LPF",
        "Delay with HPF",
        "Long reverb"
    });

    configInput(IN_R1, "R1");
    configInput(IN_R2, "R2");
    configInput(IN_R3, "R3");
    configInput(IN_R4, "R4");
    configInput(IN_R5, "R5");
    configInput(IN_R6, "R6");
    configInput(IN_MIX, "Effects Mix");

    configLight(L_R1_MOD, "Modulation amount on R1");
    configLight(L_R2_MOD, "Modulation amount on R2");
    configLight(L_R3_MOD, "Modulation amount on R3");
    configLight(L_R4_MOD, "Modulation amount on R4");
    configLight(L_R5_MOD, "Modulation amount on R5");
    configLight(L_R6_MOD, "Modulation amount on R6");
    configLight(L_MIX_MOD, "Modulation amount on Mix");

    configLight(L_MIX, "Effects Mix");

    EmccPortConfig cfg[] =         {
        EmccPortConfig::cc(P_R1, IN_R1, L_R1_MOD, Haken::ch1, Haken::ccReci1, true),
        EmccPortConfig::cc(P_R2, IN_R2, L_R2_MOD, Haken::ch1, Haken::ccReci2, true),
        EmccPortConfig::cc(P_R3, IN_R3, L_R3_MOD, Haken::ch1, Haken::ccReci3, true),
        EmccPortConfig::cc(P_R4, IN_R4, L_R4_MOD, Haken::ch1, Haken::ccReci4, true),
        EmccPortConfig::cc(P_R5, IN_R5, L_R5_MOD, Haken::ch1, Haken::ccReci5, true),
        EmccPortConfig::cc(P_R6, IN_R6, L_R6_MOD, Haken::ch1, Haken::ccReci6, true),
        EmccPortConfig::cc(P_MIX, IN_MIX, L_MIX_MOD, Haken::ch1, Haken::ccReciMix, true),
    };
    modulation.configure(Params::P_MOD_AMOUNT, NUM_MOD_PARAMS, cfg);

}

void FxModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    json_read_string(root, "haken-device", device_claim);
    json_read_bool(root, "glow-knobs", glow_knobs);

    if (!device_claim.empty()) {
        modulation.mod_from_json(root);
    }

    ModuleBroker::get()->try_bind_client(this);
}

json_t* FxModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    if (!device_claim.empty()) {
        modulation.mod_to_json(root);
    }
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    return root;
}

// IChemClient
::rack::engine::Module* FxModule::client_module() { return this; }
std::string FxModule::client_claim() { return device_claim; }

void FxModule::onConnectHost(IChemHost* host)
{
    onConnectHostModuleImpl(this, host);
}

void FxModule::onPresetChange()
{
    auto em = chem_host->host_matrix();

    auto disable = em->is_disable_recirculator();
    getParam(Params::P_DISABLE).setValue(disable);
    getLight(Lights::L_DISABLE).setBrightnessSmooth(disable, 45.f);

    modulation.set_em_and_param_low(P_R1, em->get_r1(), true);
    modulation.set_em_and_param_low(P_R2, em->get_r2(), true);
    modulation.set_em_and_param_low(P_R3, em->get_r3(), true);
    modulation.set_em_and_param_low(P_R4, em->get_r4(), true);
    modulation.set_em_and_param_low(P_R5, em->get_r5(), true);
    modulation.set_em_and_param_low(P_R6, em->get_r6(), true);
    modulation.set_em_and_param_low(P_MIX, em->get_r_mix(), true);
    getParam(P_EFFECT).setValue(em->get_recirculator_type());

    //if (chem_ui) ui()->onPresetChange();
}

void FxModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

void FxModule::do_message(PackedMidiMessage message)
{
    if (as_u8(ChemId::Fx) == midi_tag(message)) return;

    switch (message.bytes.status_byte) {
    case Haken::ccStat1: {
        in_mat_poke = false;

        int param = -1;
        switch (midi_cc(message)) {
        case Haken::ccReci1: param = P_R1; break;
        case Haken::ccReci2: param = P_R2; break;
        case Haken::ccReci3: param = P_R3; break;
        case Haken::ccReci4: param = P_R4; break;
        case Haken::ccReciMix: param = P_MIX; break;
        case Haken::ccReci5: param = P_R5; break;
        case Haken::ccReci6: param = P_R6; break;

        default: return;
        };
        assert(param != -1);
        modulation.set_em_and_param_low(param, midi_cc_value(message), true);
    } break;

    case Haken::sData: //(ch16 key pressure)
        if (in_mat_poke) {
            switch (message.bytes.data1) {

            case Haken::idReciType: {
                uint8_t kind = Haken::R_mask & message.bytes.data2;
                getParam(P_EFFECT).setValue(kind);
            } return;

            case Haken::idNoRecirc:
                getParam(P_DISABLE).setValue(message.bytes.data2);
                return;
            }
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

void FxModule::process_params(const ProcessArgs& args)
{
    modulation.pull_mod_amount();

    uint8_t effect = getParamInt(getParam(P_EFFECT));
    if (chem_host->host_matrix()->get_recirculator_type() != effect) {
        chem_host->host_haken()->recirculator_type(ChemId::Fx, effect);
    }

    int disable = getParamInt(getParam(Params::P_DISABLE));
    if (last_disable == -1) {
        last_disable = disable;
    } else if (last_disable != disable) {
        last_disable = disable;
        chem_host->host_haken()->disable_recirculator(ChemId::Fx, disable);
    }
    getLight(Lights::L_DISABLE).setBrightnessSmooth(disable, 45.f);

    if (disable) {
        getLight(Lights::L_MIX).setBrightnessSmooth(0, 45.f);
    } else {
        float v = getParam(Params::P_MIX).getValue()* .1f;
        getLight(Lights::L_MIX).setBrightnessSmooth(v, 45.f);
    }

}

void FxModule::process(const ProcessArgs& args)
{
    ChemModule::process(args);
    if (!chem_host || chem_host->host_busy()) return;

    if (((args.frame + id) % 41) == 0) {
        process_params(args);
    }

    if (modulation.sync_params_ready(args)) {
        modulation.sync_send();
    }    

    if (0 == ((args.frame + id) % 47)) {
        modulation.update_mod_lights();
    }
}

Model *modelFx = createModel<FxModule, FxUi>("chem-fx");

