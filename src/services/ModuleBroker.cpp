#include <rack.hpp>
#include "ModuleBroker.hpp"

using namespace ::rack;
namespace pachde {

ModuleBroker* main(nullptr);

struct ModuleBroker::Internal
{
    std::vector<ChemHost*> hosts;
    std::mutex mut;
};

ModuleBroker* ModuleBroker::get()
{
    if (!main) {
        main = new ModuleBroker();
    }
    return main;
}

ModuleBroker::ModuleBroker()
: my(new Internal)
{ }

ModuleBroker::~ModuleBroker()
{
    if (my) { delete my; }
}

void ModuleBroker::registerHost(IChemHost* host)
{
    std::unique_lock<std::mutex> lock(my->mut);
    if (my->hosts.cend() == std::find(my->hosts.cbegin(), my->hosts.cend(), host)) {
        my->hosts.push_back(host);
    }
}

void ModuleBroker::unregisterHost(IChemHost* host)
{
    std::unique_lock<std::mutex> lock(my->mut);
    auto item = std::find(my->hosts.cbegin(), my->hosts.cend(), host);
    if (item != my->hosts.cend())
    {
        my->hosts.erase(item);
    }  
}

bool ModuleBroker::isPrimary(IChemHost* host)
{
    if (my->hosts.empty()) return false;
    return  (host == *my->hosts.cbegin());
}

IChemHost* ModuleBroker::getPrimary()
{
    //if (my->hosts.empty()) return nullptr;
    return *my->hosts.begin();
}


}