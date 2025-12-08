#include "uniform-style.hpp"
#include "services/kv-store.hpp"

namespace pachde {
namespace style {

const char * const NotConnected = "[not connected]";
const char * const InputColorKey = "in-ring";
const char * const OutputColorKey = "out-ring";

LabelStyle warning_label      {"warning",    TextAlignment::Left,      9.f, false};
LabelStyle haken_label        {"dytext",     TextAlignment::Left,     10.f, false};
LabelStyle in_port_label      {"in-label",   TextAlignment::Center,   12.f, true};
LabelStyle control_label      {"ctl-label",  TextAlignment::Center,   14.f, false};
LabelStyle control_label_left {"ctl-label",  TextAlignment::Left,     14.f, false};
LabelStyle small_control_label{"label",      TextAlignment::Center,   10.f, false};
LabelStyle med_control_label  {"label",      TextAlignment::Center,   12.f, false};
LabelStyle heading_label      {"ctl-label",  TextAlignment::Center,   16.f, true};
LabelStyle pedal_label        {"ped-assign", TextAlignment::Left,     10.f, false};

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