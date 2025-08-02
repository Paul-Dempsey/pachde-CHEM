#pragma once
#include "../../chem.hpp"
#include "macro-usage.hpp"
#include "macro-data.hpp"
using namespace eaganmatrix;

namespace pachde {

struct IOverlayClient;

enum MacroReadyState { Unavailable, InProgress, Available };
struct IOverlay
{
    virtual IChemHost* get_host() = 0;
    virtual void overlay_register_client(IOverlayClient* client) = 0;
    virtual void overlay_unregister_client(IOverlayClient* client) = 0;
    virtual std::shared_ptr<PresetInfo> overlay_live_preset() = 0;
    virtual std::shared_ptr<PresetInfo> overlay_configured_preset() = 0;
    virtual void overlay_request_macros() = 0;
    virtual MacroReadyState overlay_macros_ready() = 0;
    virtual std::vector<MacroUsage>& overlay_macro_usage() = 0;
    virtual std::shared_ptr<MacroDescription> overlay_get_macro(int64_t module, ssize_t knob) = 0;
    virtual void overlay_remove_macro(int64_t module, ssize_t knob) = 0;
    virtual void overlay_add_macro(std::shared_ptr<MacroDescription> macro) = 0;
    virtual void overlay_add_update_macro(std::shared_ptr<MacroDescription> macro) = 0;
    virtual void used_macros(std::vector<uint8_t>* list) = 0;
};

IOverlay* find_adjacent_overlay(Module* client_module);
IOverlay* find_an_overlay(Module* client_module, std::string claim, std::string preset);

struct IOverlayClient
{
    // `host` is null when being destroyed
    virtual void on_overlay_change(IOverlay* host) = 0;
    virtual IOverlay* get_overlay() = 0;
    virtual int64_t get_module_id() = 0;
};

}