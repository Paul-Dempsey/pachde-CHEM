// Copyright (C) Paul Chase Dempsey
#include <rack.hpp>
using namespace ::rack;
#include "selector-widget.hpp"
#include "../services/misc.hpp"

namespace pachde {

SelectorWidget::SelectorWidget() :
    bright(false),
    wire(false),
    minimum(0),
    maximum(2),
    radius(4.f),
    stem_width(1.f),
    hovered_item(-1),
    tip_holder(nullptr)
{
    box.size.x = 3*radius;
    box.size.y = radius*3.f + (radius * ((maximum + 1 - minimum)*4));
}

SelectorWidget::~SelectorWidget()
{
    if (tip_holder) delete tip_holder;
    tip_holder = nullptr;
}

void SelectorWidget::initParamQuantity()
{
    Base::initParamQuantity();
    auto pq = getParamQuantity();
	if (pq) {
        pq->snapEnabled = true;
        minimum = static_cast<int>(pq->getMinValue());
        maximum = static_cast<int>(pq->getMaxValue());
        box.size.y = radius*3.f + (radius * ((maximum + 1 - minimum)*4));
    } else {
        minimum = 0;
        maximum = 1;
    }
}

bool SelectorWidget::applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme)
{
    wire = ("Wire" == theme->name);
    auto style = theme->getStyle("selector");
    if (style) {
        head_color = fromPacked(style->fillWithOpacity());
        stem_color = fromPacked(style->strokeWithOpacity());
        stem_width = style->stroke_width;
        auto pco = theme->getFillColor("selector-sel", true);
        if (isVisibleColor(pco)) {
            active_color = fromPacked(pco);
        } else {
            active_color = stem_color;
        }
    } else {
        head_color = RampGray(G_75);
        stem_color = nvgRGB(0x41, 0xbf, 0x73);
        active_color = stem_color;
        inactive_color = RampGray(G_50);
    }
    return true;
}
void SelectorWidget::ensure_tip_holder()
{
    if (!tip_holder) {
        tip_holder = new TipHolder();
    }
}
void SelectorWidget::set_tip_text(std::string text)
{
    ensure_tip_holder();
    tip_holder->setText(text);
}

void SelectorWidget::destroy_tip()
{
    if (tip_holder) { tip_holder->destroyTip(); }
}
void SelectorWidget::create_tip()
{
    ensure_tip_holder();
    if (tip_holder) { tip_holder->createTip(); }
}

void SelectorWidget::onHover(const HoverEvent& e) {
    Base::onHover(e);
    
    int item = indexOfPos(e.pos);
    if (item != hovered_item) {
        hovered_item = item;
        if (hovered_item >= 0) {
            destroy_tip();
            auto sq = dynamic_cast<SwitchQuantity*>(getParamQuantity());
            if (sq) {
                set_tip_text(sq->labels[hovered_item - minimum]);
                create_tip();
                Base::destroyTooltip();
            }
        } else {
            set_tip_text("");
            destroy_tip();
        }
    }
    e.consume(this);
}

void SelectorWidget::onEnter(const EnterEvent& e) {
    if (hovered_item == -1) {
        Base::onEnter(e);
    } else {
        Base::destroyTooltip();
        create_tip();
        e.consume(this);
    }
}

void SelectorWidget::onLeave(const LeaveEvent& e) {
    destroy_tip();
    hovered_item = -1;
    Base::onLeave(e);
}

void SelectorWidget::onDragLeave(const DragLeaveEvent& e) {
    destroy_tip();
    hovered_item = -1;
    Base::onDragLeave(e);
}

void SelectorWidget::onDragEnd(const DragEndEvent& e) {
    destroy_tip();
    hovered_item = -1;
    Base::onDragEnd(e);
}

void SelectorWidget::onButton(const ButtonEvent& e)
{
    Base::onButton(e);
    auto mods = (e.mods & RACK_MOD_MASK);
    if ((0 == mods) && (e.action == GLFW_RELEASE)) {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
            destroy_tip();
            hovered_item = -1;
            int item = indexOfPos(e.pos);
            if (item >= 0) {
                auto pq = getParamQuantity();
                if (pq) {
                    pq->setValue(minimum + item);
                }
            }
            e.consume(this);
        } else if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
            destroy_tip();
            hovered_item = -1;
            createContextMenu();
            e.consume(this);
        } 
    }
}

void SelectorWidget::drawLayer(const DrawArgs& args, int layer)
{
    Base::drawLayer(args, layer);
    if (1 != layer) return;
    if (!bright || rack::settings::rackBrightness >= .95f) return;

    auto vg = args.vg;
    auto pq = getParamQuantity();

    auto value = pq ? static_cast<int>(pq->getValue()) : 0;

    auto head_dy = radius*.8f;
    FillRect(vg, box.size.x*.5f - radius*.5f, 0.f, radius, head_dy, head_color);

    NVGcolor stem = nvgTransRGBAf(stem_color, .45);

    float y = head_dy;
    float x = box.size.x*.5f;
    for (int i = minimum; i <= maximum; ++i) {
        Line(vg, x, y, x, y + radius + radius, stem, stem_width);
        y += radius + radius + radius;
        if (value == i) {
            if (wire) {
                OpenCircle(vg, x, y, radius, active_color, stem_width * 1.5f);
            } else {
                Circle(vg, x, y, radius, active_color);
            }
        } else {
            OpenCircle(vg, x, y, radius-stem_width, inactive_color, stem_width);
        }
        y += radius;
    }
    Line(vg, x, y, x, y + radius + radius, stem, stem_width);
}

int SelectorWidget::indexOfPos(Vec(pos))
{
    if (pos.x < 0 ||pos.x > box.size.x) return -1;
    float top = radius * 1.8f;
    if (pos.y < top) return -1;
    float hitbox = 4.f * radius;
    int count = 1 + maximum - minimum;
    if (pos.y > top + hitbox * count) return -1;
    auto result = minimum + static_cast<int>(std::floor((pos.y - top) / hitbox));
    assert(in_range(result, minimum, maximum));
    return result;
}

void SelectorWidget::draw(const DrawArgs& args)
{
    auto vg = args.vg;
    auto pq = getParamQuantity();
    auto value = pq ? static_cast<int>(std::floor(pq->getValue())) : minimum;
    assert(in_range(value, minimum, maximum));

    if (bright && rack::settings::rackBrightness < .95f) return;

    auto head_dy = radius*.8f;
    FillRect(vg, box.size.x*.5f - radius*.5f, 0.f, radius, head_dy, head_color);

    float y = head_dy;
    float x = box.size.x*.5f;
    for (int i = minimum; i <= maximum; ++i) {
        Line(vg, x, y, x, y + radius + radius, stem_color, stem_width);
        y += radius + radius + radius;
        if (i == hovered_item) {
            OpenCircle(vg, x, y, radius + stem_width*.8f, active_color, stem_width);
        }
        bool current = value == i;
        if (wire) {
            OpenCircle(vg, x, y, radius, current ? active_color : inactive_color, current ? stem_width * 1.5f : stem_width);
        } else {
            Circle(vg, x, y, radius, current ? active_color : inactive_color);
        }
        y += radius;
    }
    Line(vg, x, y, x, y + radius + radius, stem_color, stem_width);

    // debug - 
    // {
    //     auto co = nvgTransRGBAf(RampGray(G_65), .25);
    //     BoxRect(vg, .25,.25, box.size.x-.5, box.size.y-.5, co, .5);

    //     float top = radius * 1.8f;
    //     float hitbox = 4.f * radius;
    //     int count = 1 + maximum - minimum;
    //     float y = top;
    //     for (int i = 0; i <= count; ++i) {
    //         Line(vg, .25, y, box.size.x - .5, y, co, .5);
    //         y += hitbox;
    //     }
    // }
}

}
