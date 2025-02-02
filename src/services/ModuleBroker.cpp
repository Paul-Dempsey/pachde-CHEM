#include <rack.hpp>
#include "ModuleBroker.hpp"
#include "midi-devices.hpp"

using namespace ::rack;
namespace pachde {

ModuleBroker* main(nullptr);

struct ModuleBroker::Internal
{
    std::vector<IChemHost*> hosts;
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

void ModuleBroker::register_host(IChemHost* host)
{
    std::unique_lock<std::mutex> lock(my->mut);
    if (my->hosts.cend() == std::find(my->hosts.cbegin(), my->hosts.cend(), host)) {
        my->hosts.push_back(host);
    }
}

void ModuleBroker::unregister_host(IChemHost* host)
{
    std::unique_lock<std::mutex> lock(my->mut);
    auto item = std::find(my->hosts.cbegin(), my->hosts.cend(), host);
    if (item != my->hosts.cend())
    {
        my->hosts.erase(item);
    }  
}

bool ModuleBroker::is_primary(IChemHost* host)
{
    if (my->hosts.empty()) return false;
    return (host == *my->hosts.cbegin());
}

IChemHost* ModuleBroker::get_primary()
{
    if (my->hosts.empty()) return nullptr;
    return *my->hosts.begin();
}

bool ModuleBroker::try_bind_client(IChemClient* client) {
    if (my->hosts.empty()) return false;
    for (auto host : my->hosts) {
        if (!host->host_has_client_model(client)) {
            if (host->host_claim() == client->client_claim()) {
                host->register_chem_client(client);
            }
            return true;
        }
    }
    return false;
}

IChemHost* ModuleBroker::get_host(const std::string& claim)
{
    for (auto host : my->hosts) {
        if (host->host_claim() == claim) {
            return host;
        }
    }
    return nullptr;
}

void ModuleBroker::addHostPickerMenu(ui::Menu* menu, IChemClient* client)
{
    if (my->hosts.empty()) {
        menu->addChild(createMenuLabel("[no Core modules available]"));
        return;
    }

    auto it = std::find_if(my->hosts.cbegin(), my->hosts.cend(), [&](IChemHost * host){
        return host->host_has_client(client);
    });
    IChemHost* current_host = (it == my->hosts.cend()) ? nullptr : *it;

    menu->addChild(createMenuItem("Disconnect", "", [=](){
        current_host->unregister_chem_client(client);
        client->onConnectHost(nullptr);
    }, nullptr == current_host));

    for (auto host : my->hosts) {
        auto conn = host->host_connection(ChemDevice::Haken);
        std::string item;
        if (conn) {
            item = conn->info.friendly(TextFormatLength::Short);
        } else {
            auto claim = host->host_claim();
            MidiDeviceConnectionInfo info;
            info.parse(claim);
            item = info.friendly(TextFormatLength::Short);
        }
        bool available = !host->host_has_client_model(client);
        bool mine = host->host_has_client(client);
        menu->addChild(createCheckMenuItem(item, "", 
            [=](){ return mine; }, 
            [=](){
                if (current_host) {
                    current_host->unregister_chem_client(client);
                }
                host->register_chem_client(client);
            },
            !available));
    }
}
}