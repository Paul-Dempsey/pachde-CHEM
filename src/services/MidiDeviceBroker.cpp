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
    if (!empty() && !holder->getClaim().empty()) {
        assert(claims.cend() == claims.find(holder->getClaim()));
    }
    holders.push_back(holder);
}
void MidiDeviceBroker::unRegisterDeviceHolder(MidiDeviceHolder* holder)
{
    auto it = std::find(holders.cbegin(), holders.cend(), holder);
    if (it != holders.cend()) {
        holders.erase(it);
    }
}

void MidiDeviceBroker::clear() {
    holders.clear();
    claims.clear();
}

bool MidiDeviceBroker::available(const std::string& claim)
{
    return claims.cend() == claims.find(claim);
}

bool MidiDeviceBroker::claimDevice(const std::string claim, MidiDeviceHolder* claimant) {
    auto it = claims.find(claim);
    if (it == claims.cend()) {
        claims.insert(std::make_pair(claim, claimant));
        return true;
    } else {
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
    }
}

void MidiDeviceBroker::sync()
{
    // reconcile currently available devices with claimed devices  
}

}