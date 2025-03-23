#pragma once
#include "slider-h-widget.hpp"

using namespace ::rack;
using namespace ::svg_theme;

namespace pachde {

// h-slider boilerplate
void HSliderBase::onEnter(const EnterEvent& e) {
    slider_impl::onEnter<HSliderBase, Base>(this, e);
}
void HSliderBase::onLeave(const LeaveEvent& e) {
    slider_impl::onLeave<HSliderBase, Base>(this, e);
}
void HSliderBase::onButton(const rack::widget::Widget::ButtonEvent &e) {
    slider_impl::onButton<HSliderBase, Base>(this, e);
}
void HSliderBase::onHoverScroll(const rack::widget::Widget::HoverScrollEvent & e) {
    slider_impl::onHoverScroll<HSliderBase, Base>(this, e);
}
void HSliderBase::onSelectKey(const rack::widget::Widget::SelectKeyEvent &e) {
    slider_impl::onSelectKey<HSliderBase, Base>(this, e);
}
void HSliderBase::onHover(const HoverEvent &e) {
    slider_impl::onHover<HSliderBase, Base>(this, e);
}

void HSliderBase::onDragStart(const DragStartEvent& e)
{

}
void HSliderBase::onDragEnd(const DragEndEvent& e)
{

}
void HSliderBase::onDragMove(const DragMoveEvent& e)
{

}
void HSliderBase::onDragLeave(const DragLeaveEvent& e)
{

}

}