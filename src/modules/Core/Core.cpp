#include "Core.hpp"
#include "em/PresetId.hpp"
#include "services/json-help.hpp"
#include "services/kv-store.hpp"
#include "services/ModuleBroker.hpp"
#include "services/rack-help.hpp"

using namespace pachde;
using namespace eaganmatrix;

CoreModule::CoreModule() : modulation(this, ChemId::Core) {
    ticker.set_interval(1.0f);
    module_id = ChemId::Core;

    InitMidiRate();

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
        + EME::TaskMessage
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

    dp4(configParam(P_Y_SLEW, 0.f, 1.f, 0.f, "Y Slew", "%", 0.f, 100.f));
    dp4(configParam(P_Z_SLEW, 0.f, 1.f, 0.f, "Z Slew", "%", 0.f, 100.f));
    configSwitch(P_EXTEND_SLEW, 0.f, 1.f, 0.f, "Slew time", { "normal", "extended"});

    configInput(IN_C1_MUTE_GATE, "MIDI 1 data block gate");
    configInput(IN_C2_MUTE_GATE, "MIDI 2 data block gate");

    configLight(L_ROUND_Y,       "Round on Y");
    configLight(L_ROUND_INITIAL, "Round initial");
    configLight(L_ROUND,         "Rounding");
    configLight(L_ROUND_RELEASE, "Round on release");
    configLight(L_READY,         "Haken device ready");

    configOutput(Outputs::OUT_READY, "Ready gate");
    configOutput(Outputs::OUT_W, "W (gate)");
    configOutput(Outputs::OUT_X, "X (V/Oct)");
    configOutput(Outputs::OUT_Y, "Y");
    configOutput(Outputs::OUT_Z, "Z");

    startup_tasks.init(this);
    recurring_tasks.init(this);
    ModuleBroker::get()->register_host(this);

    midi_relay.set_em(&em);
    midi_relay.register_target(&haken_midi_out);

    em.subscribeEMEvents(this);
    mm_to_cv.em = &em;

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
    user_presets = std::make_shared<PresetList>();
    load_pfis(pfis_filename(), user_preset_file_infos);
}

CoreModule::~CoreModule() {
    midi_relay.unregister_target(this);
    controller1_midi_in.clear();
    controller2_midi_in.clear();
    controller1.unsubscribe(this);
    controller2.unsubscribe(this);

    user_presets = nullptr;
    system_presets = nullptr;
    notify_preset_list_changed(PresetTab::User);
    notify_preset_list_changed(PresetTab::System);
    for (auto client : chem_clients) {
        client->onConnectHost(nullptr);
    }
    ModuleBroker::get()->unregister_host(this);
    auto broker = MidiDeviceBroker::get();
    broker->unRegisterDeviceHolder(&haken_device);
    broker->unRegisterDeviceHolder(&controller1);
    broker->unRegisterDeviceHolder(&controller2);

    save_pfis(pfis_filename(), user_preset_file_infos);
}

void CoreModule::enable_logging(bool enable) {
    if (enable){
        midi_log = new MidiLog();
        LOG_MSG("Core", format_string("Starting CHEM Core [%p] %s", this, string::formatTimeISO(system::getUnixTime()).c_str()));
        haken_midi.set_logger(midi_log);
        haken_midi_out.set_logger(midi_log);
        haken_midi_in.set_logger("<<H", midi_log);
        controller1_midi_in.set_logger("<C1", midi_log);
        controller2_midi_in.set_logger("<C2", midi_log);
        em.log = midi_log;
    } else {
        haken_midi.set_logger(nullptr);
        haken_midi_out.set_logger(nullptr);
        haken_midi_in.set_logger("", nullptr);
        controller1_midi_in.set_logger("", nullptr);
        controller2_midi_in.set_logger("", nullptr);
        em.log = nullptr;
        if (midi_log) {
            midi_log->close();
            delete midi_log;
            midi_log = nullptr;
        }
    }
}

std::string CoreModule::device_name(ChemDevice which) {
    switch (which)
    {
    case ChemDevice::Haken: return haken_device.device_name();
    case ChemDevice::Midi1: return controller1.device_name();
    case ChemDevice::Midi2: return controller2.device_name();
    default: return "";
    }
}


PresetId CoreModule::prev_next_id(ssize_t increment) {
    ssize_t index{-1};
    PresetId id;
    if (em.preset.id.valid() && em.preset.id.key()) {
        index = system_presets->index_of_id(em.preset.id);
        if (index >= 0) {
            index = index + increment;
            index = (index < 0)
                ? system_presets->size() -1
                : ((index >= system_presets->size()) ? 0 : index);
            id = system_presets->presets[index]->id;
        } else if (user_presets) {
            auto index = user_presets->index_of_id(em.preset.id);
            if (index >= 0) {
                index = index + increment;
                index = (index < 0)
                    ? user_presets->size() -1
                    : ((index >= user_presets->size()) ? 0 : index);
                id = user_presets->presets[index]->id;
            }
        }
    } else if (em.preset.tag) {
        index = system_presets->index_of_tag(em.preset.tag);
        if (index >= 0) {
            index = index + increment;
            index = (index < 0)
                ? system_presets->size() -1
                : ((index >= system_presets->size()) ? 0 : index);
            id = system_presets->presets[index]->id;
        } else if (user_presets) {
            auto index = user_presets->index_of_tag(em.preset.tag);
            if (index >= 0) {
                index = index + increment;
                index = (index < 0)
                    ? user_presets->size() -1
                    : ((index >= user_presets->size()) ? 0 : index);
                id = user_presets->presets[index]->id;
            }
        }
    }
    return id;
}

void CoreModule::next_preset() {
    load_lists();
    if (em.is_osmose()) {
        PresetId id = prev_next_id(1);
        if (id.valid()) {
            request_preset(ChemId::Core, id);
        }
    } else {
        haken_midi.next_system_preset(ChemId::Core);
    }
}

void CoreModule::prev_preset() {
    load_lists();
    if (em.is_osmose()) {
        PresetId id = prev_next_id(-1);
        if (id.valid()) {
            request_preset(ChemId::Core, id);
        }
    } else {
        haken_midi.previous_system_preset(ChemId::Core);
    }
}

void CoreModule::clear_presets(eaganmatrix::PresetTab which) {
    valid_tab(which);
    auto list = which == PresetTab::User ? user_presets : system_presets;
    std::string path = list->filename;

    if ((PresetTab::User == which) && !path.empty() && !user_preset_file_infos.empty()) {
        auto it = std::find_if(user_preset_file_infos.begin(), user_preset_file_infos.end(),
            [path](std::shared_ptr<PresetFileInfo> pfi){
                return 0 == path.compare(pfi->file);
            });
        if (it != user_preset_file_infos.end()) {
            user_preset_file_infos.erase(it);
        }
    }
    list->clear();
    if (!path.empty()) {
        system::remove(path);
    }
    notify_preset_list_changed(which);
}

PresetResult CoreModule::load_preset_file(PresetTab which, bool busy_load) {
    valid_tab(which);
    if (!busy_load && host_busy()) return PresetResult::NotReady;
    if (!haken_device.connection || !haken_device.connection->identified()) return PresetResult::NotReady;
    auto hardware = em.get_hardware();
    if (!hardware) return PresetResult::NotReady;
    std::string path;
    if ((PresetTab::User == which) && !user_preset_file_infos.empty()) {
        auto conn = haken_device.get_claim();
        auto it = std::find_if(user_preset_file_infos.begin(), user_preset_file_infos.end(), [conn](std::shared_ptr<PresetFileInfo> pfi){
            return 0 == conn.compare(pfi->connection);
        });
        if (it != user_preset_file_infos.end()) {
            path = (*it)->file;
        }
    }
    if (path.empty()) {
        path = preset_file_name(which, hardware, haken_device.connection->info.input_device_name);
    }
    auto list = which == PresetTab::User ? user_presets : system_presets;
    auto result = list->load(path) ? PresetResult::Ok : PresetResult::FileNotFound;
    if (result == PresetResult::Ok) {
        notify_preset_list_changed(which);
    }
    return result;
}

PresetResult CoreModule::load_quick_user_presets() {
    if (em.is_osmose()) return PresetResult::NotApplicableOsmose;
    if (host_busy()) return PresetResult::NotReady;

    if (chem_ui && !ui()->showing_busy()) {
        ui()->show_busy(true);
        ui()->em_status_label->text("Scanning quick User presets...");
    }
    gathering = QuickUserPresets;
    stash_user_preset_file = user_presets->filename;
    user_presets->clear();
    haken_midi.request_user(ChemId::Core);
    return PresetResult::Ok;
}

PresetResult CoreModule::load_quick_system_presets() {
    if (em.is_osmose()) return PresetResult::NotApplicableOsmose;
    if (host_busy()) return PresetResult::NotReady;
    if (chem_ui && !ui()->showing_busy()) {
        ui()->show_busy(true);
    }
    gathering = QuickSystemPresets;
    haken_midi.request_system(ChemId::Core);
    return PresetResult::Ok;
}

PresetResult CoreModule::load_full_user_presets() {
    if (host_busy()) return PresetResult::NotReady;
    stash_user_preset_file = user_presets->filename;
    user_presets->clear();

    if (chem_ui && !ui()->showing_busy()) {
        ui()->show_busy(true);
        ui()->create_stop_button();
        ui()->em_status_label->text("Scanning full User presets...");
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

PresetResult CoreModule::scan_osmose_presets(uint8_t page) {
    if (!em.is_osmose()) return PresetResult::NotApplicableEm;
    if (host_busy()) return PresetResult::NotReady;
    if (chem_ui && !ui()->showing_busy()) {
        ui()->show_busy(true);
        ui()->create_stop_button();
        ui()->em_status_label->text(format_string("Scanning User presets (page %d)...", page-90+1));
    }
    em.begin_user_scan();
    full_build = new PresetListBuildCoordinator(midi_log, true, new OsmosePresetEnumerator(ChemId::Core, page));
    full_build->start_building();
    gathering = FullUserPresets;
    return PresetResult::Ok;
}

void CoreModule::notify_preset_list_changed(eaganmatrix::PresetTab which) {
    for (auto client: preset_list_clients) {
        client->on_list_changed(which);
    }
}

void CoreModule::update_user_preset_file_infos() {
    if (!haken_device.connection) return;
    auto hardware = em.get_hardware();
    if (!hardware) return;
    auto default_path = preset_file_name(PresetTab::User, hardware, haken_device.connection->info.input_device_name);

    auto conn = host_claim();
    auto it = std::find_if(user_preset_file_infos.begin(), user_preset_file_infos.end(), [conn](std::shared_ptr<PresetFileInfo> pfi) {
        return 0 == conn.compare(pfi->connection);
    });
    if (it == user_preset_file_infos.end()) {
        if (0 != default_path.compare(user_presets->filename)) {
            user_preset_file_infos.push_back(std::make_shared<PresetFileInfo>(hardware, conn, user_presets->filename));
        }
    } else {
        if (0 == default_path.compare(user_presets->filename)) {
            user_preset_file_infos.erase(it);
        } else {
            (*it)->file = user_presets->filename;
        }
    }
}

void CoreModule::register_preset_list_client(IPresetListClient *client) {
    if (preset_list_clients.cend() == std::find(preset_list_clients.cbegin(), preset_list_clients.cend(), client)) {
        preset_list_clients.push_back(client);
    }
}

void CoreModule::unregister_preset_list_client(IPresetListClient *client) {
    auto cit = std::find(preset_list_clients.begin(), preset_list_clients.end(), client);
    if (cit != preset_list_clients.end()) {
        preset_list_clients.erase(cit);
    }
}

std::shared_ptr<PresetList> CoreModule::host_user_presets() {
    if (user_presets && user_presets->empty()) {
        load_preset_file(PresetTab::User);
    }
    return user_presets;
}

std::shared_ptr<PresetList> CoreModule::host_system_presets() {
    if (system_presets && system_presets->empty()) {
        load_preset_file(PresetTab::System);
    }
    return system_presets;
}

PresetResult CoreModule::load_full_system_presets() {
    if (host_busy()) return PresetResult::NotReady;
    system_presets->clear();

    if (chem_ui && !ui()->showing_busy()) {
        ui()->show_busy(true);
        ui()->create_stop_button();
        ui()->em_status_label->text("Scanning Full System presets...");
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

PresetResult CoreModule::end_scan() {
    if (!gathering) return PresetResult::NotApplicable;
    auto gather = gathering;
    gathering = GatherFlags::None;
    LOG_MSG("PLB", format_string("Completed in %.6f", full_build->total));

    PresetTab tab{PresetTab::Unset};
    MidiDeviceConnectionInfo info;
    info.parse(haken_device.device_claim);
    if (gather_user(gather)) {
        em.end_user_scan();
        if (stash_user_preset_file.empty()) {
            user_presets->save(preset_file_name(PresetTab::User, em.get_hardware(), info.input_device_name), em.get_hardware());
        } else {
            user_presets->save(stash_user_preset_file, em.get_hardware());
            stash_user_preset_file = "";
        }
        tab = PresetTab::User;
    } else {
        assert(gather_system(gather));
        em.end_system_scan();
        system_presets->sort(PresetOrder::Alpha);
        system_presets->save(preset_file_name(PresetTab::System, em.get_hardware(), info.input_device_name), em.get_hardware());
        tab = PresetTab::System;
    }
    if (chem_ui) {
        ui()->show_busy(false);
        ui()->remove_stop_button();
    }
    auto fb = full_build;
    full_build = nullptr;
    delete fb;
    stop_scan = false;
    notify_preset_list_changed(tab);
    return PresetResult::Ok;
}

void CoreModule::load_lists() {
    if (host_busy()) return;
    if (!em.get_hardware()) return;
    if (system_presets->empty()) {
        if (PresetResult::Ok == load_preset_file(PresetTab::System)) {
            notify_preset_list_changed(PresetTab::System);
        }
    }
    if (user_presets->empty()) {
        if (PresetResult::Ok == load_preset_file(PresetTab::User)) {
            notify_preset_list_changed(PresetTab::User);
        }
    }
}

void CoreModule::reset_tasks() {
    recurring_tasks.reset();
    startup_tasks.reset();
    for (size_t i = 0; i < sizeof(start_states)/sizeof(start_states[0]); ++i) {
        start_states[i] = ChemTask::State::Untried;
    }
}

void CoreModule::reboot() {
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
void CoreModule::onEditorReply(uint8_t reply) {
    if (!startup_tasks.completed()) {
        startup_tasks.heartbeat_received();
    } else {
        recurring_tasks.heartbeat_received();
    }
}

void CoreModule::onTaskMessage(uint8_t code) {
    if (Haken::tenSecsOld == code) {
        reset_tasks();
    }
}

void CoreModule::onHardwareChanged(uint8_t hardware, uint16_t firmware_version) {
    init_osmose();
}

void CoreModule::onPresetBegin() {
    if (gathering && full_build) {
        if (PresetListBuildCoordinator::Phase::PendBegin == full_build->phase) {
            full_build->preset_started();
        }
    }
}

void CoreModule::onPresetChanged() {
    LOG_MSG("Core", format_string("--- Received Preset Changed: %s", em.preset.summary().c_str()));
    in_preset_request = false;

    if (!em.preset.empty()) {
        if (!startup_tasks.completed()) {
            startup_tasks.configuration_received();
        } else if (gathering) {
            if (gather_user(gathering)) {
                if (gather_full(gathering) && !id_builder) {
                    assert(gather_presets(gathering));
                    if (em.preset.id.key() != full_build->iter->expected_id().key()) {
                        LOG_MSG("PLB", format_string("[MISMATCH] em[%6x] plb[%6x]", em.osmose_id.key(), em.preset.id.key(), full_build->iter->expected_id().key()));
                    } else {
                        full_build->preset_received();
                        user_presets->add(&em.preset);
                    }
                } else if (gather_quick(gathering)) {
                    assert(!em.is_osmose());
                    user_presets->add(&em.preset);
                }
            } else if (gather_system(gathering)) {
                if (gather_full(gathering) && !id_builder) {
                    assert(gather_presets(gathering));
                    if (em.preset.id.key() != full_build->iter->expected_id().key()) {
                        LOG_MSG("PLB", format_string("[MISMATCH] em[%6x] plb[%6x]", em.osmose_id.key(), em.preset.id.key(), full_build->iter->expected_id().key()));
                    } else {
                        full_build->preset_received();
                        system_presets->add(&em.preset);
                    }
                } else if (gather_quick(gathering)) {
                    assert(!em.is_osmose());
                    system_presets->add(&em.preset);
                }
            } else {
                assert(false);
            }
        }
    }

    // patch preset id for Osmose
    if (!gathering && (em.preset.id.key() == 0) && em.is_osmose() && em.preset.valid_tag()) {
        bool found{false};
        if (user_presets->empty()) {
            load_preset_file(PresetTab::User, true);
        }
        if (!user_presets->empty()) {
            auto index = user_presets->index_of_tag(em.preset.tag);
            if (index >= 0) {
                em.preset.id = user_presets->presets[index]->id;
                found = true;
            }
        }
        if (!found) {
            if (system_presets->empty()) {
                load_preset_file(PresetTab::System, true);
            }
            auto index = system_presets->index_of_tag(em.preset.tag);
            if (index >= 0) {
                em.preset.id = system_presets->presets[index]->id;
            }
        }
    }

    update_from_em();
    if (startup_tasks.completed() && !gathering) {
        notify_preset_changed();
    }
}

void CoreModule::onUserBegin() {
    is_busy = true;
    if (chem_ui) ui()->show_busy(is_busy);
}

void CoreModule::onUserComplete() {
    is_busy = false;
    if (chem_ui) {
        ui()->show_busy(false);
        ui()->remove_stop_button();
    }
    if (QuickUserPresets == gathering) {
        MidiDeviceConnectionInfo info;
        info.parse(haken_device.device_claim);
        if (stash_user_preset_file.empty()) {
            user_presets->save(preset_file_name(PresetTab::User, em.get_hardware(), info.input_device_name), em.get_hardware());
        } else {
            user_presets->save(stash_user_preset_file, em.get_hardware());
            stash_user_preset_file = "";
        }
        gathering = GatherFlags::None;
    }
}

void CoreModule::onSystemBegin() {
    is_busy = true;
    if (chem_ui) ui()->show_busy(true);
}

void CoreModule::onSystemComplete() {
    is_busy = false;
    if (chem_ui) {
        ui()->show_busy(false);
        ui()->remove_stop_button();
    }
    if (QuickSystemPresets == gathering) {
        MidiDeviceConnectionInfo info;
        info.parse(haken_device.device_claim);
        system_presets->save(preset_file_name(PresetTab::System, em.get_hardware(), info.input_device_name), em.get_hardware());
        gathering = GatherFlags::None;
    }
}

void CoreModule::onMahlingBegin() {
    is_busy = true;
    if (chem_ui) ui()->show_busy(is_busy);
}

void CoreModule::onMahlingComplete() {
    is_busy = false;
    if (chem_ui) ui()->show_busy(is_busy);
}

void CoreModule::request_preset(ChemId tag, PresetId id) {
    if (host_busy()) return;
    in_preset_request = true;
    em.set_osmose_id(id);
    haken_midi.select_preset(tag, id);
}

// IChemHost
void CoreModule::notify_connection_changed(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) {
    for (auto client : chem_clients) {
        client->onConnectionChange(device, connection);
    }
}

void CoreModule::notify_preset_changed() {
    for (auto client : chem_clients) {
        client->onPresetChange();
    }
}

void HandleMidiDeviceChange(MidiInput* input, const MidiDeviceHolder* source) {
    input->ring.clear();
    if (source->connection) {
        input->setDriverId(source->connection->driver_id);
        input->setDeviceId(source->connection->input_device_id);
    } else {
        input->reset();
    }
}

void CoreModule::onMidiDeviceChange(const MidiDeviceHolder* source) {
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
            LOG_MSG("Core", format_string("+++ connect HAKEN %s", source->connection->info.friendly(NameFormat::Short).c_str()).c_str());
        } else {
            haken_midi_in.reset();
            haken_midi_out.output.reset();
            haken_midi_out.output.channel = -1;
            LOG_MSG("Core", "--- disconnect HAKEN");
        }
        em.reset();
        init_osmose();
        reset_tasks();
        notify_connection_changed(ChemDevice::Haken, source->connection);
    } break;

    case ChemDevice::Midi1:
        if (!disconnected && source->connection) {
            LOG_MSG("Core", format_string("+++ connect MIDI 1 %s", source->connection->info.friendly(NameFormat::Short).c_str()));
            bool is_em = is_EMDevice(source->connection->info.input_device_name);
            controller1_midi_in.set_channel_reflect(is_em);
            getParam(P_C1_CHANNEL_MAP).setValue(controller1_midi_in.channel_reflect);
            controller1_midi_in.set_music_pass(is_em);
            getParam(P_C1_MUSIC_FILTER).setValue(controller1_midi_in.music_pass_filter);
        } else {
            LOG_MSG("Core", "--- disconnect Midi 1");
        }
        HandleMidiDeviceChange(&controller1_midi_in, source);
        notify_connection_changed(ChemDevice::Midi1, source->connection);
        break;

    case ChemDevice::Midi2:
        if (!disconnected && source->connection) {
            LOG_MSG("Core", format_string("+++ connect MIDI 2 %s", source->connection->info.friendly(NameFormat::Short).c_str()));
            bool is_em = is_EMDevice(source->connection->info.input_device_name);
            controller2_midi_in.set_channel_reflect(is_em);
            getParam(P_C2_CHANNEL_MAP).setValue(controller2_midi_in.channel_reflect);
            controller2_midi_in.set_music_pass(is_em);
            getParam(P_C2_MUSIC_FILTER).setValue(controller2_midi_in.music_pass_filter);
        } else {
            LOG_MSG("Core", "--- disconnect Midi 2");
        }
        HandleMidiDeviceChange(&controller2_midi_in, source);
        notify_connection_changed(ChemDevice::Midi2, source->connection);
        break;

    case ChemDevice::Unknown:
        break;
    }
}

void CoreModule::onRemove(const RemoveEvent &e) {
    for (auto client : chem_clients) {
        client->onConnectHost(nullptr);
    }
    chem_clients.clear();
}

void CoreModule::onReset(const ResetEvent &e) {
    LOG_MSG("Core", "onReset");
    haken_device.clear();
    controller1.clear();
    controller2.clear();
    reboot();
}

void CoreModule::onRandomize(const RandomizeEvent &e) {
    if (host_busy()) return;
    if (!user_presets && !system_presets) return;
    if (user_presets->empty() && system_presets->empty()) return;

    bool user = ::rack::random::uniform() < 0.5f;
    auto list = user ? user_presets : system_presets;
    if (!list || list->empty()) list = user ? system_presets : user_presets;
    if (list->empty()) return;
    auto index = std::round(::rack::random::uniform() * (list->size() - 1));
    request_preset(ChemId::Core, list->presets[index]->id);
}

void CoreModule::dataFromJson(json_t *root) {
    ChemModule::dataFromJson(root);
    haken_device.set_claim(get_json_string(root, "haken-device"));
    controller1.set_claim(get_json_string(root, "controller-1"));
    controller2.set_claim(get_json_string(root, "controller-2"));
    enable_logging(get_json_bool(root, "log-midi", false));
    glow_knobs = get_json_bool(root, "glow-knobs", glow_knobs);
    mm_to_cv.zero_xyz = get_json_bool(root, "zero-xyz", mm_to_cv.zero_xyz);
}

json_t* CoreModule::dataToJson() {
    json_t* root = ChemModule::dataToJson();
    set_json(root, "haken-device", haken_device.get_claim());
    set_json(root, "controller-1", controller1.get_claim());
    set_json(root, "controller-2", controller2.get_claim());
    set_json(root, "log-midi", is_logging());
    set_json(root, "glow-knobs", glow_knobs);
    set_json(root, "zero-xyz", mm_to_cv.zero_xyz);
    return root;
}

void CoreModule::update_from_em() {
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

void CoreModule::connect_midi(bool on) {
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

void CoreModule::init_osmose() {
    bool osmose = em.is_osmose() ? true : is_osmose_name(haken_device.get_claim());
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

void CoreModule::do_message(PackedMidiMessage message) {
    if (as_u8(ChemId::Core) == midi_tag(message)) return;

    mm_to_cv.do_message(message);

    if (Haken::ccStat1 != message.bytes.status_byte) return;
    if (host_busy()) return;

    uint8_t cc = midi_cc(message);
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

void CoreModule::register_chem_client(IChemClient* client) {
    if (chem_clients.cend() == std::find(chem_clients.cbegin(), chem_clients.cend(), client)) {
        chem_clients.push_back(client);
        client->onConnectHost(this);
        auto do_midi = client->client_do_midi();
        if (do_midi) {
            midi_relay.register_target(do_midi);
        }
    }
}

void CoreModule::unregister_chem_client(IChemClient* client) {
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

bool CoreModule::host_has_client_model(IChemClient* target) {
    auto target_model = target->client_module()->getModel();
    for (auto client : chem_clients) {
        auto client_model = client->client_module()->getModel();
        if (target_model == client_model) {
            return true;
        }
    }
    return false;
}

bool CoreModule::host_has_client(IChemClient* client) {
    return chem_clients.cend() != std::find(chem_clients.cbegin(), chem_clients.cend(), client);
}

std::shared_ptr<MidiDeviceConnection> CoreModule::host_connection(ChemDevice device) {
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

void CoreModule::processLights(const ProcessArgs &args) {
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

void CoreModule::onPortChange(const PortChangeEvent &e)
{
    if (e.type == Port::INPUT) return;

    if (e.connecting) {
        //++music_outs;
        getOutput(e.portId).setChannels(16);
    } else {
        //--music_outs;
        //assert(music_outs >= 0);
        getOutput(e.portId).setChannels(0);
    }
}

void CoreModule::process_params(const ProcessArgs &args)
{
    {
        float extend = getParam(P_EXTEND_SLEW).getValue();
        getLight(L_EXTEND_SLEW).setBrightness(extend);

        float slew_range = extend > .5f ? 1. : 10.;

        float p = getParam(P_Y_SLEW).getValue();
        if (p > 0.) p = (slew_range / p);
        y_slew.configure(p, p);

        p = getParam(P_Z_SLEW).getValue();
        if (p > 0.) p = (slew_range / p);
        z_slew.configure(p, p);
    }

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

void CoreModule::process_gather(const ProcessArgs &args) {
    if (!gathering) return;

    if (stop_scan) {
        end_scan();
        return;
    }

    if (id_builder) {
        if (id_builder->finished()) {
            auto t_builder = id_builder;
            id_builder = nullptr;
            em.unsubscribeEMEvents(t_builder);
            delete t_builder;
            ui()->em_status_label->text("Starting full scan...");
            full_build->start_building();
        } else if (id_builder->end_received && (-1.f == id_builder->end_time)) {
            id_builder->end_time = 0.f;
        } else if (id_builder->end_time >= 0.f) {
            id_builder->end_time += args.sampleTime;
            if (id_builder->end_time > 5.f) {
                id_builder->complete = true;
            }
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

void CoreModule::process(const ProcessArgs &args) {
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
            LOG_MSG("CoreStart", "----  Startup Tasks Complete  ----");
            notify_preset_changed();
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
    haken_midi_in.dispatch(0.f);

    if (getOutput(OUT_W).isConnected()) {
        for (uint8_t channel = 0; channel < 16; channel++) {
            getOutput(OUT_W).setVoltage(10.f * mm_to_cv.w[channel], channel);
        }
    }

    if (getOutput(OUT_X).isConnected()) {
        for (uint8_t channel = 0; channel < 16; channel++) {
            uint8_t nn = mm_to_cv.nn[channel];
            float v = nn ? ((double)nn + mm_to_cv.bend[channel] - 60.0) / 12.0 : 0.f;
            getOutput(OUT_X).setVoltage(v, channel);
        }
    }

    if (getOutput(OUT_Y).isConnected()) {
        if (y_slew.slewing()) {
            for (uint8_t channel = 0; channel < 16; channel++) {
                float sample(unipolar_14_to_rack(mm_to_cv.y[channel]));
                float last{getOutput(OUT_Y).getVoltage(channel)};
                getOutput(OUT_Y).setVoltage(y_slew.next(sample, last, args.sampleTime), channel);
            }
        } else {
            for (uint8_t channel = 0; channel < 16; channel++) {
                getOutput(OUT_Y).setVoltage(unipolar_14_to_rack(mm_to_cv.y[channel]), channel);
            }
        }
    }

    if (getOutput(OUT_Z).isConnected()) {
        if (z_slew.slewing()) {
            for (uint8_t channel = 0; channel < 16; channel++) {
                float sample {unipolar_14_to_rack(mm_to_cv.z[channel])};
                float prev {getOutput(OUT_Z).getVoltage(channel)};
                getOutput(OUT_Z).setVoltage(z_slew.next(sample, prev, args.sampleTime), channel);
            }
        } else {
            for (uint8_t channel = 0; channel < 16; channel++) {
                getOutput(OUT_Z).setVoltage(unipolar_14_to_rack(mm_to_cv.z[channel]), channel);
            }
        }
    }

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

