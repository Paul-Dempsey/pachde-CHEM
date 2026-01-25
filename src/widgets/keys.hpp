#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "services/colors.hpp"
#include "services/text.hpp"
#include "services/svg-theme.hpp"
#include "services/midi-note.hpp"
#include "tip-widget.hpp"

using namespace pachde;

namespace widgetry {

static const eNote whole_notes[] {eNote::C, eNote::D, eNote::E, eNote::F, eNote::G, eNote::A, eNote::B, };

inline bool whole_note(int n) {
    if (n < 0) return false;
    if (n < 5) return 0 == (n & 1);
    n -= 5;
    return 0 == (n & 1);
}

struct KeyboardWidget : OpaqueWidget {
    using Base = OpaqueWidget;

    bool vertical{false};
    std::function<void(eNote)> click_handler{nullptr};
    NVGcolor key_color[12];
    KeyboardWidget() { box.size = Vec(36, 24); reset_colors(); }
    KeyboardWidget(Rect bounds) { box = bounds; reset_colors(); }

    void set_vertical(bool v) { vertical = v; }
    void reset_colors() {
        NVGcolor white = WHITE;
        NVGcolor black = BLACK;
        key_color[eNote::C ] = white;
        key_color[eNote::Cs] = black;
        key_color[eNote::D ] = white;
        key_color[eNote::Eb] = black;
        key_color[eNote::E ] = white;
        key_color[eNote::F ] = white;
        key_color[eNote::Fs] = black;
        key_color[eNote::G ] = white;
        key_color[eNote::Ab] = black;
        key_color[eNote::A ] = white;
        key_color[eNote::Bb] = black;
        key_color[eNote::B ] = white;
    }

    void set_color(eNote note, PackedColor color) {
        //if (note < eNote::C || note > eNote::B) return;
        key_color[note] = fromPacked(color);
    }

    void set_handler(std::function<void(eNote)> callback) { click_handler = callback; }

    int hit_note(Vec pos) {
        if (pos.x < 0 || pos.x > box.size.x || pos.y < 0 || pos.y > box.size.y) return -1;

        if (vertical) {
            int x = static_cast<int>(box.size.y - pos.y);
            if (pos.x < box.size.x * .6) {
                int kw = static_cast<int>(box.size.y / 12.f);
                x = x / kw;
                return x;
            } else {
                int kw = static_cast<int>(box.size.x / 7.f);
                x = x / kw;
                return whole_notes[x];
            }
        } else {
            int x = static_cast<int>(pos.x);
            if (pos.y < box.size.y * .6) {
                int kw = static_cast<int>(box.size.x / 12.f);
                x = x / kw;
                return x;
            } else {
                int kw = static_cast<int>(box.size.x / 7.f);
                x = x / kw;
                return whole_notes[x];
            }
        }
    }
    void onButton(const ButtonEvent& e) override {
        auto click_note = hit_note(e.pos);
        if ((click_note >= 0)
            && (GLFW_MOUSE_BUTTON_LEFT == e.button)
            && (GLFW_PRESS == e.action)
            && click_handler
            ) {
            click_handler(static_cast<eNote>(click_note));
            e.consume(this);
        }
        Base::onButton(e);
    }

    void draw(const DrawArgs& args) override {
        auto vg = args.vg;
        NVGcolor black = BLACK;
        float x, ww, bw;
        float wide = vertical ? box.size.y : box.size.x;
        float high = vertical ? box.size.x : box.size.y;
        float unit = wide * 0.0188679f;
        bw = wide / 12.f;
        ww = wide / 7.f;

        x = 0;
        for (int i = 0; i < 7; i++) {
            FillRect(vg, x, 0, ww, high, key_color[whole_notes[i]]);
            x += ww;
        }

        float h = high * .6;

        // white lines
        float sw = std::max(.25f, unit);
        nvgBeginPath(vg);
        x = bw * 5 - unit*.5;
        nvgMoveTo(vg, x, 0.f); nvgLineTo(vg, x, box.size.y);
        x = ww;
        nvgMoveTo(vg, x, h); nvgLineTo(vg, x, box.size.y); x += ww;
        nvgMoveTo(vg, x, h); nvgLineTo(vg, x, box.size.y); x += ww;
        x += ww;
        nvgMoveTo(vg, x, h); nvgLineTo(vg, x, box.size.y); x += ww;
        nvgMoveTo(vg, x, h); nvgLineTo(vg, x, box.size.y); x += ww;
        nvgMoveTo(vg, x, h); nvgLineTo(vg, x, box.size.y); x += ww;
        nvgStrokeWidth(vg, sw);
        nvgStrokeColor(vg, black);
        nvgStroke(vg);

        // black keys
        x = bw;
        FillRect(vg, x, 0.f, bw, h, key_color[eNote::Cs]);
        //FittedBoxRect(vg, x, 0.f, bw, h, black, Fit::Inside, sw);
        x += 2*bw;
        FillRect(vg, x, 0.f, bw, h, key_color[eNote::Eb]);
        //FittedBoxRect(vg, x, 0.f, bw, h, black, Fit::Inside, sw);
        x += 3*bw;
        FillRect(vg, x, 0.f, bw, h, key_color[eNote::Fs]);
        //FittedBoxRect(vg, x, 0.f, bw, h, black, Fit::Inside, sw);
        x += 2*bw;
        FillRect(vg, x, 0.f, bw, h, key_color[eNote::Ab]);
        //FittedBoxRect(vg, x, 0.f, bw, h, black, Fit::Inside, sw);
        x += 2*bw;
        FillRect(vg, x, 0.f, bw, h, key_color[eNote::Bb]);
        //FittedBoxRect(vg, x, 0.f, bw, h, black, Fit::Inside, sw);

        FittedBoxRect(vg, 0, 0, box.size.x, box.size.y, black, Fit::Outside, .35);
   }
};

}