#include "slider-widget.hpp"
using namespace ::rack;
using namespace pachde;

namespace widgetry {
using namespace slider_impl;

BasicSlider::BasicSlider() :
    thumb_size(8.f, 4.5f),
    wire(false),
    stem("slide-stem", "hsl(200, 50%, 50%)", 3.5f),
    thumb("slide-thumb", "hsl(120, 50%, 50%)", .25f),
    mod("slide-mod", "hsl(41, 50%, 50%)", "hsl(41, 50%, 50%)", .25f)
{
    box.size = {10.f, 120.f};
}

void BasicSlider::onSelectKey(const rack::widget::Widget::SelectKeyEvent &e) {
    slider_impl::onSelectKey<BasicSlider, Base>(this, e);
}
void BasicSlider::onHoverScroll(const rack::widget::Widget::HoverScrollEvent & e) {
    slider_impl::onHoverScroll<BasicSlider, Base>(this, e);
}
void BasicSlider::onButton(const rack::widget::Widget::ButtonEvent &e) {
    slider_impl::onButton<BasicSlider, Base>(this, e);
}
void BasicSlider::onHover(const HoverEvent &e) {
    slider_impl::onHover<BasicSlider, Base>(this, e);
}
void BasicSlider::onEnter(const EnterEvent& e) {
    slider_impl::onEnter<BasicSlider, Base>(this, e);
}
void BasicSlider::onLeave(const LeaveEvent& e) {
    slider_impl::onLeave<BasicSlider, Base>(this, e);
}

void BasicSlider::applyTheme(std::shared_ptr<SvgTheme> theme)
{
    wire = theme->name == "Wire";
    stem.apply_theme(theme);
    thumb.apply_theme(theme);
    mod.apply_theme(theme);
}

void BasicSlider::draw_stem(const DrawArgs &args)
{
    float center = box.size.x * .5f;
    Line(args.vg, center, 0.f, center, box.size.y, stem.nvg_stroke_color(), stem.width());
}

float BasicSlider::thumb_pos()
{
    auto pq = getParamQuantity();
    float pos = pq ? pq->getValue() : 0.f;
    float range = box.size.y - thumb_size.y;
    return rescale(pos, pq ? pq->getMinValue() : 0.f, pq ? pq->getMaxValue() :10.f, 0.f, range);
}

void BasicSlider::draw_thumb(const DrawArgs &args)
{
    auto vg = args.vg;
    float center = box.size.x * .5f;

    float thumb_cx = thumb_size.x *.5f;
    float thumb_cy = thumb_size.y *.5f;
    float pos = box.size.y - thumb_cy - thumb_pos();
    if (wire) {
        BoxRect(vg, center - thumb_cx, pos - thumb_cy, thumb_size.x, thumb_size.y, thumb.nvg_stroke_color(), thumb.dx);
        if (packed_color::alpha(thumb.fill_color) < 1.0) {
            FillRect(vg, center - thumb_cx, pos - thumb_cy, thumb_size.x, thumb_size.y, thumb.nvg_color());
        }
    } else {
        FillRect(vg, center - thumb_cx, pos - thumb_cy, thumb_size.x, thumb_size.y, thumb.nvg_color());
    }
}

void BasicSlider::draw_mod(const DrawArgs &args)
{
    if (std::isnan(mod_value)) return;

    float range = box.size.y - thumb_size.y;
    auto pq = getParamQuantity();
    auto pos = rescale(mod_value, pq ? pq->getMinValue() : 0.f, pq ? pq->getMaxValue() :10.f, 0.f, range);
    if (wire) {
        OpenCircle(args.vg, box.size.x * .5f, box.size.y - pos - thumb_size.y *.5f, 2.5f, mod.nvg_stroke_color(), mod.width());
    } else {
        Circle(args.vg, box.size.x * .5f, box.size.y - pos - thumb_size.y *.5f, 2.5f, mod.nvg_color());
    }
}

void BasicSlider::draw(const DrawArgs &args)
{
    draw_stem(args);
    draw_mod(args);
    draw_thumb(args);
}

// ----  FillSlider  ---------------------------

void FillSlider::draw_fill(const DrawArgs &args)
{
    float center = anti_coord(axis, box.size) * .5f;
    auto pos = thumb_pos();
    Line(args.vg, center, box.size.y - pos, center, box.size.y, fill.nvg_stroke_color(), fill.width());
}

void FillSlider::draw(const DrawArgs &args)
{
    draw_stem(args);
    draw_fill(args);
    draw_mod(args);
    draw_thumb(args);
}

}