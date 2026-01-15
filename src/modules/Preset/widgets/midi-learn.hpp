#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../preset-midi.hpp"
#include "services/colors.hpp"
#include "services/svg-theme.hpp"
using namespace svg_theme;

namespace widgetry {

struct MidiLearner : OpaqueWidget, ILearner, IThemed {
    using Base = OpaqueWidget;
    bool active{false};
    uint8_t value{0xff};
    std::string value_text;
    LearnMode mode{LearnMode::Off};
    KeyAction role{KeyAction::KeySelect};
    PresetMidi* midi_handler{nullptr};
    PackedColor co_normal{colors::G85};
    PackedColor co_active{parseColor("hsl(120, .8, .5)", colors::White)};

    Widget* prev_widget{nullptr};
    Widget* next_widget{nullptr};

    MidiLearner(Rect bounds, LearnMode mode, KeyAction role, PresetMidi* pm) :
        mode(mode), role(role), midi_handler(pm)
    {
        box = bounds;
        value = midi_handler->key_code[role];
        update_text();
    }
    void update_text() { value_text = (0xFF == value) ? "" : noteFullName(value); }
    void applyTheme(std::shared_ptr<SvgTheme> theme) override {
        if (!theme->getFillColor(co_normal, "learn", true)) {
            co_normal = colors::G65;
        }
        if (!theme->getFillColor(co_active, "learn", true)) {
            co_active = parseColor("hsl(42, .6, .8)", colors::White);
        }
    }
    void learn_value(LearnMode mode, uint8_t new_value) override {
        midi_handler->key_code[role] = value = new_value;
        update_text();
    }
    void onButton( const ButtonEvent& e) override {
        e.consume(this);
        Base::onButton(e);
    }
    void onSelect(const SelectEvent& e) override {
        active = true;
        midi_handler->set_student(this);
        midi_handler->start_learning(mode);
        Base::onSelect(e);
    }
    void onSelectKey(const SelectKeyEvent& e) override {
        auto mods = e.mods & RACK_MOD_MASK;
        if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
            switch (e.key) {
            case GLFW_KEY_ESCAPE:
                midi_handler->key_code[role] = value = UndefinedCode;
                value_text = "";
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
    void onDeselect(const DeselectEvent& e) override {
        active = false;
        midi_handler->stop_learning();
        midi_handler->set_student(nullptr);
        Base::onDeselect(e);
    }

    void draw(const DrawArgs& args) override {
        auto vg = args.vg;
        if (active) {
            if (!value_text.empty()) {
                draw_text_box(vg, 0, 0, box.size.x, box.size.y,
                    0.f,0.f,0.f,0.f, // no margins
                    value_text, GetPluginFontSemiBold(), 10.f,
                    active ? co_active : co_normal, HAlign::Center, VAlign::Middle
                );
            }
        } else {
            draw_text_box(vg, 0, 0, box.size.x, box.size.y,
                0.f,0.f,0.f,0.f, // no margins
                value_text.empty() ? "unset" : value_text, GetPluginFontSemiBold(), 10.f,
                active ? co_active : co_normal, HAlign::Center, VAlign::Middle
            );
        }

        if (active) {
            nvgBeginPath(vg);
            nvgMoveTo(vg, 1.125f, 0);
            nvgLineTo(vg, 1.125f, box.size.y);
            nvgMoveTo(vg, box.size.x-1.5f, 0);
            nvgLineTo(vg, box.size.x-1.5f, box.size.y);
            nvgStrokeColor(vg, fromPacked(co_active));
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
        nvgStrokeWidth(vg, .75);
        nvgStroke(vg);
    }
};

}