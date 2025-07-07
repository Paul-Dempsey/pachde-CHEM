#include "Core.hpp"
#include "../../em/PresetId.hpp"
#include "../../services/ModuleBroker.hpp"
#include "../../services/kv-store.hpp"
#include "../../services/rack-help.hpp"

using namespace pachde;
using namespace eaganmatrix;

CoreModule::CoreModule() : modulation(this, ChemId::Core)
{
    ticker.set_interval(1.0f);
    module_id = ChemId::Core;
    using EME = IHandleEmEvents::EventMask;
    em_event_mask = static_cast<EME>(
        EME::EditorReply
        + EME::HardwareChanged
        + EME::PresetBegin
        + EME::PresetChanged
        + EME::UserBegin
        + EME::UserComplete
        + EME::SystemBegin
        + EME::SystemComplete
        + EME::MahlingBegin
        + EME::MahlingComplete
    );

    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configSwitch(Params::P_C1_MUSIC_FILTER, 0.f, 1.f, 0.f, "MIDI filter", { "All data", "Music data only"} );
    configSwitch(Params::P_C2_MUSIC_FILTER, 0.f, 1.f, 0.f, "MIDI filter", { "All data", "Music data only"} );
    configSwitch(Params::P_C1_MUTE, 0.f, 1.f, 0.f, "MIDI 1 data", { "passed", "blocked" } );
    configSwitch(Params::P_C2_MUTE, 0.f, 1.f, 0.f, "MIDI 2 data", { "passed", "blocked" } );
    configSwitch(Params::P_C1_CHANNEL_MAP, 0.f, 1.f, 0.f, "MIDI 1 Channel map", { "off", "reflect" } );
    configSwitch(Params::P_C2_CHANNEL_MAP, 0.f, 1.f, 0.f, "MIDI 2 Channel map", { "off", "reflect" } );
    snap(configParam(Params::P_NOTHING, 0.f, 60*60, 0.f, "CHEM-time", ""));
    dp2(configParam(Params::P_ATTENUATION, 0.f, 10.f, 0.f, "Volume"));

    configInput(IN_C1_MUTE_GATE, "MIDI 1 data block gate");
    configInput(IN_C2_MUTE_GATE, "MIDI 2 data block gate");

    configLight(L_ROUND_Y,       "Round on Y");
    configLight(L_ROUND_INITIAL, "Round initial");
    configLight(L_ROUND,         "Rounding");
    configLight(L_ROUND_RELEASE, "Round on release");
    configLight(L_READY,         "Haken device ready");

    configOutput(Outputs::OUT_READY, "Ready gate");

    startup_tasks.init(this);
    recurring_tasks.init(this);
    ModuleBroker::get()->register_host(this);

    midi_relay.set_em(&em);
    midi_relay.register_target(&haken_midi_out);

    em.subscribeEMEvents(this);
    haken_midi.set_handler(&midi_relay);

    haken_device.init(ChemDevice::Haken, this);
    controller1.init(ChemDevice::Midi1, this);
    controller2.init(ChemDevice::Midi2, this);

    haken_midi_in.set_target(&midi_relay);
    controller1_midi_in.set_target(&midi_relay);
    controller2_midi_in.set_target(&midi_relay);

    auto broker = MidiDeviceBroker::get();
    broker->registerDeviceHolder(&haken_device);
    broker->registerDeviceHolder(&controller1);
    broker->registerDeviceHolder(&controller2);

    EmccPortConfig cfg[] = {
        EmccPortConfig::cc(P_ATTENUATION, -1, -1, Haken::ch1, Haken::ccAtten, true),
    };
    modulation.configure(-1, 1, cfg);
    chem_host = this;
    midi_relay.register_target(this);

    system_presets = std::make_shared<PresetList>();
    system_presets->order = PresetOrder::Alpha;
    user_presets = std::make_shared<PresetList>();
}

CoreModule::~CoreModule()
{
    midi_relay.unregister_target(this);
    for (auto client : chem_clients) {
        client->onConnectHost(nullptr);
    }
    ModuleBroker::get()->unregister_host(this);
    controller1_midi_in.clear();
    controller2_midi_in.clear();
    controller1.unsubscribe(this);
    controller2.unsubscribe(this);
    auto broker = MidiDeviceBroker::get();
    broker->unRegisterDeviceHolder(&haken_device);
    broker->unRegisterDeviceHolder(&controller1);
    broker->unRegisterDeviceHolder(&controller2);
}

void CoreModule::enable_logging(bool enable)
{
    if (enable){
        midi_log = new MidiLog();
        log_message("Core", format_string("Starting CHEM Core [%p] %s", this, string::formatTimeISO(system::getUnixTime()).c_str()));
        haken_midi.set_logger(midi_log);
        haken_midi_out.set_logger(midi_log);
        haken_midi_in.set_logger("<<H", midi_log);
        controller1_midi_in.set_logger("<C1", midi_log);
        controller2_midi_in.set_logger("<C2", midi_log);
    } else {
        haken_midi.set_logger(nullptr);
        haken_midi_out.set_logger(nullptr);
        haken_midi_in.set_logger("", nullptr);
        controller1_midi_in.set_logger("", nullptr);
        controller2_midi_in.set_logger("", nullptr);
        if (midi_log) {
            midi_log->close();
            delete midi_log;
            midi_log = nullptr;
        }
    }
}

std::string CoreModule::device_name(ChemDevice which)
{
    switch (which)
    {
    case ChemDevice::Haken: return haken_device.device_name();
    case ChemDevice::Midi1: return controller1.device_name();
    case ChemDevice::Midi2: return controller2.device_name();
    default: return "";
    }
}

void CoreModule::next_preset()
{
    load_lists();
    
    if (em.is_osmose()) {
        if (!system_presets && !user_presets) return;
        PresetId id;
        if (em.preset.id.valid() && em.preset.id.key()) {
            auto index = system_presets->index_of_id(em.preset.id);
            if ((-1 == index) && user_presets) {
                index = user_presets->index_of_id(em.preset.id);
                if (-1 == index) return;
                if (++index >= user_presets->size()) index = 0;
                id = user_presets->presets[index]->id;
            } else {
                if (++index >= system_presets->size()) index = 0;
                id = system_presets->presets[index]->id;
            }
        } else if (em.preset.tag) {
            ssize_t index{-1};
            if (system_presets) {
                index = system_presets->index_of_tag(em.preset.tag);
            }
            if ((-1 == index) && user_presets) {
                index = user_presets->index_of_tag(em.preset.tag);
                if (-1 == index) return;
                if (++index >= user_presets->size()) index = 0;
                id = user_presets->presets[index]->id;
            } else {
                if (++index >= system_presets->size()) index = 0;
                id = system_presets->presets[index]->id;
            }
        }
        if (id.valid()) {
            em.set_osmose_id(id);
            haken_midi.select_preset(ChemId::Core,id);
        }
    } else {
        haken_midi.next_system_preset(ChemId::Core);
    }
}

void CoreModule::prev_preset()
{
    load_lists();
    if (em.is_osmose()) {
        if (!system_presets && !user_presets) return;
        PresetId id;
        ssize_t index{-1};
        if (em.preset.id.valid() && em.preset.id.key()) {
            if (system_presets) {
                system_presets->index_of_id(em.preset.id);
            } 
            if ((-1 == index) && user_presets) {
                index = user_presets->index_of_id(em.preset.id);
                if (-1 == index) return;
                if (--index < 0) index = user_presets->size()-1;
                id = user_presets->presets[index]->id;
            } else {
                if (--index < 0) index = system_presets->size()-1;
                id = system_presets->presets[index]->id;
            }
        } else if (em.preset.tag) {
            if (system_presets) {
                index = system_presets->index_of_tag(em.preset.tag);
            } 
            if ((-1 == index) && user_presets) {
                index = user_presets->index_of_tag(em.preset.tag);
                if (-1 == index) return;
                if (--index < 0) index = user_presets->size() - 1;
                id = user_presets->presets[index]->id;
            } else {
                if (--index < 0) index = system_presets->size() - 1;
                id = system_presets->presets[index]->id;
            }
        }
        if (id.valid()) {
            em.set_osmose_id(id);
            haken_midi.select_preset(ChemId::Core,id);
        }
    } else {
        haken_midi.previous_system_preset(ChemId::Core);
    }
}

PresetResult CoreModule::load_preset_file(PresetTab which)
{
    valid_tab(which);
    if (host_busy()) return PresetResult::NotReady;
    if (!em.hardware) return PresetResult::NotReady;
    auto path = preset_file_name(which, em.hardware);
    auto list = which == PresetTab::User ? user_presets : system_presets;
    return list->load(path) ? PresetResult::Ok : PresetResult::FileNotFound;
}

PresetResult CoreModule::load_quick_user_presets()
{
    if (em.is_osmose()) return PresetResult::NotApplicableOsmose;
    if (host_busy()) return PresetResult::NotReady;
    if (PresetResult::Ok == load_preset_file(PresetTab::User)) return PresetResult::Ok;

    if (chem_ui && !ui()->showing_busy()) {
        ui()->show_busy(true);
    }
    gathering = QuickUserPresets;
    haken_midi.request_user(ChemId::Core);
    return PresetResult::Ok;
}

PresetResult CoreModule::load_quick_system_presets()
{
    if (em.is_osmose()) return PresetResult::NotApplicableOsmose;
    if (host_busy()) return PresetResult::NotReady;
    if (PresetResult::Ok == load_preset_file(PresetTab::System)) return PresetResult::Ok;

    if (chem_ui && !ui()->showing_busy()) {
        ui()->show_busy(true);
    }
    gathering = QuickSystemPresets;
    haken_midi.request_system(ChemId::Core);
    return PresetResult::Ok;
}

PresetResult CoreModule::load_full_user_presets()
{
    if (host_busy()) return PresetResult::NotReady;
    if (PresetResult::Ok == load_preset_file(PresetTab::User)) return PresetResult::Ok;

    if (chem_ui && !ui()->showing_busy()) {
        ui()->show_busy(true);
        ui()->create_stop_button();
    }
    em.begin_user_scan();
    if (em.is_osmose()) {
        full_build = new PresetListBuildCoordinator(midi_log, true, new OsmosePresetEnumerator(ChemId::Core, 90));
        full_build->start_building();
    } else {
        auto hpe = new HakenPresetEnumerator(ChemId::Core);
        id_builder = new PresetIdListBuilder(ChemId::Core, this, hpe);
        em.subscribeEMEvents(id_builder);
        full_build = new PresetListBuildCoordinator(midi_log, false, hpe);
        haken_midi.request_user(ChemId::Core);
    }
    gathering = FullUserPresets;
    return PresetResult::Ok;
}

PresetResult CoreModule::scan_osmose_presets(uint8_t page)
{
    if (!em.is_osmose()) return PresetResult::NotApplicableEm;
    if (host_busy()) return PresetResult::NotReady;
    if (chem_ui && !ui()->showing_busy()) {
        ui()->show_busy(true);
        ui()->create_stop_button();
    }
    em.begin_user_scan();
    full_build = new PresetListBuildCoordinator(midi_log, true, new OsmosePresetEnumerator(ChemId::Core, page));
    full_build->start_building();
    gathering = FullUserPresets;
    return PresetResult::Ok;
}

std::shared_ptr<PresetList> CoreModule::host_user_presets()
{
    if (user_presets->empty() && !host_busy()) {
        load_preset_file(PresetTab::User);
    }
    return user_presets;
}

std::shared_ptr<PresetList> CoreModule::host_system_presets()
{
    if (system_presets->empty() && !host_busy()) {
        load_preset_file(PresetTab::System);
    }
    return system_presets;
}

PresetResult CoreModule::load_full_system_presets()
{
    if (host_busy()) return PresetResult::NotReady;
    if (PresetResult::Ok == load_preset_file(PresetTab::System)) return PresetResult::Ok;

    if (chem_ui && !ui()->showing_busy()) {
        ui()->show_busy(true);
        ui()->create_stop_button();
    }
    em.begin_system_scan();
    if (em.is_osmose()) {
        full_build = new PresetListBuildCoordinator(midi_log, true, new OsmosePresetEnumerator(ChemId::Core, 30, 34));
        full_build->start_building();
    } else {
        auto hpe = new HakenPresetEnumerator(ChemId::Core);
        id_builder = new PresetIdListBuilder(ChemId::Core, this, hpe);
        em.subscribeEMEvents(id_builder);
        haken_midi.request_system(ChemId::Core);
        full_build = new PresetListBuildCoordinator(midi_log, false, hpe);
    }
    gathering = FullSystemPresets;

    return PresetResult::Ok;
}

PresetResult CoreModule::end_scan()
{
    if (!gathering) return PresetResult::NotApplicable;
    auto gather = gathering;
    gathering = GatherFlags::None;
    if (midi_log) {
        midi_log->log_message("PLB", format_string("Completed in %.6f", full_build->total));
    }
    if (gather_user(gather)) {
        em.end_user_scan();
        user_presets->save(preset_file_name(PresetTab::User, em.hardware), em.hardware, haken_device.device_claim);
    } else {
        assert(gather_system(gather));
        em.end_system_scan();
        system_presets->save(preset_file_name(PresetTab::System, em.hardware), em.hardware, haken_device.device_claim);
    }
    if (chem_ui) {
        ui()->show_busy(false);
        ui()->remove_stop_button();
    }
    auto fb = full_build;
    full_build = nullptr;
    delete fb;
    stop_scan = false;
    return PresetResult::Ok;
}

void CoreModule::load_lists()
{
    if (host_busy()) return;
    if (!em.hardware) return;
    if (system_presets->empty()) {
        load_preset_file(PresetTab::System);
    }
    if (user_presets->empty()) {
        load_preset_file(PresetTab::User);
    }
}

void CoreModule::reset_tasks()
{
    recurring_tasks.reset();
    startup_tasks.reset();
    for (size_t i = 0; i < sizeof(start_states)/sizeof(start_states[0]); ++i) {
        start_states[i] = ChemTask::State::Untried;
    }
}

void CoreModule::reboot()
{
    if (in_reboot) return;

    in_reboot = true;
    disconnected = false;

    haken_midi_in.clear();
    haken_midi_out.clear();
    controller1_midi_in.clear();
    controller2_midi_in.clear();

    em.reset();
    init_osmose();
    reset_tasks();
    in_reboot = false;
}

// IHandleEmEvents
void CoreModule::onEditorReply(uint8_t reply)
{
    if (!startup_tasks.completed()) {
        startup_tasks.heartbeat_received();
    } else {
        recurring_tasks.heartbeat_received();
    }
}

void CoreModule::onHardwareChanged(uint8_t hardware, uint16_t firmware_version)
{
    saved_hardware = hardware;
    init_osmose();
}

void CoreModule::onPresetBegin()
{
    if (gathering && full_build) {
        if (PresetListBuildCoordinator::Phase::PendBegin == full_build->phase) {
            full_build->preset_started();
        }
    }
}

void CoreModule::onPresetChanged()
{
    log_message("Core", format_string("--- Received Preset Changed: %s", em.preset.summary().c_str()));
    if (!em.preset.empty()) {
        if (!startup_tasks.completed()) {
            startup_tasks.configuration_received();
        } else if (gathering) {
            if (gather_user(gathering)) {
                if (gather_full(gathering) && !id_builder) {
                    assert(gather_presets(gathering));
                    if (em.preset.id.key() != full_build->iter->expected_id().key()) {
                        log_message("Core", format_string("oid[%6x] em[%6x] plb[%6x]", em.osmose_id.key(), em.preset.id.key(), full_build->iter->expected_id().key()));
                    }
                    full_build->preset_received();
                    user_presets->add(&em.preset);
                } else if (gather_quick(gathering)) {
                    user_presets->add(&em.preset);
                }
            } else if (gather_system(gathering)) {
                if (gather_full(gathering) && !id_builder) {
                    assert(gather_presets(gathering));
                    if (em.preset.id.key() != full_build->iter->expected_id().key()) {
                        log_message("Core", format_string("oid[%6x] em[%6x] plb[%6x]", em.osmose_id.key(), em.preset.id.key(), full_build->iter->expected_id().key()));
                    }
                    full_build->preset_received();
                    system_presets->add(&em.preset);
                } else if (gather_quick(gathering)) {
                    system_presets->add(&em.preset);
                }
            } else {
                assert(false);
            }
        }
    }
    update_from_em();
    if (startup_tasks.completed() && !gathering) {
        notify_preset_changed();
    }
}

void CoreModule::onUserBegin()
{
    is_busy = true;
    if (chem_ui) ui()->show_busy(is_busy);
}

void CoreModule::onUserComplete()
{
    is_busy = false;
    if (chem_ui) {
        ui()->show_busy(false);
        ui()->remove_stop_button();
    }
    if (QuickUserPresets == gathering) {
        user_presets->save(preset_file_name(PresetTab::User, em.hardware), em.hardware, haken_device.device_claim);
        gathering = GatherFlags::None;
    }
}

void CoreModule::onSystemBegin()
{
    is_busy = true;
    if (chem_ui) ui()->show_busy(true);
}

void CoreModule::onSystemComplete()
{
    is_busy = false;
    if (chem_ui) {
        ui()->show_busy(false);
        ui()->remove_stop_button();
    }
    if (QuickSystemPresets == gathering) {
        system_presets->save(preset_file_name(PresetTab::System, em.hardware), em.hardware, haken_device.device_claim);
        gathering = GatherFlags::None;
    }
}

void CoreModule::onMahlingBegin()
{
    is_busy = true;
    if (chem_ui) ui()->show_busy(is_busy);
}

void CoreModule::onMahlingComplete()
{
    is_busy = false;
    if (chem_ui) ui()->show_busy(is_busy);
}

// IChemHost
void CoreModule::notify_connection_changed(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    for (auto client : chem_clients) {
        client->onConnectionChange(device, connection);
    }
}

void CoreModule::notify_preset_changed() {
    for (auto client : chem_clients) {
        client->onPresetChange();
    }
}

void HandleMidiDeviceChange(MidiInput* input, const MidiDeviceHolder* source)
{
    input->ring.clear();
    if (source->connection) {
        input->setDriverId(source->connection->driver_id);
        input->setDeviceId(source->connection->input_device_id);
    } else {
        input->reset();
    }
}

void CoreModule::onMidiDeviceChange(const MidiDeviceHolder* source)
{
    switch (source->role()) {
    case ChemDevice::Haken: {
        em.ready = false;
        haken_midi_in.ring.clear();
        haken_midi_out.ring.clear();
        user_presets->clear();
        system_presets->clear();
        if (!disconnected && source->connection) {
            haken_midi_in.setDriverId(source->connection->driver_id);
            haken_midi_in.setDeviceId(source->connection->input_device_id);
            haken_midi_out.output.setDeviceId(source->connection->output_device_id);
            haken_midi_out.output.channel = -1;
            load_preset_file(PresetTab::System);
            load_preset_file(PresetTab::User);
            log_message("Core", format_string("+++ connect HAKEN %s", source->connection->info.friendly(TextFormatLength::Short).c_str()).c_str());
        } else {
            haken_midi_in.reset();
            haken_midi_out.output.reset();
            haken_midi_out.output.channel = -1;
            log_message("Core", "--- disconnect HAKEN");
        }
        em.reset();
        init_osmose();
        reset_tasks();
        notify_connection_changed(ChemDevice::Haken, source->connection);
    } break;

    case ChemDevice::Midi1:
        if (!disconnected && source->connection) {
            log_message("Core", format_string("+++ connect MIDI 1 %s", source->connection->info.friendly(TextFormatLength::Short).c_str()));
            bool is_em = is_EMDevice(source->connection->info.input_device_name);
            controller1_midi_in.set_channel_reflect(is_em);
            getParam(P_C1_CHANNEL_MAP).setValue(controller1_midi_in.channel_reflect);
            controller1_midi_in.set_music_pass(is_em);
            getParam(P_C1_MUSIC_FILTER).setValue(controller1_midi_in.music_pass_filter);
        } else {
            log_message("Core", "--- disconnect Midi 1");
        }
        HandleMidiDeviceChange(&controller1_midi_in, source);
        notify_connection_changed(ChemDevice::Midi1, source->connection);
        break;

    case ChemDevice::Midi2:
        if (!disconnected && source->connection) {
            log_message("Core", format_string("+++ connect MIDI 2 %s", source->connection->info.friendly(TextFormatLength::Short).c_str()));
            bool is_em = is_EMDevice(source->connection->info.input_device_name);
            controller2_midi_in.set_channel_reflect(is_em);
            getParam(P_C2_CHANNEL_MAP).setValue(controller2_midi_in.channel_reflect);
            controller2_midi_in.set_music_pass(is_em);
            getParam(P_C2_MUSIC_FILTER).setValue(controller2_midi_in.music_pass_filter);
        } else {
            log_message("Core", "--- disconnect Midi 2");
        }
        HandleMidiDeviceChange(&controller2_midi_in, source);
        notify_connection_changed(ChemDevice::Midi2, source->connection);
        break;

    case ChemDevice::Unknown:
        break;
    }
}

void CoreModule::onRemove(const RemoveEvent &e)
{
    for (auto client : chem_clients) {
        client->onConnectHost(nullptr);
    }
}

void CoreModule::onReset(const ResetEvent &e)
{
    log_message("Core", "onReset");
    haken_device.clear();
    controller1.clear();
    controller2.clear();
    reboot();
}

void CoreModule::dataFromJson(json_t *root)
{
    ChemModule::dataFromJson(root);
    haken_device.set_claim(get_json_string(root, "haken-device"));
    saved_hardware = get_json_int(root, "hardware", 0);
    controller1.set_claim(get_json_string(root, "controller-1"));
    controller2.set_claim(get_json_string(root, "controller-2"));
    enable_logging(get_json_bool(root, "log-midi", false));
    json_read_bool(root, "glow-knobs", glow_knobs);
}

json_t* CoreModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(haken_device.get_claim().c_str()));
    json_object_set_new(root, "hardware", json_integer(em.get_hardware()));
    json_object_set_new(root, "controller-1", json_string(controller1.get_claim().c_str()));
    json_object_set_new(root, "controller-2", json_string(controller2.get_claim().c_str()));
    json_object_set_new(root, "log-midi", json_boolean(is_logging()));
    return root;
}

void CoreModule::update_from_em()
{
    if (disconnected) return;
    if (chem_host && chem_host->host_preset()) {
        auto em = chem_host->host_matrix();
        if (!em) return;
        EmControlPort& port = modulation.get_port(0);
        if (Haken::ccPost == port.cc_id) {
            modulation.set_em_and_param(0, em->get_post(), true);
        } else {
            assert(Haken::ccAtten == port.cc_id);
            modulation.set_em_and_param_low(0, em->get_attenuation(), true);
        }
    }
}

void CoreModule::connect_midi(bool on)
{
    if (disconnected == !on) return;
    disconnected = !on;
    if (disconnected) {
        haken_device.connect(nullptr);
        haken_midi_in.clear();
        haken_midi_in.enable(false);
        haken_midi_out.clear();
        haken_midi_out.enable(false);
        controller1_midi_in.clear();
        controller1_midi_in.enable(false);
        controller2_midi_in.clear();
        controller2_midi_in.enable(false);
    } else {
        reboot();
    }
}

void CoreModule::init_osmose()
{
    bool osmose = em.is_osmose() ? true : is_osmose(haken_device.get_claim());
    haken_midi.osmose_target = osmose;

    EmControlPort& port = modulation.get_port(0);
    if (osmose) {
        port.cc_id = Haken::ccPost;
        port.low_resolution = false;
    } else {
        port.cc_id = Haken::ccAtten;
        port.low_resolution = true;
    }
}

void CoreModule::do_message(PackedMidiMessage message)
{
    if (Haken::ccStat1 != message.bytes.status_byte) return;
    if (as_u8(ChemId::Core) == midi_tag(message)) return;

    auto cc = midi_cc(message);
    switch (cc) {
    case Haken::ccAtten: {
        EmControlPort& port = modulation.get_port(0);
        if (port.cc_id == cc) {
            modulation.set_em_and_param_low(0, midi_cc_value(message), true);
        }
    } break;

    case Haken::ccPost: {
        EmControlPort& port = modulation.get_port(0);
        if (port.cc_id == cc) {
            modulation.set_em_and_param(0, em.get_post(), true);
        }
    } break;
    }
}

void CoreModule::register_chem_client(IChemClient* client)
{
    if (chem_clients.cend() == std::find(chem_clients.cbegin(), chem_clients.cend(), client)) {
        chem_clients.push_back(client);
        client->onConnectHost(this);
        auto do_midi = client->client_do_midi();
        if (do_midi) {
            midi_relay.register_target(do_midi);
        }
    }
}

void CoreModule::unregister_chem_client(IChemClient* client)
{
    auto item = std::find(chem_clients.cbegin(), chem_clients.cend(), client);
    if (item != chem_clients.cend())
    {
        auto do_midi = client->client_do_midi();
        if (do_midi) {
            midi_relay.unregister_target(do_midi);
        }
        chem_clients.erase(item);
    }
}

bool CoreModule::host_has_client_model(IChemClient* target)
{
    auto target_model = target->client_module()->getModel();
    for (auto client : chem_clients) {
        auto client_model = client->client_module()->getModel();
        if (target_model == client_model) {
            return true;
        }
    }
    return false;
}

bool CoreModule::host_has_client(IChemClient* client)
{
    return chem_clients.cend() != std::find(chem_clients.cbegin(), chem_clients.cend(), client);
}

std::shared_ptr<MidiDeviceConnection> CoreModule::host_connection(ChemDevice device)
{
    switch (device)
    {
    case ChemDevice::Haken: return haken_device.connection; break;
    case ChemDevice::Midi1: return controller1.connection; break;
    case ChemDevice::Midi2: return controller2.connection; break;
    default: return nullptr;
    }
}

constexpr const uint64_t PROCESS_LIGHT_INTERVAL = 48;
constexpr const uint64_t PROCESS_PARAM_INTERVAL = 47;

void CoreModule::processLights(const ProcessArgs &args)
{
    getLight(L_READY).setBrightnessSmooth(host_busy() ? 0.f : 1.0f, args.sampleTime * 20);

    octave.set_shift(em.get_octave_shift());
    octave.update_lights(this, Lights::L_OCT_SHIFT_FIRST);

    round_leds.set_initial(em.is_round_initial());
    round_leds.set_mode(em.get_round_mode());
    round_leds.set_rate(em.get_round_rate());
    round_leds.update_lights(this, Lights::L_ROUND);
    getLight(L_C1_MUSIC_FILTER).setBrightnessSmooth(getParam(P_C1_MUSIC_FILTER).getValue(), 45.f);
    getLight(L_C2_MUSIC_FILTER).setBrightnessSmooth(getParam(P_C2_MUSIC_FILTER).getValue(), 45.f);
    getLight(L_C1_MUTE).setBrightnessSmooth(getParam(P_C1_MUTE).getValue(), 45.f);
    getLight(L_C2_MUTE).setBrightnessSmooth(getParam(P_C2_MUTE).getValue(), 45.f);
    getLight(L_C1_CHANNEL_MAP).setBrightnessSmooth(getParam(P_C1_CHANNEL_MAP).getValue(), 45.f);
    getLight(L_C2_CHANNEL_MAP).setBrightnessSmooth(getParam(P_C2_CHANNEL_MAP).getValue(), 45.f);
}

void CoreModule::process_params(const ProcessArgs &args)
{
    if (is_controller_1_connected()) {
        controller1_midi_in.set_channel_reflect(getParamInt(getParam(P_C1_CHANNEL_MAP)));
        controller1_midi_in.set_music_pass(getParamInt(getParam(P_C1_MUSIC_FILTER)));
        controller1_midi_in.enable(0 == getParamInt(getParam(P_C1_MUTE)));
    } else {
        controller1_midi_in.set_channel_reflect(false);
        controller1_midi_in.set_music_pass(false);
        controller1_midi_in.enable(true);
        getParam(P_C1_CHANNEL_MAP).setValue(0.f);
        getParam(P_C1_MUSIC_FILTER).setValue(0.f);
        getParam(P_C1_MUTE).setValue(0.f);
    }

    if (is_controller_2_connected()) {
        controller2_midi_in.set_channel_reflect(getParamInt(getParam(P_C2_CHANNEL_MAP)));
        controller2_midi_in.set_music_pass(getParamInt(getParam(P_C2_MUSIC_FILTER)));
        controller2_midi_in.enable(0 == getParamInt(getParam(P_C2_MUTE)));
    } else {
        controller2_midi_in.set_channel_reflect(false);
        controller2_midi_in.set_music_pass(false);
        controller2_midi_in.enable(true);
        getParam(P_C2_CHANNEL_MAP).setValue(0.f);
        getParam(P_C2_MUSIC_FILTER).setValue(0.f);
        getParam(P_C2_MUTE).setValue(0.f);
    }
}

void CoreModule::process_gather(const ProcessArgs &args)
{
    if (!gathering) return;

    if (stop_scan) {
        end_scan();
        return;
    }

    if (id_builder) {
        if (id_builder->finished()) {
            em.unsubscribeEMEvents(id_builder);
            delete id_builder;
            id_builder = nullptr;
            full_build->start_building();
        }
        return;
    } 

    if (full_build) {
        using PHASE = PresetListBuildCoordinator::Phase;
        if (chem_ui && (PHASE::Start == full_build->phase)) {
            ui()->em_status_label->text(format_string("Scanning %s", full_build->iter->next_text().c_str()));
        }
        if (!full_build->process(&haken_midi, &em, args)) {
            switch (full_build->get_phase()) {
            case PHASE::PendBegin:
                full_build->resume();
                break;
            case PHASE::PendReceive:
                // TODO: retry list?
                full_build->resume();
                break;
            case PHASE::End:
                end_scan();
                break;
            default:
                break;
            }
        }
    }
}

void CoreModule::process(const ProcessArgs &args)
{
    //DO NOT ChemModule::process(args);

    if (0 == ((args.frame + id) % PROCESS_LIGHT_INTERVAL)) {
        processLights(args);
        if (ticker.lap()) {
            auto next = getParam(P_NOTHING).getValue() + 1.0f;
            if (next > getParamQuantity(P_NOTHING)->getMaxValue()) {
                next = 0.f;
            }
            getParam(P_NOTHING).setValue(next);
        }
    }
    if (disconnected) return;

    if (!startup_tasks.completed()) {
        startup_tasks.process(args);
        if (startup_tasks.completed()) {
            log_message("CoreStart", "----  Startup Tasks Complete  ----");
        }
    } else {
        if (!recurring_tasks.started) {
            recurring_tasks.start();
        }
        recurring_tasks.process(args);
    }

    auto sample_time = args.sampleTime;
    controller1_midi_in.dispatch(sample_time);
    controller2_midi_in.dispatch(sample_time);
    haken_midi_in.dispatch(sample_time);

    if (haken_midi_out.ring.size() > 2*(haken_midi_out.ring.capacity()/3)) {
        haken_midi_out.dispatch(DISPATCH_NOW);
    } else {
        haken_midi_out.dispatch(sample_time);
    }

    process_gather(args);

    if (modulation.sync_params_ready(args)) {
        modulation.sync_send();
    }    

    if (getInput(IN_C1_MUTE_GATE).isConnected()) {
        bool mute = getInput(IN_C1_MUTE_GATE).getVoltage() >= 0.8f;
        getParam(P_C1_MUTE).setValue(mute ? 1.f : 0.f);
        controller1_midi_in.enable(!mute);
    }

    if (getInput(IN_C2_MUTE_GATE).isConnected()) {
        bool mute = getInput(IN_C2_MUTE_GATE).getVoltage() >= 0.8f;
        getParam(P_C2_MUTE).setValue(mute ? 1.f : 0.f);
        controller2_midi_in.enable(!mute);
    }
    
    if (getOutput(OUT_READY).isConnected()) {
        getOutput(OUT_READY).setVoltage(host_busy() ? 0.0f : 10.0f);
    }
    if (0 == ((args.frame + id) % PROCESS_PARAM_INTERVAL)) {
        process_params(args);
    }

}

Model *modelCore = createModel<CoreModule, CoreModuleWidget>("chem-core");

