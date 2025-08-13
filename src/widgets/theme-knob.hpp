// Copyright (C) Paul Chase Dempsey
#pragma once
#include "../plugin.hpp"
#include "../services/svgtheme.hpp"
#include "../services/colors.hpp"
using namespace svg_theme;

namespace pachde {

struct GlowKnob : rack::RoundKnob {
    using Base = rack::RoundKnob;
    bool enabled{true};
    bool bright{false};
    bool wire{false};
    NVGcolor disabled_screen{nvgRGBAf(.4, .4, .4, .6f)};

    GlowKnob() {
        box.size.x = 32.f;
        box.size.y = 32.f;
        this->shadow->hide();
    }

    // enabling
    void enable(bool on) { enabled = on; }

    void onButton(const ButtonEvent& e) override{
        if (!enabled) {
            e.stopPropagating();
            return;
        }
        Base::onButton(e);
    }

    void onHoverScroll(const HoverScrollEvent& e) override {
        if (enabled) Base::onHoverScroll(e);
        e.consume(this);
    }

    void onChange(const ChangeEvent& e) override {
        Base::onChange(e);
        e.consume(this);
    }

    // "glowing"
    void glowing(bool glow) { bright = glow; }

    void drawLayer(const DrawArgs& args, int layer) override
    {
        if (layer != 1) return;
        if (!enabled) return;
        if (rack::settings::rackBrightness > .95f) return;
        if (!module) return;
        if (bright) fb->draw(args);
    }

    void draw(const DrawArgs& args) override
    {
        if (module) {
            if (enabled && bright && rack::settings::rackBrightness < .95f) return;
        }
        if (enabled) {
            Base::draw(args);
        } else {
            if (wire) {
                float cx = box.size.x*.5f;
                float cy = box.size.y*.5f;
                float r = box.size.x*.5f - 1.25f;
                auto co = RampGray(G_40);
                OpenCircle(args.vg, cx, cy, r, co, .75f);
                Line(args.vg, cx, cy - 1.f, cx, cy - r, co, .75f);
            } else {
                Base::draw(args);
                Circle(args.vg, box.size.x*.5f, box.size.y*.5f, box.size.x*.5f - 1.f, disabled_screen);
            }
        }
    }
};

template<typename TSvg>
struct TKnob : GlowKnob, IApplyTheme
{
    using Base = GlowKnob;

    // IApplyTheme
    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override
    {
        wire = (theme->name == "Wire");
        bg->setSvg(engine.loadSvg(asset::plugin(pluginInstance, TSvg::bg()), theme));
        setSvg(engine.loadSvg(asset::plugin(pluginInstance, TSvg::knob()), theme));
        auto screen = theme->getFillColor("k-disabled", true);
        if (isVisibleColor(screen)) {
            disabled_screen = fromPacked(screen);
        } else {
            disabled_screen = nvgRGBAf(.4, .4, .4, .6f);
        }
        return true;
    }

    void drawLayer(const DrawArgs& args, int layer) override
    {
        if (layer != 1) return;
        if (rack::settings::rackBrightness > .95f) return;
        if (!module) return;

        bool base_bright = bright;
        if (wire) {
            bright = true;
        }

        Base::drawLayer(args, layer);

        if (wire) {
            bright = base_bright;
        }
    }
};

struct GrayKnobSvg {
    static std::string bg() { return "res/widgets/knob-gray-bg.svg"; }
    static std::string knob() { return "res/widgets/knob-gray.svg"; }
};
struct RedKnobSvg {
    static std::string bg() { return "res/widgets/knob-red-bg.svg"; }
    static std::string knob() { return "res/widgets/knob-red.svg"; }
};
struct GreenKnobSvg {
    static std::string bg() { return "res/widgets/knob-green-bg.svg"; }
    static std::string knob() { return "res/widgets/knob-green.svg"; }
};
struct BlueKnobSvg {
    static std::string bg() { return "res/widgets/knob-blue-bg.svg"; }
    static std::string knob() { return "res/widgets/knob-blue.svg"; }
};
struct YellowKnobSvg {
    static std::string bg() { return "res/widgets/knob-yellow-bg.svg"; }
    static std::string knob() { return "res/widgets/knob-yellow.svg"; }
};
struct VioletKnobSvg {
    static std::string bg() { return "res/widgets/knob-violet-bg.svg"; }
    static std::string knob() { return "res/widgets/knob-violet.svg"; }
};
struct TrimPotSvg {
    static std::string bg() { return "res/widgets/trimpot-bg.svg"; }
    static std::string knob() { return "res/widgets/trimpot.svg"; }
};
struct GreenTrimPotSvg {
    static std::string bg() { return "res/widgets/trimpot-green-bg.svg"; }
    static std::string knob() { return "res/widgets/trimpot-green.svg"; }
};
struct GrayTrimPotSvg {
    static std::string bg() { return "res/widgets/trimpot-gray-bg.svg"; }
    static std::string knob() { return "res/widgets/trimpot-gray.svg"; }
};
struct UselessSvg {
    static std::string bg() { return "res/widgets/useless-knob-bg.svg"; }
    static std::string knob() { return "res/widgets/useless-knob.svg"; }
};
using BasicKnob = TKnob<GrayKnobSvg>;
using UselessKnob = TKnob<UselessSvg>;
using GrayKnob = TKnob<GrayKnobSvg>;
using BlueKnob = TKnob<BlueKnobSvg>;
using RedKnob = TKnob<RedKnobSvg>;
using GreenKnob = TKnob<GreenKnobSvg>;
using YellowKnob = TKnob<YellowKnobSvg>;
using VioletKnob = TKnob<VioletKnobSvg>;

using TrimPot = TKnob<TrimPotSvg>;
using GreenTrimPot = TKnob<GreenTrimPotSvg>;
using GrayTrimPot = TKnob<GrayTrimPotSvg>;

template <typename TKnob>
TKnob* createChemKnob(Vec pos, Module * module, int paramId, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
	auto o = createParam<TKnob>(pos, module, paramId);
    o->applyTheme(engine, theme);
	o->box.pos = o->box.pos.minus(o->box.size.div(2));
    return o;
}


}