#include "preset-midi.hpp"
#include "chem-id.hpp"
#include "em/wrap-HakenMidi.hpp"
#include "services/json-help.hpp"

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
    key_code[KeyAction::KeySelect] = get_json_int(root, "key-select", key_code[KeyAction::KeySelect]);
    key_code[KeyAction::KeyPage]   = get_json_int(root, "key-page",   key_code[KeyAction::KeyPage]);
    key_code[KeyAction::KeyIndex]  = get_json_int(root, "key-index",  key_code[KeyAction::KeyIndex]);
    key_code[KeyAction::KeyPrev]   = get_json_int(root, "key-prev",   key_code[KeyAction::KeyPrev]);
    key_code[KeyAction::KeyNext]   = get_json_int(root, "key-next",   key_code[KeyAction::KeyNext]);
    key_code[KeyAction::KeyFirst]  = get_json_int(root, "key-first",  key_code[KeyAction::KeyFirst]);
}

json_t *PresetMidi::toJson()
{
    json_t* root = json_object();
    set_json(root, "midi-device", midi_device_claim);
    set_json(root, "midi-log", is_logging());
    set_json_int(root, "key-select", key_code[KeyAction::KeySelect]);
    set_json_int(root, "key-page",   key_code[KeyAction::KeyPage]);
    set_json_int(root, "key-index",  key_code[KeyAction::KeyIndex]);
    set_json_int(root, "key-prev",   key_code[KeyAction::KeyPrev]);
    set_json_int(root, "key-next",   key_code[KeyAction::KeyNext]);
    set_json_int(root, "key-first",  key_code[KeyAction::KeyFirst]);
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
    return midi_device.connection
        ? midi_device.connection->info.friendly(NameFormat::Short)
        : "[no connection]";
}

bool PresetMidi::some_configuration() {
    uint8_t* pc = &key_code[0];
    uint8_t* lim = pc + KeyAction::Size;
    while ((UndefinedCode == *pc) && (pc < lim)) {
        pc++;
    }
    return pc != lim;
}

bool PresetMidi::is_valid_configuration() {
    if (UndefinedCode == key_code[KeyAction::KeySelect]) {
        return false;
    }
    if (key_code[KeyAction::KeyPrev] == key_code[KeyAction::KeyNext]) {
        return false;
    }
    if (   (UndefinedCode == key_code[KeyAction::KeyPrev])
        && (UndefinedCode == key_code[KeyAction::KeyNext])
        && (UndefinedCode == key_code[KeyAction::KeyFirst])
    ) {
        return false;
    }
    if (UndefinedCode != key_code[KeyAction::KeyFirst]) {
        if (   (key_code[KeyAction::KeyFirst] == key_code[KeyAction::KeySelect])
            || (key_code[KeyAction::KeyFirst] == key_code[KeyAction::KeyPage])
            || (key_code[KeyAction::KeyFirst] == key_code[KeyAction::KeyIndex])
            || (key_code[KeyAction::KeyFirst] == key_code[KeyAction::KeyPrev])
            || (key_code[KeyAction::KeyFirst] == key_code[KeyAction::KeyNext])
        ) return false;
    }

    return true;
}

void PresetMidi::reset_keyboard() {
    key_channel = UndefinedCode;
    memset(key_code, UndefinedCode, KeyAction::Size);
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

void PresetMidi::enable_logging(bool logging) {
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

void PresetMidi::process(float sample_time) {
    midi_in.dispatch(sample_time);
}

void PresetMidi::learn_keyboard(PackedMidiMessage msg) {
    if (!student || (Haken::keyOn != midi_status(msg))) return;

    const uint8_t msg_channel{midi_channel(msg)};
    if ((UndefinedCode == key_channel) || (msg_channel == key_channel)) {
        uint8_t note = midi_note(msg);
        student->learn_value(learn, note);
    }
}

void PresetMidi::learn_cc(PackedMidiMessage msg) {
}

void PresetMidi::do_key(PackedMidiMessage msg) {
    assert(!key_mute);
    assert(midi_status(msg) == Haken::keyOn);
    assert(is_valid_configuration());

    uint8_t note = midi_note(msg);
    if (note == key_code[KeyAction::KeySelect]) {
        if (is_logging()) midi_log->log_message("PresetMidi", "send()");
        client->nav_send();
        return;
    }

    if (note == key_code[KeyAction::KeyPage]) {
        if ((note == key_code[KeyAction::KeyIndex]) || (UndefinedCode == key_code[KeyAction::KeyIndex])) {
            // toggling
            if (is_logging()) midi_log->log_message("PresetMidi", "Toggle mode");
            key_paging = !key_paging;
            client->nav_set_unit(key_paging ? NavUnit::Page : NavUnit::Index);
        } else {
            if (is_logging()) midi_log->log_message("PresetMidi", "Page mode");
            client->nav_set_unit(NavUnit::Page);
            key_paging = true;
        }
        return;
    }

    if (note == key_code[KeyAction::KeyIndex]) {
        if (note == key_code[KeyAction::KeyPage]  || (UndefinedCode == key_code[KeyAction::KeyPage])) {
            // toggling
            if (is_logging()) midi_log->log_message("PresetMidi", "Toggle mode");
            key_paging = !key_paging;
            client->nav_set_unit(key_paging ? NavUnit::Page : NavUnit::Index);
        } else {
            if (is_logging()) midi_log->log_message("PresetMidi", "Index mode");
            client->nav_set_unit(NavUnit::Index);
            key_paging = false;
        }
        return;
    }

    if (note == key_code[KeyAction::KeyPrev]) {
        if (is_logging()) midi_log->log_message("PresetMidi", "Prev");
        client->nav_previous();
        return;
    }

    if (note == key_code[KeyAction::KeyNext]) {
        if (is_logging()) midi_log->log_message("PresetMidi", "Next");
        client->nav_next();
        return;
    }

    if ((UndefinedCode != key_code[KeyAction::KeyFirst])
        && (note >= key_code[KeyAction::KeyFirst])
    ) {
        if (is_logging()) midi_log->log_message("PresetMidi", format_string("Index %d", note - key_code[KeyAction::KeyFirst]));
        client->nav_item(note - key_code[KeyAction::KeyFirst]);
    }
}

void PresetMidi::do_message(PackedMidiMessage msg)
{
    if (!client) return;
    switch (learn) {
    case LearnMode::Off: {
        if (key_mute) return;
        if (!((UndefinedCode == key_channel) || (midi_channel(msg) == key_channel))) return;
        if (midi_status(msg) != Haken::keyOn) return;
        if (!is_valid_configuration()) return;
        do_key(msg);
    } break;

    case LearnMode::Note:
        assert(student);
        learn_keyboard(msg);
        break;

    case LearnMode::Cc:
        assert(student);
        learn_cc(msg);
        break;
    }
}
