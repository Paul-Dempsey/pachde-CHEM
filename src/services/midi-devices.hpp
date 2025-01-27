// Copyright (C) Paul Chase Dempsey
#pragma once
#include <rack.hpp>
#include <string>
#include "misc.hpp"
namespace pachde {

const char CLAIM_SEPARATOR = '~';
struct MidiDeviceHolder; // forward reference

enum class TextFormatLength { Short, Long, Compact, Abbreviated };
bool ExcludeDriver(const std::string & name);
bool ExcludeDevice(const std::string & name);
bool is_EMDevice(const std::string& name);
std::string FilterDeviceName(const std::string& raw);
bool matchInOut(const std::string& input, const std::string& output);

struct MidiDeviceConnectionInfo
{
    std::string driver_name;
    std::string input_device_name;
    std::string output_device_name;
    int sequence;
    std::string claim_spec; // generated on demand

    explicit MidiDeviceConnectionInfo()
    : sequence(-1)
    {}

    const std::string & input() const {
        return input_device_name;
    }
    const std::string & output() const {
        if (output_device_name.empty()) {
            return input_device_name;
        }
        return output_device_name;
    }
    const std::string & driver() const {
        return driver_name;
    }
    int nth() const { return sequence; }

    void clear() {
        claim_spec.clear();
        driver_name.clear();
        input_device_name.clear();
        output_device_name.clear();
        sequence = -1;
    }
    bool ok() const {
        return sequence >= 0
            && !input_device_name.empty()
            && !driver_name.empty();
    }
    bool parse(const std::string & spec);
    std::string claim() const;
    std::string friendly(TextFormatLength length) const;
};

struct MidiDeviceConnection
{
    int driver_id;
    int input_device_id;
    int output_device_id;
    MidiDeviceConnectionInfo info;

    explicit MidiDeviceConnection()
    : driver_id(-1), input_device_id(-1), output_device_id(-1)
    {}

    void clear_ids() {
        driver_id = input_device_id = output_device_id = -1;
    }

    bool identified() const {
        return driver_id >= 0
            && input_device_id >= 0
            && output_device_id >= 0;
    }

    bool is_same_connection(std::shared_ptr<MidiDeviceConnection> other) const {
        return 0 == info.claim().compare(other->info.claim());
    }
};

std::vector<std::shared_ptr<MidiDeviceConnection>> EnumerateMidiConnections(bool emOnly);

struct IMidiDeviceNotify {
    virtual void onMidiDeviceChange(const MidiDeviceHolder* source) = 0;
};

struct MidiDeviceHolder
{
    MidiDeviceHolder & operator=(const MidiDeviceHolder &) = delete;
    MidiDeviceHolder(const MidiDeviceHolder&) = delete;

    std::shared_ptr<MidiDeviceConnection> connection;
    std::string device_claim;
    IMidiDeviceNotify * client;

    MidiDeviceHolder() : connection(nullptr), client(nullptr) {}
    const std::string & getClaim() { return device_claim; }
    void setClaim(const std::string &claim) { device_claim = claim; }
    void notifyChanged();
    void clear();
    void connect(std::shared_ptr<MidiDeviceConnection> connection);
    void subscribe(IMidiDeviceNotify *client);
    void unsubscribe(IMidiDeviceNotify *client);
};

struct MidiDeviceBroker
{
    MidiDeviceBroker & operator=(const MidiDeviceBroker &) = delete;
    MidiDeviceBroker(const MidiDeviceBroker&) = delete;

    std::vector<MidiDeviceHolder*> holders;
    std::map<std::string, MidiDeviceHolder*> claims;

    MidiDeviceBroker() {}
    static MidiDeviceBroker* get();

    bool isPrimary(MidiDeviceHolder* holder);
    void registerDeviceHolder(MidiDeviceHolder* holder);
    void unRegisterDeviceHolder(MidiDeviceHolder* holder);
    void clear();
    bool empty() { return claims.empty(); }
    bool available(const std::string& claim);
    bool claimDevice(const std::string claim, MidiDeviceHolder* claimant);
    void revokeClaim(const std::string& claim);
    void revokeClaim(MidiDeviceHolder* claimant);
    void sync();
    // returns true if an available EM device was bound
    bool bindAvailableEm(MidiDeviceHolder* holder);
};

}