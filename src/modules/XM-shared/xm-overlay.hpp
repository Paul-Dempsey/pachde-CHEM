#pragma once
#include "../../chem.hpp"
#include "macro-usage.hpp"

namespace pachde {

struct IOverlayClient;

struct IOverlay
{
    //virtual IChemHost* get_host() = 0;
    virtual void overlay_register_client(IOverlayClient* client) = 0;
    virtual void overlay_unregister_client(IOverlayClient* client) = 0;
    virtual std::shared_ptr<PresetInfo> overlay_live_preset() = 0;
    virtual std::shared_ptr<PresetInfo> overlay_configured_preset() = 0;
    virtual void overlay_request_macros() = 0;
    virtual std::vector<MacroUsage>& overlay_macro_usage() = 0;
};

IOverlay* find_adjacent_overlay(Module* client_module);
IOverlay* find_an_overlay(Module* client_module, std::string claim, std::string preset);

struct IOverlayClient
{
    // `host` is null when being destroyed
    virtual void on_overlay_change(IOverlay* host) = 0;
    // Overlay's Core connection/device changed
    //  virtual void on_overlay_connection_changed() = 0;
    //  virtual void on_preset_changed() = 0;
    virtual IOverlay* get_overlay() = 0;
};

}