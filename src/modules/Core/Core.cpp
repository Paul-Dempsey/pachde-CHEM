#include "Core.hpp"
#include "../../em/PresetId.hpp"
#include "../../services/ModuleBroker.hpp"

using namespace pachde;

using EME = IHandleEmEvents::EventMask;

CoreModule::CoreModule() :
    is_busy(false),
    in_reboot(false),
    heartbeat(false),
    restore_last_preset(false)
{
    em_event_mask = EME::LoopDetect
        + EME::EditorReply 
        + EME::PresetChanged
        + EME::UserBegin
        + EME::UserComplete
        + EME::SystemBegin
        + EME::SystemComplete;

    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configSwitch(Params::P_C1_MUSIC_FILTER, 0.f, 1.f, 0.f, "MIDI filter", { "All data", "Music data only"} );
    configSwitch(Params::P_C2_MUSIC_FILTER, 0.f, 1.f, 0.f, "MIDI filter", { "All data", "Music data only"} );
    configSwitch(Params::P_C1_MUTE, 0.f, 1.f, 0.f, "MIDI 1 data", { "passed", "blocked" } );
    configSwitch(Params::P_C2_MUTE, 0.f, 1.f, 0.f, "MIDI 2 data", { "passed", "blocked" } );

    configInput(IN_C1_MUTE_GATE, "MIDI 1 data block gate");
    configInput(IN_C2_MUTE_GATE, "MIDI 2 data block gate");

    configLight(L_ROUND_Y,       "Round on Y");
    configLight(L_ROUND_INITIAL, "Round initial");
    configLight(L_ROUND,         "Rounding");
    configLight(L_ROUND_RELEASE, "Round on release");
    
    configLight(L_READY,         "Haken device ready");
    configLight(L_PULSE,         "Loopback pulse");

    configOutput(Outputs::OUT_READY, "Ready gate");

    ModuleBroker::get()->register_host(this);

    midi_relay.set_em(&em);
    midi_relay.register_target(&haken_midi_out);

    em.subscribeEMEvents(this);
    tasks.setCoreModule(this);
    haken_midi.set_handler(&midi_relay);
    haken_device.subscribe(this);
    controller1.subscribe(this);
    controller2.subscribe(this);

    haken_midi_in.setTarget(&midi_relay);
    controller1_midi_in.setTarget(&midi_relay);
    controller2_midi_in.setTarget(&midi_relay);

    auto broker = MidiDeviceBroker::get();
    broker->registerDeviceHolder(&haken_device);
    broker->registerDeviceHolder(&controller1);
    broker->registerDeviceHolder(&controller2);

    tasks.subscribeChange(this);
}

CoreModule::~CoreModule() {
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
        }
    }
}

ChemDevice CoreModule::device_identifier(const MidiDeviceHolder* holder)
{
    if (holder == &haken_device) return ChemDevice::Haken;
    if (holder == &controller1) return ChemDevice::Midi1;
    if (holder == &controller2) return ChemDevice::Midi2;
    return ChemDevice::Unknown;
}

std::string CoreModule::device_name(const MidiDeviceHolder& holder) {
    if (holder.connection) {
        return holder.connection->info.friendly(TextFormatLength::Short);
    } else if (!holder.device_claim.empty()) {
        return holder.device_claim;
    }
    return "";
}

std::string CoreModule::device_name(ChemDevice which) {
    switch (which)
    {
    case ChemDevice::Haken: return device_name(haken_device);
    case ChemDevice::Midi1: return device_name(controller1);
    case ChemDevice::Midi2: return device_name(controller2);
    default: return "";
    }
}

void CoreModule::reboot()
{
    if (in_reboot) return;
    in_reboot = true;

    controller1_midi_in.ring.clear();
    controller2_midi_in.ring.clear();
    haken_midi_in.ring.clear();
    haken_midi_out.ring.clear();

    controller1_midi_in.reset();
    controller2_midi_in.reset();
    haken_midi_in.reset();

    haken_midi_out.output.reset();
    haken_midi_out.output.channel = -1;

    em.reset();
    tasks.refresh();
    in_reboot = false;
}

void CoreModule::send_midi_rate(HakenMidiRate rate)
{
    haken_midi.midi_rate(ChemId::Core, rate);
}

void CoreModule::restore_midi_rate()
{
    if (HakenMidiRate::Full != tasks.midi_rate) {
        haken_midi.midi_rate(ChemId::Core, HakenMidiRate::Full);
        tasks.midi_rate = HakenMidiRate::Full;
    }
}

// IHandleEmEvents
void CoreModule::onLoopDetect(uint8_t cc, uint8_t value)
{
    heartbeat = !heartbeat;
    getLight(L_PULSE).setBrightness(heartbeat ? 0.f : value);
}

void CoreModule::onEditorReply(uint8_t reply)
{
    tasks.complete_task(HakenTask::HeartBeat);
}

void CoreModule::onPresetChanged()
{
    auto task = tasks.get_task(HakenTask::PresetInfo);
    if (task->pending()) {
        task->complete();
    }
    task = tasks.get_task(HakenTask::LastPreset);
    if (task->pending()) {
        task->complete();
    }
    log_message("Core", format_string("--- Received Preset Changed: %s", em.preset.summary().c_str()));
    notify_preset_changed();
}

void CoreModule::onUserBegin()
{
    is_busy = true;
}

void CoreModule::onUserComplete()
{
    is_busy = false;
}

void CoreModule::onSystemBegin()
{
    is_busy = true;
}

void CoreModule::onSystemComplete()
{
    is_busy = false;
}

void CoreModule::onHakenTaskChange(HakenTask id)
{
    if (id == HakenTask::HeartBeat) {
        auto task = tasks.get_task(id);
        if (task->waiting()) {
            heartbeat = !heartbeat;
            getLight(L_PULSE).setBrightness(heartbeat);
        }
    }
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
    auto sample_time = APP->engine->getSampleTime();
    getLight(L_PULSE).setBrightnessSmooth(0.f, 10 * sample_time);
    getLight(L_READY).setBrightnessSmooth(0.f, 20 * sample_time);

    auto id = device_identifier(source);
    switch (id) {
    case ChemDevice::Haken: {
        em.ready = false; // todo clear?
        haken_midi_in.ring.clear();
        haken_midi_out.ring.clear();
        if (source->connection) {
            haken_midi_in.setDriverId(source->connection->driver_id);
            haken_midi_in.setDeviceId(source->connection->input_device_id);
            haken_midi_out.output.setDeviceId(source->connection->output_device_id);
            haken_midi_out.output.channel = -1;

            log_message("Core", format_string("+++ connect HAKEN %s", source->connection->info.friendly(TextFormatLength::Short).c_str()).c_str());
            haken_midi.editor_present(ChemId::Core);
            haken_midi_out.dispatch(DISPATCH_NOW);
        } else {
            log_message("Core", "--- disconnect HAKEN");
            haken_midi_in.reset();
            haken_midi_out.output.reset();
            haken_midi_out.output.channel = -1;
        }
        em.reset();
        tasks.refresh();
        notify_connection_changed(ChemDevice::Haken, source->connection);
    } break;

    case ChemDevice::Midi1:
        if (source->connection) {
            log_message("Core", format_string("+++ connect MIDI 1 %s", source->connection->info.friendly(TextFormatLength::Short).c_str()));
            controller1_midi_in.setMusicPassFilter(is_EMDevice(source->connection->info.input_device_name));
            getParam(P_C1_MUSIC_FILTER).setValue(controller1_midi_in.music_pass_filter);
        } else {
            log_message("Core", "--- disconnect Midi 1");
        }
        HandleMidiDeviceChange(&controller1_midi_in, source);
        notify_connection_changed(ChemDevice::Midi1, source->connection);
        break;

    case ChemDevice::Midi2:
        if (source->connection) {
            log_message("Core", format_string("+++ connect MIDI 2 %s", source->connection->info.friendly(TextFormatLength::Short).c_str()));
            controller2_midi_in.setMusicPassFilter(is_EMDevice(source->connection->info.input_device_name));
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
    json_t* j;

    j = json_object_get(root, "haken-device");
    if (j) {
        haken_device.setClaim(json_string_value(j));
    }
    j = json_object_get(root, "controller-1");
    if (j) {
        controller1.setClaim(json_string_value(j));
    }
    j = json_object_get(root, "controller-2");
    if (j) {
        controller2.setClaim(json_string_value(j));
    }
    j = json_object_get(root, "log-midi");
    if (j) {
        enable_logging(json_boolean_value(j));
    } else {
        enable_logging(false);
    }

    j = json_object_get(root, "restore-preset");
    if (j) {
        restore_last_preset = json_boolean_value(j);
    }
    j = json_object_get(root, "last-preset");
    if (j) {
        last_preset.fromJson(j);
    }
}

json_t* CoreModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(haken_device.getClaim().c_str()));
    json_object_set_new(root, "controller-1", json_string(controller1.getClaim().c_str()));
    json_object_set_new(root, "controller-2", json_string(controller2.getClaim().c_str()));
    json_object_set_new(root, "log-midi", json_boolean(is_logging()));
    json_object_set_new(root, "restore-preset", json_boolean(restore_last_preset));
    if (!last_preset.empty()) {
        json_object_set_new(root, "last-preset", last_preset.toJson(true, true, false));
    }
    return root;
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
    getLight(L_READY).setBrightnessSmooth(em.ready ? 1.0f : 0.f, args.sampleTime * 20);

    bool round = em.get_round_rate() > 0;
    bool initial = em.is_round_initial();
    bool on_y = em.get_round_mode() >= Haken::rViaY;
    bool release = em.get_round_mode() <= Haken::rRelease;
    getLight(Lights::L_ROUND_Y).setBrightness(1.0f * on_y);
    getLight(Lights::L_ROUND_INITIAL).setBrightness(1.0f * initial);
    getLight(Lights::L_ROUND).setBrightness(1.0f * round);
    getLight(Lights::L_ROUND_RELEASE).setBrightness(1.0f * (round && release));

    getLight(L_C1_MUSIC_FILTER).setBrightnessSmooth(getParam(P_C1_MUSIC_FILTER).getValue(), 45.f);
    getLight(L_C2_MUSIC_FILTER).setBrightnessSmooth(getParam(P_C2_MUSIC_FILTER).getValue(), 45.f);
    getLight(L_C1_MUTE).setBrightnessSmooth(getParam(P_C1_MUTE).getValue(), 45.f);
    getLight(L_C2_MUTE).setBrightnessSmooth(getParam(P_C2_MUTE).getValue(), 45.f);
}

void CoreModule::process_params(const ProcessArgs &args)
{
    if (is_controller_1_connected()) {
        controller1_midi_in.setMusicPassFilter(getParamInt(getParam(P_C1_MUSIC_FILTER)));
        controller1_midi_in.enable(0 == getParamInt(getParam(P_C1_MUTE)));
    } else {
        controller1_midi_in.setMusicPassFilter(false);
        getParam(P_C1_MUTE).setValue(0.f);
        getParam(P_C1_MUSIC_FILTER).setValue(0.f);
    }

    if (is_controller_2_connected()) {
        controller2_midi_in.setMusicPassFilter(getParamInt(getParam(P_C2_MUSIC_FILTER)));
        controller2_midi_in.enable(0 == getParamInt(getParam(P_C2_MUTE)));
    } else {
        controller2_midi_in.setMusicPassFilter(false);
        controller2_midi_in.enable(true);
        getParam(P_C2_MUTE).setValue(0.f);
        getParam(P_C2_MUSIC_FILTER).setValue(0.f);
    }
}

void CoreModule::process(const ProcessArgs &args)
{
    auto sample_time = args.sampleTime;
    controller1_midi_in.dispatch(sample_time);
    controller2_midi_in.dispatch(sample_time);
    if (haken_midi_out.ring.size() > 2*(haken_midi_out.ring.capacity()/3)) {
        haken_midi_out.dispatch(DISPATCH_NOW);
    }
    haken_midi_in.dispatch(sample_time);
    if (haken_midi_out.ring.size() > 2*(haken_midi_out.ring.capacity()/3)) {
        haken_midi_out.dispatch(DISPATCH_NOW);
    } else {
        haken_midi_out.dispatch(sample_time);
    }

    // todo: after a reset, need to have a ~20sec pause before rescanning
    if (!haken_midi_out.pending()) {
        tasks.process(args);
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
        getOutput(OUT_READY).setVoltage(em.ready && !host_busy() ? 10.0f : 0.0f);
    }
    if (0 == ((args.frame + id) % PROCESS_PARAM_INTERVAL)) {
        process_params(args);
    }
    if (0 == ((args.frame + id) % PROCESS_LIGHT_INTERVAL)) {
        processLights(args);
    }
}

Model *modelCore = createModel<CoreModule, CoreModuleWidget>("chem-core");

