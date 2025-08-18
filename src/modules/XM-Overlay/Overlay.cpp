#include "Overlay.hpp"
using namespace pachde;
#include "../../em/wrap-HakenMidi.hpp"
#include "../../services/rack-help.hpp"

OverlayModule::OverlayModule()
{
    config(0, 0, 0, Lights::NUM_LIGHTS);
    configLight(L_CONNECTED, "Preset connected");

    mac_build.client_id = ChemId::Overlay;
    mac_build.set_on_complete([=](){ on_macro_request_complete(); });
    midi_timer.time = (random::uniform() * MIDI_RATE); // jitter
}

OverlayModule::~OverlayModule()
{
    pending_client_clear = true;
    for (auto info: clients) {
        info->client->on_overlay_change(nullptr);
    }
    if (chem_host) {
        chem_host->unregister_chem_client(this);
    }
}

void OverlayModule::reset() {
    title = "";
    bg_color = 0;
    overlay_preset = nullptr;
    preset_connected = false;
    notify_connect_preset();
}

bool OverlayModule::client_editing()
{
    return paused_clients > 0;
}

void OverlayModule::notify_connect_preset()
{
    for (auto info: clients) {
        info->client->on_connect_preset();
    }
}

void OverlayModule::overlay_register_client(IOverlayClient *client)
{
    if (pending_client_clear) return;
    for (auto info: clients) {
        if (client == info->client) {
            return;
        }
    }
    clients.push_back(std::make_shared<ClientInfo>(client));
}

void OverlayModule::overlay_unregister_client(IOverlayClient *client)
{
    if (pending_client_clear) return;
    for (auto it = clients.begin(); it != clients.end(); it++) {
        if (it->get()->client == client) {
            if (it->get()->pause) {
                paused_clients--;
            }
            clients.erase(it);
            return;
        }
    }
}

void OverlayModule::overlay_client_pause(IOverlayClient *client, bool pausing)
{
    if (pending_client_clear) return;
    if (pausing) { paused_clients++; } else { paused_clients--; }
    for (auto info: clients) {
        if (client == info->client) {
            info->pause = pausing;
            return;
        }
    }
    assert(false); // unregistered client
}

void OverlayModule::overlay_request_macros()
{
    if (!overlay_preset) return;
    if (!chem_host) return;
    auto haken = chem_host->host_haken();
    if (!haken) return;
    mac_build.request_macros(haken);
    in_macro_request = true;
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
    expect_preset_change = true;
    haken->select_preset(ChemId::Overlay, overlay_preset->id);
}

MacroReadyState OverlayModule::overlay_macros_ready()
{
    if (mac_build.in_archive) return MacroReadyState::InProgress;
    if (macro_usage.empty()) return MacroReadyState::Unavailable;
    return MacroReadyState::Available;
}

void OverlayModule::overlay_used_macros(std::vector<uint8_t> *list)
{
    if (!list) return;
    list->clear();
    for (auto macro: macros.data) {
        list->push_back(macro->macro_number);
    }
}

void OverlayModule::overlay_remove_macro(int64_t module, ssize_t knob)
{
    macros.remove(module, knob);
}

void OverlayModule::overlay_add_update_macro(std::shared_ptr<MacroDescription> macro)
{
    macros.add_update(macro);
}

void OverlayModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    device_claim = get_json_string(root, "haken-device");
    auto j = json_object_get(root, "overlay-preset");
    if (j) {
        overlay_preset = std::make_shared<PresetInfo>();
        overlay_preset->fromJson(j);
    }
    title = get_json_string(root, "overlay-title");
    bg_color = parse_color(get_json_string(root, "background-color"));
    fg_color = parse_color(get_json_string(root, "foreground-color"));

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

    return root;
}

void OverlayModule::onReset(const ResetEvent &e)
{
    macros.clear();
    overlay_preset = nullptr;
    preset_connected = false;
    getLightInfo(L_CONNECTED)->name = "Preset connected";

    title = "";
    bg_color = 0;
    fg_color = parse_color("hsl(42,60%,50%)");
    macro_usage.clear();
    in_macro_request = false;
    expect_preset_change = false;
    if (chem_ui) {
        auto my_ui = ui();
        my_ui->set_bg_color(bg_color);
        my_ui->set_fg_color(fg_color);
        my_ui->set_title(title);
    }
    notify_connect_preset();
}

void OverlayModule::onRemove(const RemoveEvent &e)
{
    pending_client_clear = true;
    for (auto info: clients) {
        info->client->on_overlay_change(nullptr);
    }
    clients.clear();
    paused_clients = 0;
    pending_client_clear = false;

    if (chem_host) {
        chem_host->unregister_chem_client(this);
        chem_host = nullptr;
    }
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
    if (p && !p->empty()) {
        live_preset = std::make_shared<PresetInfo>(p);
        preset_connected = (nullptr != overlay_preset) && (p->id.key() == overlay_preset->id.key());
        getLightInfo(L_CONNECTED)->name = preset_connected ? overlay_preset->name + " connected" : "Preset connected";
        notify_connect_preset();
    }
    if (expect_preset_change) {
        expect_preset_change = false;
    } else {
        macro_usage.clear();
    }
}

void OverlayModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
    onPresetChange();
}

bool OverlayModule::sync_params_ready(const rack::engine::Module::ProcessArgs &args, float rate)
{
    if (midi_timer.process(args.sampleTime) > rate) {
        midi_timer.reset();
        return true;
    }
    return false;
}

void OverlayModule::do_message(PackedMidiMessage message)
{
    if (mac_build.in_request) {
        mac_build.do_message(message);
    }
}

void OverlayModule::process(const ProcessArgs& args)
{
    ChemModule::process(args);
    if (!host_connected(chem_host) || chem_host->host_busy()) return;

    int64_t jitter_frame = args.frame + id;

    if (0 == (jitter_frame % 63)) {
        getLight(L_CONNECTED).setBrightness(preset_connected);
    }
    if (!preset_connected && (0 == (jitter_frame % 120))) {
        onPresetChange(); // check for one
    }

    getLight(L_CONNECTED).setBrightness(preset_connected);

    if (!preset_connected) {
        return;
    }
    if (in_macro_request) {
        return;
    }
    if (expect_preset_change) {
        return;
    }
    if (macros.empty()) {
        return;
    }
    if (client_editing()) {
        return;
    }

    auto haken = chem_host->host_haken();
    if (!haken) return;

    if (sync_params_ready(args)) {
        for (auto macro: macros.data) {
            if (macro->valid()) {
                if (macro->pending()) {
                    macro->un_pend();
                    haken->extended_macro(ChemId::Overlay, macro->macro_number, macro->em_value);
                }
            }
        }
    }

}

Model *modelOverlay = createModel<OverlayModule, OverlayUi>("chem-overlay");

