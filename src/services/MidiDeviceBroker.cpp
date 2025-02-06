#include "midi-devices.hpp"

namespace pachde {

std::shared_ptr<MidiDeviceBroker> the_broker_instance = std::make_shared<MidiDeviceBroker>();

std::shared_ptr<MidiDeviceBroker> MidiDeviceBroker::get()
{
    return the_broker_instance; 
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
        if (!holder->device_claim.empty()) {
            revokeClaim(holder);
        }
        holders.erase(it);
    }
}

void MidiDeviceBroker::clear()
{
    holders.clear();
    claims.clear();
}

bool MidiDeviceBroker::available(const std::string& claim)
{
    return claims.cend() == claims.find(claim);
}

bool MidiDeviceBroker::claimDevice(const std::string claim, MidiDeviceHolder* claimant)
{
    assert(!claim.empty());
    auto it = claims.find(claim);
    if (it == claims.cend()) {
        claims.insert(std::make_pair(claim, claimant));
        return true;
    } else {
        // ok to claim same device twice
        return it->second == claimant;
    }
}

void MidiDeviceBroker::revokeClaim(const std::string& claim)
{
    claims.erase(claim);
}

void MidiDeviceBroker::revokeClaim(MidiDeviceHolder* claimant) {
    if (claimant) {
        revokeClaim(claimant->getClaim());
    } else {
        assert(false);
    }
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
        auto holder_claim = holder->getClaim();
        auto cit = connections.find(holder_claim);
        if (cit != connections.cend()) {
            if (holder->connection) {
                auto c1 = holder->connection;
                auto c2 = cit->second;
                if ((c1->input_device_id != c2->input_device_id) 
                    || (c1->driver_id != c2->driver_id)
                    || (c1->output_device_id != c2->output_device_id))
                {
                    holder->connect(c2);
                }
            } else {
                holder->connect(cit->second);
            }
        }
    }
}

bool MidiDeviceBroker::bindAvailableEm(MidiDeviceHolder* holder)
{
    assert(std::find(holders.cbegin(), holders.cend(), holder) != holders.cend()); // only registered holders should call this

    for (auto connection : EnumerateMidiConnections(true)) {
        auto claim = connection->info.claim();
        if (available(claim)) {
            bool ok = claimDevice(claim, holder);
            if (ok) {
                holder->device_claim = claim;
                holder->connection = connection;
            }
            return ok;
        }
    }
    return false;
}

}