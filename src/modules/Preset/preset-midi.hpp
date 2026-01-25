#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "em/midi-message.h"
#include "services/midi-devices.hpp"
#include "services/midi-io.hpp"
#include "services/midi-log.hpp"

using namespace pachde;
struct PresetModule;

inline ssize_t index_from_paged(uint8_t page, uint8_t offset, ssize_t page_size) {
    return (page * page_size) + offset;
}
inline ssize_t page_of_index(ssize_t index, ssize_t page_size) {
    if (index < 0) return 0;
    return index / page_size;
}
inline ssize_t offset_of_index(ssize_t index, ssize_t page_size) {
    if (index < 0) return 0;
    return index % page_size;
}

constexpr const uint8_t UndefinedCode{0xFF};
constexpr const uint8_t AnyChannel{UndefinedCode};
inline bool defined(uint8_t code) { return UndefinedCode != code; }
inline bool undefined(uint8_t code) { return UndefinedCode == code; }

enum LearnMode { Off, Note, Cc };
enum class ControllerType { Unknown, Toggle, Momentary, Continuous, Endless };
const char * controller_type_name(ControllerType ct);
ControllerType parse(const char * text);

struct ILearner {
    virtual void learn_value(LearnMode mode, PackedMidiMessage msg) = 0;
};

struct INavigateList {
    virtual void nav_send() = 0;
    virtual ssize_t nav_get_index() = 0;
    virtual void nav_set_index(ssize_t index) = 0;
    virtual ssize_t nav_get_page_size() = 0;
    virtual ssize_t nav_get_size() = 0; // current count of items
};

enum KeyAction {
    KeySelect, // send current preset (note on)
    KeyPage,   // prev/next/index sets page
    KeyIndex,  // prev/next/index sets index
    KeyPrev,   // decrement page/index
    KeyNext,   // increment page/index
    KeyFirst,  // offset from page
    Size
};
enum class ccAction {
    Unknown,
    Select,
    Page,
    Index
    // ToggleMode,
    // PageMode,
    // IndexMode,
    // Prev,
    // Next
};

struct CcControl {
    uint8_t cc{UndefinedCode};
    uint8_t last_value{UndefinedCode};
    uint8_t base_value{UndefinedCode};
    ccAction role{ccAction::Unknown};
    ControllerType kind{ControllerType::Unknown};

    void init(const CcControl& source) {
        cc         = source.cc;
        last_value = source.last_value;
        base_value = source.base_value;
        role       = source.role;
        kind       = source.kind;
    }
    void fromJson(json_t* root);
    json_t* to_json() const;

    void clear();
};

struct PresetMidi: IDoMidi, IMidiDeviceNotify {

    INavigateList* client{nullptr};

    MidiDeviceHolder midi_device;
    std::string midi_device_claim;
    MidiInput midi_in;
    MidiLog* midi_log{nullptr};

    // keys config
    bool key_mute{false};
    bool key_paging{false};
    uint8_t key_channel{UndefinedCode}; // 0xFF == any
    uint8_t key_code[KeyAction::Size] {
        UndefinedCode, // KeySelect
        UndefinedCode, // KeyPage
        UndefinedCode, // KeyIndex
        UndefinedCode, // KeyPrev
        UndefinedCode, // KeyNext
        UndefinedCode, // KeyFirst
    };

    // cc config
    uint8_t cc_channel{UndefinedCode};
    uint8_t cc_current_page{UndefinedCode};
    std::vector<CcControl> cc_control;

    // learning
    LearnMode learn{LearnMode::Off};
    ILearner* student{nullptr};

    PresetMidi(ChemId client_id, ChemDevice device);
    void init(INavigateList* nav);

    ~PresetMidi();

    void fromJson(json_t *root);
    json_t* toJson();

    std::string connection_name();
    bool some_key_configuration();
    bool is_valid_key_configuration();
    void reset_keyboard();
    void mute_keys(bool mute) { key_mute = mute; }
    bool key_muted() { return key_mute; }
    void set_student(ILearner* client);
    void start_learning(LearnMode what);
    void stop_learning();
    void enable_logging(bool logging);
    bool is_logging();
    bool is_connected() { return midi_device.connected(); }
    void learn_keyboard(PackedMidiMessage msg);
    void learn_cc(PackedMidiMessage msg);

    void process(float sampleTime);

    // IMidiDeviceHolder
    void onMidiDeviceChange(const MidiDeviceHolder* source) override;

    void do_key(PackedMidiMessage msg);
    void do_cc(PackedMidiMessage msg);
    void do_message(PackedMidiMessage msg) override;
};
