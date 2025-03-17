#include "slider-widget.hpp"
using namespace ::rack;

#ifndef is_nan_f
#if defined ARCH_MAC
#define is_nan_f isnan
#else
#define is_nan_f isnanf
#endif
#endif

namespace pachde {

void SliderBase::onSelectKey(const rack::widget::Widget::SelectKeyEvent &e)
{
    if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT)
    {
        auto pq = getParamQuantity();
        if (pq)
        {
            bool plain = (e.action != GLFW_REPEAT) && ((e.mods & RACK_MOD_MASK) == 0);
            bool shift = e.mods & GLFW_MOD_SHIFT;
            // bool ctrl = e.mods & GLFW_MOD_CONTROL;
            switch (e.key)
            {
            case GLFW_KEY_0: case GLFW_KEY_1: case GLFW_KEY_2: case GLFW_KEY_3: case GLFW_KEY_4:
            case GLFW_KEY_5: case GLFW_KEY_6: case GLFW_KEY_7: case GLFW_KEY_8: case GLFW_KEY_9:
                pq->setValue(rescale((e.key - GLFW_KEY_0), 0.f, 9.f, pq->getMinValue(), pq->getMaxValue()));
                e.consume(this);
                break;

            case GLFW_KEY_HOME:
                if (plain) {
                    pq->setValue(pq->getMaxValue());
                    e.consume(this);
                }
                break;
            case GLFW_KEY_END:
                if (plain) {
                    pq->setValue(pq->getMinValue());
                    e.consume(this);
                }
                break;
            case GLFW_KEY_UP:
                pq->setValue(pq->getValue() + increment * (shift ? 10.f : 1.f));
                e.consume(this);
                break;
            case GLFW_KEY_DOWN:
                pq->setValue(pq->getValue() - increment * (shift ? 10.f : 1.f));
                e.consume(this);
                break;
            case GLFW_KEY_PAGE_UP:
                pq->setValue(pq->getValue() + increment * 10.f);
                e.consume(this);
                break;
            case GLFW_KEY_PAGE_DOWN:
                pq->setValue(pq->getValue() - increment * 10.f);
                e.consume(this);
                break;
            }
        }
    }
    if (e.isConsumed())
        return;
    Base::onSelectKey(e);
}

void SliderBase::onHoverScroll(const rack::widget::Widget::HoverScrollEvent & e)
{
    auto pq = getParamQuantity();
    if (pq) {
        auto dx = e.scrollDelta;
        auto mods = APP->window->getMods();
        if (dx.y < 0.f) {
            pq->setValue(pq->getValue() - increment * ((mods & GLFW_MOD_SHIFT) ? 10.f : 1.f));
        } else if (dx.y > 0.f) {
            pq->setValue(pq->getValue() + increment * ((mods & GLFW_MOD_SHIFT) ? 10.f : 1.f));
        }
        e.consume(this);
    } else {
        Base::onHoverScroll(e);
    }
}

void SliderBase::onButton(const rack::widget::Widget::ButtonEvent &e)
{
    if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
        e.consume(this);
        auto pq = getParamQuantity();
        if (pq) {
            float v = box.size.y - shrinky - e.pos.y;
            v = rescale(v, 0.f, box.size.y, pq->getMinValue(), pq->getMaxValue());
            pq->setValue(v);
        }
    } else {
        Base::onButton(e);
    }
}

void SliderBase::onHover(const HoverEvent &e)
{
    Base::onHover(e);
    e.consume(this);
}

void SliderBase::onEnter(const EnterEvent& e)
{
    APP->event->setSelectedWidget(this);
    e.consume(this);
    Base::onEnter(e);
}

void SliderBase::onLeave(const LeaveEvent& e)
{
    APP->event->setSelectedWidget(nullptr);
    Base::onLeave(e);
}

// ----  BasicSlider  ---------------------------

BasicSlider::BasicSlider() :
    thumb_size(8.f, 4.5f),
    wire(false),
    stem("slide-stem", "hsl(200, 50%, 50.00%)", 2.5f),
    thumb("slide-thumb", "hsl(120, 50%, 50%)", .25f),
    mod("slide-mod", "hsl(41, 50%, 50%)", "hsl(41, 50%, 50%)", .25f)
{
    box.size = {10.f, 120.f};
}

bool BasicSlider::applyTheme(SvgThemeEngine &theme_engine, std::shared_ptr<SvgTheme> theme)
{
    wire = theme->name == "Wire";
    stem.apply_theme(theme);
    thumb.apply_theme(theme);
    mod.apply_theme(theme);
    return true;
}

void BasicSlider::draw_stem(const DrawArgs &args)
{
    float CENTER = box.size.x * .5f;
    Line(args.vg, CENTER, 0.f, CENTER, box.size.y, stem.nvg_stroke_color(), stem.width());
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
    float CENTER = box.size.x * .5f;

    float thumb_cx = thumb_size.x *.5f;
    float thumb_cy = thumb_size.y *.5f;
    float pos = box.size.y - thumb_cy - thumb_pos();
    if (wire) {
        BoxRect(vg, CENTER - thumb_cx, pos - thumb_cy, thumb_size.x, thumb_size.y, thumb.nvg_stroke_color(), thumb.dx);
        if (Alpha(thumb.fill_color) < 1.0) {
            FillRect(vg, CENTER - thumb_cx, pos - thumb_cy, thumb_size.x, thumb_size.y, thumb.nvg_color());
        }
    } else {
        FillRect(vg, CENTER - thumb_cx, pos - thumb_cy, thumb_size.x, thumb_size.y, thumb.nvg_color());
    }
}

void BasicSlider::draw_mod(const DrawArgs &args)
{
    if (is_nan_f(mod_value)) return;

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
    float CENTER = box.size.x * .5f;
    auto pos = thumb_pos();
    Line(args.vg, CENTER, box.size.y - pos, CENTER, box.size.y, fill.nvg_stroke_color(), fill.width());
}

void FillSlider::draw(const DrawArgs &args)
{
    draw_stem(args);
    draw_fill(args);
    draw_mod(args);
    draw_thumb(args);
}

}