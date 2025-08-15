// File is "SettingsModule.cpp" to avoid debugger confusion of same file name in Rack
#include "Settings.hpp"
#include "../../services/rack-help.hpp"
#include "../../em/wrap-HakenMidi.hpp"
using namespace pachde;

SettingsModule::SettingsModule() :
    in_mat_poke(false)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    std::vector<std::string> off_on{"off", "on"};
    std::vector<std::string> on_off{"on", "off"};

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
    configSwitch(P_BASE_POLYPHONY, 0.f, 7.f, 3.f, "Base polyphony", {"0", "1", "2", "3", "4", "5", "6", "7", "8"});
    configSwitch(P_EXPAND_POLYPHONY, 0.f, 1.f, 0.f, "Expand polyphony", off_on);
    configSwitch(P_DOUBLE_COMPUTATION, 0.f, 1.f, 0.f, "Double computation rate", off_on);
    configSwitch(P_MONO, 0.f, 1.f, 0.f, "Mono", off_on);
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

    configSwitch(P_ROUND_TYPE, 0.f, 3.f, 0.f, "Round type", {
        "Normal",
        "Release",
        "Y",
        "Reverse Y",
    });
    configSwitch(P_ROUND_INITIAL, 0.f, 1.f, 0.f, "Round initial", off_on);
    configParam(P_ROUND_RATE, 0.f, 10.f, 0.f, "Round rate");

    auto pq = configParam(P_TOUCH_AREA, 16.f, 108.f, 16.f, "Touch area", " nn");
    pq->snapEnabled = true;

    pq = configParam(P_MIDDLE_C, 0.f, 127.f, 60.f, "Middle C", " nn");
    pq->snapEnabled = true;

    pq = configParam(P_FINE, 4.f, 124.f, 64.f, "Fine tuning", "\u00a2", 0.f, 1.f, -64.f);
    pq->snapEnabled = true;

    pq = configParam(P_ACTUATION, 0.f, 127.f, 0.f, "Actuation");
    pq->snapEnabled = true;

    configParam(P_AUDIO_IN, 0.f, 10.f, 0.f, "Audio in level");

    configTuningParam(this, P_TUNING);

    configParam(P_MIDI_ROUTING, 0.f, Haken::defaultRoute, Haken::defaultRoute, "MIDI Routing");
    configSwitch(P_MIDI_DSP,     0.f, 1.f, 1.f, "Route Midi to DSP",     off_on);
    configSwitch(P_MIDI_CVC,     0.f, 1.f, 1.f, "Route Midi to CVC",     off_on);
    configSwitch(P_MIDI_MIDI,    0.f, 1.f, 1.f, "Route Midi to MIDI",    off_on);
    configSwitch(P_SURFACE_DSP,  0.f, 1.f, 1.f, "Route Surface to DSP",  off_on);
    configSwitch(P_SURFACE_CVC,  0.f, 1.f, 1.f, "Route Surface to CVC",  off_on);
    configSwitch(P_SURFACE_MIDI, 0.f, 1.f, 1.f, "Route Surface to MIDI", off_on);
    configSwitch(P_KEEP_MIDI,    0.f, 1.f, 0.f, "Keep MIDI settings across preset changes", off_on);
    configSwitch(P_KEEP_SURFACE, 0.f, 1.f, 0.f, "Keep Surface settings across preset changes", off_on);
    configSwitch(P_AES3, 0.f, 3.f, 0.f, "AES3", {
        "48k",
        "96k",
        "Sync to incoming"
    });

    dp4(configParam(P_MOD_AMOUNT, -100.f, 100.f, 0.f, "Modulation amount", "%"));

    configInput(IN_ROUND_INITIAL, "Round initial gate");
    configInput(IN_ROUND_RATE, "Round rate");
    rounding_port.init(0, EmccPortConfig::cc(P_ROUND_RATE, IN_ROUND_RATE, -1, Haken::ch1, Haken::ccRoundRate, true));

    // button lights unconfigured to disable tooltip
    //configLight(L_MONO, "Mono mode");

    configLight(L_ROUND_Y,       "Round on Y");
    configLight(L_ROUND_INITIAL, "Round initial");
    configLight(L_ROUND,         "Rounding");
    configLight(L_ROUND_RELEASE, "Round on release");
}

bool SettingsModule::connected()
{
    if (!chem_host || !chem_host->host_matrix()) return false;
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
    json_read_string(root, "haken-device", device_claim);
    ModuleBroker::get()->try_bind_client(this);
}

json_t* SettingsModule::dataToJson()
{
    json_t* root = ChemModule::dataToJson();
    json_object_set_new(root, "haken-device", json_string(device_claim.c_str()));
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

inline uint8_t is_bit(uint8_t value, uint8_t mask) {
    return 1 * (0 != (value & mask));
}

void SettingsModule::update_from_em()
{
    if (!connected()) return;
    auto em = chem_host->host_matrix();
    if (!em || !em->is_ready()) { return; }

    uint8_t value;

    update_from_em(P_SURFACE_DIRECTION, em->is_reverse_surface());
    update_from_em(P_MIDDLE_C, em->get_middle_c());
    update_from_em(P_TOUCH_AREA, em->get_touch_area());
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
    update_from_em(P_ROUND_TYPE, em->get_round_mode());
    update_from_em(P_ROUND_INITIAL, em->get_round_initial());
    update_from_em(P_KEEP_MIDI, em->is_keep_midi_encoding());
    update_from_em(P_KEEP_SURFACE, em->is_keep_surface_processing());
    update_from_em(P_ACTUATION, em->get_actuation());
    update_from_em(P_FINE, em->get_fine_tune());
    update_from_em(P_AES3, em->get_aes3());

    value = em->get_audio_in();
    em_values[P_AUDIO_IN] = value;
    audio_in_port.set_em_and_param_low(value);

    value = em->get_round_rate();
    em_values[P_ROUND_RATE] = value;
    rounding_port.set_em_and_param_low(value);

    value = em->get_tuning();
    em_values[P_TUNING] = value;
    getParamQuantity(P_TUNING)->setValue(packTuning(static_cast<Tuning>(value)));

    value = em->get_midi_routing();
    update_from_em(P_MIDI_ROUTING, value);
    em_values[P_MIDI_DSP] = is_bit(value, Haken::bMidiInDsp);
    em_values[P_MIDI_CVC] = is_bit(value, Haken::bMidiInCvc);
    em_values[P_MIDI_MIDI] = is_bit(value, Haken::bMidiInTrad);
    em_values[P_SURFACE_DSP] = is_bit(value, Haken::bSurfaceDsp);
    em_values[P_SURFACE_CVC] = is_bit(value, Haken::bSurfaceCvc);
    em_values[P_SURFACE_MIDI] = is_bit(value, Haken::bSurfaceTrad);

    getParam(P_MIDI_DSP).setValue(em_values[P_MIDI_DSP]);
    getParam(P_MIDI_CVC).setValue(em_values[P_MIDI_CVC]);
    getParam(P_MIDI_MIDI).setValue(em_values[P_MIDI_MIDI]);
    getParam(P_SURFACE_DSP).setValue(em_values[P_SURFACE_DSP]);
    getParam(P_SURFACE_CVC).setValue(em_values[P_SURFACE_CVC]);
    getParam(P_SURFACE_MIDI).setValue(em_values[P_SURFACE_MIDI]);
}

uint8_t SettingsModule::get_param_routing()
{
    uint8_t r = 0;
    if (getParamInt(getParam(P_MIDI_DSP))) r |= Haken::bMidiInDsp;
    if (getParamInt(getParam(P_MIDI_CVC))) r |= Haken::bMidiInCvc;
    if (getParamInt(getParam(P_MIDI_MIDI))) r |= Haken::bMidiInTrad;
    if (getParamInt(getParam(P_SURFACE_DSP))) r |= Haken::bSurfaceDsp;
    if (getParamInt(getParam(P_SURFACE_CVC))) r |= Haken::bSurfaceCvc;
    if (getParamInt(getParam(P_SURFACE_MIDI))) r |= Haken::bSurfaceTrad;
    return r;
}

void SettingsModule::do_message(PackedMidiMessage message)
{
    if (as_u8(ChemId::Settings) == message.bytes.tag) return;
    uint8_t value;
    switch (message.bytes.status_byte) {
    case Haken::ccStat1: {
        in_mat_poke = false;
        switch (midi_cc(message)) {
        case Haken::ccFineTune:
            update_from_em(P_FINE, midi_cc_value(message));
            break;

        case Haken::ccAudIn:
            value = midi_cc_value(message);
            em_values[P_AUDIO_IN] = value;
            audio_in_port.set_em_and_param_low(value);
            break;

        case Haken::ccRoundRate:
            value = midi_cc_value(message);
            em_values[P_ROUND_RATE] = value;
            rounding_port.set_em_and_param_low(value);
            break;

        case Haken::ccRndIni:
            update_from_em(P_ROUND_INITIAL, midi_cc_value(message));
            break;

        case Haken::ccActuation:
            update_from_em(P_ACTUATION, midi_cc_value(message));
            break;

        case Haken::ccMonoOn:
            update_from_em(P_MONO, midi_cc_value(message));
            break;
        }
    } break;

    case Haken::sData: // ch16 key pressure
        if (in_mat_poke) {
            int param_index = -1; //invalid

            switch (message.bytes.data1) {
            case Haken::idNoteMode:  param_index = P_NOTE_PROCESSING; break;
            case Haken::idPrio:      param_index = P_NOTE_PRIORITY; break;
            case Haken::idReverse:   param_index = P_SURFACE_DIRECTION; break;
            case Haken::idRoundMode: param_index = P_ROUND_TYPE; break;
            case Haken::idOkExpPoly: param_index = P_EXPAND_POLYPHONY; break;
            case Haken::idRouting: {
                param_index = P_MIDI_ROUTING;

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
            case Haken::idMiddleC:   param_index = P_MIDDLE_C; break;
            case Haken::idTArea:     param_index = P_TOUCH_AREA; break;
            case Haken::idMonoFunc:  param_index = P_MONO_MODE; break;
            case Haken::idMonoInt:   param_index = P_MONO_INTERVAL; break;
            case Haken::idPresEnc:   param_index = P_KEEP_MIDI; break;
            case Haken::idPresSurf:  param_index = P_KEEP_SURFACE; break;
            case Haken::idAes3:      param_index = P_AES3; break;
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

void SettingsModule::build_update(std::vector<PackedMidiMessage>& stream_data, int param_id, uint8_t haken_id)
{
    uint8_t value = getParamInt(getParam(param_id));
    if (value != em_values[param_id]) {
        stream_data.push_back(MakeStreamData(ChemId::Settings, haken_id, value));
        em_values[param_id] = value;
    }
}

void SettingsModule::process(const ProcessArgs &args)
{
    ChemModule::process(args);

    if (!host_connected(chem_host) || chem_host->host_busy()) return;
    auto haken = chem_host->host_haken();
    auto em = chem_host->host_matrix();

    if (0 == ((args.frame + id) % 45)) {
        process_params(args);
    }

    if (midi_timer.process(args.sampleTime) > MOD_MIDI_RATE) {
        midi_timer.reset();

        // cc
        rounding_port.pull_param_cv(this);
        rounding_port.send(chem_host, ChemId::Settings);
        em_values[P_ROUND_RATE] = rounding_port.em_low();

        audio_in_port.pull_param_cv(this);
        audio_in_port.send(chem_host, ChemId::Settings);
        em_values[P_AUDIO_IN] = audio_in_port.em_low();

        if (getInput(IN_ROUND_INITIAL).isConnected()) {
            auto v = getInput(IN_ROUND_INITIAL).getVoltage();
            bool high = v > 9.5f;
            bool low = v < .125f;
            bool initial = chem_host->host_matrix()->is_round_initial();
            if (high && (high != initial)) {
                haken->control_change(ChemId::Settings, Haken::ch1, Haken::ccRndIni, 1);
                update_from_em(P_ROUND_INITIAL, 1);
            } else if (low && (low != initial)) {
                haken->control_change(ChemId::Settings, Haken::ch1, Haken::ccRndIni, 0);
                update_from_em(P_ROUND_INITIAL, 0);
            }
        } else {
            bool initial = getParamInt(getParam(P_ROUND_INITIAL));
            if (initial != static_cast<bool>(em_values[P_ROUND_INITIAL])) {
                em_values[P_ROUND_INITIAL] = initial;
                haken->control_change(ChemId::Settings, Haken::ch1, Haken::ccRndIni, U8(initial));
            }
        }

        auto mono = getParamInt(getParam(P_MONO));
        if (mono != em_values[P_MONO]) {
            em_values[P_MONO] = mono;
            haken->control_change(ChemId::Settings, Haken::ch1, Haken::ccMonoOn, U8(mono));
        }

        auto fine = getParamInt(getParam(P_FINE));
        if (fine != em_values[P_FINE]) {
            haken->control_change(ChemId::Settings, Haken::ch1, Haken::ccFineTune, U8(fine));
        }

        // s_Mat_Poke
        std::vector<PackedMidiMessage> stream_data;

        uint8_t routing = get_param_routing();
        if (routing != em_values[P_MIDI_ROUTING]) {
            stream_data.push_back(MakeStreamData(ChemId::Settings, Haken::idRouting, routing));
            em_values[P_MIDI_ROUTING] = routing;
        }
        build_update(stream_data, P_SURFACE_DIRECTION, Haken::idReverse);
        build_update(stream_data, P_TOUCH_AREA, Haken::idTArea);
        build_update(stream_data, P_MIDDLE_C, Haken::idMiddleC);
        build_update(stream_data, P_X, Haken::idBendRange);
        build_update(stream_data, P_Y, Haken::idFrontBack);
        build_update(stream_data, P_Z, Haken::idPressure);
        build_update(stream_data, P_NOTE_PROCESSING, Haken::idNoteMode);
        build_update(stream_data, P_NOTE_PRIORITY, Haken::idPrio);
        build_update(stream_data, P_ROUND_TYPE, Haken::idRoundMode);
        build_update(stream_data, P_BASE_POLYPHONY, Haken::idPoly);
        build_update(stream_data, P_EXPAND_POLYPHONY, Haken::idOkExpPoly);
        build_update(stream_data, P_DOUBLE_COMPUTATION, Haken::idOkIncComp);
        build_update(stream_data, P_MONO_MODE, Haken::idMonoFunc);
        build_update(stream_data, P_MONO_INTERVAL, Haken::idMonoInt);
        build_update(stream_data, P_KEEP_MIDI, Haken::idPresEnc);
        build_update(stream_data, P_KEEP_SURFACE, Haken::idPresSurf);
        build_update(stream_data, P_AES3, Haken::idAes3);

        if (!stream_data.empty()) {
            haken->begin_stream(ChemId::Settings, Haken::s_Mat_Poke);
            for (auto msg: stream_data) {
                haken->send_message(msg);
            }
            haken->end_stream(ChemId::Settings);
        }
    }

    if (((args.frame + id) % 61) == 0) {
        round_leds.set_initial(em->is_round_initial());
        round_leds.set_mode(em->get_round_mode());
        round_leds.set_rate(em->get_round_rate());
        round_leds.update_lights(this, Lights::L_ROUND);

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

