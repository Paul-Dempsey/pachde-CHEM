#include "midi_devices.hpp"

namespace pachde {

void MidiDeviceHolder::clear() {
    connection = nullptr;
    device_claim.clear();
    notifyChanged();
}

void MidiDeviceHolder::notifyChanged() {
    if (client) {
        client->onMidiDeviceChange(this);
    }
}

void MidiDeviceHolder::connect(std::shared_ptr<MidiDeviceConnection> connection)
{
    this->connection = connection;
    if (connection) {
        this->device_claim = connection->info.spec();
    }
    notifyChanged();
}

void MidiDeviceHolder::subscribe(IMidiDeviceNotify *client) {
    assert(nullptr == this->client);
    this->client = client;
}

void MidiDeviceHolder::unsubscribe(IMidiDeviceNotify *client) {
    assert(client == this->client);
    this->client = nullptr;
}
}
