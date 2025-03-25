#include "slider-h-widget.hpp"
using namespace ::rack;
using namespace ::svg_theme;

namespace pachde {
using namespace slider_impl;

BasicHSlider::BasicHSlider() :
    thumb_size(4.5, 8.f),
    stem("slide-stem", "hsl(200, 50%, 50.00%)", 2.5f),
    thumb("slide-thumb", "hsl(120, 50%, 50%)", .25f),
    foot("slide-foot", "hsl(200, 50%, 50.00%)", "hsl(200, 50%, 50.00%)", .5f),
    mod("slide-mod", "hsl(41, 50%, 50%)", "hsl(41, 50%, 50%)", .25f),
    wire(false)
{
    box.size = {127.f, 12.f};
}

void BasicHSlider::onEnter(const EnterEvent& e) {
    slider_impl::onEnter<BasicHSlider, Base>(this, e);
}
void BasicHSlider::onLeave(const LeaveEvent& e) {
    slider_impl::onLeave<BasicHSlider, Base>(this, e);
}
void BasicHSlider::onButton(const rack::widget::Widget::ButtonEvent &e) {
    slider_impl::onButton<BasicHSlider, Base>(this, e);
}
void BasicHSlider::onHoverScroll(const rack::widget::Widget::HoverScrollEvent & e) {
    slider_impl::onHoverScroll<BasicHSlider, Base>(this, e);
}
void BasicHSlider::onSelectKey(const rack::widget::Widget::SelectKeyEvent &e) {
    slider_impl::onSelectKey<BasicHSlider, Base>(this, e);
}
void BasicHSlider::onHover(const HoverEvent &e) {
    slider_impl::onHover<BasicHSlider, Base>(this, e);
}

void BasicHSlider::onDragStart(const DragStartEvent& e)
{
	if (e.button != GLFW_MOUSE_BUTTON_LEFT) return;
    drag_distance = 0.f;
    Base::onDragStart(e);
}

void BasicHSlider::onDragEnd(const DragEndEvent& e)
{
	if (e.button != GLFW_MOUSE_BUTTON_LEFT) return;

    // Dispatch Action event if mouse traveled less than a threshold distance
    const float actionDistThreshold = 8.f;
    if (drag_distance < actionDistThreshold) {
        ActionEvent eAction;
        onAction(eAction);
    }

    ParamWidget::onDragEnd(e);
}

void BasicHSlider::onDragMove(const DragMoveEvent& e)
{
	if (e.button != GLFW_MOUSE_BUTTON_LEFT) return;
	engine::ParamQuantity* pq = getParamQuantity();
	if (pq) {
        float dx = coord(axis, e.mouseDelta) / APP->scene->rackScroll->getZoom();
        float sign = (dx < 0) ? -1.f : 1.f;
        float range = coord(axis, box.size) - coord(axis, thumb_size);
        dx = sign * rescale(std::abs(dx), 0.f, range, pq->getMinValue(), pq->getMaxValue());

		float value = dx + pq->getValue();
		pq->setValue(value);
    }
    Base::onDragMove(e);
}

void BasicHSlider::onDragLeave(const DragLeaveEvent& e)
{
    Base::onDragLeave(e);
}

float BasicHSlider::thumb_pos()
{
    auto pq = getParamQuantity();
    float pos = pq ? pq->getValue() : 0.f;
    float range = coord(axis, box.size) - coord(axis, thumb_size);
    return rescale(pos, pq ? pq->getMinValue() : 0.f, pq ? pq->getMaxValue() : 10.f, 0.f, range);
}

float BasicHSlider::mod_pos()
{
    if (std::isnan(mod_value)) return mod_value;
    auto pq = getParamQuantity();
    if (!pq) return NAN;
    float range = coord(axis, box.size) - coord(axis, thumb_size);
    return rescale(mod_value, pq->getMinValue(), pq->getMaxValue(), 0.f, range);
}

bool BasicHSlider::applyTheme(SvgThemeEngine &theme_engine, std::shared_ptr<SvgTheme> theme)
{
    wire = theme->name == "Wire";
    stem.apply_theme(theme);
    thumb.apply_theme(theme);
    foot.apply_theme(theme);
    mod.apply_theme(theme);
    return false;
}

void BasicHSlider::draw_feet(const DrawArgs &args)
{
    Line(args.vg, 0.f, 0.f, 0.f, box.size.y, foot.nvg_stroke_color(), foot.width());
    Line(args.vg, box.size.x, 0.f, box.size.x, box.size.y, foot.nvg_stroke_color(), foot.width());
}

void BasicHSlider::draw_stem(const DrawArgs &args)
{
    float CENTER = box.size.y * .5f;
    Line(args.vg, 0.f, CENTER, box.size.x, CENTER, stem.nvg_stroke_color(), stem.width());
}

void BasicHSlider::draw_thumb(const DrawArgs &args)
{
    auto vg = args.vg;
    float CENTER = box.size.y * .5f;

    float thumb_cx = thumb_size.x *.5f;
    float thumb_cy = thumb_size.y *.5f;
    float pos = thumb_pos() + thumb_cx;
    if (wire) {
        BoxRect(vg, pos - thumb_cx, CENTER - thumb_cy, thumb_size.x, thumb_size.y, thumb.nvg_stroke_color(), thumb.dx);
        if (Alpha(thumb.fill_color) < 1.0) {
            FillRect(vg, pos - thumb_cx, CENTER - thumb_cy, thumb_size.x, thumb_size.y, thumb.nvg_color());
        }
    } else {
        FillRect(vg, pos - thumb_cx, CENTER - thumb_cy, thumb_size.x, thumb_size.y, thumb.nvg_color());
    }
}

void BasicHSlider::draw_mod(const DrawArgs &args)
{
    if (std::isnan(mod_value)) return;

    float middle = box.size.y *.5f;
    float thumb_cx = thumb_size.x *.5f;
    float pos = mod_pos() + thumb_cx;
    float r = std::max(1.5f, thumb_cx);
    if (wire) {
        OpenCircle(args.vg, pos, middle, r, mod.nvg_stroke_color(), mod.width());
    } else {
        Circle(args.vg, pos, middle, r, mod.nvg_color());
    }
}

void BasicHSlider::draw(const DrawArgs &args)
{
    draw_stem(args);
    draw_feet(args);
    draw_mod(args);
    draw_thumb(args);
}

// ----------------------------------------------

void FillHSlider::draw_fill(const DrawArgs &args)
{
    float CENTER = box.size.y *.5f;
    auto pos = thumb_pos();
    Line(args.vg, 0, CENTER, pos, CENTER, fill.nvg_stroke_color(), fill.width());
}

void FillHSlider::draw(const DrawArgs &args)
{
    draw_stem(args);
    draw_fill(args);
    draw_feet(args);
    draw_mod(args);
    draw_thumb(args);
}
}