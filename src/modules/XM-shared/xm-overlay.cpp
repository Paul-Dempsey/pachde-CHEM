#include "xm-overlay.hpp"

namespace pachde {

IOverlay* find_adjacent_overlay(Module* mod)
{
    while (mod) {
        mod = mod->getLeftExpander().module;
        if (mod) {
            if (mod->getModel() == modelOverlay) {
                return dynamic_cast<IOverlay*>(mod);
            }
            if (mod->getModel() == modelXM) {
                auto over = dynamic_cast<IOverlayClient*>(mod)->get_overlay();
                if (over) return over;
            }
        }
        //require only chem to the left? if (!isChemModule(mod)) return nullptr;
    }
    return nullptr;
}

IOverlay * find_an_overlay(Module *client_module, std::string claim, std::string preset)
{
    auto engine = APP->engine;
    std::vector<IOverlay*> candidates;
    auto mods = engine->getModuleIds();
    for (auto id: mods) {
        auto module = engine->getModule(id);
        if (module == client_module) continue;
        if (module->getModel() == modelOverlay) {
            auto io = dynamic_cast<IOverlay*>(module);
            auto chem = dynamic_cast<ChemModule*>(module);
            auto host = chem ? chem->get_host() : nullptr;
            if (host) {
                if (0 == claim.compare(host->host_claim())) {
                    auto p = host->host_preset();
                    if (p && (0 == preset.compare(p->name))) {
                        return io;
                    }
                }
            }
            candidates.push_back(io);
        }
    }
    if (candidates.empty()) {
        return nullptr;
    }
    if (1 == candidates.size()) {
        return *candidates.begin();
    }
    for (auto mod: candidates) {
        auto host = reinterpret_cast<ChemModule*>(mod)->get_host();
        if (host && !claim.empty() && (0 == claim.compare(host->host_claim()))) {
            return mod;
        }
    }
    return *candidates.begin();
}

}