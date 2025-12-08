#include <rack.hpp>
#include "ModuleBroker.hpp"
#include "midi-devices.hpp"
#include "widgets/hamburger.hpp"
using namespace ::rack;
namespace pachde {

std::shared_ptr<ModuleBroker> the_module_broker_instance = std::make_shared<ModuleBroker>();

std::shared_ptr<ModuleBroker> ModuleBroker::get()
{
    return the_module_broker_instance;
}

ModuleBroker::ModuleBroker()
{
    hosts.reserve(4);
}

ModuleBroker::~ModuleBroker()
{
}

void ModuleBroker::register_host(IChemHost* host)
{
    if (hosts.cend() == std::find(hosts.cbegin(), hosts.cend(), host)) {
        hosts.push_back(host);
    } else {
        assert(false);
    }
}

void ModuleBroker::unregister_host(IChemHost* host)
{
    auto item = std::find(hosts.cbegin(), hosts.cend(), host);
    if (item != hosts.cend()) {
        hosts.erase(item);
    }
}

bool ModuleBroker::is_primary(IChemHost* host)
{
    if (hosts.empty()) return false;
    return (host == *hosts.cbegin());
}

IChemHost* ModuleBroker::get_primary()
{
    if (hosts.empty()) return nullptr;
    return *hosts.begin();
}

bool ModuleBroker::try_bind_client(IChemClient* client)
{
    if (hosts.empty()) return false;
    for (auto host : hosts) {
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
    for (auto host : hosts) {
        if (host->host_claim() == claim) {
            return host;
        }
    }
    return nullptr;
}

void ModuleBroker::addHostPickerMenu(ui::Menu* menu, IChemClient* client)
{
    menu->addChild(createMenuLabel<HamburgerTitle>("Link to Core Module"));
    if (hosts.empty()) {
        menu->addChild(createMenuLabel("[no Core modules available]"));
        return;
    }

    auto it = std::find_if(hosts.cbegin(), hosts.cend(), [&](IChemHost* host){
        return host->host_has_client(client);
    });
    IChemHost* current_host = (it == hosts.cend()) ? nullptr : *it;

    menu->addChild(createMenuItem("Disconnect", "", [=](){
        current_host->unregister_chem_client(client);
        client->onConnectHost(nullptr);
    }, nullptr == current_host));

    for (auto host : hosts) {
        auto conn = host->host_connection(ChemDevice::Haken);
        std::string item;
        if (conn) {
            item = conn->info.friendly(NameFormat::Short);
        } else {
            auto claim = host->host_claim();
            MidiDeviceConnectionInfo info;
            info.parse(claim);
            item = info.friendly(NameFormat::Short);
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