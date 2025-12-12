#pragma once
#include "slider-common.hpp"

using namespace ::rack;
using namespace ::svg_theme;

namespace widgetry {

struct BasicHSlider: ParamWidget, IThemed
{
    using Base = ParamWidget;

    static const Axis axis{Axis::X};

    Vec thumb_size;
    ElementStyle stem;
    ElementStyle thumb;
    ElementStyle foot;
    ElementStyle mod;
    bool wire;

    float mod_value{NAN};
    float increment{10.f / 127.f};
    float shrinky{2.f};
	float drag_distance{0.f};

    BasicHSlider();

    void unmodulate() { mod_value = NAN; }
    void set_modulation(float mod) { mod_value = mod; }
    float thumb_pos();
    float mod_pos();

    bool applyTheme(std::shared_ptr<SvgTheme> theme) override;

    void onEnter(const EnterEvent& e) override;
    void onLeave(const LeaveEvent& e) override;
    void onButton(const rack::widget::Widget::ButtonEvent &e) override;
    void onHoverScroll(const rack::widget::Widget::HoverScrollEvent & e) override;
    void onSelectKey(const rack::widget::Widget::SelectKeyEvent &e) override;
    void onHover(const HoverEvent &e) override;
	void onDragStart(const DragStartEvent& e) override;
	void onDragEnd(const DragEndEvent& e) override;
	void onDragMove(const DragMoveEvent& e) override;
	void onDragLeave(const DragLeaveEvent& e) override;

    void draw_feet(const DrawArgs& args);
    void draw_stem(const DrawArgs& args);
    void draw_thumb(const DrawArgs& args);
    void draw_mod(const DrawArgs& args);
    void draw(const DrawArgs& args) override;

};

struct FillHSlider: BasicHSlider
{
    using Base = BasicHSlider;
    ElementStyle fill;

    FillHSlider() : fill("hslide-fill", "#f9a54b", 2.5f) {}

    bool applyTheme(std::shared_ptr<SvgTheme> theme) override
    {
        fill.apply_theme(theme);
        return Base::applyTheme(theme);
    }
    void draw_fill(const DrawArgs& args);
    void draw(const DrawArgs& args) override;
};


}