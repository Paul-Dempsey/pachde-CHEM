#pragma once
#include "chem-core.hpp"

namespace pachde {

struct ModuleBroker
{
    std::vector<IChemHost*> hosts;

    ModuleBroker();
    ~ModuleBroker();
    ModuleBroker & operator=(const ModuleBroker &) = delete;
    ModuleBroker(const ModuleBroker&) = delete;
    static std::shared_ptr<ModuleBroker> get();

    void register_host(IChemHost* host);
    void unregister_host(IChemHost* host);
    bool is_primary(IChemHost* host);
    IChemHost* get_primary();
    bool try_bind_client(IChemClient* client);
    IChemHost* get_host(const std::string& claim);

    void addHostPickerMenu(ui::Menu* menu, IChemClient* client);

};

}