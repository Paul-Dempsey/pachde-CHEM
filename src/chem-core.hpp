#pragma once
#include <rack.hpp>
#include "chem-id.hpp"
#include "em/preset.hpp"
#include "em/preset-list.hpp"
#include "em/EaganMatrix.hpp"
#include "services/rack-em-convert.hpp"
#include "services/midi-devices.hpp"
#include "services/haken-midi.hpp"

namespace pachde {

struct IChemClient;

enum class PresetResult {
    Ok,
    FileNotFound,
    NotReady,
    NotApplicable,
    NotApplicableOsmose,
    NotApplicableEm
};

struct IPresetListClient
{
    virtual void on_list_changed(eaganmatrix::PresetTab which) = 0;
};

struct IPresetList
{
    virtual void register_preset_list_client(IPresetListClient* client) = 0;
    virtual void unregister_preset_list_client(IPresetListClient* client) = 0;
    virtual std::shared_ptr<PresetList> host_user_presets() = 0;
    virtual std::shared_ptr<PresetList> host_system_presets() = 0;
};


struct IChemHost
{
    virtual void register_chem_client(IChemClient* client) = 0;
    virtual void unregister_chem_client(IChemClient* client) = 0;
    virtual bool host_has_client_model(IChemClient* client) = 0;
    virtual bool host_has_client(IChemClient* client) = 0;
    virtual std::shared_ptr<MidiDeviceConnection> host_connection(ChemDevice device) = 0;
    virtual std::string host_claim() = 0;
    virtual bool host_busy() = 0;
    virtual HakenMidi* host_haken() = 0;
    virtual eaganmatrix::EaganMatrix* host_matrix() = 0;
    virtual const eaganmatrix::PresetDescription* host_preset() = 0;
    virtual IPresetList* host_ipreset_list() = 0;
    virtual void request_preset(ChemId tag, PresetId id) = 0;
};

struct IChemClient
{
    virtual ::rack::engine::Module* client_module() = 0;
    virtual std::string client_claim() = 0;
    virtual void onConnectHost(IChemHost* host) = 0;
    virtual void onPresetChange() {};
    virtual void onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) = 0;
    virtual IDoMidi* client_do_midi() { return nullptr; }
};

}