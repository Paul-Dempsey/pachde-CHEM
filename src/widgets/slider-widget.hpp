#pragma once
#include "slider-common.hpp"

namespace widgetry {

struct BasicSlider : Knob, IThemed
{
    using Base = Knob;
    static const Axis axis{Axis::Y};

    Vec thumb_size;
    bool wire;
    ElementStyle stem;
    ElementStyle thumb;
    ElementStyle mod;
    float mod_value{NAN};
    float shrinky = 2.f;
    float increment = 10.f / 127.f;

    BasicSlider();

    void onEnter(const EnterEvent& e) override;
    void onLeave(const LeaveEvent& e) override;
    void onButton(const rack::widget::Widget::ButtonEvent &e) override;
    void onHoverScroll(const rack::widget::Widget::HoverScrollEvent & e) override;
    void onSelectKey(const rack::widget::Widget::SelectKeyEvent &e) override;
    void onHover(const HoverEvent &e) override;

    void unmodulate() { mod_value = NAN; }
    void set_modulation(float mod) { mod_value = mod; }
    float thumb_pos();
    bool applyTheme(std::shared_ptr<SvgTheme> theme) override;
    void draw_stem(const DrawArgs& args);
    void draw_thumb(const DrawArgs& args);
    void draw_mod(const DrawArgs& args);
    void draw(const DrawArgs& args) override;
};

struct FillSlider : BasicSlider
{
    using Base = BasicSlider;

    ElementStyle fill;

    FillSlider() : fill("slide-fill", "hsl(32,90%,60%)", "hsl(32,90%,60%)", 3.5f) {}

    bool applyTheme(std::shared_ptr<SvgTheme> theme) override
    {
        fill.apply_theme(theme);
        return Base::applyTheme(theme);
    }
    void draw_fill(const DrawArgs& args);
    void draw(const DrawArgs& args) override;
};

}