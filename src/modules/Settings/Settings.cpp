#include "Settings.hpp"
#include "../../services/rack-help.hpp"
#include "../../em/wrap-HakenMidi.hpp"
#include "tuning.hpp"
using namespace pachde;

SettingsModule::SettingsModule() :
    modulation(this, ChemId::Settings)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    std::vector<std::string> offon{"off", "on"};
    std::vector<std::string> onoff{"on", "off"};

    configSwitch(P_SURFACE_DIRECTION, 0.f, 1.f, 0.f, "Surface direction", {"normal", "reversed"});
    // TODO: intervals continuous from 1 to 96 in engine?
    configSwitch(P_X, 0.f, 15.f, 11.f, "X Bend Range", {
        "1", "2", "3", "4", "5", "6", "7", "12", "24", "36", "48", "96",
        "96: 2 (MPE)",
        "96: 5",
        "96: 7",
        "96: 12"
    });
    configSwitch(P_Y, 0.f, 8.f, 7.f, "Y assign", {"off", "cc 1 (mod wheel)", "cc 2 (breath)", "cc 3", "cc 4 (foot)", "cc 7 (vol)", "cc 11 (expression)", "cc 74 (MPE*)", "cc 74 (no shelf)"});
    configSwitch(P_Z, 0.f, 9.f, 8.f, "Z assign", {"off", "cc 1 (mod wheel)", "cc 2 (breath)", "cc 3", "cc 4 (foot)", "cc 7 (vol)", "cc 11 (expression)", "Channel pressure", "MPE+ (14-bit)", "MPE (Channel pressure)"});
    configSwitch(P_NOTE_PROCESSING, 0.f, 5.f, 0.f, "Note processing", {
        "Static velocity (127)", 
        "Dynamic velocity",
        "Formula velocity (ƒV)",
        "No Notes",
        "Ethervox",
        "Kyma"
    });
    configSwitch(P_NOTE_PRIORITY, 0.f, 6.f, 0.f, "Note Priority", {
        "Oldest channel",
        "Same pitch",
        "Lowest channel",
        "Highest channel",
        "Highest 2 channels",
        "Highest 3 channels",
        "Highest 4 channels",
    });
    configSwitch(P_BASE_POLYPHONY, 0.f, 7.f, 3.f, "Base polyphony", {"1", "2", "3", "4", "5", "6", "7", "8"});
    configSwitch(P_EXPAND_POLYPHONY, 0.f, 1.f, 0.f, "Expand polyphony", offon);
    configSwitch(P_DOUBLE_COMPUTATION, 0.f, 1.f, 0.f, "Double computation rate", offon);
    configSwitch(P_MONO, 0.f, 1.f, 0.f, "Mono", offon);
    configSwitch(P_MONO_MODE, 0.f, 8.f, 0.f, "Mono mode", {
        "Weighted portamento",
        "Legato to high pressure",
        "Retrigger high pressure",
        "Legato to touch",
        "Retrig touch",
        "Retrig touch & lift",
        "Legato to touch, max Z",
        "Retrig touch, max Z",
        "Retrig touch & lift, max Z"
    });
    configSwitch(P_MONO_INTERVAL, 0.f, 14.f, 0.f, "Mono interval", {
        "Off",
        "1 semitone",
        "Second",
        "Minor third",
        "Major third",
        "Fourth",
        "Augmented fourth",
        "Fifth",
        "Sixth",
        "Minor seventh",
        "Major seventh",
        "1 octave",
        "2 octaves",
        "3 octaves",
        "Maximum",
    });
    configSwitch(P_OCTAVE_SWITCH, 0.f, 1.f, 0.f, "Octave switch", offon);
    configSwitch(P_OCTAVE_TYPE, 0.f, 2.f, 0.f, "Octave switch type", {
        "Switch",
        "Toggle",
        "Instant"
    });
    configSwitch(P_OCTAVE_RANGE, 0.f, 7.f, 3.f, "Octave switch range", {
        "-4 octaves",
        "-3 octaves",
        "-2 octaves",
        "-1 octave",
        "+1 octave",
        "+2 octaves",
        "+3 octaves",
        "+4 octaves",
    });
    configSwitch(P_ROUND_TYPE, 0.f, 3.f, 0.f, "Round type", {
        "Normal",
        "Release",
        "Y",
        "Reverse Y",
    });
    configSwitch(P_ROUND_INITIAL, 0.f, 1.f, 0.f, "Round initial", offon);
    configParam(P_ROUND_RATE, 0.f, 10.f, 0.f, "Round rate");

    configTuningParam(this, P_TUNING);

    configSwitch(P_MIDI_DSP,     0.f, 1.f, 1.f, "Route Midi to DSP",     offon);
    configSwitch(P_MIDI_CVC,     0.f, 1.f, 1.f, "Route Midi to CVC",     offon);
    configSwitch(P_MIDI_MIDI,    0.f, 1.f, 1.f, "Route Midi to MIDI",    offon);
    configSwitch(P_SURFACE_DSP,  0.f, 1.f, 1.f, "Route Surface to DSP",  offon);
    configSwitch(P_SURFACE_CVC,  0.f, 1.f, 1.f, "Route Surface to CVC",  offon);
    configSwitch(P_SURFACE_MIDI, 0.f, 1.f, 1.f, "Route Surface to MIDI", offon);
    configSwitch(P_KEEP_MIDI,    0.f, 1.f, 0.f, "Keep MIDI settings",    offon);
    dp4(configParam(P_MOD_AMOUNT, -100.f, 100.f, 0.f, "Modulation amount", "%")); 


    configInput(IN_ROUND_INITIAL, "Round initial gate");
    configInput(IN_ROUND_RATE, "Round rate");

    // button lights unconfigured to disable tooltip
    //configLight(L_MONO, "Mono mode");
    //configLight(L_OCTAVE_SWITCH, "Octave switch");

    configLight(L_ROUND_Y,       "Round on Y");
    configLight(L_ROUND_INITIAL, "Round initial");
    configLight(L_ROUND,         "Rounding");
    configLight(L_ROUND_RELEASE, "Round on release");

    // EmccPortConfig cfg[] = {
    // };
    //modulation.configure(Params::P_MOD_AMOUNT, NUM_MOD_PARAMS, cfg);
}

bool SettingsModule::connected()
{
    if (!chem_host) return false;
    auto conn = chem_host->host_connection(ChemDevice::Haken);
    return conn && conn->identified();
}

void SettingsModule::dataFromJson(json_t *root)
{
    ChemModule::dataFromJson(root);
    json_t* j = json_object_get(root, "haken-device");
    if (j) {
        device_claim = json_string_value(j);
    }
    if (!device_claim.empty()) {
        modulation.mod_from_json(root);
    }

    ModuleBroker::get()->try_bind_client(this);
}

json_t* SettingsModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    if (!device_claim.empty()) {
        modulation.mod_to_json(root);
    }
    return root;
}

void SettingsModule::update_from_em()
{
    if (!connected()) return;
    auto em = chem_host->host_matrix();
    if (!em->is_ready()) { return; }

    //modulation.set_em_and_param_low(P_PRE_LEVEL, em->get_pre(), true);
}

void SettingsModule::do_message(PackedMidiMessage message)
{
    if (Haken::ccStat1 != message.bytes.status_byte) return;
    if (as_u8(ChemId::Settings) == message.bytes.tag) return;

//    modulation.set_em_and_param_low(param, midi_cc_value(message), true);
}

// IChemClient
::rack::engine::Module* SettingsModule::client_module() { return this; }
std::string SettingsModule::client_claim() { return device_claim; }

void SettingsModule::onConnectHost(IChemHost* host)
{
    onConnectHostModuleImpl(this, host);
}

void SettingsModule::onPresetChange()
{
    if (connected()) {
        update_from_em();
        //if (chem_ui) ui()->onPresetChange(); // ui does nothing
    }
}

void SettingsModule::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    if (chem_ui) ui()->onConnectionChange(device, connection);
}

void SettingsModule::process_params(const ProcessArgs &args)
{
    // auto v = getParam(Params::P_MIX).getValue()*.1f;
    // getLight(Lights::L_MIX).setBrightnessSmooth(v, 45.f);

    //modulation.pull_mod_amount();
}

void SettingsModule::process(const ProcessArgs &args)
{
    ChemModule::process(args);

    if (!connected() || chem_host->host_busy()) return;

    if (0 == ((args.frame + id) % 45)) {
        process_params(args);
    }

    // if (modulation.sync_params_ready(args)) {
    //      modulation.sync_send();
    // }

    if (((args.frame + id) % 61) == 0) {
        //modulation.update_lights();

        auto em = chem_host->host_matrix();
        float rate   = em->get_round_rate() / 127.f;
        bool initial = em->is_round_initial();
        bool on_y    = em->get_round_mode() >= Haken::rViaY;
        bool release = em->get_round_mode() <= Haken::rRelease;
        getLight(Lights::L_ROUND_Y).setBrightness(1.0f * on_y);
        getLight(Lights::L_ROUND_INITIAL).setBrightness(1.0f * initial);
        getLight(Lights::L_ROUND).setBrightness(1.0f * rate);
        getLight(Lights::L_ROUND_RELEASE).setBrightness(1.0f * ((rate > 0.f) && release));
        
        float v = getParam(P_MONO).getValue();
        getLight(Lights::L_MONO).setBrightness(v);

        v = getParam(P_OCTAVE_SWITCH).getValue();
        getLight(Lights::L_OCTAVE_SWITCH).setBrightness(v);

        for (int p = Params::P_MIDI_DSP; p <= Params::P_SURFACE_MIDI; ++p) {
            v = getParam(p).getValue();
            getLight(p - Params::P_MIDI_DSP + L_MIDI_DSP).setBrightness(v);
        }
    }

}

Model *modelSettings = createModel<SettingsModule, SettingsUi>("chem-settings");

