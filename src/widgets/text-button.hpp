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
        return true;
    }

    void draw(const DrawArgs& args) override
    {
        Base::draw(args);

        auto font = label_style.bold ? GetPluginFontSemiBold() : GetPluginFontRegular();
        if (!FontOk(font)) return;
        auto vg = args.vg;

        nvgScissor(vg, 1, 1, box.size.x-2, box.size.y -2);
        Vec pos{0.f, 0.f};
        NVGalign align{NVGalign(NVG_ALIGN_TOP|NVG_ALIGN_LEFT)};
        
        switch (label_style.align) {
        case TextAlignment::Center:
            align = NVGalign(NVG_ALIGN_TOP|NVG_ALIGN_CENTER);
            pos.x = box.size.x * .5;
            break;
        case TextAlignment::Right:
            align = NVGalign(NVG_ALIGN_TOP|NVG_ALIGN_RIGHT);
            pos.x = box.size.x;
            break;
        default:
            break;
        }
        SetTextStyle(vg, font, fromPacked(color), label_style.height);
        nvgTextAlign(vg, align);
        nvgText(vg, pos.x, pos.y, text.c_str(), nullptr);

        nvgResetScissor(vg);
    }
};

}