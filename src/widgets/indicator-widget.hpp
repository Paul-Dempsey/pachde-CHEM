#pragma once
#include "services/text.hpp"
#include "services/svgtheme.hpp"
#include "element-style.hpp"
using namespace svg_theme;

namespace pachde {

struct DotWidget : TransparentWidget {
    NVGcolor color;
    bool filled;
    float stroke;

    DotWidget() : color(RampGray(G_35)), filled(false), stroke(.5f) {
        box.size.x = box.size.y = 6.f;
    }

    void draw(const DrawArgs& args) override {
        Dot(args.vg, box.size.x * .5f, box.size.y * .5f, color, filled, 3.f, stroke);
    }
};

struct IndicatorWidget : TipWidget
{
    DotWidget* _dot = nullptr;

    IndicatorWidget()
    {
        _dot = new DotWidget();
        box.size = _dot->box.size;
        addChild(_dot);
    }

    void setLook(const NVGcolor color, bool fill = true) {
        _dot->color = color;
        _dot->filled = fill;
    }
    void setColor(const NVGcolor color) {
        _dot->color = color;
    }
    void setFill(bool  fill) {
        _dot->filled = fill;
    }

};

inline IndicatorWidget * createIndicatorCentered(float x, float y, const NVGcolor co, const char * tip = nullptr, bool filled = true)
{
    IndicatorWidget* w = createWidgetCentered<IndicatorWidget>(Vec(x,y));
    w->setLook(co, filled);
    if (tip) {
        w->describe(tip);
    }

    return w;
}

struct StateIndicatorWidget : IndicatorWidget, IApplyTheme
{
    ElementStyle option_on{"state-on", "hsl(216,50%,50%)", "hsl(216,50%,50%)", 1};
    ElementStyle option_off{"state-off", "#00000000", "#808080", .35f};

    bool yes{true};
    bool get_state() { return yes; }
    void set_state(bool on) {
        yes = on;
        _dot->filled = on ? !wire : false;
        _dot->color  = on ? option_on.nvg_color() : option_off.nvg_stroke_color();
        _dot->stroke = on ? option_on.width() : option_off.width();
    }

    bool wire{false};

    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override
    {
        wire = (0 == theme->name.compare("Wire"));
        option_on.apply_theme(theme);
        option_off.apply_theme(theme);
        set_state(get_state());
        return true;
    }
};

inline StateIndicatorWidget* createIndicatorCentered(float x, float y, bool on, const char * tip = nullptr)
{
    auto w = createWidgetCentered<StateIndicatorWidget>(Vec(x,y));
    w->set_state(on);
    if (tip) {
        w->describe(tip);
    }
    return w;
}

}