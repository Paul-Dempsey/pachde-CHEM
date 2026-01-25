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

inline bool defined(uint8_t code) { return UndefinedCode != code; }
inline bool undefined(uint8_t code) { return UndefinedCode == code; }

bool PresetMidi::some_key_configuration() {
    uint8_t* pc = &key_code[0];
    uint8_t* lim = pc + KeyAction::Size;
    while (undefined(*pc) && (pc < lim)) {
        pc++;
    }
    return pc != lim;
}

bool PresetMidi::is_valid_key_configuration() {
    if (undefined(key_code[KeyAction::KeySelect])) {
        return false;
    }

    if (key_code[KeyAction::KeyPrev] == key_code[KeyAction::KeyNext]) {
        return false;
    }

    if (   undefined(key_code[KeyAction::KeyPrev])
        && undefined(key_code[KeyAction::KeyNext])
        && undefined(key_code[KeyAction::KeyFirst])
    ) {
        return false;
    }

    auto first_code = key_code[KeyAction::KeyFirst];
    if (defined(first_code)) {
        for (int i = KeyAction::KeySelect; i < KeyAction::KeyFirst; i++) {
            if (defined(key_code[i]) && (first_code <= key_code[i])) {
                return false;
            }
        }
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

    if (undefined(key_channel) || (midi_channel(msg) == key_channel)) {
        student->learn_value(learn, msg);
    }
}

void PresetMidi::learn_cc(PackedMidiMessage msg) {
    if (!student || (Haken::ctlChg != midi_status(msg))) return;
    if (undefined(cc_channel) || (midi_channel(msg) == cc_channel)) {
        student->learn_value(learn, msg);
    }
}

void do_page(INavigateList* client, ssize_t page_dx) {
    if (0 == page_dx) return;
    ssize_t index = client->nav_get_index();
    ssize_t page_size = client->nav_get_page_size();
    ssize_t page = page_of_index(index, page_size);
    ssize_t offset = offset_of_index(index, page_size);
    page += page_dx;
    if (page >= 0) {
        // if (page_dx > 0) {
        //     ssize_t total = client->nav_get_size();
        //     if (total <= 0) return;
        //     auto last_page = page_of_index(total - 1);
        //     if (page > last_page) return;
        // }
        index = index_from_paged(page, offset, page_size);
        if (index < client->nav_get_size()) {
            client->nav_set_index(index);
        }
    }
}

void PresetMidi::do_key(PackedMidiMessage msg) {
    assert (client);
    assert(!key_mute);
    assert(midi_status(msg) == Haken::keyOn);
    assert(is_valid_key_configuration());

    uint8_t note = midi_note(msg);
    if (note == key_code[KeyAction::KeySelect]) {
        if (is_logging()) midi_log->log_message("PresetMidi", "send()");
        client->nav_send();
        return;
    }

    if (note == key_code[KeyAction::KeyPage]) {
        if ((note == key_code[KeyAction::KeyIndex]) || undefined(key_code[KeyAction::KeyIndex])) {
            // toggling
            key_paging = !key_paging;
            if (is_logging()) midi_log->log_message("PresetMidi", key_paging ? "Toggle mode to Page" : "Toggle mode to Index");
        } else {
            if (is_logging()) midi_log->log_message("PresetMidi", "Page mode");
            key_paging = true;
        }
        return;
    }

    if (note == key_code[KeyAction::KeyIndex]) {
        if (note == key_code[KeyAction::KeyPage] || undefined(key_code[KeyAction::KeyPage])) {
            // toggling
            key_paging = !key_paging;
            if (is_logging()) midi_log->log_message("PresetMidi", key_paging ? "Toggle mode to Ppage" : "Toggle mode to Index");
        } else {
            if (is_logging()) midi_log->log_message("PresetMidi", "Index mode");
            key_paging = false;
        }
        return;
    }

    if (note == key_code[KeyAction::KeyPrev]) {
        if (is_logging()) midi_log->log_message("PresetMidi", "Prev");
        ssize_t index = client->nav_get_index();
        if (key_paging) {
            do_page(client, -1);
        }
        else if (--index >= 0) {
            client->nav_set_index(index);
        }
        return;
    }

    if (note == key_code[KeyAction::KeyNext]) {
        if (is_logging()) midi_log->log_message("PresetMidi", "Next");
        ssize_t index = client->nav_get_index();
        if (key_paging) {
            do_page(client, 1);
        } else {
            index++;
            if (index < client->nav_get_size()) {
                client->nav_set_index(index);
            }
        }
        return;
    }

    auto first_code = key_code[KeyAction::KeyFirst];
    if (defined(first_code) && (note >= first_code)) {
        ssize_t increment = note - first_code;
        if (is_logging()) midi_log->log_message("PresetMidi", format_string("Index %d", increment));
        if (key_paging) {
            do_page(client, increment);
        } else {
            ssize_t page_size = client->nav_get_page_size();
            ssize_t index = client->nav_get_index();
            ssize_t page = page_of_index(index, page_size);
            index = std::min(index_from_paged(page, increment, page_size), client->nav_get_size()-1);
            client->nav_set_index(index);
        }
    }
}

void PresetMidi::do_cc(PackedMidiMessage msg)
{
    assert (client);
    const uint8_t cc = midi_cc(msg);
    if ((cc == cc_code[ccAction::ccSelect]) && (0 != midi_cc_value(msg))) {
        client->nav_send();
    }
    else if (cc == cc_code[ccAction::ccPage]) {
        ssize_t total = client->nav_get_size();
        if (total <= 0) return;
        ssize_t page_size = client->nav_get_page_size();
        ssize_t max_page = total/page_size;
        if (max_page) {
            ssize_t increment = 127/(1 + max_page);
            cc_current_page = std::min(static_cast<ssize_t>(midi_cc_value(msg)/increment), max_page);
        } else {
            cc_current_page = 0;
        }
        ssize_t index = client->nav_get_index();
        ssize_t offset = offset_of_index(index, page_size);
        index = index_from_paged(cc_current_page, offset, page_size);
        index = clamp(index, 0, total - 1);
        client->nav_set_index(index);
    }
    else if (cc == cc_code[ccAction::ccIndex]) {
        ssize_t total = client->nav_get_size();
        if (total <= 0) return;
        ssize_t page_size = client->nav_get_page_size();
        ssize_t offset = std::min(static_cast<ssize_t>(midi_cc_value(msg))/3, page_size-1);
        ssize_t index = index_from_paged(cc_current_page, offset, page_size);
        index = clamp(index, 0, total - 1);
        client->nav_set_index(index);
    }
}

void PresetMidi::do_message(PackedMidiMessage msg)
{
    if (!client) return;
    switch (learn) {
    case LearnMode::Off: {
        if (!key_mute) {
            if ((midi_status(msg) == Haken::keyOn)
                && ((AnyChannel == key_channel) || (midi_channel(msg) == key_channel))
                && is_valid_key_configuration()
            ) {
                do_key(msg);
                return;
            }
        }
        if ((midi_status(msg) == Haken::ctlChg)
            && ((AnyChannel == cc_channel) || (midi_channel(msg) == cc_channel))
        ) {
            do_cc(msg);
        }
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
