#include "Preset.hpp"
#include "services/json-help.hpp"
#include "services/rack-help.hpp"
PresetModule::PresetModule() :
    preset_midi(ChemId::Preset, ChemDevice::Preset)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
#ifdef NAV_ENDLESS
    auto pq = configParam(P_NAV, -INFINITY, INFINITY, 0.f, "Navigate presets", "");
    //pq->snapEnabled = true;
    pq->smoothEnabled = false;
    pq->smoothEnabled = false;
    pq->randomizeEnabled = false;
#else
    snap(configParam(P_NAV, 0, 1000, 0, "Navigate presets", ""));
#endif
    configSwitch(P_MUTE_KEY_NAV, 0.f, 1.f, 0.f, "Mute Keyboard nav", {"unmuted", "muted"});
    configButton(P_SELECT, "Select preset");
    preset_midi.init(this);
}

PresetModule::~PresetModule() {
    if (chem_host) {
        chem_host->unregister_chem_client(this);
    }
}

void PresetModule::set_nav_index(ssize_t index) {
#ifdef NAV_ENDLESS
    actual_nav_index = index;
#else
    getParam(P_NAV).setValue(index);
#endif
}

ssize_t PresetModule::get_nav_index() {
#ifdef NAV_ENDLESS
    return actual_nav_index;
#else
    return getParam(P_NAV).getValue();
#endif
}

// INavigateList
void PresetModule::nav_send() {
    if (!chem_ui) return;
    ui()->send_preset(get_nav_index());
}
ssize_t PresetModule::nav_get_index() {
    return get_nav_index();
}
void PresetModule::nav_set_index(ssize_t index) {
    set_nav_index(index);
}
ssize_t PresetModule::nav_get_page_size() {
    return PAGE_CAPACITY;
}
ssize_t PresetModule::nav_get_size() {
    if (!chem_ui) return 0;
    return ui()->active_tab().count();
}

void PresetModule::dataFromJson(json_t *root)
{
    ChemModule::dataFromJson(root);
    preset_midi.fromJson(json_object_get(root, "preset-midi"));

    device_claim = get_json_string(root, "haken-device");
    track_live = get_json_bool(root, "track-live", track_live);
    keep_search_filters = get_json_bool(root,"keep-search", keep_search_filters);
    search_name = get_json_bool(root, "search-name", search_name);
    search_meta = get_json_bool(root, "search-text", search_meta);
    search_anchor = get_json_bool(root, "search-anchored", search_anchor);
    search_incremental = get_json_bool(root, "search-incremental", search_incremental);
    active_tab = PresetTab(get_json_int(root, "tab", int(PresetTab::System)));
#ifdef NAV_ENDLESS
    actual_nav_index = get_json_int(root, "nav-index", -1);
    getParam(P_NAV).setValue(0.f);
    last_nav = 0.f;
#endif
    nav_include_loopback = get_json_bool(root, "nav-loopback", nav_include_loopback);

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
    set_json(root, "haken-device", device_claim);
    set_json(root, "preset-midi", preset_midi.toJson());
    set_json(root, "track-live", track_live);
    set_json(root, "keep-search", keep_search_filters);
    set_json(root, "search-name", search_name);
    set_json(root, "search-text", search_meta);
    set_json(root, "search-anchored", search_anchor);
    set_json(root, "search-incremental", search_incremental);
    set_json_int(root, "tab", int(active_tab));

#ifdef NAV_ENDLESS
    set_json_int(root, "nav-index", actual_nav_index);
#endif
    set_json(root, "nav-loopback", nav_include_loopback);

    if (keep_search_filters) {
        auto jar = json_array();
        for (int i = 0; i < 5; ++i) {
            json_array_append_new(jar, json_string(u64_to_string(user_filters[i]).c_str()));
        }
        set_json(root, "user-filters", jar);
        jar = json_array();
        for (int i = 0; i < 5; ++i) {
            json_array_append_new(jar, json_string(u64_to_string(system_filters[i]).c_str()));
        }
        set_json(root, "system-filters", jar);
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
    case PresetTab::System: std::memset(system_filters, 0, sizeof(system_filters)); return;
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
    if (chem_ui && (device == ChemDevice::Haken)) ui()->onConnectionChange(device, connection);
}

// Module

void PresetModule::onRemove() {
    chem_ui = nullptr;
}

void PresetModule::onRandomize()
{
    if (chem_ui && ui()->ready()) {
        ui()->send_random_preset();
    }
}

void PresetModule::process(const ProcessArgs &args)
{
    ChemModule::process(args);

    if (chem_ui && ui()->ready() && !chem_host->host_busy()) {
        if (getParamInt(getParam(P_SELECT))) {
            ui()->send_preset(get_nav_index());
            getParam(P_SELECT).setValue(0);
        }
    }
#ifdef NAV_ENDLESS
    if (((args.frame + id) % 180) == 0) {
        auto pq = getParamQuantity(P_NAV);
        auto p = pq->getValue();
        int n = static_cast<int>(std::abs(p - last_nav) * 1000.f);
        if (n) {
            if (last_nav < p) {
                actual_nav_index += n;
            } else if (last_nav > p) {
                actual_nav_index -= n;
            }
            pq->setValue(0.f);
            last_nav = 0.f;
        }
    }
#endif

    if (((args.frame + id) % 80) == 0) {

        preset_midi.mute_keys(getParamInt(getParam(P_MUTE_KEY_NAV)));

        if (!search_query.empty() || any_filter(filters())) {
            getLight(L_FILTER).setBrightness(1.f);
        } else {
            getLight(L_FILTER).setBrightness(0.f);
        }
    }

    preset_midi.process(args.sampleTime);
}

Model *modelPreset = createModel<PresetModule, PresetUi>("chem-preset");

