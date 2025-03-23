#pragma once
#include <rack.hpp>
#include "../services/colors.hpp"
#include "../services/svt_rack.hpp"
#include "element-style.hpp"
using namespace ::rack;
using namespace ::svg_theme;

namespace pachde {

enum class Axis { X, Y };

namespace slider_impl {

inline float coord(Axis axis, const Vec& pos) { 
    switch (axis) {
    case Axis::X: return pos.x;
    case Axis::Y: return pos.y;
    default: return pos.y;
    }
}
    
template <typename TSelf, typename TBase>
void onSelectKey(TSelf* self, const rack::widget::Widget::SelectKeyEvent &e)
{
    if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT)
    {
        auto pq = self->getParamQuantity();
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
                e.consume(self);
                break;

            case GLFW_KEY_HOME:
                if (plain) {
                    pq->setValue(pq->getMaxValue());
                    e.consume(self);
                }
                break;
            case GLFW_KEY_END:
                if (plain) {
                    pq->setValue(pq->getMinValue());
                    e.consume(self);
                }
                break;
            case GLFW_KEY_UP:
                pq->setValue(pq->getValue() + self->increment * (shift ? 10.f : 1.f));
                e.consume(self);
                break;
            case GLFW_KEY_DOWN:
                pq->setValue(pq->getValue() - self->increment * (shift ? 10.f : 1.f));
                e.consume(self);
                break;
            case GLFW_KEY_PAGE_UP:
                pq->setValue(pq->getValue() + self->increment * 10.f);
                e.consume(self);
                break;
            case GLFW_KEY_PAGE_DOWN:
                pq->setValue(pq->getValue() - self->increment * 10.f);
                e.consume(self);
                break;
            }
        }
    }
    if (e.isConsumed())
        return;
    self->TBase::onSelectKey(e);
}

template <typename TSelf, typename TBase>
void onEnter(TSelf* self, const ::rack::widget::Widget::EnterEvent& e)
{
    APP->event->setSelectedWidget(self);
    e.consume(self);
    self->TBase::onEnter(e);
}

template <typename TSelf, typename TBase>
void onLeave(TSelf* self, const ::rack::widget::Widget::LeaveEvent& e)
{
    APP->event->setSelectedWidget(nullptr);
    self->TBase::onLeave(e);
}

template <typename TSelf, typename TBase>
void onButton(TSelf* self, const ::rack::widget::Widget::ButtonEvent &e)
{
    if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
        e.consume(self);
        auto pq = self->getParamQuantity();
        if (pq) {
            float extent = coord(TSelf::axis, self->box.size);
            float v = extent - self->shrinky - coord(TSelf::axis, e.pos);
            v = rescale(v, 0.f, extent, pq->getMinValue(), pq->getMaxValue());
            pq->setValue(v);
        }
    } else {
        self->TBase::onButton(e);
    }
}

template <typename TSelf, typename TBase>
void onHoverScroll(TSelf* self, const rack::widget::Widget::HoverScrollEvent & e)
{
    auto pq = self->getParamQuantity();
    if (pq) {
        auto delta = coord(TSelf::axis, e.scrollDelta);
        auto mods = APP->window->getMods();
        if (delta < 0.f) {
            pq->setValue(pq->getValue() - self->increment * ((mods & GLFW_MOD_SHIFT) ? 10.f : 1.f));
        } else if (delta > 0.f) {
            pq->setValue(pq->getValue() + self->increment * ((mods & GLFW_MOD_SHIFT) ? 10.f : 1.f));
        }
        e.consume(self);
    } else {
        self->Base::onHoverScroll(e);
    }
}

template <typename TSelf, typename TBase>
void onHover(TSelf* self, const rack::widget::Widget::HoverEvent &e)
{
    self->TBase::onHover(e);
    e.consume(self);
}

}}