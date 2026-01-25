#include "midi-learn.hpp"
#include "em/wrap-HakenMidi.hpp"

namespace widgetry {

void draw_learning_frame(
    const widget::Widget::DrawArgs &args,
    Rect box,
    bool active,
    bool ok,
    PackedColor co_active,
    PackedColor co_normal
) {
    auto vg = args.vg;
    if (active || !ok) {
        NVGcolor co = fromPacked(ok ? co_active: colors::BrightRed);
        nvgBeginPath(vg);
        nvgMoveTo(vg, 1.125f, 0);
        nvgLineTo(vg, 1.125f, box.size.y);
        nvgMoveTo(vg, box.size.x-1.5f, 0);
        nvgLineTo(vg, box.size.x-1.5f, box.size.y);
        nvgStrokeColor(vg, co);
        nvgStrokeWidth(vg, 2.25);
        nvgStroke(vg);
    }
    nvgBeginPath(vg);
    nvgMoveTo(vg, 4.f, 0);
    nvgLineTo(vg, 0, 0);
    nvgLineTo(vg, 0, box.size.y);
    nvgLineTo(vg, 4.f, box.size.y);

    nvgMoveTo(vg, box.size.x - 4.f, 0);
    nvgLineTo(vg, box.size.x, 0);
    nvgLineTo(vg, box.size.x, box.size.y);
    nvgLineTo(vg, box.size.x - 4.f, box.size.y);

    nvgStrokeColor(vg, fromPacked(co_normal));
    nvgStrokeWidth(vg, .5);
    nvgStroke(vg);
}

const char * controller_type_name(ControllerType ct) {
    switch (ct) {
    default:
    case ControllerType::Unknown:    return "";
    case ControllerType::Toggle:     return "Toggle";
    case ControllerType::Momentary:  return "Momentary";
    case ControllerType::Continuous: return "Continuous";
    case ControllerType::Endless:    return "Endless";
    }
}

void MidiLearnerBase::applyTheme(std::shared_ptr<SvgTheme> theme) {
    if (!theme->getFillColor(co_normal, "learn", true)) {
        co_normal = colors::G65;
    }
    if (!theme->getFillColor(co_active, "learn", true)) {
        co_active = parseColor("hsl(42, .6, .8)", colors::White);
    }
}

void MidiLearnerBase::onButton(const ButtonEvent &e){
    if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) {
        e.consume(this); // allow onSelect, etc...
    }
    Base::onButton(e);
}

void MidiLearnerBase::onSelect(const SelectEvent &e){
    active = true;
    midi_handler->set_student(this);
    midi_handler->start_learning(mode);
    Base::onSelect(e);
}

void MidiLearnerBase::onSelectKey(const SelectKeyEvent &e){
    auto mods = e.mods & RACK_MOD_MASK;
    if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
        switch (e.key) {
        case GLFW_KEY_ESCAPE:
            reset();
            e.consume(this);
            break;
        case GLFW_KEY_TAB:
            if (0 != (mods & RACK_MOD_SHIFT)) {
                if (prev_widget) {
                    APP->event->setSelectedWidget(prev_widget);
                    e.consume(this);
                }
            } else {
                if (next_widget) {
                    APP->event->setSelectedWidget(next_widget);
                    e.consume(this);
                }
            }
            break;
        }
    }
}

void MidiLearnerBase::onDeselect(const DeselectEvent &e){
    active = false;
    midi_handler->stop_learning();
    midi_handler->set_student(nullptr);
    Base::onDeselect(e);
}

//
// LearnMidiControlType
//

LearnMidiControlType::LearnMidiControlType(Rect bounds, PresetMidi *pm) {
    box = bounds;
    midi_handler = pm;
    mode = LearnMode::Cc;
    reset();
}

void LearnMidiControlType::onSelect(const SelectEvent &e) {
    Base::onSelect(e);
    clock.start(LEARN_TIME);
}

void LearnMidiControlType::onDeselect(const DeselectEvent &e) {
    clock.stop();
    Base::onDeselect(e);
}

void LearnMidiControlType::reset() {
    cc = UndefinedCode;
    min_value = 0xFF;
    max_value = 0xFF;
    endless = false;
    msg_count = 0;
    last_message = INVALID_MSG;
    controller_type = ControllerType::Unknown;
    cc_text = min_text = max_text = "";
    ztoa = atoz = interior = false;
}

void LearnMidiControlType::learn_value(LearnMode mode, PackedMidiMessage msg) {
    assert (Haken::ctlChg == midi_status(msg));
    uint8_t new_cc = midi_cc(msg);
    auto cclast = midi_cc(last_message);
    if ((UndefinedCode != cclast) && (new_cc != cclast)) { // changed cc: start over
        reset();
    }
    uint8_t new_value = midi_cc_value(msg);
    if (0xFFFFFFFF == last_message.data) {
        // first cc
        msg_count = 1;
        cc = new_cc;
        cc_text = format_string("cc%d", cc);
        min_value = max_value = new_value;
        min_text = format_string("%d", min_value);
        max_text = min_text;
    } else {
        msg_count++;
        auto last_value = midi_cc_value(last_message);
        switch (new_value) {
        case 0:
            if (127 == last_value) {
                ztoa = true;
            }
            break;
        case 127:
            if (0 == last_value) {
                atoz = true;
            }
            break;
        default:
            interior = true;
            break;
        }
        if (min_value != new_value) {
            min_value = std::min(min_value, new_value);
            min_text = format_string("%d", min_value);
        }
        if (max_value != new_value) {
            max_value = std::max(max_value, new_value);
            max_text = format_string("%d", max_value);
        }
    }
    last_message = msg;
}

float LearnMidiControlType::remaining() {
    return clock.running() ? LEARN_TIME - clock.elapsed() : 0.f;
}

void LearnMidiControlType::evaluate_controller_type() {
    if (msg_count > 4) {
        controller_type = (interior && (ztoa || atoz)) ? ControllerType::Endless : ControllerType::Continuous;
    } else if (msg_count == 4) {
        controller_type = ControllerType::Momentary;
    } else if (msg_count == 2) {
        controller_type = ControllerType::Toggle;
    } else {
        controller_type = ControllerType::Unknown;
    }
}

void LearnMidiControlType::step() {
    Base::step();
    if (clock.running() && clock.finished()) {
        clock.stop();
        midi_handler->stop_learning();
        evaluate_controller_type();
    }
}

void LearnMidiControlType::draw(const DrawArgs &args) {
    if (!cc_text.empty()) {
        draw_text_box(args.vg, 0, 0, box.size.x, box.size.y,
            0.f,0.f,0.f,0.f, // no margins
            cc_text, GetPluginFontSemiBold(), 10.f,
            active ? co_active : co_normal, HAlign::Center, VAlign::Middle
        );
    }
    draw_learning_frame(args, box, active, midi_handler->is_connected(), co_active, co_normal);
}

//
// LearnMidiNote
//

LearnMidiNote::LearnMidiNote(Rect bounds, KeyAction role, PresetMidi *pm) :
    role(role)
{
    box = bounds;
    this->mode = LearnMode::Note;
    midi_handler = pm;
    value = midi_handler->key_code[role];
    update_text();
}

void LearnMidiNote::learn_value(LearnMode mode, PackedMidiMessage msg) {
    midi_handler->key_code[role] = value = midi_note(msg);
    update_text();
}

void LearnMidiNote::reset() {
    midi_handler->key_code[role] = value = UndefinedCode;
    value_text = "";
}

void LearnMidiNote::draw(const DrawArgs &args) {
    auto vg = args.vg;

    std::string text = value_text;
    if (!active && text.empty()) {
        text = "unset";
    }
    if (!text.empty()) {
        draw_text_box(vg, 0, 0, box.size.x, box.size.y,
            0.f,0.f,0.f,0.f, // no margins
            text, GetPluginFontSemiBold(), 10.f,
            active ? co_active : co_normal, HAlign::Center, VAlign::Middle
        );
    }
    draw_learning_frame(args, box, active, midi_handler->is_connected(), co_active, co_normal);
}

//
// LearnMidiCcSelect
//

LearnMidiCcSelect::LearnMidiCcSelect(Rect bounds, PresetMidi *pm) {
    box = bounds;
    midi_handler = pm;

}

void LearnMidiCcSelect::learn_value(LearnMode mode, PackedMidiMessage msg) {
}

void LearnMidiCcSelect::reset() {
    midi_handler->cc_code[ccAction::ccSelect] = UndefinedCode;
}

void LearnMidiCcSelect::draw(const DrawArgs &args) {
    //auto vg = args.vg;
    if (active) {

    } else {

    }
    draw_learning_frame(args, box, active, midi_handler->is_connected(), co_active, co_normal);

}
}