#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../../em/midi-message.h"
#include "../../services/color-help.hpp"
#include "../../services/colors.hpp"
#include "../../widgets/element-style.hpp"
#include "../../widgets/TipWidget.hpp"
using namespace ::svg_theme;

namespace pachde {

const PackedColor DEFAULT_PAD_COLOR = 0xd08c8c8c;
const PackedColor DEFAULT_LIVE_PAD_COLOR = 0xff8c8c8c;

struct MidiPad
{
    int id;
    bool ok{true};
    PackedColor color;
    std::string name;    
    std::string def;
    std::vector<PackedMidiMessage> midi;
    std::string error_message;
    int error_pos;

    MidiPad(int id);
    MidiPad(json_t* j);

    bool compile();
    bool defined() { return ok && !midi.empty(); }

    json_t * to_json();
    void from_json(json_t* root);
};

struct PadWidget : TipWidget, IApplyTheme
{
    using Base = TipWidget;

    int id{-1};
    bool selected{false};
    std::shared_ptr<MidiPad> pad{nullptr};
    std::function<void(int)> on_click{nullptr};
    TinyLight<WhiteLight>* light;

    bool wire{false};
    ElementStyle pad_style{"pad", DEFAULT_PAD_COLOR, DEFAULT_PAD_COLOR, .35f};
    ElementStyle pad_sel_style{"pad-sel", 0xffefef20, 0xffefef20, 1.f};

    PadWidget()
    {
        box.size = Vec(24.f, 24.f);
    }

    void init(
        int identifier, 
        std::shared_ptr<MidiPad> the_pad,
        Module* module,
        std::function<void(int)> callback)
    {
        id = identifier;
        set_pad(the_pad);
        on_click = callback;
        addChild(light = createLightCentered<TinyLight<WhiteLight>>(Vec(20,4), module, identifier));
    }

    void set_pad(std::shared_ptr<MidiPad> the_pad) {
        pad = the_pad;
        describe(pad ? pad->name : "");
        assert(!pad || id == pad->id);
    }

    void onButton(const ButtonEvent& e) override
    {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
            if (on_click) {
                on_click(id);
            }
        }
        Base::onButton(e);
    }

    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override
    {
        wire = (0 == theme->name.compare("Wire"));
        pad_style.apply_theme(theme);
        pad_sel_style.apply_theme(theme);
        return true;
    }

    void step() override
    {
        Base::step();
        
        if (pad && (pad->color == DEFAULT_PAD_COLOR) && pad->defined()) {
            pad->color = DEFAULT_LIVE_PAD_COLOR;
        }

        NVGcolor co{0};
        if (selected) {
            co = SCHEME_YELLOW;
        } else if (pad) {
            co = pad->ok ? SCHEME_GREEN : SCHEME_RED;
        }
        (*light->baseColors.begin()) = co;
    }

    void draw(const DrawArgs& args) override
    {
        Base::draw(args);

        auto vg = args.vg;
        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0, 0, 24, 24, 1.5);
        if (wire && (pad_style.width() > .01f)) {
            nvgStrokeColor(vg, fromPacked(pad ? pad->color : pad_style.stroke_color));
            nvgStrokeWidth(vg, pad_style.width());
            nvgStroke(vg);
         } else {
            nvgFillColor(vg, fromPacked(pad ? pad->color : pad_style.fill_color));
            nvgFill(vg);
        }

        if (selected) {
            nvgBeginPath(vg);
            nvgRoundedRect(vg, -.5, -.5, 25, 25, 1.5);
            nvgStrokeColor(vg, pad_sel_style.nvg_stroke_color());
            nvgStrokeWidth(vg, pad_sel_style.width());
            nvgStroke(vg);
        }
    }

};

}