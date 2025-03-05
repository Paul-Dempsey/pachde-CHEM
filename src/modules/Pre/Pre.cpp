#include "Pre.hpp"
using namespace pachde;
#include "../../em/wrap-HakenMidi.hpp"

PreModule::PreModule() :
    modulation(this, MidiTag::Pre),
    last_select(-1),
    glow_knobs(false)
{
    EmccPortConfig cfg[] = {
        EmccPortConfig::cc(Haken::ch1, Haken::ccPre, true),
        EmccPortConfig::cc(Haken::ch1, Haken::ccCoThMix, true),
        EmccPortConfig::cc(Haken::ch1, Haken::ccThrDrv, true),
        EmccPortConfig::cc(Haken::ch1, Haken::ccAtkCut, true),
        EmccPortConfig::cc(Haken::ch1, Haken::ccRatMkp, true)
    };
    modulation.configure(Params::P_MOD_AMOUNT, Params::P_PRE_LEVEL, Inputs::IN_PRE_LEVEL, Lights::L_PRE_LEVEL_MOD, NUM_MOD_PARAMS, cfg);

    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configParam(Params::P_PRE_LEVEL,       0.f, 10.f, 5.f, "Pre-level")->displayPrecision = 4;
    configParam(Params::P_MIX,             0.f, 10.f,  0.f, "Mix")->displayPrecision = 4;
    configParam(Params::P_THRESHOLD_DRIVE, 0.f, 10.f, 10.f, "Threshold/Drive")->displayPrecision = 4;
    configParam(Params::P_ATTACK,          0.f, 10.f,  5.f, "Attack/-")->displayPrecision = 4;
    configParam(Params::P_RATIO_MAKEUP,    0.f, 10.f,  5.f, "Ratio/Makeup")->displayPrecision = 4;

    configParam(P_MOD_AMOUNT, -100.f, 100.f, 0.f, "Modulation amount", "%")->displayPrecision = 4;

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
}

bool PreModule::connected()
{
    if (!chem_host) return false;
    auto conn = chem_host->host_connection(ChemDevice::Haken);
    return conn && conn->identified();
}

void PreModule::dataFromJson(json_t *root)
{
    ChemModule::dataFromJson(root);
    json_t* j = json_object_get(root, "haken-device");
    if (j) {
        device_claim = json_string_value(j);
    }
    if (!device_claim.empty()) {
        modulation.mod_from_json(root);
    }
    j = json_object_get(root, "glow-knobs");
    if (j) {
        glow_knobs = json_boolean_value(j);
    }

    ModuleBroker::get()->try_bind_client(this);
}

json_t* PreModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    if (!device_claim.empty()) {
        modulation.mod_to_json(root);
    }
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    return root;
}

void PreModule::update_from_em(bool with_knobs)
{
    if (!connected()) return;
    auto em = chem_host->host_matrix();
    if (!em->is_ready()) { return; }

    modulation.set_em_and_param_low(P_PRE_LEVEL, em->get_pre(), with_knobs);
    modulation.set_em_and_param_low(P_MIX, em->get_cotan_mix(), with_knobs);
    modulation.set_em_and_param_low(P_THRESHOLD_DRIVE, em->get_thresh_drive(), with_knobs);
    modulation.set_em_and_param_low(P_ATTACK, em->get_attack(), with_knobs);
    modulation.set_em_and_param_low(P_RATIO_MAKEUP, em->get_ratio_makeup(), with_knobs);

    last_select = em->is_tanh();
    getParam(P_SELECT).setValue(last_select);
}

// IChemClient
::rack::engine::Module* PreModule::client_module() { return this; }
std::string PreModule::client_claim() { return device_claim; }

void PreModule::onConnectHost(IChemHost* host)
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

void PreModule::onPresetChange()
{
    if (connected()) {
        update_from_em(true);
        if (chem_ui) ui()->onPresetChange();
    }
}

void PreModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

void PreModule::process_params(const ProcessArgs &args)
{
    auto v = getParam(Params::P_MIX).getValue()*.1f;
    getLight(Lights::L_MIX).setBrightnessSmooth(v, 45.f);

    int sel = getParamInt(getParam(Params::P_SELECT));
    if (sel != last_select) {
        last_select = sel;
        chem_host->host_haken()->compressor_option(MidiTag::Pre, sel);
    }

    modulation.pull_mod_amount();
    update_from_em(false);
}

void PreModule::process(const ProcessArgs &args)
{
    if (!connected() || chem_host->host_busy()) return;

    if (modulation.sync_params_ready(args)) {
        modulation.sync_send();
    }

    if (0 == ((args.frame + id) % 45)) {
        process_params(args);
    }

    if (((args.frame + id) % 61) == 0) {
        modulation.update_lights();
    }

}

Model *modelPre = createModel<PreModule, PreUi>("chem-pre");

