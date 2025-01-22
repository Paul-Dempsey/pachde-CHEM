#include "midi_devices.hpp"

namespace pachde {

MidiDeviceBroker * brokerInstance = nullptr;

MidiDeviceBroker* MidiDeviceBroker::get()
{
    if (!brokerInstance) {
        brokerInstance = new MidiDeviceBroker();
    }
    return brokerInstance;
}

void MidiDeviceBroker::registerDeviceHolder(MidiDeviceHolder* holder)
{
    assert(holders.cend() == std::find(holders.cbegin(), holders.cend(), holder));
    // if (!empty() && !holder->getClaim().empty()) {
    //     assert(claims.cend() == claims.find(holder->getClaim()));
    // }
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
            holder->connect(cit->second);
        }
    }
}

}