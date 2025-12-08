#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "widgets/widgets.hpp"

namespace pachde {

struct PlusMinusButton: DrawButtonCtlBase
{
    using Base = DrawButtonCtlBase;
    bool plus;

    void set_plus(bool p) {
        plus = p;
        describe(plus ? "Add": "Remove");
    }

    PlusMinusButton() {
        box.size = Vec{12.f, 12.f};
        set_plus(true);
    }

    void onAction(const ActionEvent& e) override {
        bool was_plus = plus;
        Base::onAction(e);
        set_plus(!was_plus);
    }

    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override
    {
        return Base::applyTheme(theme_engine, theme);
    };

    void draw(const DrawArgs& args) override
    {
        Base::draw(args);
        auto vg = args.vg;

        BoxRect(vg, .5f, .5f, box.size.x - 1.f, box.size.y - .5f, color_styles[ColorIndex::Frame].color, .75f);

        if (hovered && enabled && !button_down) {
            FillRect(vg, .5f, .5f, box.size.x - 1.f, box.size.y - .5f, color_styles[ColorIndex::Hovered].color);
        }

        auto co_glyph = glyph_color_for_state(enabled, button_down);
        nvgBeginPath(vg);
        if (plus) {
            Line(vg, 2.f, 6.f, 10.f, 6.f, co_glyph, 1.5f);
            Line(vg, 6.f, 2.f, 6.f, 10.f, co_glyph, 1.5f);
        } else {
            Line(vg, 2.f, 6.f, 10.f, 6.f, co_glyph, 1.5f);
        }
        nvgClosePath(vg);
        nvgFillColor(vg, glyph_color_for_state(enabled, button_down));
        nvgFill(vg);
    }
};

}