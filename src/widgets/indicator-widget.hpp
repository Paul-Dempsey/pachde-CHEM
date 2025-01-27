#pragma once
#include "../services/text.hpp"
#include "../services/svgtheme.hpp"
using namespace svg_theme;

namespace pachde {

struct DotWidget : TransparentWidget {
    NVGcolor color;
    bool filled;

    DotWidget() : color(RampGray(G_35)), filled(false) {
        box.size.x = box.size.y = 6.f;
    }

    void draw(const DrawArgs& args) override {
        Dot(args.vg, box.size.x * .5f, box.size.y * .5f, color, filled, 3.f);
    }
};

struct IndicatorWidget : TipWidget
{
    DotWidget* _dot = nullptr;
    FramebufferWidget* _fb = nullptr;

    IndicatorWidget()
    {
        _dot = new DotWidget();
        box.size = _dot->box.size;
        _fb = new widget::FramebufferWidget;
        _fb->addChild(_dot);
        box.size = _dot->box.size;
        _fb->box.size = _dot->box.size;
	    addChild(_fb);
        dirty();
    }
    void dirty() { _fb->setDirty(); }
    void setColor(const NVGcolor color) {
        _dot->color = color;
        dirty();
    }
    void setFill(bool  fill) {
        _dot->filled = fill;
        dirty();
    }
};

inline IndicatorWidget * createIndicatorCentered(float x, float y, const NVGcolor co, const char * tip = nullptr, bool filled = true)
{
    IndicatorWidget* w = createWidgetCentered<IndicatorWidget>(Vec(x,y));
    w->setColor(co);
    w->setFill(filled);
    if (tip) {
        w->describe(tip);
    }

    return w;
}

}