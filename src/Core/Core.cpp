#include "Core.hpp"
#include "../em/PresetId.hpp"
#include "../services/ModuleBroker.hpp"

using namespace pachde;

Model *modelCore = createModel<CoreModule, CoreModuleWidget>("chem-core");

CoreModule::CoreModule()
{
    haken_midi.subscribe(this);
    controller1.subscribe(this);
    controller2.subscribe(this);

    auto broker = MidiDeviceBroker::get();
    broker->registerDeviceHolder(&haken_midi);
    broker->registerDeviceHolder(&controller1);
    broker->registerDeviceHolder(&controller2);

    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    configLight(L_READY, "Haken device ready");
    configLight(L_ROUND_Y, "Round on Y");
    configLight(L_ROUND_INITIAL, "Round initial");
    configLight(L_ROUND, "Rounding");
    configLight(L_ROUND_RELEASE, "Round on release");
    configOutput(Outputs::OUT_READY, "Ready trigger");
}

CoreModule::~CoreModule() {
    auto broker = MidiDeviceBroker::get();
    broker->unRegisterDeviceHolder(&haken_midi);
    broker->unRegisterDeviceHolder(&controller1);
    broker->unRegisterDeviceHolder(&controller2);
}


void HandleMidiDeviceChange(MidiInput* input, rack::midi::Output * output, const MidiDeviceHolder* source, StaticTextLabel *label)
{
    if (source->connection) {
        input->setDriverId(source->connection->driver_id);
        input->setDeviceId(source->connection->input_device_id);
        if (output) {
            output->setDeviceId(source->connection->output_device_id);
        }
        if (label) {
            label->text(source->connection->info.friendly(TextFormatLength::Short));
        }
    } else {
        input->setDeviceId(-1);
        input->setDeviceId(-1);
        if (output) {
            output->setDeviceId(-1);
        }
        if (label) {
            label->text("");
        }
    }
}

void CoreModule::notify_connection_changed()
{
    for (auto item : clients) {
        item->onConnectionChange();
    }
}

void CoreModule::onMidiDeviceChange(const MidiDeviceHolder* source)
{
    auto id = MidiDeviceIdentifier(source);
    switch (id) {
    case MidiDevice::Haken: {
        ready = false;
        HandleMidiDeviceChange(&haken_midi_in, &haken_midi_out, source, ui ? ui->haken_device_label : nullptr);
        notify_connection_changed();
    } break;
    case MidiDevice::Midi1: {
        HandleMidiDeviceChange(&controller1_midi_in, nullptr, source, ui ? ui->controller1_device_label : nullptr);
    } break;
    case MidiDevice::Midi2: {
        HandleMidiDeviceChange(&controller2_midi_in, nullptr, source, ui ? ui->controller2_device_label : nullptr); 
    } break;
    default: break;
    }
}

void CoreModule::dataFromJson(json_t* root)
{
    ChemModule::dataFromJson(root);
    json_t* j;

    j = json_object_get(root, "haken_device");
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
}

json_t* CoreModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken_device", json_string(haken_midi.getClaim().c_str()));
    json_object_set_new(root, "controller-1", json_string(controller1.getClaim().c_str()));
    json_object_set_new(root, "controller-2", json_string(controller2.getClaim().c_str()));
    return root;
}

void CoreModule::register_client(IChemClient* client)
{
    if (clients.cend() == std::find(clients.cbegin(), clients.cend(), client)) {
        clients.push_back(client);
    }
}

void CoreModule::unregister_client(IChemClient* client)
{
    auto item = std::find(clients.cbegin(), clients.cend(), client);
    if (item != clients.cend())
    {
        clients.erase(item);
    }
}

bool CoreModule::host_has_client(IChemClient* client)
{
    auto item = std::find(clients.cbegin(), clients.cend(), client);
    return item != clients.cend();
}

const uint64_t PROCESS_LIGHT_INTERVAL = 120;

void CoreModule::processLights(const ProcessArgs &args)
{
    getLight(L_READY).setBrightnessSmooth(this->ready ? 1.0f : 0.f, args.sampleTime * 20);
}

void CoreModule::process(const ProcessArgs &args)
{
    if (0 == ((args.frame + id) % PROCESS_LIGHT_INTERVAL)) {
        processLights(args);
    }

    if (getOutput(OUT_READY).isConnected()) {
        getOutput(OUT_READY).setVoltage(ready ? 10.0f : 0.0f);
    }
}

