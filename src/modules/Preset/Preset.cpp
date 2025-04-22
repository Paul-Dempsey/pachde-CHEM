#include "Preset.hpp"

PresetModule::PresetModule()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
}

void PresetModule::dataFromJson(json_t *root)
{
    ChemModule::dataFromJson(root);
    json_read_string(root, "haken-device", device_claim);
    json_read_bool(root, "track-live", track_live);
    json_read_bool(root, "cache_user", use_cached_user_presets);
    json_read_bool(root,"keep-search", keep_search_filters);
    //hardware = get_json_int(root, "hardware", 0);
    //firmware = get_json_int(root, "firmware", 0);
    active_tab = PresetTab(get_json_int(root, "tab", int(PresetTab::System)));
    user_order = PresetOrder(get_json_int(root, "user-sort-order", int(PresetOrder::Natural)));
    system_order = PresetOrder(get_json_int(root, "system-sort-order", int(PresetOrder::Alpha)));

    json_read_bool(root, "search-name", search_name);
    json_read_bool(root, "search-text", search_meta);
    json_read_bool(root, "search-anchored", search_anchor);
    json_read_bool(root, "search-incremental", search_incremental);

    if (keep_search_filters) {
        auto jar = json_object_get(root, "user-filters");
        if (jar) {
            json_t* jp;
            size_t index;
            json_array_foreach(jar, index, jp) {
                user_filters[index] = parse_hex_u64(json_string_value(jp));
            }   
        } else {
            memset(user_filters, 0, sizeof(user_filters));
        }
        jar = json_object_get(root, "system-filters");
        if (jar) {
            json_t* jp;
            size_t index;
            json_array_foreach(jar, index, jp) {
                system_filters[index] = parse_hex_u64(json_string_value(jp));
            }   
        } else {
            memset(system_filters, 0, sizeof(system_filters));
        }
    }
    ModuleBroker::get()->try_bind_client(this);
}

json_t* PresetModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    json_object_set_new(root, "track-live", json_boolean(track_live));
    json_object_set_new(root, "cache_user", json_boolean(use_cached_user_presets));
    json_object_set_new(root, "keep-search", json_boolean(keep_search_filters));
    //json_object_set_new(root, "hardware", json_integer(hardware));
    //json_object_set_new(root, "firmware", json_integer(firmware));
    json_object_set_new(root, "tab", json_integer(int(active_tab)));
    json_object_set_new(root, "user-sort-order", json_integer(int(user_order)));
    json_object_set_new(root, "system-sort-order", json_integer(int(system_order)));
    json_object_set_new(root, "search-name", json_boolean(search_name));
    json_object_set_new(root, "search-text", json_boolean(search_meta));
    json_object_set_new(root, "search-anchored", json_boolean(search_anchor));
    json_object_set_new(root, "search-incremental", json_boolean(search_incremental));

    if (keep_search_filters) {
        auto jar = json_array();
        for (int i = 0; i < 5; ++i) {
            json_array_append_new(jar, json_string(u64_to_string(user_filters[i]).c_str()));
        }
        json_object_set_new(root, "user-filters", jar);
        jar = json_array();
        for (int i = 0; i < 5; ++i) {
            json_array_append_new(jar, json_string(u64_to_string(system_filters[i]).c_str()));
        }
        json_object_set_new(root, "system-filters", jar);
    }
    return root;
}

uint64_t* PresetModule::filters()
{
    switch (active_tab) {
    case PresetTab::User: return user_filters;
    case PresetTab::System: return system_filters;
    default: return nullptr;
    }
}

void PresetModule::clear_filters(PresetTab tab_id)
{
    switch (tab_id) {
    case PresetTab::User: std::memset(user_filters, 0, sizeof(user_filters)); return;
    case PresetTab::System:std::memset(system_filters, 0, sizeof(system_filters)); return;
    default: break;
    }
}

void PresetModule::set_order(PresetTab tab, PresetOrder order)
{
    switch (tab) {
    case PresetTab::User: user_order = order; return;
    case PresetTab::System: system_order = order; return;
    default: break;
    }
}

// void PresetModule::set_filter(PresetTab tab_id, FilterId which, uint64_t mask)
// {
//     switch (tab_id) {
//     case PresetTab::User: user_filters[which] = mask; return;
//     case PresetTab::System: system_filters[which] = mask; return;
//     default: assert(false); break;
//     }
// }

// IChemClient

void PresetModule::onConnectHost(IChemHost* host)
{
    onConnectHostModuleImpl(this, host);
}

void PresetModule::onPresetChange()
{
    if (chem_ui) ui()->onPresetChange();
}

void PresetModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) 
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

// Module

void PresetModule::process(const ProcessArgs &args)
{
    ChemModule::process(args);
    if (((args.frame + id) % 80) == 0) {
        if (!search_query.empty() || any_filter(filters())) {
            getLight(L_FILTER).setBrightness(1.f);
        } else {
            getLight(L_FILTER).setBrightness(0.f);
        }
    }
}

Model *modelPreset = createModel<PresetModule, PresetUi>("chem-preset");
