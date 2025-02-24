#include "Jack.hpp"
using namespace pachde;

static const std::vector<uint8_t> ccmap = {
    Haken::ccUndef,
    Haken::ccPre,
    Haken::ccPost,
    Haken::ccAudIn,
    Haken::ccI,
    Haken::ccII,
    Haken::ccIII,
    Haken::ccIV,
    Haken::ccV,
    Haken::ccVI,
    Haken::ccSus,
    Haken::ccSos,
    Haken::ccSos2,
    Haken::ccOctShift,
    Haken::ccMonoSwitch,
    Haken::ccRoundRate,
    Haken::ccRndIni,
    Haken::ccStretch,
    Haken::ccFineTune,
    Haken::ccAdvance,
    Haken::ccReci1,
    Haken::ccReci2,
    Haken::ccReci3,
    Haken::ccReci4,
    Haken::ccReci5,
    Haken::ccReci6,
    Haken::ccReciMix

};

int pedal_to_index(uint8_t assign) {
    auto it = std::find(ccmap.cbegin(), ccmap.cend(), assign);
    return it == ccmap.cend() ? 0 : it - ccmap.cbegin();
}

uint8_t index_to_pedal_cc(int index) {
    return ccmap[index];
}

JackModule::JackModule()
:   chem_host(nullptr),
    ui(nullptr),
    glow_knobs(false),
    host_connection(false),
    last_assign_1(-1),
    last_assign_2(-1),
    last_keep(-1)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    // What happened to recirculator assignments that were in 10.09?
    // Todo: adapt HC-1 PedalParam
    std::vector<std::string> labels = {
        "(free)",
        "Pre-level",
        "Post-level",
        "Audio in",
        "Macro i",
        "Macro ii",
        "Macro iii",
        "Macro iv",
        "Macro v",
        "Macro vi",
        "Sustain",
        "Sos 1",
        "Sos 2",
        "Oct switch",
        "Mono switch",
        "Round rate",
        "Round initial",
        "Oct stretch",
        "Fine tune",
        "Advance",
        "R1",
        "R2", 
        "R3",
        "R4",
        "R5",
        "R6",
        "RMix"
    };
    configSwitch(Params::P_ASSIGN_JACK_1, 0.f, labels.size(), 10.f, "Jack 1 assign", labels);
    configSwitch(Params::P_ASSIGN_JACK_2, 0.f, labels.size(), 12.f, "Jack 2 assign", labels);

    auto pq = configParam(Params::P_MIN_JACK_1, 0.f, 10.f, 0.f, "Jack 1 minimum");
    pq->displayPrecision = 2;
    pq = configParam(Params::P_MAX_JACK_1, 0.f, 10.f, 10.f, "Jack 1 maximum");
    pq->displayPrecision = 2;

    pq = configParam(Params::P_MIN_JACK_2, 0.f, 10.f, 0.f, "Jack 1 minimum");
    pq->displayPrecision = 2;
    pq = configParam(Params::P_MAX_JACK_2, 0.f, 10.f, 10.f, "Jack 1 maximum");
    pq->displayPrecision = 2;

    configSwitch(Params::P_KEEP, 0.f, 1.f, 0.f, "Keep pedal settings", {"off", "on"});

    configOutput(Outputs::OUT_JACK_1, "Jack 1");
    configOutput(Outputs::OUT_JACK_2, "Jack 2");
}

void JackModule::dataFromJson(json_t* root)
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
    chem_host = host;
    if (!host) {
        device_claim = "";
        if (ui) ui->onConnectHost(nullptr);
        return;
    }
    auto conn = chem_host->host_connection(ChemDevice::Haken);
    if (conn) {
        device_claim = conn->info.claim();
    }
    if (ui) ui->onConnectHost(host);
}

void JackModule::onPresetChange()
{
    pull_jack_data();
    if (ui) ui->onPresetChange();
}

void JackModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    host_connection = connection ? connection->identified() : false;
    if (ui) ui->onConnectionChange(device, connection);
}

void JackModule::pull_jack_data()
{
    assert(chem_host);
    auto em = chem_host->host_matrix();
    last_assign_1 = pedal_to_index(em->get_jack_1_assign());
    last_assign_2 = pedal_to_index(em->get_jack_2_assign());
    getParam(P_ASSIGN_JACK_1).setValue(last_assign_1);
    getParam(P_ASSIGN_JACK_2).setValue(last_assign_2);
    getParam(P_MIN_JACK_1).setValue(unipolar_7_to_rack(em->get_pedal_1_min()));
    getParam(P_MAX_JACK_1).setValue(unipolar_7_to_rack(em->get_pedal_1_max()));
    getParam(P_MIN_JACK_2).setValue(unipolar_7_to_rack(em->get_pedal_2_min()));
    getParam(P_MAX_JACK_2).setValue(unipolar_7_to_rack(em->get_pedal_2_max()));

    last_keep = em->is_keep_pedals();
    getParam(P_KEEP).setValue(last_keep);
}

void JackModule::process_params(const ProcessArgs &args)
{
    assert(chem_host);

    auto pkeep = getParamInt(getParam(P_KEEP));
    if (last_keep == -1) {
        last_keep = pkeep;
    } else if (last_keep != pkeep) {
        last_keep = pkeep;
        chem_host->host_haken()->keep_pedals(pkeep);
    }
    getLight(L_KEEP).setBrightnessSmooth(pkeep, 45.f);
    
    int assign_1 = getParamInt(getParam(P_ASSIGN_JACK_1));
    int assign_2 = getParamInt(getParam(P_ASSIGN_JACK_2));
    if ((assign_1 != last_assign_1) || (assign_2 != last_assign_2)) {
        auto haken = chem_host->host_haken();
        haken->log->logMessage("Jack", "Pedal assign");
        haken->begin_stream(Haken::s_Mat_Poke);
        if (assign_1 != last_assign_1) {
            haken->stream_data(Haken::idPedal1, index_to_pedal_cc(assign_1));
        }
        if (assign_2 != last_assign_2) {
            haken->stream_data(Haken::idPedal2, index_to_pedal_cc(assign_2));
        }
        haken->end_stream();

        last_assign_1 = assign_1;
        last_assign_2 = assign_2;
    }
}

void JackModule::process(const ProcessArgs &args)
{
    if (!connected()) return;
    if (!chem_host || chem_host->host_busy()) return;

    if (0 == ((args.frame + id) % 45)) {
        process_params(args);
    }

    auto em = chem_host->host_matrix();
    uint16_t j = em->get_jack_1();
    getOutput(OUT_JACK_1).setVoltage(j ? unipolar_14_to_rack(j) : 0.f);

    j = em->get_jack_2();
    getOutput(OUT_JACK_2).setVoltage(j ? unipolar_14_to_rack(j) : 0.f);
}

Model *modelJack = createModel<JackModule, JackUi>("chem-jack");

