#include "preset-midi.hpp"
#include "chem-id.hpp"
#include "em/wrap-HakenMidi.hpp"
#include "services/json-help.hpp"

// Reset?
// midi_device.clear();

PresetMidi::~PresetMidi() {
    midi_in.clear();
    auto broker = MidiDeviceBroker::get();
    broker->unRegisterDeviceHolder(&midi_device);
    midi_device.unsubscribe(this);
}

PresetMidi::PresetMidi(ChemId client_id, ChemDevice device) :
    midi_in(client_id)
{
    midi_device.init(device, this);
    auto broker = MidiDeviceBroker::get();
    broker->registerDeviceHolder(&midi_device);

    midi_in.set_target(this);
}

void PresetMidi::init(INavigateList *nav) {
    client = nav;
}

void PresetMidi::fromJson(json_t *root) {
    if (!root) return;
    midi_device_claim = get_json_string(root, "midi-device");
    midi_device.set_claim(midi_device_claim);
    enable_logging(get_json_bool(root, "midi-log", is_logging()));
    code[PresetAction::KeySelect] = get_json_int(root, "key-select", code[PresetAction::KeySelect]);
    code[PresetAction::KeyPage]   = get_json_int(root, "key-page",   code[PresetAction::KeyPage]);
    code[PresetAction::KeyIndex]  = get_json_int(root, "key-index",  code[PresetAction::KeyIndex]);
    code[PresetAction::KeyPrev]   = get_json_int(root, "key-prev",   code[PresetAction::KeyPrev]);
    code[PresetAction::KeyNext]   = get_json_int(root, "key-next",   code[PresetAction::KeyNext]);
    code[PresetAction::KeyFirst]  = get_json_int(root, "key-first",  code[PresetAction::KeyFirst]);
    // code[PresetAction::CcSelect]  = get_json_int(root, "cc-select",  code[PresetAction::CcSelect]);
    // code[PresetAction::CcPage]    = get_json_int(root, "cc-page",    code[PresetAction::CcPage]);
    // code[PresetAction::CcIndex]   = get_json_int(root, "cc-index",   code[PresetAction::CcIndex]);
    // code[PresetAction::CcPrev]    = get_json_int(root, "cc-prev",    code[PresetAction::CcPrev]);
    // code[PresetAction::CcNext]    = get_json_int(root, "cc-next",    code[PresetAction::CcNext]);
    // code[PresetAction::CcFirst]   = get_json_int(root, "cc-first",   code[PresetAction::CcFirst]);
}

json_t *PresetMidi::toJson()
{
    json_t* root = json_object();
    set_json(root, "midi-device", midi_device_claim);
    set_json(root, "midi-log", is_logging());
    set_json_int(root, "key-select", code[PresetAction::KeySelect]);
    set_json_int(root, "key-page",   code[PresetAction::KeyPage]);
    set_json_int(root, "key-index",  code[PresetAction::KeyIndex]);
    set_json_int(root, "key-prev",   code[PresetAction::KeyPrev]);
    set_json_int(root, "key-next",   code[PresetAction::KeyNext]);
    set_json_int(root, "key-first",  code[PresetAction::KeyFirst]);
    // set_json_int(root, "cc-select",  code[PresetAction::CcSelect]);
    // set_json_int(root, "cc-page",    code[PresetAction::CcPage]);
    // set_json_int(root, "cc-index",   code[PresetAction::CcIndex]);
    // set_json_int(root, "cc-prev",    code[PresetAction::CcPrev]);
    // set_json_int(root, "cc-next",    code[PresetAction::CcNext]);
    // set_json_int(root, "cc-first",   code[PresetAction::CcFirst]);
    return root;
}

void PresetMidi::onMidiDeviceChange(const MidiDeviceHolder *source)
{
    if (source != &midi_device) return;
    if (source) {
        midi_device_claim = source->get_claim();
        if (source->connection) {
            midi_in.ring.clear();
            midi_in.setDriverId(source->connection->driver_id);
            midi_in.setDeviceId(source->connection->input_device_id);
            if (is_logging()) {
                midi_log->log_message("PresetMidi", format_string("Connecting %s", midi_device_claim.c_str()));
            }
        }
    } else {
        midi_device_claim.clear();
        midi_in.reset();
        if (is_logging()) {
            midi_log->log_message("PresetMidi", "Disconnect");
        }
    }
}

std::string PresetMidi::connection_name() {
    return midi_device.connection ? midi_device.connection->info.friendly(NameFormat::Short) : "[no connection]";
}

bool PresetMidi::is_valid_configuration() {
    if (UndefinedCode == code[PresetAction::KeySelect]) return false;
    if (code[PresetAction::KeyPrev] == code[PresetAction::KeyNext]) return false;
    if (   (UndefinedCode == code[PresetAction::KeyPrev])
        && (UndefinedCode == code[PresetAction::KeyNext])
        && (UndefinedCode == code[PresetAction::KeyFirst])
    ) return false;
    if (UndefinedCode != code[PresetAction::KeyFirst]) {
        if (   (code[PresetAction::KeyFirst] == code[PresetAction::KeySelect])
            || (code[PresetAction::KeyFirst] == code[PresetAction::KeyPage])
            || (code[PresetAction::KeyFirst] == code[PresetAction::KeyIndex])
            || (code[PresetAction::KeyFirst] == code[PresetAction::KeyPrev])
            || (code[PresetAction::KeyFirst] == code[PresetAction::KeyNext])
        ) return false;
    }
    return true;
}

void PresetMidi::set_student(ILearner *client) {
    if (nullptr == client) learn = LearnMode::Off;
    student = client;
}

void PresetMidi::start_learning(LearnMode what) {
    assert(student);
    learn = what;
}

void PresetMidi::stop_learning() {
    learn = LearnMode::Off;
}

void PresetMidi::enable_logging(bool logging)
{
    if (logging) {
        if (!midi_log) {
            midi_log = new MidiLog;
            midi_in.set_logger("<P", midi_log);
            midi_log->log_message("PresetMidi", format_string("Time %s", string::formatTimeISO(system::getUnixTime()).c_str()));
        }
    } else {
        auto log = midi_log;
        midi_log = nullptr;
        delete log;
    }
}

bool PresetMidi::is_logging() {
    return nullptr != midi_log;
}

void PresetMidi::learn_keyboard(PackedMidiMessage msg) {
    if (!student || (Haken::keyOn != midi_status(msg))) return;

    auto msg_channel = midi_channel(msg);
    if ((msg_channel == channel) || (UndefinedCode == channel)) {
        auto note = midi_note(msg);
        student->learn_value(learn, note);
    }
}

void PresetMidi::process(float sample_time) {
    midi_in.dispatch(sample_time);
}

void PresetMidi::do_message(PackedMidiMessage msg)
{
    if (!client) return;
    switch (learn) {
    case LearnMode::Off: {
        if (!((UndefinedCode == channel) || (midi_channel(msg) == channel))) return;
        if (midi_status(msg) != Haken::keyOn) return;
        if (!is_valid_configuration()) return;

        auto note = midi_note(msg);
        int action{0};
        bool handled{false};
        for(; !handled && action <= PresetAction::KeyFirst; action++) {
            if (note == code[action]) {
                switch (action) {
                case PresetAction::KeySelect:
                    if (is_logging()) midi_log->log_message("PresetMidi", "send()");
                    client->nav_send();
                    handled = true;
                    break;
                case PresetAction::KeyPage:
                    if ((note == code[PresetAction::KeyIndex]) || (UndefinedCode == code[PresetAction::KeyIndex])) {
                        // toggling
                        if (is_logging()) midi_log->log_message("PresetMidi", "Toggle mode");
                        page_mode = !page_mode;
                        client->nav_set_unit(page_mode ? NavUnit::Page : NavUnit::Index);
                    } else {
                        if (is_logging()) midi_log->log_message("PresetMidi", "Page mode");
                        client->nav_set_unit(NavUnit::Page);
                        page_mode = true;
                    }
                    handled = true;
                    break;
                case PresetAction::KeyIndex:
                    if (note == code[PresetAction::KeyPage]  || (UndefinedCode == code[PresetAction::KeyPage])) {
                        // toggling
                        if (is_logging()) midi_log->log_message("PresetMidi", "Toggle mode");
                        page_mode = !page_mode;
                        client->nav_set_unit(page_mode ? NavUnit::Page : NavUnit::Index);
                    } else {
                        if (is_logging()) midi_log->log_message("PresetMidi", "Index mode");
                        client->nav_set_unit(NavUnit::Index);
                        page_mode = false;
                    }
                    handled = true;
                    break;
                case PresetAction::KeyPrev:
                    if (is_logging()) midi_log->log_message("PresetMidi", "Prev");
                    client->nav_previous();
                    handled = true;
                    break;
                case PresetAction::KeyNext:
                    if (is_logging()) midi_log->log_message("PresetMidi", "Next");
                    client->nav_next();
                    handled = true;
                    break;
                case PresetAction::KeyFirst:
                    if (is_logging()) midi_log->log_message("PresetMidi", "Index 0");
                    client->nav_item(0);
                    handled = true;
                    break;
                }
            }
        }
        if (!handled
            && (UndefinedCode != code[PresetAction::KeyFirst])
            && (note > code[PresetAction::KeyFirst])
        ) {
            if (is_logging()) midi_log->log_message("PresetMidi", format_string("Index %d", note - code[PresetAction::KeyFirst]));
            client->nav_item(note - code[PresetAction::KeyFirst]);
            handled = true;
        }
    } break;

    case LearnMode::Note:
        assert(student);
        learn_keyboard(msg);
        break;

    case LearnMode::Cc:
        assert(student);
        break;
    }
}
