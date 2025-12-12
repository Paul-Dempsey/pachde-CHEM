// Copyright (C) Paul Chase Dempsey
#include "label-widget.hpp"
using namespace pachde;

namespace widgetry {

BasicTextLabel::BasicTextLabel() :
    _color(RampGray(G_90)),
    bright(false)
{
    layout_hint_color = Overlay(GetStockColor(StockColor::Yellow), .5f);
}

// IThemed
bool BasicTextLabel::applyTheme(std::shared_ptr<SvgTheme> theme)
{
    if (0 == *_style.key || !theme) return false;
    auto style = theme->getStyle(_style.key);
    PackedColor co = colors::NoColor;
    if (style) {
        co = style->fill_color();
    }
    _color = packed_color::isVisible(co) ? fromPacked(co) : RampGray(G_65);
    return true;
}

void BasicTextLabel::render(const DrawArgs& args)
{
    if (_text.empty()) return;

    auto vg = args.vg;
    auto font = _style.bold ? GetPluginFontSemiBold() : GetPluginFontRegular();
    if (!FontOk(font)) return;

    nvgSave(vg);
    SetTextStyle(vg, font, _color, _style.height);
    float x{0}, y{0};
    switch (_style.align) {
        case TextAlignment::Left: x = 0; break;
        case TextAlignment::Center: x = box.size.x * .5; break;
        case TextAlignment::Right: x = box.size.x; break;
    }
    switch (_style.valign) {
        case VAlignment::Top: y = 0; break;
        case VAlignment::Middle: y = box.size.y * .5; break;
        case VAlignment::Bottom: y = box.size.y; break;
    }
    nvgTextAlign(vg, _style.nvg_alignment());
    nvgText(vg, x, y, _text.c_str(), nullptr);
    nvgRestore(vg);
}

void BasicTextLabel::drawLayer(const DrawArgs& args, int layer)
{
    Base::drawLayer(args, layer);

    if (1 != layer) return;
    if (!bright) return;
    render(args);
}

void BasicTextLabel::draw(const DrawArgs& args)
{
    Base::draw(args);

    if (layout_hints) {
        draw_widget_bounds(this, args);
    }

    if (bright) return;
    render(args);
}

}