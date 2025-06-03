#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "../../em/midi-message.h"
#include "../../services/color-help.hpp"
#include "../../services/colors.hpp"
#include "../../widgets/element-style.hpp"
#include "../../widgets/label-widget.hpp"
#include "../../widgets/TipWidget.hpp"
using namespace ::svg_theme;

namespace pachde {

const PackedColor DEFAULT_PAD_COLOR = 0xff8c8c8c;
const PackedColor DEFAULT_PAD_TEXT_COLOR = 0xffe6e6e6;
extern const char * default_pad_name[];

struct MidiPad
{
    int id;
    bool ok{true};
    PackedColor color;
    PackedColor text_color;
    std::string name;    
    std::string def;
    std::vector<PackedMidiMessage> midi;
    std::string error_message;
    int error_pos;

    MidiPad(int id);
    MidiPad(json_t* j);

    bool compile();
    bool defined() { return ok && !midi.empty(); }
    bool empty() {
        if (-1 == id) return true;
        if (def.empty()) {
            return name.empty() || (0 == name.compare(default_pad_name[id]));
        }
        return false;
    }
    void clear() {
        midi.clear();
        def.clear();
        error_message.clear();
        name = id >= 0 ? default_pad_name[id]: "";
        error_pos = 0;
    }
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
    TextLabel* label;

    bool wire{false};
    ElementStyle pad_style{"pad", DEFAULT_PAD_COLOR, DEFAULT_PAD_COLOR, .35f};
    ElementStyle pad_sel_style{"pad-sel", 0xffefef20, 0xffefef20, 1.f};

    PadWidget()
    {
        box.size = Vec(24.f, 24.f);
    }

    std::string extract_description()
    {
        if (!pad || pad->def.empty()) return "";
        const char * p = pad->def.c_str();
        while (std::isspace(*p)) ++p;
        if (*p == '"') {
            ++p;
            const char * start = p;
            while (*p && *p != '"') {
                ++p;
            }
            return std::string(start, p);
        }
        return "";
    }

    void init(
        int identifier, 
        std::shared_ptr<MidiPad> the_pad,
        Module* module,
        SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme,
        std::function<void(int)> callback)
    {
        id = identifier;
        on_click = callback;
        addChild(light = createLightCentered<TinyLight<WhiteLight>>(Vec(20,4), module, identifier));
        addChild(label = createLabel(Vec(12, 8.5), 24, the_pad ? the_pad->name : "", engine, theme, LabelStyle{"", TextAlignment::Center, 12.f}));
        applyTheme(engine, theme);
        set_pad(the_pad);
    }

    void set_pad(std::shared_ptr<MidiPad> the_pad)
    {
        pad = the_pad;
        assert(!pad || id == pad->id);
        on_pad_change(true, true);
    }

    void on_pad_change(bool name, bool description)
    {
        if (pad) {
            label->color(fromPacked(pad->text_color));
            if (name) label->text(pad->name);
            if (description) {
                auto desc = extract_description();
                desc = desc.empty() ? pad->name : pad->name + ": " + desc;
                if (pad->ok) {
                    if (pad->midi.empty()) {
                        desc.append("\n(no midi defined)");
                    }
                } else {
                    desc.push_back('\n');
                    desc.append(pad->error_message);
                }
                describe(desc);
            }
        } else {
            label->color(fromPacked(OPAQUE_BLACK));
            label->text("");
            describe("(undefined)");
        }
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

    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override
    {
        wire = (0 == theme->name.compare("Wire"));
        pad_style.apply_theme(theme);
        pad_sel_style.apply_theme(theme);
        return true;
    }

    void step() override
    {
        Base::step();

        NVGcolor co(fromPacked(0));
        if (selected) {
            co = SCHEME_YELLOW;
        } else if (pad) {
            co = pad->ok ? SCHEME_GREEN : SCHEME_RED;
        }
        (*light->baseColors.begin()) = co;
    }

    void draw(const DrawArgs& args) override
    {
        auto vg = args.vg;
        if (wire && (pad_style.width() > .01f)) {
            nvgBeginPath(vg);
            nvgRoundedRect(vg, 0, 0, 24, 24, 1.5);
            nvgStrokeColor(vg, fromPacked(pad ? pad->color : pad_style.stroke_color));
            nvgStrokeWidth(vg, pad_style.width());
            nvgStroke(vg);
         } else {
            auto co = fromPacked(pad ? pad->color : pad_style.fill_color);
            if (pad && !pad->empty()) {
                RoundRect(vg, 0, 0, 24, 24, co, 1.5);
            } else {
                nvgBeginPath(vg);
                nvgRoundedRect(vg, 1, 1, 23, 23, 1.5);
                nvgStrokeColor(vg, co);
                nvgStrokeWidth(vg, .5f);
                nvgStroke(vg);
            }
        }

        Base::draw(args);

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