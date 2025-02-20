#include "flip-switch.hpp"
#include "../services/colors.hpp"
namespace pachde {

FlipSwitch::FlipSwitch() :
    pressed(false)
{
    box.size.x = 12.f;
    box.size.y = 12.f;
}

bool FlipSwitch::applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme)
{
    auto style = theme->getStyle("fswhi");
    if (style)     {
        hi = fromPacked(style->strokeWithOpacity());
        hi_width = style->stroke_width;
    } else {
        hi = RampGray(G_75);
        hi_width = 1.5f;
    }
    style = theme->getStyle("fswlo");
    if (style)     {
        lo = fromPacked(style->strokeWithOpacity());
        lo_width = style->stroke_width;
    } else {
        lo = RampGray(G_55);
        lo_width = 1.25f;
    }
    return false;
}

void FlipSwitch::onDragStart(const DragStartEvent& e)
{
    Base::onDragStart(e);
    pressed = true;
}

void FlipSwitch::onDragEnd(const DragEndEvent& e)
{
    Base::onDragEnd(e);
    pressed = false;
}

void FlipSwitch::draw(const DrawArgs& args)
{
    auto vg = args.vg;
    float center = box.size.x*.5;
    NVGcolor& co_hi = pressed ? lo : hi;
    NVGcolor& co_lo = pressed ? hi : lo;
    Line(vg, center, 1.f, center, box.size.x-1.f, co_hi, hi_width);

    float dx = 1.75f * hi_width;
    float x = center - dx;
    Line(vg, x, 2.f, x, box.size.x-2.f, co_lo, lo_width);
    x -= dx;
    Line(vg, x, 2.f, x, box.size.x-2.f, co_lo, lo_width);

    x = center + dx;
    Line(vg, x, 2.f, x, box.size.x-2.f, co_lo, lo_width);
    x += dx;
    Line(vg, x, 2.f, x, box.size.x-2.f, co_lo, lo_width);
}

}