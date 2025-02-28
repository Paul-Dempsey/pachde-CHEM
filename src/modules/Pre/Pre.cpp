#include "Pre.hpp"
using namespace pachde;
#include "../../services/em-param-quantity.hpp"
#include "../../em/wrap-HakenMidi.hpp"

PreModule::PreModule()
:   last_select(-1),
    glow_knobs(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configInput(IN_PRE_LEVEL, "Pre-level");
    configInput(IN_MIX,       "Mix");
    configInput(IN_THRESHOLD, "Threshold");
    configInput(IN_ATTACK,    "Attack");
    configInput(IN_RATIO,     "Ratio");
    
    configU7EmParam(Haken::ch1, Haken::ccPre,     this, Params::P_PRE_LEVEL,       0.f, 10.f, 5.f, "Pre-level");
    configU7EmParam(Haken::ch1, Haken::ccCoThMix, this, Params::P_MIX,             0.f, 10.f,  0.f, "Mix");
    configU7EmParam(Haken::ch1, Haken::ccThrDrv,  this, Params::P_THRESHOLD_DRIVE, 0.f, 10.f, 10.f, "Threshold");
    configU7EmParam(Haken::ch1, Haken::ccAtkCut,  this, Params::P_ATTACK_,         0.f, 10.f,  5.f, "Attack");
    configU7EmParam(Haken::ch1, Haken::ccRatMkp,  this, Params::P_RATIO_MAKEUP,    0.f, 10.f,  5.f, "Ratio");

    configParam(P_ATTENUVERT,   -100.f, 100.f, 0.f, "Input attenuverter", "%")->displayPrecision = 4;

    configSwitch(P_SELECT, 0.f, 1.f, 0.f, "Select Compressor/Tanh", { "Compressor", "Tanh"});

    configLight(L_MIX, "Compressor/Tanh Mix");
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
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    return root;
}

void PreModule::pull_params()
{
    if (!connected()) return;
    auto em = chem_host->host_matrix();
    
    auto pq = get_u7_em_param_quantity(this, P_PRE_LEVEL);
    if (pq) pq->set_em_midi_value(em->get_pre());
    
    pq = get_u7_em_param_quantity(this, P_MIX);
    if (pq) pq->set_em_midi_value(em->get_cotan_mix());

    pq = get_u7_em_param_quantity(this, P_THRESHOLD_DRIVE);
    if (pq) pq->set_em_midi_value(em->get_thresh_drive());

    pq = get_u7_em_param_quantity(this, P_ATTACK_);
    if (pq) pq->set_em_midi_value(em->get_attack());

    pq = get_u7_em_param_quantity(this, P_RATIO_MAKEUP);
    if (pq) pq->set_em_midi_value(em->get_ratio_makeup());

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
        pull_params();
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
        chem_host->host_haken()->compressor_option(sel);
    }
}

void PreModule::process(const ProcessArgs &args)
{
    if (!connected() || chem_host->host_busy()) return;
    if (0 == ((args.frame + id) % 45)) {
        process_params(args);
    }
}

Model *modelPre = createModel<PreModule, PreUi>("chem-pre");

