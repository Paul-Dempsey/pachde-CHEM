#pragma once
#include <rack.hpp>
#include "../services/colors.hpp"
#include "../services/svt_rack.hpp"
#include "element-style.hpp"

using namespace ::rack;
using namespace ::svg_theme;

namespace pachde {

struct SliderBase : Knob
{
    using Base = Knob;
    float shrinky = 2.f;
    float increment = 10.f / 127.f;

    void onEnter(const EnterEvent& e) override;
    void onLeave(const LeaveEvent& e) override;
    void onButton(const rack::widget::Widget::ButtonEvent &e) override;
    void onHoverScroll(const rack::widget::Widget::HoverScrollEvent & e) override;
    void onSelectKey(const rack::widget::Widget::SelectKeyEvent &e) override;
    void onHover(const HoverEvent &e) override;

    // Dragging implemented by Knob
	// void onDragStart(const DragStartEvent& e) override;
	// void onDragEnd(const DragEndEvent& e) override;
	// void onDragMove(const DragMoveEvent& e) override;
	// void onDragLeave(const DragLeaveEvent& e) override;
};

struct BasicSlider : SliderBase, IApplyTheme
{
    Vec thumb_size;
    bool wire;
    ElementStyle stem;
    ElementStyle thumb;

    BasicSlider();

    float thumb_pos();
    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override;
    void draw_stem(const DrawArgs& args);
    void draw_thumb(const DrawArgs& args);
    void draw(const DrawArgs& args) override;
};

struct FillSlider : BasicSlider
{
    using Base = BasicSlider;

    ElementStyle fill;

    FillSlider() : fill("slide-fill", "#f9a54b", 2.5f) {}

    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override
    {
        fill.apply_theme(theme);
        return Base::applyTheme(engine, theme);
    }
    void draw_fill(const DrawArgs& args);
    void draw(const DrawArgs& args) override;
};

template <typename TSlider>
TSlider* createSlider(Vec pos, float length, ::rack::engine::Module* module, int param_id, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
    TSlider* o = createParam<TSlider>(pos, module, param_id);
    o->box.size.y = length;
    o->applyTheme(engine, theme);
    return Center(o);
}

}