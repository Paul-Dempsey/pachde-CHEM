#include "Settings.hpp"
#include "../../services/rack-help.hpp"
#include "../../em/wrap-HakenMidi.hpp"
using namespace pachde;

SettingsModule::SettingsModule() :
    in_mat_poke(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    std::vector<std::string> offon{"off", "on"};
    std::vector<std::string> onoff{"on", "off"};

    configSwitch(P_SURFACE_DIRECTION, 0.f, 1.f, 0.f, "Surface direction", {"normal", "reversed"});
    configBendParam(this, P_X, "X Bend Range");
    configYParam(this, P_Y, "Y assign");
    configZParam(this, P_Z, "Z assign");
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
    configSwitch(P_JACK_SHIFT, 0.f, 7.f, 3.f, "Octave switch range", {
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
    rounding_port.init(0, EmccPortConfig::cc(P_ROUND_RATE, IN_ROUND_RATE, -1, Haken::ch1, Haken::ccRoundRate, true));

    // button lights unconfigured to disable tooltip
    //configLight(L_MONO, "Mono mode");
    //configLight(L_OCTAVE_SWITCH, "Octave switch");

    configLight(L_ROUND_Y,       "Round on Y");
    configLight(L_ROUND_INITIAL, "Round initial");
    configLight(L_ROUND,         "Rounding");
    configLight(L_ROUND_RELEASE, "Round on release");
}

bool SettingsModule::connected()
{
    if (!chem_host) return false;
    auto conn = chem_host->host_connection(ChemDevice::Haken);
    return conn && conn->identified();
}

void SettingsModule::zero_modulation()
{
    getParam(P_MOD_AMOUNT).setValue(0);
    rounding_port.set_mod_amount(0);
}

void SettingsModule::dataFromJson(json_t *root)
{
    ChemModule::dataFromJson(root);
    json_t* j = json_object_get(root, "haken-device");
    if (j) {
        device_claim = json_string_value(j);
    }
    if (!device_claim.empty()) {
        //modulation.mod_from_json(root);
    }

    ModuleBroker::get()->try_bind_client(this);
}

json_t* SettingsModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
    if (!device_claim.empty()) {
        //modulation.mod_to_json(root);
    }
    return root;
}

void SettingsModule::onPortChange(const PortChangeEvent &e)
{
    if (e.type == Port::OUTPUT) return;
    if (e.portId == IN_ROUND_RATE) {
        if (e.connecting) {
            // nothing
        } else {
            zero_modulation();        
        }
    }
}

void SettingsModule::update_from_em(int param_id, uint8_t value) {
    em_values[param_id] = value;
    getParam(param_id).setValue(value);
}

void SettingsModule::update_from_em()
{
    if (!connected()) return;
    auto em = chem_host->host_matrix();
    if (!em->is_ready()) { return; }

    update_from_em(P_SURFACE_DIRECTION, em->is_reverse_surface());
    update_from_em(P_X, em->get_bend_range());
    update_from_em(P_Y, em->get_y_assign());
    update_from_em(P_Z, em->get_z_assign());
    update_from_em(P_NOTE_PROCESSING, em->get_note_mode());
    update_from_em(P_NOTE_PRIORITY, em->get_note_priority());
    update_from_em(P_BASE_POLYPHONY, em->get_base_polyphony());
    update_from_em(P_EXPAND_POLYPHONY, em->is_extend_polyphony());
    update_from_em(P_DOUBLE_COMPUTATION, em->is_increase_computation_rate());
    update_from_em(P_MONO, em->is_mono());
    update_from_em(P_MONO_MODE, em->get_mono_func());
    update_from_em(P_MONO_INTERVAL, em->get_mono_interval());
    update_from_em(P_OCTAVE_SWITCH, em->get_octave_shift());
    update_from_em(P_JACK_SHIFT, em->get_jack_shift());
    update_from_em(P_ROUND_TYPE, em->get_round_mode());
    update_from_em(P_ROUND_INITIAL, em->get_round_initial());

    uint8_t value;
    value = em->get_round_rate();
    em_values[P_ROUND_RATE] = value;
    rounding_port.set_em_and_param_low(value);

    value = em->get_tuning();
    em_values[P_TUNING] = value;
    getParamQuantity(P_TUNING)->setValue(packTuning(static_cast<Tuning>(value)));

    value = em->get_midi_routing();
    getParam(P_MIDI_DSP).setValue(value & Haken::bMidiInDsp);
    getParam(P_MIDI_CVC).setValue(value & Haken::bMidiInCvc);
    getParam(P_MIDI_MIDI).setValue(value & Haken::bMidiInTrad);
    getParam(P_SURFACE_DSP).setValue(value & Haken::bSurfaceDsp);
    getParam(P_SURFACE_CVC).setValue(value & Haken::bMidiInCvc);
    getParam(P_SURFACE_MIDI).setValue(value & Haken::bSurfaceTrad);
}

void SettingsModule::do_message(PackedMidiMessage message)
{
    if (as_u8(ChemId::Settings) == message.bytes.tag) return;

    switch (message.bytes.status_byte) {
    case Haken::ccStat1: {
        in_mat_poke = false;
        switch (midi_cc(message)) {
        case Haken::ccStream:
            in_mat_poke = Haken::s_Mat_Poke == midi_cc_value(message);
            return;

        case Haken::ccMonoOn:            
            update_from_em(P_MONO, midi_cc_value(message));
            break;

        case Haken::ccOctShift:
            update_from_em(P_OCTAVE_SWITCH, midi_cc_value(message));
            break;

        case Haken::ccRndIni: 
            update_from_em(P_ROUND_INITIAL, midi_cc_value(message));
            break;

        case Haken::ccRoundRate:
            auto value = midi_cc_value(message);
            em_values[P_ROUND_RATE] = value;
            rounding_port.set_em_and_param_low(value);
            break;
        }
    } break;

    case Haken::sData: // ch16 key pressure
        if (in_mat_poke) {
            int param_index = -1; //invalid

            switch (message.bytes.data1) {
            case Haken::idPrio:      param_index = P_NOTE_PRIORITY; break;
            case Haken::idNoteMode:  param_index = P_NOTE_PROCESSING; break;
            case Haken::idReverse:   param_index = P_SURFACE_DIRECTION; break;
            case Haken::idSwTogInst: param_index = P_OCTAVE_TYPE; break;
            case Haken::idRoundMode: param_index = P_ROUND_TYPE; break;
            case Haken::idOkExpPoly: param_index = P_EXPAND_POLYPHONY; break;
            case Haken::idRouting: {
                uint8_t routing = message.bytes.data2;
                update_from_em(P_MIDI_DSP, routing & Haken::bMidiInDsp);
                update_from_em(P_MIDI_CVC, routing & Haken::bMidiInCvc);
                update_from_em(P_MIDI_MIDI, routing & Haken::bMidiInTrad);
                update_from_em(P_SURFACE_DSP, routing & Haken::bSurfaceDsp);
                update_from_em(P_SURFACE_CVC, routing & Haken::bMidiInCvc);
                update_from_em(P_SURFACE_MIDI, routing & Haken::bSurfaceTrad);
            } break;
            case Haken::idPoly:      param_index = P_BASE_POLYPHONY; break;
            case Haken::idBendRange: param_index = P_X; break;
            case Haken::idFrontBack: param_index = P_Y; break;
            case Haken::idPressure:  param_index = P_Z; break;
            case Haken::idMonoFunc:  param_index = P_MONO_MODE; break;
            case Haken::idMonoInt:   param_index = P_MONO_INTERVAL; break;
            case Haken::idJackShift: param_index = P_JACK_SHIFT; break;
            case Haken::idPresEnc:   param_index = P_KEEP_MIDI; break;
            default: break;
            }
            if (param_index >= 0) {
                update_from_em(param_index, message.bytes.data2);
            }
        }
        break;

    case Haken::ccStat16:
        switch (midi_cc(message)) {
        case Haken::ccStream:
            in_mat_poke = (Haken::s_Mat_Poke == midi_cc_value(message));
            break;

        case Haken::ccGrid:
            update_from_em(P_TUNING, packTuning((Tuning)midi_cc_value(message)));
            break;
        }
        break;

    default: break;
    }
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
    rounding_port.set_mod_amount(getParamQuantity(P_MOD_AMOUNT)->getValue());
}

void SettingsModule::process(const ProcessArgs &args)
{
    ChemModule::process(args);

    if (!connected() || chem_host->host_busy()) return;

    if (0 == ((args.frame + id) % 45)) {
        process_params(args);
    }

    if (midi_timer.process(args.sampleTime) > MOD_MIDI_RATE) {
        midi_timer.reset();

        rounding_port.pull_param_cv(this);
        rounding_port.send(chem_host, ChemId::Settings);
    }

    if (((args.frame + id) % 61) == 0) {
        auto em = chem_host->host_matrix();

        round_leds.set_initial(em->is_round_initial());
        round_leds.set_mode(em->get_round_mode());
        round_leds.set_rate(em->get_round_rate());
        round_leds.update_lights(this, Lights::L_ROUND);
    
        octave.set_shift(em->get_octave_shift());
        octave.update_lights(this, Lights::L_OCT_SHIFT_FIRST);

        float v;
        
        v = getParam(P_MONO).getValue();
        getLight(Lights::L_MONO).setBrightness(v);

        for (int p = Params::P_MIDI_DSP; p <= Params::P_SURFACE_MIDI; ++p) {
            v = getParam(p).getValue();
            getLight(p - Params::P_MIDI_DSP + L_MIDI_DSP).setBrightness(v);
        }
    }
}

Model *modelSettings = createModel<SettingsModule, SettingsUi>("chem-settings");

