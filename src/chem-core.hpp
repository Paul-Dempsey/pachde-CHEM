#pragma once
#include <rack.hpp>
#include "em/preset.hpp"
#include "services/midi-devices.hpp"

namespace pachde {

struct IChemClient;
enum class ChemDevice { Unknown, Haken, Midi1, Midi2 };

struct IChemHost
{
    virtual void register_chem_client(IChemClient* client) = 0;
    virtual void unregister_chem_client(IChemClient* client) = 0;
    virtual bool host_has_client_model(IChemClient* client) = 0;
    virtual bool host_has_client(IChemClient* client) = 0;
    virtual std::shared_ptr<MidiDeviceConnection> host_connection(ChemDevice device) = 0;
    virtual std::string host_claim() = 0;
    virtual const PresetDescription* host_preset() = 0;
};

struct IChemClient
{
    virtual ::rack::engine::Module* client_module() = 0;
    virtual std::string client_claim() = 0;

    virtual void onConnectHost(IChemHost* host) = 0;
    virtual void onPresetChange() = 0;
    virtual void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) = 0;
};


}