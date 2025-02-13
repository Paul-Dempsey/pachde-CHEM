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
    stem_width(1.f)
{
    box.size.x = 3*radius;
    box.size.y = radius + radius + radius + (radius * ((maximum - minimum)*2));
}

void SelectorWidget::initParamQuantity()
{
    Base::initParamQuantity();
    auto pq = getParamQuantity();
	if (pq) {
        pq->snapEnabled = true;
        minimum = static_cast<int>(pq->getMinValue());
        maximum = static_cast<int>(pq->getMaxValue());
        box.size.y = radius + radius + radius + (radius * ((maximum - minimum)*2));
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
                OpenCircle(vg, x, y, radius, active_color, stem_width);
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

void SelectorWidget::draw(const DrawArgs& args)
{
    auto vg = args.vg;
    auto pq = getParamQuantity();
    auto value = pq ? static_cast<int>(pq->getValue()) : 0;
    assert(in_range(value, minimum, maximum));

    if (bright && rack::settings::rackBrightness < .95f) return;

    auto head_dy = radius*.8f;
    FillRect(vg, box.size.x*.5f - radius*.5f, 0.f, radius, head_dy, head_color);

    float y = head_dy;
    float x = box.size.x*.5f;
    for (int i = minimum; i <= maximum; ++i) {
        Line(vg, x, y, x, y + radius + radius, stem_color, stem_width);
        y += radius + radius + radius;
        if (wire) {
            OpenCircle(vg, x, y, radius, (value == i) ? active_color : inactive_color, stem_width);
        } else {
            Circle(vg, x, y, radius, (value == i) ? active_color : inactive_color);
        }
        y += radius;
    }
    Line(vg, x, y, x, y + radius + radius, stem_color, stem_width);
}

}
