#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../preset-midi.hpp"
#include "services/colors.hpp"
#include "services/midi-note.hpp"
#include "services/svg-theme.hpp"

using namespace svg_theme;

namespace widgetry {

void draw_learning_frame(
    const widget::Widget::DrawArgs& args,
    Rect box,
    bool active,
    bool ok,
    PackedColor co_active,
    PackedColor co_normal);

struct MidiLearnerBase  : OpaqueWidget, ILearner, IThemed {
    using Base = OpaqueWidget;
    bool active{false};
    LearnMode mode{LearnMode::Off};
    PresetMidi* midi_handler{nullptr};
    PackedColor co_normal{colors::G85};
    PackedColor co_active{parseColor("hsl(120, .8, .5)", colors::White)};

    Widget* prev_widget{nullptr};
    Widget* next_widget{nullptr};

    virtual void reset() = 0;

    void applyTheme(std::shared_ptr<SvgTheme> theme) override;
    void onButton(const ButtonEvent& e) override;
    void onSelect(const SelectEvent& e) override;
    void onSelectKey(const SelectKeyEvent& e) override;
    void onDeselect(const DeselectEvent& e) override;

};

constexpr const PackedMidiMessage INVALID_MSG{0xFFFFFFFF};
constexpr const float LEARN_TIME{5.f};

struct LearnMidiControlType : MidiLearnerBase {
    using Base = MidiLearnerBase;

    uint8_t cc;
    uint8_t min_value;
    uint8_t max_value;
    int64_t msg_count;
    bool endless;

    WallTimer clock;

    std::string cc_text;
    std::string min_text;
    std::string max_text;
    PackedMidiMessage last_message{INVALID_MSG};
    ControllerType controller_type {ControllerType::Unknown};
    bool interior{false};
    bool ztoa{false};
    bool atoz{false};

    LearnMidiControlType(Rect bounds, PresetMidi* pm);

    void onSelect(const SelectEvent& e) override;
    void onDeselect(const DeselectEvent& e) override;
    void reset() override;
    void learn_value(LearnMode mode, PackedMidiMessage msg) override;
    bool learning() { return clock.running(); }
    float remaining();
    void evaluate_controller_type();
    void step() override;
    void draw(const DrawArgs& args) override;
};

struct LearnMidiNote : MidiLearnerBase {
    using Base = MidiLearnerBase;

    uint8_t value{0xff};
    std::string value_text;
    KeyAction role{KeyAction::KeySelect};

    LearnMidiNote(Rect bounds, KeyAction role, PresetMidi* pm);
    void update_text();
    void learn_value(LearnMode mode, PackedMidiMessage msg) override;
    void reset() override;
    void draw(const DrawArgs& args) override;
};

struct LearnMidiCcSelect : MidiLearnerBase {
    using Base = MidiLearnerBase;

    CcControl action;
    std::string cc_text;
    WallTimer clock;
    int message_count{0};

    LearnMidiCcSelect(Rect bounds, PresetMidi* pm);

    void init(const CcControl& ctl);
    void start_listen(uint8_t code);
    void update_text();
    void learn_value(LearnMode mode, PackedMidiMessage msg) override;
    void reset() override;
    void step() override;
    void draw(const DrawArgs& args) override;
};

}