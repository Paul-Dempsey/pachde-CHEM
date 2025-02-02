#include "Core.hpp"
#include "../em/PresetId.hpp"
#include "../services/ModuleBroker.hpp"

using namespace pachde;

void RelayMidiToEM::doMessage(PackedMidiMessage message)
{
    core->em.onMessage(message);
}

CoreModule::CoreModule() : in_reboot(false), heartbeat(false), loop(0)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    configLight(L_READY, "Haken device ready");
    configLight(L_ROUND_Y, "Round on Y");
    configLight(L_ROUND_INITIAL, "Round initial");
    configLight(L_ROUND, "Rounding");
    configLight(L_ROUND_RELEASE, "Round on release");
    configLight(L_PULSE, "Loopback pulse");

    configOutput(Outputs::OUT_READY, "Ready trigger");

    ModuleBroker::get()->register_host(this);
    to_em.core = this;
    tasks.setCoreModule(this);
    em.subscribeEMEvents(this);
    haken_midi_out.setEm(&em);

    haken_midi.subscribe(this);
    controller1.subscribe(this);
    controller2.subscribe(this);

    haken_midi_in.addTarget(&to_em);
    controller1_midi_in.addTarget(&haken_midi_out);
    controller2_midi_in.addTarget(&haken_midi_out);

    auto broker = MidiDeviceBroker::get();
    broker->registerDeviceHolder(&haken_midi);
    broker->registerDeviceHolder(&controller1);
    broker->registerDeviceHolder(&controller2);

    enable_logging(true);
    tasks.subscribeChange(this);
}

CoreModule::~CoreModule() {
    //haken_midi_in.clear();
    ModuleBroker::get()->unregister_host(this);
    controller1_midi_in.clear();
    controller2_midi_in.clear();
    controller1.unsubscribe(this);
    controller2.unsubscribe(this);
    auto broker = MidiDeviceBroker::get();
    broker->unRegisterDeviceHolder(&haken_midi);
    broker->unRegisterDeviceHolder(&controller1);
    broker->unRegisterDeviceHolder(&controller2);
}

void CoreModule::enable_logging(bool enable)
{
    log_midi = enable;
    if (enable){
        haken_midi_out.setLogger(&midilog);
        haken_midi_in.setLogger("<<H", &midilog);
        controller1_midi_in.setLogger("<C1", &midilog);
        controller2_midi_in.setLogger("<C2", &midilog);
    } else {
        haken_midi_out.setLogger(nullptr);
        haken_midi_in.setLogger("", nullptr);
        controller1_midi_in.setLogger("", nullptr);
        controller2_midi_in.setLogger("", nullptr);
        midilog.close();
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
    haken_midi_out.sendMidiRate(rate);
}

void CoreModule::restore_midi_rate()
{
    if (HakenMidiRate::Full != tasks.midi_rate) {
        haken_midi_out.sendMidiRate(HakenMidiRate::Full);
        tasks.midi_rate = HakenMidiRate::Full;
    }
}

// IHandleEmEvents
void CoreModule::onLoopDetect(uint8_t cc, uint8_t value)
{
    ++loop;
    //if (0 == (loop & 1)) {
        heartbeat = !heartbeat;
        getLight(L_PULSE).setBrightness(heartbeat ? 0.f : value);
   // }
}

void CoreModule::onEditorReply(uint8_t reply)
{
    tasks.complete_task(HakenTask::HeartBeat);
//    getLight(L_PULSE).setBrightnessSmooth(0.f, 4.f * APP->engine->getSampleTime());
}

void CoreModule::onHardwareChanged(uint8_t hardware, uint16_t firmware_version)
{
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
    notify_preset_changed();
}
void CoreModule::onUserComplete()
{
    tasks.complete_task(HakenTask::UserPresets);
}
void CoreModule::onSystemComplete()
{
    tasks.complete_task(HakenTask::SystemPresets);
}
void CoreModule::onTaskMessage(uint8_t code)
{
}
void CoreModule::onLED(uint8_t led)
{
}

void CoreModule::onHakenTaskChange(HakenTask id)
{
    if (id == HakenTask::HeartBeat) {
        auto task = tasks.get_task(id);
        if (task->waiting()) {
            heartbeat = !heartbeat;
            getLight(L_PULSE).setBrightness(!heartbeat);
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

    auto id = DeviceIdentifier(source);
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

            logMessage("Core", format_string("+++ connect HAKEN %s", source->connection->info.friendly(TextFormatLength::Short).c_str()).c_str());
            haken_midi_out.sendEditorPresent();
            haken_midi_out.dispatch(DISPATCH_NOW);
        } else {
            logMessage("Core", "--- disconnect HAKEN");
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
            logMessage("Core", format_string("+++ connect MIDI1 %s", source->connection->info.friendly(TextFormatLength::Short).c_str()).c_str());
        } else {
            logMessage("Core", "--- disconnect Midi1");
        }
        HandleMidiDeviceChange(&controller1_midi_in, source);
        notify_connection_changed(ChemDevice::Midi1, source->connection);
        break;

    case ChemDevice::Midi2:
        if (source->connection) {
            logMessage("Core", format_string("+++ connect MIDI2 %s", source->connection->info.friendly(TextFormatLength::Short).c_str()).c_str());
        } else {
            logMessage("Core", "--- disconnect Midi2");
        }
        HandleMidiDeviceChange(&controller2_midi_in, source);
        notify_connection_changed(ChemDevice::Midi2, source->connection);
        break;

    case ChemDevice::Unknown:
        DEFAULT_UNREACHABLE
        break;
    }
}

void CoreModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    json_t* j;

    j = json_object_get(root, "haken-device");
    if (j) {
        haken_midi.setClaim(json_string_value(j));
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
        log_midi = json_boolean_value(j);
    }
    enable_logging(log_midi);
}

json_t* CoreModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(haken_midi.getClaim().c_str()));
    json_object_set_new(root, "controller-1", json_string(controller1.getClaim().c_str()));
    json_object_set_new(root, "controller-2", json_string(controller2.getClaim().c_str()));
    json_object_set_new(root, "log-midi", json_boolean(log_midi));
    return root;
}

void CoreModule::register_chem_client(IChemClient* client)
{
    if (chem_clients.cend() == std::find(chem_clients.cbegin(), chem_clients.cend(), client)) {
        chem_clients.push_back(client);
        client->onConnectHost(this);
    }
}

void CoreModule::unregister_chem_client(IChemClient* client)
{
    auto item = std::find(chem_clients.cbegin(), chem_clients.cend(), client);
    if (item != chem_clients.cend())
    {
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
    case ChemDevice::Haken: return haken_midi.connection; break;
    case ChemDevice::Midi1: return controller1.connection; break;
    case ChemDevice::Midi2: return controller2.connection; break;
    default: return nullptr;
    }
}

constexpr const uint64_t PROCESS_LIGHT_INTERVAL = 120;
void CoreModule::processLights(const ProcessArgs &args)
{
    getLight(L_READY).setBrightnessSmooth(em.ready ? 1.0f : 0.f, args.sampleTime * 20);
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
    tasks.process(args);

    if (getOutput(OUT_READY).isConnected()) {
        getOutput(OUT_READY).setVoltage(em.ready ? 10.0f : 0.0f);
    }
    if (0 == ((args.frame + id) % PROCESS_LIGHT_INTERVAL)) {
        processLights(args);
    }
}

Model *modelCore = createModel<CoreModule, CoreModuleWidget>("chem-core");

