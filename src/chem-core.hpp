#pragma once
#include <rack.hpp>
#include "em/preset.hpp"
#include "services/midi-devices.hpp"

namespace pachde {

struct IChemClient;

struct IChemHost
{
    virtual void register_chem_client(IChemClient* client) = 0;
    virtual void unregister_chem_client(IChemClient* client) = 0;
    virtual bool host_has_client(IChemClient* client) = 0;
    virtual std::shared_ptr<MidiDeviceConnection> host_connection() = 0;
    virtual const PresetDescription* host_preset() = 0;
};

enum class ChemDevice { Unknown, Haken, Midi1, Midi2 };

struct IChemClient
{
    virtual ::rack::engine::Module* client_module() = 0;
    virtual void onConnectHost(IChemHost* host) = 0;
    virtual void onPresetChange() = 0;
    virtual void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) = 0;
};


}