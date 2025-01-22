#pragma once
#include "../chem-core.hpp"

namespace pachde {

struct ModuleBroker
{
    struct Internal;
    Internal * my;

    ModuleBroker();
    ~ModuleBroker();
    ModuleBroker & operator=(const ModuleBroker &) = delete;
    ModuleBroker(const ModuleBroker&) = delete;
    static ModuleBroker* get();

    void registerHost(IChemHost* host);
    void unregisterHost(IChemHost* host);
    bool isPrimary(IChemHost* host);
    IChemHost* getPrimary();
};

}