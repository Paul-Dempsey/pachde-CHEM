// Copyright (C) Paul Chase Dempsey
#pragma once
#include <rack.hpp>
#include <string>
#include "misc.hpp"
#include "../chem-id.hpp"

namespace pachde {

const char CLAIM_SEPARATOR = '~';

enum class TextFormatLength { Short, Long, Compact, Abbreviated };
bool ExcludeDriver(const std::string & name);
bool ExcludeDevice(const std::string & name);
bool is_EMDevice(const std::string& name);
bool is_osmose(const std::string& name);
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

    bool identified() const {
        return driver_id >= 0
            && input_device_id >= 0
            && output_device_id >= 0;
    }
};

std::vector<std::shared_ptr<MidiDeviceConnection>> EnumerateMidiConnections(bool emOnly);

struct IMidiDeviceNotify;

struct MidiDeviceHolder
{
    MidiDeviceHolder & operator=(const MidiDeviceHolder &) = delete;
    MidiDeviceHolder(const MidiDeviceHolder&) = delete;

    std::shared_ptr<MidiDeviceConnection> connection;
    std::string device_claim;
    ChemDevice device_role;
    IMidiDeviceNotify * client;

    MidiDeviceHolder() : connection(nullptr), device_role(ChemDevice::Unknown), client(nullptr) {}
    void init(ChemDevice role, IMidiDeviceNotify *client) { device_role = role; subscribe(client); }
    ChemDevice role() const { return device_role; }

    const std::string & get_claim() const { return device_claim; }
    void set_claim(const std::string &claim) { device_claim = claim; }
    void notifyChanged();
    void clear();
    void connect(std::shared_ptr<MidiDeviceConnection> connection);
    void subscribe(IMidiDeviceNotify *client);
    void unsubscribe(IMidiDeviceNotify *client);
};

struct IMidiDeviceNotify {
    virtual void onMidiDeviceChange(const MidiDeviceHolder* source) = 0;
};

struct MidiDeviceBroker
{
    MidiDeviceBroker & operator=(const MidiDeviceBroker &) = delete;
    MidiDeviceBroker(const MidiDeviceBroker&) = delete;

    std::vector<MidiDeviceHolder*> holders;

    MidiDeviceBroker();
    static std::shared_ptr<MidiDeviceBroker> get();

    bool is_primary(MidiDeviceHolder* holder);
    void registerDeviceHolder(MidiDeviceHolder* holder);
    void unRegisterDeviceHolder(MidiDeviceHolder* holder);
    void clear();
    bool available(const std::string& claim);
    void sync();
    bool bindAvailableEm(MidiDeviceHolder* holder);
};

}