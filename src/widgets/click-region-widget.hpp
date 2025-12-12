#pragma once
#include <rack.hpp>
using namespace ::rack;
#include "services/colors.hpp"
#include "layout-help.hpp"
#include "element-style.hpp"
using namespace pachde;

namespace widgetry {

struct ClickRegion : Widget, ILayoutHelp, IThemed
{
    using Base = Widget;
    int identifier;
    int mods;
    bool enabled;
    bool hoverable{false};
    bool hovered{false};
    ElementStyle hover_style{"click-hover", "hsla(0,0%,0%,0%)", "hsla(0, 0%, 65%, 100%)", .25f};

    std::function<void(int, int)> handler;

    ClickRegion(int id) :
        identifier(id),
        mods(0),
        enabled(true)
    {
        box.size.x = 0;
        box.size.y = 0;
    }

    void set_hover_key(const char* key)
    {
        hover_style.key = key;
    }

    bool applyTheme(std::shared_ptr<svg_theme::SvgTheme> theme) override
    {
        if (hoverable) {
            hover_style.apply_theme(theme);
            return true;
        } else {
            return false;
        }
    }

    void set_handler(std::function<void (int, int)> callback)
    {
        handler = callback;
    }

    int id() { return identifier; }

    void enable(bool enabled = true) {
        this->enabled = enabled;
    }

    void onButton(const ButtonEvent& e) override {
        Base::onButton(e);

        if (!enabled) return;
        // Dispatch ActionEvent on left click release
        if (e.action == GLFW_RELEASE && e.button == GLFW_MOUSE_BUTTON_LEFT) {
            mods = e.mods;
            ActionEvent eAction;
            onAction(eAction);
        }
    }

    void onAction(const ActionEvent& e) override
    {
        if (enabled && handler) {
            handler(identifier, mods);
        } else {
            Base::onAction(e);
        }
    }
    void onHover(const HoverEvent& e) override {
        if (enabled && hoverable) {
            e.consume(this);
        }
        Base::onHover(e);
    }
    void onEnter(const EnterEvent& e) override {
        hovered = true;
        Base::onEnter(e);
    }
    void onLeave(const LeaveEvent& e) override {
        hovered = false;
        Base::onLeave(e);
    }

    void draw(const DrawArgs& args) override {
        Base::draw(args);
        if (hovered) {
            FittedBoxRect(args.vg, 0.f, 0.f, VEC_ARGS(box.size), hover_style.nvg_stroke_color(), Fit::Inside, hover_style.width());
        }
        if (layout_hints) {
            draw_widget_bounds(this, args);
        }
    }
};

template <typename TR = ClickRegion>
TR* createClickRegion(float x, float y, float width, float height, int id,
    std::function<void (int, int)> callback,
    bool enabled = true
    )
{
    TR* o = new TR(id);
    o->box.pos = Vec(x, y);
    o->box.size = Vec(width, height);
    o->set_handler(callback);
    o->enable(enabled);
    return o;
}

template <typename TR = ClickRegion>
TR* createHoverClickRegion(float x, float y, float width, float height, int id,
    std::function<void (int, int)> callback,
    const char * hover_key
    )
{
    TR* o = new TR(id);
    o->box.pos = Vec(x, y);
    o->box.size = Vec(width, height);
    o->set_handler(callback);
    o->enable(true);
    o->hoverable = true;
    o->set_hover_key(hover_key);
    return o;
}

}