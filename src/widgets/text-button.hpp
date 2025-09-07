#pragma once
#include "theme-button.hpp"
#include "label-widget.hpp"
#include "draw-button.hpp"

namespace pachde {

struct TextButton : FrameButton
{
    using Base = FrameButton;

    std::string text;
    PackedColor color = 0xffe8e8e8;
    PackedColor bg = 0xff181818;
    LabelStyle label_style{"ctl-glyph", TextAlignment::Center, VAlignment::Middle, 12.f, true};

    void set_text(std::string t) { text = t; }
    void set_style(LabelStyle style) { label_style = style; }

    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override
    {
        Base::applyTheme(theme_engine, theme);
        color = theme_engine.getFillColor(theme->name, label_style.key, true);
        if (!isVisibleColor(color)) {
            color = GetPackedStockColor(StockColor::Gray_65p);
        }
        bg = theme_engine.getFillColor(theme->name, "tbtn-face", true);
        return true;
    }

    void draw(const DrawArgs& args) override
    {
        auto vg = args.vg;
        auto font = label_style.bold ? GetPluginFontSemiBold() : GetPluginFontRegular();
        if (!FontOk(font)) return;

        if (isVisibleColor(bg)) {
            FillRect(vg, 0, 0, box.size.x, box.size.y, fromPacked(bg));
        }

        Base::draw(args);

        nvgScissor(vg, 1, 1, box.size.x-2, box.size.y -2);

        float x{0}, y{0};
        switch (label_style.align) {
        case TextAlignment::Left: x = 0; break;
        case TextAlignment::Center: x = box.size.x * .5; break;
        case TextAlignment::Right: x = box.size.x; break;
        }
        switch (label_style.valign) {
        case VAlignment::Top: y = 0; break;
        case VAlignment::Middle: y = box.size.y * .5; break;
        case VAlignment::Bottom: y = box.size.y; break;
        }

        SetTextStyle(vg, font, fromPacked(color), label_style.height);
        nvgTextAlign(vg, label_style.nvg_alignment());
        nvgText(vg, x, y, text.c_str(), nullptr);

        nvgResetScissor(vg);
    }
};

}