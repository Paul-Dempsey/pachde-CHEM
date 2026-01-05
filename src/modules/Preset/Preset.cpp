#include "Preset.hpp"
#include "services/json-help.hpp"
#include "services/rack-help.hpp"


PresetModule::PresetModule()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    snap(configParam(P_NAV, 0, 1000, 0, "Navigate presets", ""));
    configInput(IN_SELECT_PRESET, "Select preset trigger");

    midi_device.init(ChemDevice::Preset, this);
    midi_handler.init(this);
    midi_in.set_target(&this->midi_handler);
    // midi_in.set_logger("<P", midi_log);
    auto broker = MidiDeviceBroker::get();
    broker->registerDeviceHolder(&midi_device);
}

PresetModule::~PresetModule(){
    midi_device.unsubscribe(this);
    midi_in.clear();
    if (chem_host) {
        chem_host->unregister_chem_client(this);
    }
}

void PresetModule::onMidiDeviceChange(const MidiDeviceHolder* source) {
    if (source != &midi_device) return;
    if (source) {
        midi_device_claim = source->get_claim();
    } else {
        midi_device_claim.clear();
    }
}

void PresetModule::dataFromJson(json_t *root)
{
    ChemModule::dataFromJson(root);
    device_claim = get_json_string(root, "haken-device");
    midi_device_claim = get_json_string(root, "midi-device");
    midi_device.set_claim(midi_device_claim);
    log_midi = get_json_bool(root, "midi-log", log_midi);
    //nav_input_relative = get_json_bool(root, "nav-input-relative", nav_input_relative);
    track_live = get_json_bool(root, "track-live", track_live);
    keep_search_filters = get_json_bool(root,"keep-search", keep_search_filters);
    search_name = get_json_bool(root, "search-name", search_name);
    search_meta = get_json_bool(root, "search-text", search_meta);
    search_anchor = get_json_bool(root, "search-anchored", search_anchor);
    search_incremental = get_json_bool(root, "search-incremental", search_incremental);
    active_tab = PresetTab(get_json_int(root, "tab", int(PresetTab::System)));

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
    set_json(root, "midi-device", midi_device_claim);
    set_json(root, "midi-log", log_midi);
    //set_json(root, "nav-input-relative", nav_input_relative);
    set_json(root, "track-live", track_live);
    set_json(root, "keep-search", keep_search_filters);
    set_json(root, "search-name", search_name);
    set_json(root, "search-text", search_meta);
    set_json(root, "search-anchored", search_anchor);
    set_json(root, "search-incremental", search_incremental);
    set_json_int(root, "tab", int(active_tab));

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
    case PresetTab::System:std::memset(system_filters, 0, sizeof(system_filters)); return;
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

void PresetModule::onRandomize()
{
    if (chem_ui) {
        ui()->send_random_preset();
    }
}

void PresetModule::process(const ProcessArgs &args)
{
    ChemModule::process(args);

    auto pui = ui();
    if (pui) {
        int idx = getParam(P_NAV).getValue();
        if (pui->get_current_index() != idx) {
            pui->set_current_index(idx);
            pui->scroll_to_page_of_index(idx);
        }
    }

    auto input = getInput(IN_SELECT_PRESET);
    if (input.isConnected()) {
        if (select_preset_trigger.process(input.getVoltage(), .1f, .5f)) {
            select_preset_trigger.reset();
            if (pui) {
                pui->send_preset(pui->get_current_index());
            }
        }
    }
    if (((args.frame + id) % 80) == 0) {
        if (!search_query.empty() || any_filter(filters())) {
            getLight(L_FILTER).setBrightness(1.f);
        } else {
            getLight(L_FILTER).setBrightness(0.f);
        }
    }
}

void PresetMidiHandler::enable_logging(bool logging) {
    if (logging) {
        if (!logger) {
            logger = new MidiLog;
        }
    } else {
        auto log = logger;
        logger = nullptr;
        delete log;
    }
}

void PresetMidiHandler::do_message(PackedMidiMessage msg)
{
    if (!host) return;
    if (logger) {
        logger->ensure_file();
        logger->logMidi(IO_Direction::In, msg);
    }
}

Model *modelPreset = createModel<PresetModule, PresetUi>("chem-preset");

