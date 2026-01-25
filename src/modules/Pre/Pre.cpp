#include "Pre.hpp"
#include "services/json-help.hpp"
#include "services/rack-help.hpp"
#include "em/wrap-HakenMidi.hpp"
using namespace pachde;

PreModule::PreModule() :
    modulation(this, ChemId::Pre),
    last_select(-1),
    glow_knobs(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    dp4(no_randomize(configParam(Params::P_PRE_LEVEL, 0.f, 10.f, 5.f, "Pre-level")));
    dp4(configParam(Params::P_MIX,             0.f, 10.f, 0.f, "Mix"));
    dp4(configParam(Params::P_THRESHOLD_DRIVE, 0.f, 10.f, 5.f, "Threshold/Drive"));
    dp4(configParam(Params::P_ATTACK,          0.f, 10.f, 5.f, "Attack/-"));
    dp4(configParam(Params::P_RATIO_MAKEUP,    0.f, 10.f, 5.f, "Ratio/Makeup"));

    dp4(configParam(P_MOD_AMOUNT, -100.f, 100.f, 0.f, "Modulation amount", "%"));

    configSwitch(P_SELECT, 0.f, 1.f, 0.f, "Select", { "Compressor", "Tanh"});

    configInput(IN_PRE_LEVEL,       "Pre-level");
    configInput(IN_MIX,             "Mix");
    configInput(IN_THRESHOLD_DRIVE, "Threshold/Drive");
    configInput(IN_ATTACK,          "Attack/-");
    configInput(IN_RATIO_MAKEUP,    "Ratio/Makeup");

    configLight(L_PRE_LEVEL_MOD,        "Modulation amount on Pre-level");
    configLight(L_MIX_MOD,              "Modulation amount on Mix");
    configLight(L_THRESHOLD_DRIVE_MOD,  "Modulation amount on Threshold/Drive");
    configLight(L_ATTACK_MOD,           "Modulation amount on Attack/-");
    configLight(L_RATIO_MAKEUP_MOD,     "Modulation amount on Ratio/Makeup");
    configLight(L_MIX, "Mix");

    EmccPortConfig cfg[] = {
        EmccPortConfig::cc(P_PRE_LEVEL,       IN_PRE_LEVEL,       L_PRE_LEVEL_MOD,       Haken::ch1, Haken::ccPre, true),
        EmccPortConfig::cc(P_MIX,             IN_MIX,             L_MIX_MOD,             Haken::ch1, Haken::ccCoThMix, true),
        EmccPortConfig::cc(P_THRESHOLD_DRIVE, IN_THRESHOLD_DRIVE, L_THRESHOLD_DRIVE_MOD, Haken::ch1, Haken::ccThrDrv, true),
        EmccPortConfig::cc(P_ATTACK,          IN_ATTACK,          L_ATTACK_MOD,          Haken::ch1, Haken::ccAtkCut, true),
        EmccPortConfig::cc(P_RATIO_MAKEUP,    IN_RATIO_MAKEUP,    L_RATIO_MAKEUP_MOD,    Haken::ch1, Haken::ccRatMkp, true)
    };
    modulation.configure(Params::P_MOD_AMOUNT, NUM_MOD_PARAMS, cfg);
}

bool PreModule::connected() {
    return host_connected(chem_host);
}

void PreModule::dataFromJson(json_t *root)
{
    ChemModule::dataFromJson(root);
    glow_knobs = get_json_bool(root, "glow-knobs", glow_knobs);
    device_claim = get_json_string(root, "haken-device");
    if (!device_claim.empty()) {
        modulation.mod_from_json(root);
    }
    ModuleBroker::get()->try_bind_client(this);
}

json_t* PreModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    set_json(root, "haken-device", device_claim);
    if (!device_claim.empty()) {
        modulation.mod_to_json(root);
    }
    set_json(root, "glow-knobs", glow_knobs);
    return root;
}

void PreModule::update_from_em()
{
    if (!connected()) return;
    auto em = chem_host->host_matrix();
    if (!em || !em->is_ready()) { return; }

    modulation.set_em_and_param_low(P_PRE_LEVEL, em->get_pre(), true);
    modulation.set_em_and_param_low(P_MIX, em->get_cotan_mix(), true);
    modulation.set_em_and_param_low(P_THRESHOLD_DRIVE, em->get_thresh_drive(), true);
    modulation.set_em_and_param_low(P_ATTACK, em->get_attack(), true);
    modulation.set_em_and_param_low(P_RATIO_MAKEUP, em->get_ratio_makeup(), true);

    last_select = em->is_tanh();
    getParam(P_SELECT).setValue(last_select);
}

void PreModule::do_message(PackedMidiMessage message)
{
    if (Haken::ctlChg1 != message.bytes.status_byte) return;
    if (as_u8(ChemId::Pre) == message.bytes.tag) return;
    if (!chem_host || chem_host->host_busy()) return;

    int param = -1;
    switch (midi_cc(message)) {
    case Haken::ccPre: param = P_PRE_LEVEL; break;
    case Haken::ccCoThMix: param = P_MIX; break;
    case Haken::ccThrDrv: param = P_THRESHOLD_DRIVE; break;
    case Haken::ccAtkCut: param = P_ATTACK; break;
    case Haken::ccRatMkp: param =P_RATIO_MAKEUP; break;
    default: return;
    }
    assert(param != -1);
    modulation.set_em_and_param_low(param, midi_cc_value(message), true);
}

// IChemClient
void PreModule::onConnectHost(IChemHost* host)
{
    onConnectHostModuleImpl(this, host);
}

void PreModule::onPresetChange()
{
    if (connected() && !chem_host->host_busy()) {
        update_from_em();
        //if (chem_ui) ui()->onPresetChange(); // ui does nothing
    }
}

void PreModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui && (device == ChemDevice::Haken)) ui()->onConnectionChange(device, connection);
}

void PreModule::process_params(const ProcessArgs &args)
{
    auto v = getParam(Params::P_MIX).getValue()*.1f;
    getLight(Lights::L_MIX).setBrightnessSmooth(v, 45.f);

    int sel = getParamInt(getParam(Params::P_SELECT));
    if (sel != last_select) {
        last_select = sel;
        chem_host->host_haken()->compressor_option(ChemId::Pre, sel);
    }

    modulation.pull_mod_amount();
}

void PreModule::process(const ProcessArgs &args)
{
    ChemModule::process(args);

    if (!host_connected(chem_host) || chem_host->host_busy()) return;

    if (0 == ((args.frame + id) % 45)) {
        process_params(args);
    }

    if (modulation.sync_params_ready(args)) {
        modulation.sync_send();
    }

    if (((args.frame + id) % 61) == 0) {
        modulation.update_mod_lights();
    }

}

Model *modelPre = createModel<PreModule, PreUi>("chem-pre");

