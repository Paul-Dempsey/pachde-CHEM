#include "midi-devices.hpp"

namespace pachde {

void MidiDeviceHolder::clear()
{
    device_claim.clear();
    connection = nullptr;
    notifyChanged();
}

void MidiDeviceHolder::notifyChanged()
{
    if (client) {
        client->onMidiDeviceChange(this);
    }
}

void MidiDeviceHolder::connect(std::shared_ptr<MidiDeviceConnection> connection)
{
    this->connection = connection;
    if (connection) {
        // always override claim on connection
        this->device_claim = connection->info.claim();
    }
    notifyChanged();
}

void MidiDeviceHolder::subscribe(IMidiDeviceNotify *client)
{
    assert(nullptr == this->client);
    this->client = client;
}

void MidiDeviceHolder::unsubscribe(IMidiDeviceNotify *client)
{
    assert(client == this->client);
    this->client = nullptr;
}
}
