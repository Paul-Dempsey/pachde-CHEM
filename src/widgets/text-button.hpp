#pragma once
#include "theme-button.hpp"
#include "label.hpp"
#include "draw-button.hpp"

namespace widgetry {

struct TextButton : FrameButton
{
    using Base = FrameButton;

    std::string text;
    PackedColor color = 0xffe8e8e8;
    PackedColor bg = 0xff181818;
    LabelStyle label_style{"ctl-glyph", HAlign::Center, VAlign::Middle, 12.f, true};

    void set_text(std::string t) { text = t; }
    void set_style(LabelStyle style) { label_style = style; }

    void applyTheme(std::shared_ptr<SvgTheme> theme) override {
        Base::applyTheme(theme);

        if (!theme->getFillColor(color, label_style.key, true)) {
            color = GetPackedStockColor(StockColor::Gray_65p);
        }
        theme->getFillColor(bg, "tbtn-face", true);
    }

    void draw(const DrawArgs& args) override
    {
        auto vg = args.vg;
        auto font = label_style.bold ? GetPluginFontSemiBold() : GetPluginFontRegular();
        if (!FontOk(font)) return;

        if (packed_color::isVisible(bg)) {
            FillRect(vg, 0, 0, box.size.x, box.size.y, fromPacked(bg));
        }

        Base::draw(args);

        Rect draw_box{Vec(1,1), Vec(box.size.x-2, box.size.y -2)};
        nvgScissor(vg, RECT_ARGS(draw_box));

        draw_oriented_text_box(vg,
            draw_box, 0.f, 0.f, 0.f, 0.f,
            text, font, label_style.text_height, color,
            label_style.halign, label_style.valign, label_style.orientation
        );

        nvgResetScissor(vg);
    }
};

}