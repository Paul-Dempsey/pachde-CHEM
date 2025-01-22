#pragma once
#include <rack.hpp>
#include "em/preset.hpp"
#include "services/midi_devices.hpp"

namespace pachde {

struct IChemClient;

struct IChemHost
{
    virtual void register_client(IChemClient* client) = 0;
    virtual void unregister_client(IChemClient* client) = 0;
    virtual bool host_has_client(IChemClient* client) = 0;
    virtual bool host_ready() = 0;
    virtual std::shared_ptr<MidiDeviceConnection> host_connection() = 0;
    virtual const PresetDescription* host_preset() = 0;

};

struct IChemClient
{
    virtual rack::engine::Module* client_module() = 0;
    virtual void releaseHost() = 0;

    virtual void onPresetChange() = 0;
    virtual void onConnectionChange() = 0;
};


}