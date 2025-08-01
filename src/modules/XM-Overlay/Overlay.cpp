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
    for (auto client: clients) {
        client->on_overlay_change(nullptr);
    }
    if (chem_host) {
        chem_host->unregister_chem_client(this);
    }
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
    in_macro_request = true;
    mac_build.request_macros(chem_host->host_haken());
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

void OverlayModule::used_macros(std::vector<uint8_t> *list)
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

void OverlayModule::onRemove(const RemoveEvent &e)
{
    for (auto client: clients) {
        client->on_overlay_change(nullptr);
    }
    clients.clear();
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
        if (preset_connected) {
            update_from_em();
        }
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
}

void OverlayModule::do_message(PackedMidiMessage message)
{
    if (mac_build.in_request) {
        mac_build.do_message(message);
    } else {
        if (macros.empty()) return;
        if (!chem_host) return;
        if (Haken::ccStat1 != message.bytes.status_byte) return;
        auto em = chem_host->host_matrix();
        if (!em) return;

        auto tag = midi_tag(message);
        if (as_u8(ChemId::Unknown) == tag) return;
        if (as_u8(ChemId::Overlay) == tag) return;
        if (as_u8(ChemId::XM) == tag) return;

        auto cc = midi_cc(message);
        if (cc < Haken::ccM7 || cc > Haken::ccM90) return;

        uint8_t number = 0;
        if (cc <= Haken::ccM30) {
            number = (cc - Haken::ccM7);
        } else if (cc < Haken::ccM31) {
            return;
        } else if (cc <= Haken::ccM48) {
            number = (cc - Haken::ccM31);
        }
        if (!number) return;
        if (em->frac_hi) number += 48;

        for (auto macro: macros.data) {
            if (number == macro->macro_number) {
                macro->set_em_value(em->get_macro_value(number));
            }
        }
    }
}

bool OverlayModule::sync_params_ready(const rack::engine::Module::ProcessArgs &args, float rate)
{
    if (midi_timer.process(args.sampleTime) > rate) {
        midi_timer.reset();
        return true;
    }
    return false;
}

void OverlayModule::update_from_em()
{
        if (chem_host && chem_host->host_preset()) {
        auto em = chem_host->host_matrix();
        if (!em) return;
        for (auto macro: macros.data) {
            auto em_value = em->get_macro_value(macro->macro_number);
            macro->set_em_value(em_value);
        }
    }
}

void OverlayModule::process(const ProcessArgs& args)
{
    ChemModule::process(args);
    if (!chem_host || chem_host->host_busy()) return;

    if (0 == ((args.frame + id) % 47)) {
        getLight(L_CONNECTED).setBrightness(preset_connected);
    }

    if (macros.empty()) return;
    auto haken = chem_host->host_haken();
    if (!haken) return;

    if (sync_params_ready(args)) {
        for (auto macro: macros.data) {
            if (macro->valid() && macro->pending()) {
                macro->un_pend();
                haken->extended_macro(ChemId::Overlay, macro->macro_number, macro->em_value);
            }
        }
    }

}

Model *modelOverlay = createModel<OverlayModule, OverlayUi>("chem-overlay");

