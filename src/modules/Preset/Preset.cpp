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
    hardware = get_json_int(root, "hardware", 0);
    firmware = get_json_int(root, "firmware", 0);
    active_tab = PresetTab(get_json_int(root, "tab", int(PresetTab::System)));
    user_order = PresetOrder(get_json_int(root, "user-sort-order", int(PresetOrder::Natural)));
    system_order = PresetOrder(get_json_int(root, "system-sort-order", int(PresetOrder::Alpha)));
    
    cat_filter = parse_hex_u64(get_json_string(root, "cat-filter"));
    type_filter = parse_hex_u64(get_json_string(root, "type-filter"));
    character_filter = parse_hex_u64(get_json_string(root, "character-filter"));
    matrix_filter = parse_hex_u64(get_json_string(root, "matrix-filter"));
    gear_filter = parse_hex_u64(get_json_string(root, "gear-filter"));

    ModuleBroker::get()->try_bind_client(this);
}

json_t* PresetModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    json_object_set_new(root, "track-live", json_boolean(track_live));
    json_object_set_new(root, "cache_user", json_boolean(use_cached_user_presets));
    json_object_set_new(root, "hardware", json_integer(hardware));
    json_object_set_new(root, "firmware", json_integer(firmware));
    json_object_set_new(root, "tab", json_integer(int(active_tab)));
    json_object_set_new(root, "user-sort-order", json_integer(int(user_order)));
    json_object_set_new(root, "system-sort-order", json_integer(int(system_order)));
    json_object_set_new(root, "cat-filter", json_string(u64_to_string(cat_filter).c_str()));
    json_object_set_new(root, "type-filter", json_string(u64_to_string(type_filter).c_str()));
    json_object_set_new(root, "character-filter", json_string(u64_to_string(character_filter).c_str()));
    json_object_set_new(root, "matrix-filter", json_string(u64_to_string(matrix_filter).c_str()));
    json_object_set_new(root, "gear-filter", json_string(u64_to_string(gear_filter).c_str()));
    return root;
}

void PresetModule::set_order(PresetTab tab, PresetOrder order)
{
    switch (tab) {
    case PresetTab::User: user_order = order; return;
    case PresetTab::System: system_order = order; return;
    default: break;
    }
}

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
}

Model *modelPreset = createModel<PresetModule, PresetUi>("chem-preset");
