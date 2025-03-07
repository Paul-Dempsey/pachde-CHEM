#include "Macro.hpp"
#include "../../em/wrap-HakenMidi.hpp"
#include "../../services/rack-help.hpp"
using namespace pachde;

MacroModule::MacroModule() :
    modulation(this, ChemId::Macro),
    glow_knobs(false)
{
    EmccPortConfig cfg[] = {
        EmccPortConfig::cc(Haken::ch1, Haken::ccI),
        EmccPortConfig::cc(Haken::ch1, Haken::ccII),
        EmccPortConfig::cc(Haken::ch1, Haken::ccIII),
        EmccPortConfig::cc(Haken::ch1, Haken::ccIV),
        EmccPortConfig::cc(Haken::ch1, Haken::ccV),
        EmccPortConfig::cc(Haken::ch1, Haken::ccVI)
    };
    modulation.configure(Params::P_MOD_AMOUNT, Params::P_M1, Inputs::IN_M1, Lights::L_M1a, NUM_MOD_PARAMS, cfg);

    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    configInput(IN_M1, "Macro 1 (i)");
    configInput(IN_M2, "Macro 2 (ii)");
    configInput(IN_M3, "Macro 3 (iii)");
    configInput(IN_M4, "Macro 4 (iv)");
    configInput(IN_M5, "Macro 5 (v)");
    configInput(IN_M6, "Macro 6 (vi)");

    dp4(configParam(Params::P_M1, 0.f, 10.f, 5.f, "Macro 1", "v"));
    dp4(configParam(Params::P_M2, 0.f, 10.f, 5.f, "Macro 2", "v"));
    dp4(configParam(Params::P_M3, 0.f, 10.f, 5.f, "Macro 3", "v"));
    dp4(configParam(Params::P_M4, 0.f, 10.f, 5.f, "Macro 4", "v"));
    dp4(configParam(Params::P_M5, 0.f, 10.f, 5.f, "Macro 5", "v"));
    dp4(configParam(Params::P_M6, 0.f, 10.f, 5.f, "Macro 6", "v"));

    dp4(configParam(P_MOD_AMOUNT, -100.f, 100.f, 0.f, "Modulation amount", "%"));

    configLight(L_M1a, "Modulation amount on M1");
    configLight(L_M2a, "Modulation amount on M2");
    configLight(L_M3a, "Modulation amount on M3");
    configLight(L_M4a, "Modulation amount on M4");
    configLight(L_M5a, "Modulation amount on M5");
    configLight(L_M6a, "Modulation amount on M6");
}

void MacroModule::dataFromJson(json_t* root)
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
    if (!device_claim.empty()) {
        modulation.mod_from_json(root);
    }
    ModuleBroker::get()->try_bind_client(this);
}

json_t* MacroModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    if (!device_claim.empty()) {
        modulation.mod_to_json(root);
    }
    json_object_set_new(root, "glow-knobs", json_boolean(glow_knobs));
    return root;
}

bool MacroModule::connected()
{
    if (!chem_host) return false;
    auto conn = chem_host->host_connection(ChemDevice::Haken);
    return conn && conn->identified();
}

// IChemClient
::rack::engine::Module* MacroModule::client_module() { return this; }
std::string MacroModule::client_claim() { return device_claim; }

void MacroModule::onConnectHost(IChemHost* host)
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

void MacroModule::onPresetChange()
{
    if (chem_host) {
        if (chem_host->host_busy()) return;
        auto preset = chem_host->host_preset();
        if (preset) {
            macro_names.fill_macro_names();
            macro_names.parse_text(preset->text);
            update_from_em();
        }
    }
    if (chem_ui) ui()->onPresetChange();
}

void MacroModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

void MacroModule::doMessage(PackedMidiMessage message)
{
    if (Haken::ccStat1 != message.bytes.status_byte) return;
    if (as_u8(ChemId::Macro) == midi_tag(message)) return;

    auto cc = midi_cc(message);
    switch (cc) {

    case Haken::ccFracPed:
        macro_lsb = midi_cc_value(message);
        break;

    case Haken::ccI:
    case Haken::ccII:
    case Haken::ccIII:
    case Haken::ccIV:
    case Haken::ccV:
    case Haken::ccVI: {
        int param = cc - Haken::ccI;
        uint16_t value = ((midi_cc_value(message) << 7) + macro_lsb);
        macro_lsb = 0;
        modulation.set_em_and_param(param, value, true);
    } break;
    }
}

void MacroModule::update_from_em()
{
    if (chem_host && chem_host->host_preset()) {
        auto em = chem_host->host_matrix();
        for (int param = 0; param <= P_M6; ++param) {
            modulation.set_em_and_param(param, em->get_macro_value(param), true);
        }
    }
}

void MacroModule::process_params(const ProcessArgs& args)
{
    modulation.pull_mod_amount();
}

void MacroModule::process(const ProcessArgs& args)
{
    find_and_bind_host(this, args);
    if (!chem_host || chem_host->host_busy()) return;
    
    if (((args.frame + id) % 40) == 0) {
        process_params(args);
    }

    if (modulation.sync_params_ready(args)) {
        modulation.sync_send();
    }    

    if (((args.frame + id) % 63) == 0) {
        modulation.update_lights();
    }

}

Model *modelMacro = createModel<MacroModule, MacroUi>("chem-macro");

