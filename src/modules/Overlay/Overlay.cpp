#include "Overlay.hpp"
using namespace pachde;
#include "../../em/wrap-HakenMidi.hpp"
#include "../../services/rack-help.hpp"

OverlayModule::OverlayModule() :
//    modulation(this, ChemId::Unknown),
    in_mat_poke(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

//    dp4(configParam(P_MOD_AMOUNT, -100.f, 100.f, 0.f, "Modulation amount", "%"));

    // EmccPortConfig cfg[] =         {
    // };
    // modulation.configure(Params::P_MOD_AMOUNT, NUM_MOD_PARAMS, cfg);

}

void OverlayModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    device_claim = get_json_string(root, "haken-device");
    if (!device_claim.empty()) {
        //modulation.mod_from_json(root);
    }
    overlay_preset = get_json_string(root, "overlay-preset");
    title = get_json_string(root, "overlay-title");
    bg_color = parse_color(get_json_string(root, "background-color"));
    fg_color = parse_color(get_json_string(root, "foreground-color"));
    ModuleBroker::get()->try_bind_client(this);
}

json_t* OverlayModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    json_object_set_new(root, "overlay-preset", json_string(overlay_preset.c_str()));
    json_object_set_new(root, "overlay-title", json_string(title.c_str()));
    json_object_set_new(root, "background-color", json_string(hex_string(bg_color).c_str()));
    json_object_set_new(root, "foreground-color", json_string(hex_string(fg_color).c_str()));
    return root;
}

// IChemClient
::rack::engine::Module* OverlayModule::client_module() { return this; }
std::string OverlayModule::client_claim() { return device_claim; }

void OverlayModule::onConnectHost(IChemHost* host)
{
    onConnectHostModuleImpl(this, host);
    onPresetChange();
}

void OverlayModule::onPresetChange()
{
    auto p = chem_host->host_preset();
    preset_connected = p && p->name == overlay_preset;

    //    auto em = chem_host->host_matrix();

    // modulation.set_em_and_param_low(P_R1, em->get_r1(), true);
    // getParam(P_EFFECT).setValue(em->get_recirculator_type());

    //if (chem_ui) ui()->onPresetChange();
}

void OverlayModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

void OverlayModule::do_message(PackedMidiMessage message)
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

void OverlayModule::process_params(const ProcessArgs& args)
{
    //modulation.pull_mod_amount();
}

void OverlayModule::process(const ProcessArgs& args)
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
        getLight(L_CONNECTED).setBrightness(preset_connected);
    }
}

Model *modelOverlay = createModel<OverlayModule, OverlayUi>("chem-overlay");

