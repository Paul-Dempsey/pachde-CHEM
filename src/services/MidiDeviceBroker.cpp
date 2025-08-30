#include "midi-devices.hpp"

namespace pachde {

std::shared_ptr<MidiDeviceBroker> the_broker_instance = std::make_shared<MidiDeviceBroker>();

std::shared_ptr<MidiDeviceBroker> MidiDeviceBroker::get()
{
    return the_broker_instance;
}

MidiDeviceBroker::MidiDeviceBroker()
{
    holders.reserve(8);
}

bool MidiDeviceBroker::is_primary(MidiDeviceHolder* holder)
{
    assert(!holders.empty());
    return *holders.cbegin() == holder;
}

void MidiDeviceBroker::registerDeviceHolder(MidiDeviceHolder* holder)
{
    assert(holders.cend() == std::find(holders.cbegin(), holders.cend(), holder));
    holders.push_back(holder);
}

void MidiDeviceBroker::unRegisterDeviceHolder(MidiDeviceHolder* holder)
{
    auto it = std::find(holders.cbegin(), holders.cend(), holder);
    if (it != holders.cend()) {
        holders.erase(it);
    }
}

void MidiDeviceBroker::clear()
{
    holders.clear();
}

bool MidiDeviceBroker::available(const std::string& claim)
{
    for (auto holder: holders) {
        if (holder->device_claim == claim) {
            return holder->device_role != ChemDevice::Haken;
        }
    }
    return true;
}


// reconcile currently available devices with claimed devices
void MidiDeviceBroker::sync()
{
    std::map<std::string,std::shared_ptr<MidiDeviceConnection>> connections;
    for (auto connection : EnumerateMidiConnections(false)) {
        connections.insert(std::make_pair(connection->info.claim(), connection));
    }

    // bind registered holder claims
    for (auto holder: holders) {
        auto holder_claim = holder->get_claim();
        auto cit = connections.find(holder_claim);
        if (cit != connections.cend()) {
            if (holder->connection) {
                auto hc = holder->connection;
                auto c2 = cit->second;
                if ((hc->input_device_id != c2->input_device_id)
                    || (hc->driver_id != c2->driver_id)
                    || (hc->output_device_id != c2->output_device_id))
                {
                    holder->connect(c2);
                }
            } else {
                holder->connect(cit->second);
            }
        } else {
            holder->connect(nullptr);
        }
    }
}

bool MidiDeviceBroker::bindAvailableEm(MidiDeviceHolder* holder)
{
    assert(std::find(holders.cbegin(), holders.cend(), holder) != holders.cend()); // only registered holders should call this

    for (auto connection : EnumerateMidiConnections(true)) {
        auto claim = connection->info.claim();
        if (available(claim)) {
            holder->connect(connection);
            return true;
        }
    }
    return false;
}

}