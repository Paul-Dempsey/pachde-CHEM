#include "Macro.hpp"
#include "em/wrap-HakenMidi.hpp"
#include "services/rack-help.hpp"
#include "services/json-help.hpp"
using namespace pachde;

MacroModule::MacroModule() :
    modulation(this, ChemId::Macro),
    glow_knobs(false)
{
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

    configLight(L_M1, "Modulation amount on M1");
    configLight(L_M2, "Modulation amount on M2");
    configLight(L_M3, "Modulation amount on M3");
    configLight(L_M4, "Modulation amount on M4");
    configLight(L_M5, "Modulation amount on M5");
    configLight(L_M6, "Modulation amount on M6");

    EmccPortConfig cfg[] = {
        EmccPortConfig::cc(P_M1, IN_M1, L_M1,  Haken::ch1, Haken::ccI),
        EmccPortConfig::cc(P_M2, IN_M2, L_M2,  Haken::ch1, Haken::ccII),
        EmccPortConfig::cc(P_M3, IN_M3, L_M3,  Haken::ch1, Haken::ccIII),
        EmccPortConfig::cc(P_M4, IN_M4, L_M4,  Haken::ch1, Haken::ccIV),
        EmccPortConfig::cc(P_M5, IN_M5, L_M5,  Haken::ch1, Haken::ccV),
        EmccPortConfig::cc(P_M6, IN_M6, L_M6,  Haken::ch1, Haken::ccVI)
    };
    modulation.configure(Params::P_MOD_AMOUNT, NUM_MOD_PARAMS, cfg);
}

void MacroModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    glow_knobs = get_json_bool(root, "glow-knobs", glow_knobs);
    device_claim = get_json_string(root, "haken-device");
    if (!device_claim.empty()) {
        modulation.mod_from_json(root);
    }
    ModuleBroker::get()->try_bind_client(this);
}

json_t* MacroModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();

    set_json(root, "haken-device", device_claim);
    if (!device_claim.empty()) {
        modulation.mod_to_json(root);
    }
    set_json(root, "glow-knobs", glow_knobs);
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
    onConnectHostModuleImpl(this, host);
}

void MacroModule::onPresetChange()
{
    if (chem_host) {
        if (batch_busy() || chem_host->host_busy()) return;
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

void MacroModule::do_message(PackedMidiMessage message)
{
    em_batch.do_message(message);
    if (em_batch.busy()) return;
    if (!chem_host || chem_host->host_busy()) return;

    if (Haken::ccStat1 != message.bytes.status_byte) return;
    if (as_u8(ChemId::Macro) == midi_tag(message)) return;

    auto cc = midi_cc(message);
    switch (cc) {

    case Haken::ccFracIM48:
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
        if (!em) return;
        for (int param = 0; param <= P_M6; ++param) {
            modulation.set_em_and_param(param, em->get_macro_value(param), true);
        }
    }
}

void MacroModule::onRandomize()
{
    for (int i = 0; i < NUM_MOD_PARAMS; ++i) {
        modulation.ports[i].set_mod_amount((random::uniform() * 200.0) - 100.0);
        modulation.set_em_and_param(i, randomZeroTo(Haken::max14), true);
    }
}

void MacroModule::process_params(const ProcessArgs &args)
{
    modulation.pull_mod_amount();
}

void MacroModule::process(const ProcessArgs& args)
{
    ChemModule::process(args);

    if (!host_connected(chem_host) || chem_host->host_busy()) return;

    if (((args.frame + id) % 40) == 0) {
        process_params(args);
    }

    if (modulation.sync_params_ready(args)) {
        modulation.sync_send();
    }

    if (((args.frame + id) % 63) == 0) {
        modulation.update_mod_lights();
    }

}

Model *modelMacro = createModel<MacroModule, MacroUi>("chem-macro");

