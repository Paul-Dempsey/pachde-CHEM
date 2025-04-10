#include "Preset.hpp"

PresetModule::PresetModule()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configSwitch(P_CAT, 0.f, 13.f, 0.f, "Category", {
        "- any -",
        "Strings",
        "Winds",
        "Vocal",
        "Keyboard",
        "Classic",
        "Other",
        "Percussion",
        "Tuned Percussion",
        "Processor",
        "Drone",
        "Midi",
        "Control Voltage",
        "Utility"
    });
    configSwitch(P_TYPE, 0.f, 14.f, 0.f, "Type", {
        "- any -",
        "Atonal",
        "Bass",
        "Bowed",
        "Brass",
        "Demo Preset",
        "Electric Piano",
        "Flute",
        "Lead",
        "Organ",
        "Pad",
        "Plucked",
        "Double Reed",
        "Single Reed",
        "Struck"
    });
    configSwitch(P_CHARACTER, 0.f, 39.f, 0.f, "Character", {
        "- any -",
        "Acoustic",
        "Aggressive",
        "Airy",
        "Analog",
        "Arpeggio",
        "Big",
        "Bright",
        "Chords",
        "Clean",
        "Dark",
        "Digital",
        "Distorted",
        "Dry",
        "Echo",
        "Electric",
        "Ensemble",
        "Evolving",
        "FM",
        "Hybrid",
        "Icy",
        "Intimate",
        "Lo-fi",
        "Looping",
        "Layered",
        "Morphing",
        "Metallic",
        "Nature",
        "Noise",
        "Random",
        "Reverberant",
        "Sound Design",
        "Stereo",
        "Shaking",
        "Simple",
        "Soft",
        "Strumming",
        "Synthetic",
        "Warm",
        "Woody"
    });
    configSwitch(P_MATRIX, 0.f, 18.f, 0.f, "EaganMatrix", {
        "- any -",
        "Additive",
        "BiqBank",
        "BiqGraph",
        "BiqMouth",
        "Cutoff Mod",
        "Formula Delay",
        "Micro Delay",
        "Sum Delay",
        "Voice Delay",
        "HarMan",
        "Kinetic",
        "ModMan",
        "Osc Jenny",
        "Osc Phase",
        "Osc DSF",
        "SineBank",
        "SineSpray",
        "WaveBank"
    });
    configSwitch(P_GEAR, 0.f, 8.f, 0.f, "Setting", {
        "- any - ",
        "Channel 1",
        "External Midi Clock",
        "Mono Interval",
        "Portamento",
        "Rounding",
        "Split Voice",
        "Single Voice",
        "Touch Area"
    });
}

void PresetModule::dataFromJson(json_t *root)
{
    ChemModule::dataFromJson(root);
    json_read_string(root, "haken-device", device_claim);
    json_read_bool(root, "track-live", track_live);
    hardware = get_json_int(root, "hardware", 0);
    firmware = get_json_int(root, "firmware", 0);
    active_tab = get_json_int(root, "tab", 0);
    ModuleBroker::get()->try_bind_client(this);
}

json_t* PresetModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    json_object_set_new(root, "track-live", json_boolean(track_live));
    json_object_set_new(root, "hardware", json_integer(hardware));
    json_object_set_new(root, "firmware", json_integer(firmware));
    json_object_set_new(root, "tab", json_integer(active_tab));
    return root;
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
    if (!chem_host || chem_host->host_busy() || !ui()) return;
}

Model *modelPreset = createModel<PresetModule, PresetUi>("chem-preset");
