#include "Overlay.hpp"
using namespace pachde;
#include "../../em/wrap-HakenMidi.hpp"
#include "../../services/rack-help.hpp"

OverlayModule::OverlayModule()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    configLight(L_CONNECTED, "Preset connected");
    mac_build.client_id = ChemId::Overlay;
    mac_build.set_on_complete([=](){ on_macro_request_complete(); });
}

void OverlayModule::reset() {
    title = "";
    overlay_preset = nullptr;
    bg_color = 0;
}

void OverlayModule::overlay_register_client(IOverlayClient *client)
{
    auto it = std::find(clients.cbegin(), clients.cend(), client);
    if (it == clients.cend()) {
        clients.push_back(client);
    }
}

void OverlayModule::overlay_unregister_client(IOverlayClient *client)
{
    auto it = std::find(clients.begin(), clients.end(), client);
    if (it != clients.end()) {
        clients.erase(it);
    }
}

void OverlayModule::overlay_request_macros()
{
    if (!overlay_preset) return;
    if (!chem_host) return;
    auto haken = chem_host->host_haken();
    mac_build.haken = haken;
    haken->request_archive_0(ChemId::Overlay);
}

void send_store_preset(ChemId tag, HakenMidi* haken, std::shared_ptr<PresetInfo> preset)
{
    haken->begin_stream(tag, Haken::s_Name);

    PackedMidiMessage msg = Tag(MakePolyKeyPressure(Haken::ch16, 0, 0), tag);
    bool odd = true;
    for (auto ch: preset->name) {
        if (odd) {
            msg.bytes.data1 = ch;
        } else {
            msg.bytes.data2 = ch;
            haken->send_message(msg);
        }
        odd = !odd;
    }
    if (!odd) {
        msg.bytes.data2 = 0;
        haken->send_message(msg);
    }
    haken->end_stream(tag);

    haken->control_change(tag, Haken::ch16, Haken::ccBankH, 126);
    haken->control_change(tag, Haken::ch16, Haken::ccBankL, 0);
    haken->control_change(tag, Haken::ch16, Haken::ccStore, 0);
    haken->control_change(tag, Haken::ch16, Haken::ccTask, Haken::curGloToFlash);
}

void OverlayModule::on_macro_request_complete()
{
    if (!overlay_preset) {
        assert(false);
        return;
    }
    auto haken = chem_host->host_haken();
    send_store_preset(ChemId::Overlay, haken, overlay_preset);
    haken->select_preset(ChemId::Overlay, overlay_preset->id);
}

void OverlayModule::prune_missing_clients()
{
    if (macros.empty()) return;
    std::vector<int64_t> client_ids;
    for (auto client : clients) {
        client_ids.push_back(client->get_module_id());
    }
    macros.prune_leaving(client_ids);
}

MacroReadyState OverlayModule::overlay_macros_ready()
{
    if (mac_build.in_archive) return MacroReadyState::InProgress;
    if (macro_usage.empty()) return MacroReadyState::Unavailable;
    return MacroReadyState::Available;
}

void OverlayModule::overlay_remove_macro(int64_t module, ssize_t knob)
{
    macros.remove(module, knob);
}

void OverlayModule::overlay_add_macro(std::shared_ptr<MacroDescription> macro)
{
    macros.add(macro);
}

void OverlayModule::overlay_add_update_macro(std::shared_ptr<MacroDescription> macro)
{
    macros.add_update(macro);
}

void OverlayModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    device_claim = get_json_string(root, "haken-device");
    if (!device_claim.empty()) {
        //modulation.mod_from_json(root);
    }
    auto j = json_object_get(root, "overlay-preset");
    if (j) {
        overlay_preset = std::make_shared<PresetInfo>();
        overlay_preset->fromJson(j);
    }
    title = get_json_string(root, "overlay-title");
    bg_color = parse_color(get_json_string(root, "background-color"));
    fg_color = parse_color(get_json_string(root, "foreground-color"));
    macros.from_json(root);

    ModuleBroker::get()->try_bind_client(this);
}

json_t* OverlayModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    if (overlay_preset) {
        json_object_set_new(root, "overlay-preset", overlay_preset->toJson(true, true, true));
    }
    json_object_set_new(root, "overlay-title", json_string(title.c_str()));
    json_object_set_new(root, "background-color", json_string(hex_string(bg_color).c_str()));
    json_object_set_new(root, "foreground-color", json_string(hex_string(fg_color).c_str()));
    macros.to_json(root);

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
    live_preset = nullptr;
    preset_connected = false;
    if (!chem_host) return;
    auto p = chem_host->host_preset();
    if (p) {
        live_preset = std::make_shared<PresetInfo>(p);
        preset_connected = (nullptr != overlay_preset) && (p->name == overlay_preset->name);
    }
    //    auto em = chem_host->host_matrix();

    // modulation.set_em_and_param_low(P_R1, em->get_r1(), true);
    //if (chem_ui) ui()->onPresetChange();
}

void OverlayModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}


void OverlayModule::do_message(PackedMidiMessage message)
{
    mac_build.do_message(message);
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

