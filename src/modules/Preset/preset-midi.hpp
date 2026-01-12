#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "em/midi-message.h"
#include "services/midi-devices.hpp"
#include "services/midi-io.hpp"
#include "services/midi-log.hpp"

using namespace pachde;
struct PresetModule;

enum LearnMode { Off, Note, Cc };
struct ILearner {
    virtual void learn_value(LearnMode mode, uint8_t value) = 0;
};

enum NavUnit { Page, Index};
struct INavigateList {
    virtual void nav_send() = 0;
    //virtual NavUnit nav_get_unit() = 0;
    virtual void nav_set_unit(NavUnit unit) = 0;
    virtual void nav_previous() = 0;
    virtual void nav_next() = 0;
    virtual void nav_item(uint8_t offset) = 0;
    virtual void nav_absolute(uint16_t offset) = 0;
};

enum PresetAction {
    KeySelect, // send current preset note on
    KeyPage,   // prev/next/index sets page
    KeyIndex,  // prev/next/index sets index
    KeyPrev,   // decrement page/index
    KeyNext,   // increment page/index
    KeyFirst,  // absolute index
    // CcSelect,  // send current preset cc > 0 (127)
    // CcPage,
    // CcIndex,
    // CcPrev,
    // CcNext,
    // CcFirst,
    Size
};

constexpr const uint8_t UndefinedCode = 0xFF;

struct PresetMidi: IDoMidi, IMidiDeviceNotify {

    INavigateList* client{nullptr};

    MidiDeviceHolder midi_device;
    std::string midi_device_claim;
    MidiInput midi_in;
    MidiLog* midi_log{nullptr};

    uint8_t channel{UndefinedCode}; // 0xFF == any
    uint8_t code[PresetAction::Size]{
        UndefinedCode, // KeySelect
        UndefinedCode, // KeyPage
        UndefinedCode, // KeyIndex
        UndefinedCode, // KeyPrev
        UndefinedCode, // KeyNext
        UndefinedCode, // KeyFirst
        // UndefinedCode,
        // UndefinedCode,
        // UndefinedCode,
        // UndefinedCode,
        // UndefinedCode,
        // UndefinedCode
    };
    bool page_mode{false};

    LearnMode learn{LearnMode::Off};
    ILearner* student{nullptr};

    PresetMidi(ChemId client_id, ChemDevice device);
    void init(INavigateList* nav);

    ~PresetMidi();

    void fromJson(json_t *root);
    json_t* toJson();

    std::string connection_name();
    bool is_valid_configuration();
    void set_student(ILearner* client);
    void start_learning(LearnMode what);
    void stop_learning();
    void enable_logging(bool logging);
    bool is_logging();
    bool is_connected() { return midi_device.connected(); }
    void do_keyboard_cursor(PackedMidiMessage msg);
    void do_keyboard_range(PackedMidiMessage msg);
    void learn_keyboard(PackedMidiMessage msg);

    void process(float sampleTime);

    // IMidiDeviceHolder
    void onMidiDeviceChange(const MidiDeviceHolder* source) override;

    void do_message(PackedMidiMessage msg) override;
};
