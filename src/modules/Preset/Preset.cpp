#include "Preset.hpp"

PresetModule::PresetModule()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configSwitch(P_CAT, 0.f, 13.f, 0.f, "Category", {
        "- none -",
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
        "- none -",
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
        "- none -",
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
        "- none -",
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
        "- none - ",
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
    json_t* j = json_object_get(root, "haken-device");
    if (j) {
        device_claim = json_string_value(j);
    }
}

json_t* PresetModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    return root;
}

// IChemClient
rack::engine::Module* PresetModule::client_module()
{
    return this; 
}
std::string PresetModule::client_claim()
{
    return device_claim;
}
void PresetModule::onConnectHost(IChemHost* host)
{
    chem_host = host;
}
void PresetModule::onPresetChange()
{
}
void PresetModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection) 
{
}

Model *modelPreset = createModel<PresetModule, PresetModuleWidget>("chem-preset");
