#include "Post.hpp"
#include "../../services/rack-help.hpp"
#include "../../em/wrap-HakenMidi.hpp"
using namespace pachde;

PostModule::PostModule() :
    modulation(this, ChemId::Post),
    glow_knobs(false),
    muted(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    dp4(no_randomize(configParam(Params::P_POST_LEVEL, 0.f, 10.f, 5.f, "Post-level")));
    dp4(configParam(Params::P_MIX,          0.f, 10.f, 0.f, "EQ Mix"));
    dp4(configParam(Params::P_TILT,         0.f, 10.f, 5.f, "EQ Tilt"));
    dp4(configParam(Params::P_FREQUENCY,    0.f, 10.f, 5.f, "EQ Frequency"));
    dp4(configParam(P_MOD_AMOUNT, -100.f, 100.f, 0.f, "Modulation amount", "%"));

    configSwitch(P_MUTE, 0.f, 1.f, 0.f, "Mute", { "Voiced", "Muted" });

    configInput(IN_POST_LEVEL, "Post-level");
    configInput(IN_MIX,        "EQ Mix");
    configInput(IN_TILT,       "EQ Tilt");
    configInput(IN_FREQUENCY,  "EQ Frequency");

    configInput(IN_MUTE,       "Mute trigger");

    configLight(L_POST_LEVEL_MOD, "Modulation amount on Post level");
    configLight(L_MIX_MOD,        "Modulation amount on EQ Mix");
    configLight(L_TILT_MOD,       "Modulation amount on EQ Tilt");
    configLight(L_FREQUENCY_MOD,  "Modulation amount on EQ Frequency");
    configLight(L_MIX, "EQ active");
    configLight(L_MUTE, "Mute");

    EmccPortConfig cfg[] = {
        EmccPortConfig::cc(P_POST_LEVEL, IN_POST_LEVEL, L_POST_LEVEL_MOD, Haken::ch1, Haken::ccPost, false),
        EmccPortConfig::cc(P_MIX,        IN_MIX,        L_MIX_MOD,        Haken::ch1, Haken::ccEqMix, true),
        EmccPortConfig::cc(P_TILT,       IN_TILT,       L_TILT_MOD,       Haken::ch1, Haken::ccEqTilt, true),
        EmccPortConfig::cc(P_FREQUENCY,  IN_FREQUENCY,  L_FREQUENCY_MOD,  Haken::ch1, Haken::ccEqFreq, true),
    };
    modulation.configure(Params::P_MOD_AMOUNT, NUM_MOD_PARAMS, cfg);
}

void PostModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    json_read_string(root, "haken-device", device_claim);
    json_read_bool(root, "glow-knobs", glow_knobs);

    if (!device_claim.empty()) {
        modulation.mod_from_json(root);
    }
    ModuleBroker::get()->try_bind_client(this);
}

json_t* PostModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    if (!device_claim.empty()) {
        modulation.mod_to_json(root);
    }
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    return root;
}

void PostModule::do_message(PackedMidiMessage message)
{
    if (Haken::ccStat1 != message.bytes.status_byte) return;
    if (as_u8(ChemId::Post) == midi_tag(message)) return;

    int param = -1;
    switch (midi_cc(message)) {
    case Haken::ccFracIM48:
        cc_lsb = midi_cc_value(message);
        return;

    case Haken::ccPost: {
        uint16_t value = ((message.bytes.data2 << 7) + cc_lsb);
        cc_lsb = 0;
        modulation.set_em_and_param(P_POST_LEVEL, value, true);
        return;
    }
    case Haken::ccEqMix: param = P_MIX; break;
    case Haken::ccEqTilt: param = P_TILT; break;
    case Haken::ccEqFreq: param = P_FREQUENCY; break;

    default:
        return;
    }
    assert(param != -1);
    modulation.set_em_and_param_low(param, midi_cc_value(message), true);
}

// IChemClient
::rack::engine::Module* PostModule::client_module() { return this; }
std::string PostModule::client_claim() { return device_claim; }

void PostModule::onConnectHost(IChemHost* host)
{
    onConnectHostModuleImpl(this, host);
}

void PostModule::onPresetChange()
{
    if (!connected()) return;
    update_from_em();
    //if (chem_ui) ui()->onPresetChange(); // ui doesn't do anything on preset change
}

void PostModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{ 
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

bool PostModule::connected()
{
    if (!chem_host) return false;
    auto conn = chem_host->host_connection(ChemDevice::Haken);
    return conn && conn->identified();
}

void PostModule::update_from_em()
{
    if (!connected()) return;
    auto em = chem_host->host_matrix();
    modulation.set_em_and_param(P_POST_LEVEL, em->get_post(), true);
    modulation.set_em_and_param_low(P_MIX, em->get_eq_mix(), true);
    modulation.set_em_and_param_low(P_TILT, em->get_eq_tilt(), true);
    modulation.set_em_and_param_low(P_FREQUENCY, em->get_eq_freq(), true);
}

void PostModule::sync_mute()
{
    auto new_mute = getParamInt(getParam(Params::P_MUTE));
    getLight(Lights::L_MUTE).setBrightnessSmooth(new_mute, 45.f);
    if (new_mute != muted) {
        muted = new_mute;
        auto haken = chem_host->host_haken();
        if (muted) {
            haken->control_change(ChemId::Post, Haken::ch1, Haken::ccFracIM48, 0);
            haken->control_change(ChemId::Post, Haken::ch1, Haken::ccPost, 0);
        } else {
            modulation.get_port(P_POST_LEVEL).send(chem_host, ChemId::Post, true);
        }
    }
}

void PostModule::process_params(const ProcessArgs &args)
{
    float v = getParam(Params::P_MIX).getValue()* .1f;
    getLight(Lights::L_MIX).setBrightnessSmooth(v, 45.f);

    modulation.pull_mod_amount();
    sync_mute();
}

void PostModule::process(const ProcessArgs& args)
{
    ChemModule::process(args);

    if (!chem_host || chem_host->host_busy()) return;

    if (modulation.sync_params_ready(args)) {
        modulation.sync_send();
    }

    if (0 == ((args.frame + id) % 45)) {
        process_params(args);
    }

    auto mute_in = getInput(Inputs::IN_MUTE);
    if (mute_in.isConnected()) {
        auto v = mute_in.getVoltage();
        if (mute_trigger.process(v, 0.1f, 5.f)) {
            mute_trigger.reset();
            getParam(P_MUTE).setValue(getParamInt(getParam(Params::P_MUTE)) ? 0.f : 1.f);
            sync_mute();
        }
    }
    if (((args.frame + id) % 61) == 0) {
        modulation.update_mod_lights();
    }
}

Model *modelPost = createModel<PostModule, PostUi>("chem-post");

