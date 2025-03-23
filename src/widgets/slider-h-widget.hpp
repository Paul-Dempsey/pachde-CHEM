#pragma once
#include "slider-common.hpp"

using namespace ::rack;
using namespace ::svg_theme;

namespace pachde {

struct HSliderBase : ParamWidget
{
    using Base = ParamWidget;
    float increment = 10.f / 127.f;
    float shrinky = 2.f;
    static const Axis axis{Axis::X};

    void onEnter(const EnterEvent& e) override;
    void onLeave(const LeaveEvent& e) override;
    void onButton(const rack::widget::Widget::ButtonEvent &e) override;
    void onHoverScroll(const rack::widget::Widget::HoverScrollEvent & e);
    void onSelectKey(const rack::widget::Widget::SelectKeyEvent &e) override;
    void onHover(const HoverEvent &e) override;
	void onDragStart(const DragStartEvent& e) override;
	void onDragEnd(const DragEndEvent& e) override;
	void onDragMove(const DragMoveEvent& e) override;
	void onDragLeave(const DragLeaveEvent& e) override;
};

}