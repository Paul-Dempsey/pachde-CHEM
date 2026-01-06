#include "uniform-style.hpp"
#include "services/kv-store.hpp"
using namespace widgetry;
namespace pachde {
namespace style {

const char * const NotConnected = "[not connected]";
const char * const InputColorKey = "in-ring";
const char * const OutputColorKey = "out-ring";

LabelStyle warning_label      {"warning",    HAlign::Left,      9.f, false};
LabelStyle haken_label        {"dytext",     HAlign::Left,     10.f, false};
LabelStyle in_port_label      {"in-label",   HAlign::Center,   12.f, true};
LabelStyle out_port_label     {"out-label",  HAlign::Center,   12.f, true};
LabelStyle control_label      {"ctl-label",  HAlign::Center,   14.f, false};
LabelStyle control_label_small{"ctl-label",  HAlign::Center,   12.f, false};
LabelStyle control_label_left {"ctl-label",  HAlign::Left,     14.f, false};
LabelStyle small_label        {"label",      HAlign::Center,   10.f, false};
LabelStyle med_label          {"label",      HAlign::Center,   12.f, false};
LabelStyle heading_label      {"ctl-label",  HAlign::Center,   16.f, true};
LabelStyle pedal_label        {"ped-assign", HAlign::Left,     10.f, false};
LabelStyle pedal_label_center {"ped-assign", HAlign::Center,   10.f, false};

const Vec light_dx{PORT_MOD_DX, PORT_MOD_DX};

bool show_screws()
{
    auto kv = get_plugin_kv_store();
    if (kv && kv->load()) {
        const char* key = "show-screws";
        auto value = kv->lookup(key);
        if (value.empty()) {
            kv->update(key, KVStore::bool_text(true));
            kv->save();
            return true;
        }
        return KVStore::bool_value(value);
    }
    return true;
}

bool show_browser_logo()
{
    auto kv = get_plugin_kv_store();
    if (kv && kv->load()) {
        const char* key = "browser-logo";
        auto value = kv->lookup(key);
        if (value.empty()) {
            kv->update(key, KVStore::bool_text(true));
            kv->save();
            return true;
        }
        return KVStore::bool_value(value);
    }
    return true;
}

}};